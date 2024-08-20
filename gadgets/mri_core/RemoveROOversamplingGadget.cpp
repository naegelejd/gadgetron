#include "RemoveROOversamplingGadget.h"

#include <xtensor/xadapt.hpp>
namespace Gadgetron {

RemoveROOversamplingGadget::RemoveROOversamplingGadget(const Core::Context& context,
                                                       const Core::GadgetProperties& props)
    : Core::ChannelGadget<Core::Acquisition>(context, props) {
    auto h = (context.header);

    if (h.encoding.size() == 0) {
        GDEBUG("Number of encoding spaces: %d\n", h.encoding.size());
        GERROR("This Gadget needs an encoding description\n");
        return;
    }

    mrd::EncodingSpaceType e_space = h.encoding[0].encoded_space;
    mrd::EncodingSpaceType r_space = h.encoding[0].recon_space;

    encodeNx_ = e_space.matrix_size.x;
    encodeFOV_ = e_space.field_of_view_mm.x;
    reconNx_ = r_space.matrix_size.x;
    reconFOV_ = r_space.field_of_view_mm.x;

#ifdef USE_OMP  // TODO: Should this be removed? Its from the old version
    omp_set_num_threads(1);
    GDEBUG_STREAM("RemoveROOversamplingGadget:omp_set_num_threads(1) ... ");
#endif // USE_OMP

    // If the encoding and recon matrix size and FOV are the same
    // then the data is not oversampled and we can safely pass
    // the data onto the next gadget
    if ((encodeNx_ == reconNx_) && (encodeFOV_ == reconFOV_)) {
        dowork_ = false;
    } else {
        dowork_ = true;
    }
}

void RemoveROOversamplingGadget::process(Core::InputChannel<Core::Acquisition>& in, Core::OutputChannel& out) {
    for (auto acq : in) {
        if (dowork_) {
            auto shape = acq.data.shape();

            // We'll use hoNDFFT for the FFTs, so we need to "adapt" the MRD arrays (pointers) to hoNDArrays

            // Reverse the dimensions from MRD -> hoNDArray
            std::vector<size_t> data_in_dims(std::rbegin(shape), std::rend(shape));
            if (!ifft_buf_.dimensions_equal(data_in_dims)) {
                ifft_buf_.create(data_in_dims);
                ifft_res_.create(data_in_dims);
            }

            float ratioFOV = encodeFOV_ / reconFOV_;
            std::vector<size_t> data_out_dims = data_in_dims;
            data_out_dims[0] = (size_t)(data_out_dims[0] / ratioFOV);
            if (!fft_buf_.dimensions_equal(data_out_dims)) {
                fft_buf_.create(data_out_dims);
                fft_res_.create(data_out_dims);
            }

            hoNDArray<std::complex<float>> adapted(data_in_dims, acq.data.data());
            hoNDFFT<float>::instance()->ifft1c(adapted, ifft_res_, ifft_buf_);

            hoNDArray<std::complex<float>> temp(data_out_dims);

            size_t sRO = acq.Samples();
            size_t start = (size_t)((sRO - data_out_dims[0]) / 2);

            size_t dRO = temp.get_size(0);
            size_t numOfBytes = data_out_dims[0] * sizeof(std::complex<float>);

            int CHA = (int)(data_out_dims[1]);
            for (size_t c = 0; c < CHA; c++) {
                memcpy(temp.get_data_ptr() + c * dRO, ifft_res_.get_data_ptr() + c * sRO + start, numOfBytes);
            }

            hoNDFFT<float>::instance()->fft1c(temp, fft_res_, fft_buf_);

            // Reverse the dimensions for mrd::Acquisition::data
            std::vector<size_t> shape_out(std::rbegin(data_out_dims), std::rend(data_out_dims));
            acq.data.resize(shape_out);
            memcpy(acq.data.data(), fft_res_.get_data_ptr(), fft_res_.get_number_of_bytes());

            acq.head.center_sample = (uint32_t)(acq.head.center_sample.value_or(0) / ratioFOV);
            acq.head.discard_pre = (uint32_t)(acq.head.discard_pre.value_or(0) / ratioFOV);
            acq.head.discard_post = (uint32_t)(acq.head.discard_post.value_or(0) / ratioFOV);
        }

        out.push(acq);
    }
}

GADGETRON_GADGET_EXPORT(RemoveROOversamplingGadget)

} // namespace Gadgetron
