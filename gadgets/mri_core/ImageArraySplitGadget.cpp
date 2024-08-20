#include "ImageArraySplitGadget.h"

using namespace Gadgetron;
using namespace Gadgetron::Core;

namespace {

void splitInputData(AnyImage image, Core::OutputChannel& out) {
    out.push(image);
}

void splitInputData(IsmrmrdImageArray imagearr, Core::OutputChannel& out) {
    auto LOC = imagearr.data.shape(0);
    auto S = imagearr.data.shape(1);
    auto N = imagearr.data.shape(2);
    auto CHA = imagearr.data.shape(3);
    auto Z = imagearr.data.shape(4);
    auto Y = imagearr.data.shape(5);
    auto X = imagearr.data.shape(6);

    // Each image will be [CHA,Z,Y,X] big
    std::vector<size_t> img_dims{CHA, Z, Y, X};

    // Loop over LOC, S, and N
    for (auto loc = 0; loc < LOC; loc++) {
        for (auto s = 0; s < S; s++) {
            for (auto n = 0; n < N; n++) {
                mrd::Image<std::complex<float>> img;
                img.head = imagearr.headers(loc, s, n);
                if (imagearr.meta.has_value()) {
                    img.meta = imagearr.meta.value()[loc, s, n];
                }

                img.data.resize(img_dims);
                /** TODO Joe: Just use xtensor view to assign img.data ... */
                memcpy(img.data.data(), &imagearr.data(loc, s, n, 0, 0, 0, 0), X * Y * Z * CHA * sizeof(std::complex<float>));

                // Pass the image down the chain
                out.push(img);
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
