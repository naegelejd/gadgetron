
#include "ImageIndexGadget.h"

#include "Node.h"
#include "Types.h"

#include "log.h"


using namespace Gadgetron::Core;

namespace {

    template<class T, class F>
    Image<T> update_image_index(Image<T> image, F &index) {
        image.head.image_index = index(image.head.image_series_index.value_or(0));
        return image;
    }
}

namespace Gadgetron {

    ImageIndexGadget::ImageIndexGadget(const Context &context, const GadgetProperties &properties)
        : ChannelGadget(context, properties) {}

    void ImageIndexGadget::process(InputChannel<Core::AnyImage> &input, OutputChannel &output) {

        std::map<uint16_t, uint16_t> indices{};

        auto index = [&](auto series) { return indices.count(series) ? indices.at(series) : 1; };
        auto increment = [&](auto series) { indices[series] = index(series) + 1; };
        auto increment_and_return = [&](auto series) {
            auto idx = index(series); increment(series);
            GDEBUG_STREAM("Generated image index " << idx << " for image series " << series);
            return idx;
        };

        for (auto image : input) {
            visit([&](auto image) {
                output.push(update_image_index(image, increment_and_return));
            }, image);
        }
    }

    GADGETRON_GADGET_EXPORT(ImageIndexGadget);
}