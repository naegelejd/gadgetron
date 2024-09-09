/*
 *       ComplexToFloatGadget.cpp
 *       Author: Hui Xue
 */

#include "ComplexToFloatGadget.h"

#include "log.h"

#include <xtensor/xcomplex.hpp>


Gadgetron::ComplexToFloatGadget::ComplexToFloatGadget(
    const Gadgetron::Core::Context& context, const Gadgetron::Core::GadgetProperties& props)
    : PureGadget(context,props)
{

    converters = { { mrd::ImageType::kMagnitude, [](const auto& image) { return xt::abs(image); } },
                   { mrd::ImageType::kPhase,     [](const auto& image) { return xt::arg(image); } },
                   { mrd::ImageType::kReal,      [](const auto& image) { return xt::real(image); } },
                   { mrd::ImageType::kImag,      [](const auto& image) { return xt::imag(image); } }};
};

Gadgetron::Core::Image<float> Gadgetron::ComplexToFloatGadget::process_function(
    Gadgetron::Core::Image<std::complex<float>> input_image) const
{
    mrd::Image<float> out;
    out.head = input_image.head;
    out.meta = input_image.meta;

    hoNDArray<float> output_data;

    if (converters.count(input_image.head.image_type)) {
        out.data = converters.at(input_image.head.image_type)(input_image.data);
    }
    else {
        GDEBUG_STREAM("Image type not set; defaulting to magnitude image.");
        out.data = converters.at(mrd::ImageType::kMagnitude)(input_image.data);
        out.head.image_type = mrd::ImageType::kMagnitude;
    }

    GDEBUG_STREAM("Joe: Image mean IN: " << xt::mean(input_image.data)() << ", OUT: " << xt::mean(out.data)());

    return out;
}

namespace Gadgetron{
    GADGETRON_GADGET_EXPORT(ComplexToFloatGadget)
}