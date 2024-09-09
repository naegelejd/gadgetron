#include "ImageInverter.h"

#include "hoNDArray_math.h"

using namespace Gadgetron;
using namespace Gadgetron::Core;

namespace {
    template<class T>
    Image<T> invert_image(const Image<T> &image)
    {
        Image<T> out;
        out.head = image.head;
        out.meta = image.meta;
        out.data = image.data;

		auto max_value = *std::max_element(image.data.begin(), image.data.end());
		for (auto& d : out.data) d = max_value - d;

        return out;
    }

    template<class T>
    Image<std::complex<T>> invert_image(const Image<std::complex<T>> &image) {
        throw std::runtime_error("Invert image is not well defined for complex images.");
    }
}

namespace Gadgetron::Examples {


    AnyImage ImageInverter::process_function(AnyImage image) const {
        GINFO_STREAM("Inverting image.")
        return visit([](const auto &image) -> AnyImage { return invert_image(image); }, image);
    }

    GADGETRON_GADGET_EXPORT(ImageInverter);
}

