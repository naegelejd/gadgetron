#include "AccumulatorGadget.h"

namespace {
int addPrePostZeros(size_t centre_column, size_t samples) {
    // 1 : pre zeros
    // 2 : post zeros
    // 0 : no zeros
    if (2 * centre_column == samples) {
        return 0;
    }
    if (2 * centre_column < samples) {
        return 1;
    }
    if (2 * centre_column > samples) {
        return 2;
    }
    return 0;
}
} // namespace

namespace Gadgetron {

AccumulatorGadget::AccumulatorGadget(const Core::Context& context, const Core::GadgetProperties& props)
    : Core::ChannelGadget<Core::Acquisition>(context, props) {
    buffer_ = 0;
    image_counter_ = 0;
    image_series_ = 0;

    auto h = (context.header);
    if (h.encoding.size() != 1) {
        GDEBUG("Number of encoding spaces: %d\n", h.encoding.size());
        GDEBUG("This simple AccumulatorGadget only supports one encoding space\n");
        // TODO: How to throw Gadget failures?
    }

    mrd::EncodingSpaceType e_space = h.encoding[0].encoded_space;
    mrd::EncodingSpaceType r_space = h.encoding[0].recon_space;
    mrd::EncodingLimitsType e_limits = h.encoding[0].encoding_limits;

    GDEBUG("Matrix size: %d, %d, %d\n", r_space.matrix_size.x, e_space.matrix_size.y, e_space.matrix_size.z);
    dimensions_.push_back(r_space.matrix_size.x);
    dimensions_.push_back(e_space.matrix_size.y);
    dimensions_.push_back(e_space.matrix_size.z);

    field_of_view_[0] = r_space.field_of_view_mm.x;
    field_of_view_[1] = e_space.field_of_view_mm.y;
    field_of_view_[2] = e_space.field_of_view_mm.z;
    GDEBUG("FOV: %f, %f, %f\n", r_space.field_of_view_mm.x, e_space.field_of_view_mm.y, e_space.field_of_view_mm.z);

    slices_ = e_limits.slice ? e_limits.slice->maximum + 1 : 1;
}

AccumulatorGadget::~AccumulatorGadget() {
    if (buffer_)
        delete buffer_;
}

void AccumulatorGadget::process(Core::InputChannel<Core::Acquisition>& in, Core::OutputChannel& out) {
    mrd::Acquisition ref_acq;
    for (auto acq : in) {
        if (!buffer_) {
            dimensions_.push_back(acq.Coils());
            dimensions_.push_back(slices_);

            if (!(buffer_ = new hoNDArray<std::complex<float>>())) {
                GDEBUG("Failed create buffer\n");
              // TODO: How to throw Gadget failures?
            }

            try {
                buffer_->create(dimensions_);
            } 
            catch (std::runtime_error& err) {
                GEXCEPTION(err, "Failed allocate buffer array\n");
                // TODO: How to throw Gadget failures?
            }
            image_series_ = image_series;
        }

        bool is_first_scan_in_slice = acq.flags.HasFlags(mrd::AcquisitionFlags::kFirstInSlice);
        if (is_first_scan_in_slice) {
            ref_acq = acq;
        }

        std::complex<float>* buffer_raw = buffer_->get_data_ptr();
        std::complex<float>* data_raw = acq.data.data();

        int samples = acq.Samples();
        int line = acq.idx.kspace_encode_step_1.value_or(0);
        int partition = acq.idx.kspace_encode_step_2.value_or(0);
        int slice = acq.idx.slice.value_or(0);
        int center_sample = acq.center_sample.value_or(samples / 2);

        if (samples > dimensions_[0]) {
            GDEBUG("Wrong number of samples received\n");
            // TODO: How to throw Gadget failures
        }

        size_t offset = 0;
        // Copy the data for all the channels
        for (int chan = 0; chan < acq.Coils(); chan++) {
            offset = slice * dimensions_[0] * dimensions_[1] * dimensions_[2] * dimensions_[3] +
                     chan * dimensions_[0] * dimensions_[1] * dimensions_[2] +
                     partition * dimensions_[0] * dimensions_[1] + line * dimensions_[0] + (dimensions_[0] >> 1) -
                     center_sample;
            memcpy(buffer_raw + offset, data_raw + chan * samples, sizeof(std::complex<float>) * samples);
        }

        bool is_last_scan_in_slice = acq.flags.HasFlags(mrd::AcquisitionFlags::kLastInSlice);
        if (is_last_scan_in_slice) {
            mrd::Image<std::complex<float>> img;

            std::vector<size_t> img_dims(4);
            img_dims[0] = dimensions_[0];
            img_dims[1] = dimensions_[1];
            img_dims[2] = dimensions_[2];
            img_dims[3] = dimensions_[3];

            img.data.resize(img_dims);

            size_t data_length = img.data.size();
            offset = slice * data_length;
            memcpy(img.data.data(), buffer_raw + offset, sizeof(std::complex<float>) * data_length);
            img.measurement_uid = acq.measurement_uid;
            img.field_of_view[0] = field_of_view_[0];
            img.field_of_view[1] = field_of_view_[1];
            img.field_of_view[2] = field_of_view_[2];
            img.position = acq.position;
            img.col_dir = acq.read_dir;
            img.line_dir = acq.phase_dir;
            img.slice_dir = acq.slice_dir;
            img.patient_table_position = acq.patient_table_position;
            img.average = acq.idx.average;
            img.slice = acq.idx.slice;
            img.contrast = acq.idx.contrast;
            img.phase = acq.idx.phase;
            img.repetition = acq.idx.repetition;
            img.set = acq.idx.set;
            img.acquisition_time_stamp = acq.acquisition_time_stamp;
            img.physiology_time_stamp = acq.physiology_time_stamp;
            img.image_type = mrd::ImageType::kComplex;
            img.image_index = ++image_counter_;
            img.image_series_index = image_series_;

            out.push(img);
        }
    }
}
GADGETRON_GADGET_EXPORT(AccumulatorGadget)
} // namespace Gadgetron