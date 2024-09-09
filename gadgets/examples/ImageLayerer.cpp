#include "ImageLayerer.h"

#include <vector>

#include "hoNDArray_math.h"
#include "hoNDArray_utils.h"

#include "mri_core_utility.h"

using namespace Gadgetron;
using namespace Gadgetron::Core;

namespace {

    template<class T>
    Image<T> merge(const Image<T> &a, const Image<T> &b)
    {
        const auto& data_a = a.data;
        const auto& data_b = b.data;

        if (data_a.dimensions() != data_b.dimensions()) {
            throw std::runtime_error("Images missized. Can't merge mismatched images.");
        }

        std::vector<size_t> size = {
                data_a.get_size(0),
                data_a.get_size(1),
                data_a.get_size(2),
                data_a.get_size(4) * 2
        };

        auto data = concat(std::vector<hoNDArray<T>>{data_a, data_b});
        data.reshape(size);

        mrd::Image<T> out{.head=a.head, .data=data, .meta=a.meta};

        return out;
    }

    template<class A, class B>
    Image<A> merge(const Image<A> &, const Image<B> &) {
        throw std::runtime_error("Images have different types; merging will be a hassle.");
    }
}

namespace Gadgetron::Examples {

    ImageLayerer::ImageLayerer(const Context &, const GadgetProperties &properties) : Merge(properties) {}

    void ImageLayerer::process(std::map<std::string, GenericInputChannel> input, OutputChannel output) {

        auto unchanged = InputChannel<AnyImage>(input.at("unchanged"), output);
        auto inverted = InputChannel<AnyImage>(input.at("inverted"), output);

        for (auto image : unchanged) {
            auto merged = Core::visit(
                    [](const auto &a, const auto &b) -> AnyImage { return merge(a, b); },
                    image,
                    inverted.pop()
            );



            GINFO_STREAM("Images combined; pushing out result.");

            output.push(std::move(merged));
        }
    }

    GADGETRON_MERGE_EXPORT(ImageLayerer)
}
