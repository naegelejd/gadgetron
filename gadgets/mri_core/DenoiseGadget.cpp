
#include "DenoiseGadget.h"
#include "GadgetronTimer.h"
#include "non_local_bayes.h"
#include "non_local_means.h"
#include "mri_core_utility.h"

namespace Gadgetron {
    template <class T>
    Gadgetron::hoNDArray<T> Gadgetron::DenoiseGadget::denoise_function(const Gadgetron::hoNDArray<T>& input) const {

        if (denoiser == "non_local_bayes") {
            return Denoise::non_local_bayes(input, image_std, search_radius);
        } else if (denoiser == "non_local_means") {
            return Denoise::non_local_means(input, image_std, search_radius);
        } else {
            throw std::invalid_argument(std::string("DenoiseGadget: Unknown denoiser type: ") + std::string(denoiser));
        }
    }

    DenoiseSupportedTypes DenoiseGadget::process_function(DenoiseSupportedTypes input) const {
        return Core::visit(
            [&,this](auto& image) { return DenoiseSupportedTypes(this->denoise(std::move(image))); }, input);
    }

    template <class T> mrd::Image<T> DenoiseGadget::denoise(mrd::Image<T> image) const {
        /** TODO Joe: Copying MRD array to and from hoNDArray to do denoise operation */
        auto data = Gadgetron::copy_mrd_to_hoNDArray(image.data);
        data = denoise_function(data);
        Gadgetron::copy_hoNDArray_to_mrd(data, image.data);
        return std::move(image);
    }

    mrd::ImageArray DenoiseGadget::denoise(mrd::ImageArray image_array) const {
        /** TODO Joe: Copying MRD array to and from hoNDArray to do denoise operation */
        auto input = Gadgetron::copy_mrd_to_hoNDArray(image_array.data);
        input = denoise_function(input);
        Gadgetron::copy_hoNDArray_to_mrd(input, image_array.data);
        return std::move(image_array);
    }

    GADGETRON_GADGET_EXPORT(DenoiseGadget)
}