#include "ImageArraySplitGadget.h"

#include <xtensor/xview.hpp>

using namespace Gadgetron;
using namespace Gadgetron::Core;

namespace {

void splitInputData(AnyImage image, Core::OutputChannel& out) {
    out.push(image);
}

void splitInputData(IsmrmrdImageArray imagearr, Core::OutputChannel& out) {
        // 7D, fixed order [X, Y, Z, CHA, N, S, LOC]
    uint16_t X = imagearr.data.get_size(0);
    uint16_t Y = imagearr.data.get_size(1);
    uint16_t Z = imagearr.data.get_size(2);
    uint16_t CHA = imagearr.data.get_size(3);
    uint16_t N = imagearr.data.get_size(4);
    uint16_t S = imagearr.data.get_size(5);
    uint16_t LOC = imagearr.data.get_size(6);

    // Each image will be [X,Y,Z,CHA] big
    std::vector<size_t> img_dims(4);
    img_dims[0] = X;
    img_dims[1] = Y;
    img_dims[2] = Z;
    img_dims[3] = CHA;
    // auto LOC = imagearr.data.shape(0);
    // auto S = imagearr.data.shape(1);
    // auto N = imagearr.data.shape(2);
    // auto CHA = imagearr.data.shape(3);
    // auto Z = imagearr.data.shape(4);
    // auto Y = imagearr.data.shape(5);
    // auto X = imagearr.data.shape(6);

    // // Each image will be [CHA,Z,Y,X] big
    // std::vector<size_t> img_dims{CHA, Z, Y, X};

    GDEBUG_STREAM("Joe: Splitting image array with dimensions: " << LOC << " " << S << " " << N << " " << CHA << " " << Z << " " << Y << " " << X);
    // Loop over LOC, S, and N
    for (auto loc = 0; loc < LOC; loc++) {
        for (auto s = 0; s < S; s++) {
            for (auto n = 0; n < N; n++) {
                mrd::Image<std::complex<float>> img;
                GDEBUG_STREAM("Joe: Settings Image.head " << loc << " " << s << " " << n);
                // img.head = imagearr.headers(loc, s, n);
                img.head = imagearr.headers(n, s, loc);

                if (imagearr.meta.size() >= LOC * S * N) {
                    GDEBUG_STREAM("Joe: Settings Image.meta " << loc << " " << s << " " << n);
                    // img.meta = imagearr.meta[loc, s, n];
                    img.meta = imagearr.meta(n, s, loc);
                }

                // img.data.resize(img_dims);
                img.data = hoNDArray<std::complex<float>>(img_dims);

                GDEBUG_STREAM("Joe: Settings Image.data " << loc << " " << s << " " << n);
                /** TODO Joe: Just use xtensor view to assign img.data ... 
                 * 
                 * DONE... Now just clean up vvvvv
                */
                // memcpy(img.data.data(), &imagearr.data(loc, s, n, 0, 0, 0, 0), X * Y * Z * CHA * sizeof(std::complex<float>));
                // xt::view(img.data, xt::all(), xt::all(), xt::all(), xt::all()) = xt::view(
                //     imagearr.data, loc, s, n, xt::all(), xt::all(), xt::all(), xt::all());
                memcpy(img.data.data(), &imagearr.data(0, 0, 0, 0, n, s, loc), X * Y * Z * CHA * sizeof(std::complex<float>));


                // Pass the image down the chain
                out.push(img);
                GDEBUG_STREAM("Joe: Pushed image " << loc << " " << s << " " << n);
            }
        }
    }
}

} // namespace

namespace Gadgetron {

void ImageArraySplitGadget::process(Core::InputChannel<ImageOrImageArray>& in, Core::OutputChannel& out) {
    for (auto msg : in) {
        visit([&](auto message){splitInputData(message, out);}, msg);
    }
}

GADGETRON_GADGET_EXPORT(ImageArraySplitGadget);
} // namespace Gadgetron
