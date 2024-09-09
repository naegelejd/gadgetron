#include "SimpleReconGadget.h"

namespace Gadgetron {

    SimpleReconGadget::SimpleReconGadget(const Core::Context& context, const Core::GadgetProperties& props)
        : ChannelGadget(context, props)
    {
        header = context.header;
        image_counter_ = 0;
    }

    void SimpleReconGadget::process(Core::InputChannel<mrd::ReconData>& input, Core::OutputChannel& out)
    {
        for (mrd::ReconData reconData : input) {
            GDEBUG_STREAM("Processing reconData containing " << reconData.rbits.size() << " recon bits");
            // Iterate over all the recon bits
            for (mrd::ReconBit& rbit : reconData.rbits) {
                // Grab a reference to the buffer containing the imaging data
                // We are ignoring the reference data
                mrd::BufferedData& dbuff = rbit.data;

                // Data 7D, fixed order [LOC, S, N, CHA, E2, E1, E0]
                auto E0 = dbuff.data.get_size(0);
                auto E1 = dbuff.data.get_size(1);
                auto E2 = dbuff.data.get_size(2);
                auto CHA = dbuff.data.get_size(3);
                auto N = dbuff.data.get_size(4);
                auto S = dbuff.data.get_size(5);
                auto LOC = dbuff.data.get_size(6);
                // auto LOC = dbuff.data.shape(0);
                // auto S = dbuff.data.shape(1);
                // auto N = dbuff.data.shape(2);
                // auto CHA = dbuff.data.shape(3);
                // auto E2 = dbuff.data.shape(4);
                // auto E1 = dbuff.data.shape(5);
                // auto E0 = dbuff.data.shape(6);

                ImageArray imarray;

                // The image array data will be [LOC,S,N,1,E2,E1,E0] big
                // Will collapse across coils at the end
                // imarray.data.resize({LOC, S, N, 1, E2, E1, E0});
                std::vector<size_t> data_dims(7);
                data_dims[0] = E0;
                data_dims[1] = E1;
                data_dims[2] = E2;
                data_dims[3] = 1;
                data_dims[4] = N;
                data_dims[5] = S;
                data_dims[6] = LOC;
                imarray.data.create(data_dims);

                // ImageHeaders will be [LOC, S, N]
                // imarray.headers.resize({LOC, S, N});
                std::vector<size_t> header_dims(3);
                header_dims[0] = N;
                header_dims[1] = S;
                header_dims[2] = LOC;
                imarray.headers.create(header_dims);

                // Loop over LOC, S, and N
                for (auto loc = 0; loc < LOC; loc++) {
                    for (auto s = 0; s < S; s++) {
                        for (auto n = 0; n < N; n++) {
                            // Set some information into the image header
                            // Use the middle acquisition header for some info
                            //[E1, E2, N, S, LOC]
                            // mrd::AcquisitionHeader& acqhdr = dbuff.headers(loc, s, n,
                            //     dbuff.sampling.sampling_limits.e2.center,
                            //     dbuff.sampling.sampling_limits.e1.center);
                            // mrd::ImageHeader& imghdr = imarray.headers(loc, s, n);
                            mrd::AcquisitionHeader& acqhdr =
                                dbuff.headers(dbuff.sampling.sampling_limits.e1.center,
                                            dbuff.sampling.sampling_limits.e2.center, n, s, loc);
                            mrd::ImageHeader& imghdr = imarray.headers(n, s, loc);

                            imghdr.measurement_uid = acqhdr.measurement_uid;
                            imghdr.field_of_view[0] = dbuff.sampling.recon_fov.x;
                            imghdr.field_of_view[1] = dbuff.sampling.recon_fov.y;
                            imghdr.field_of_view[2] = dbuff.sampling.recon_fov.z;
                            imghdr.position[0] = acqhdr.position[0];
                            imghdr.position[1] = acqhdr.position[1];
                            imghdr.position[2] = acqhdr.position[2];
                            imghdr.col_dir[0] = acqhdr.read_dir[0];
                            imghdr.col_dir[1] = acqhdr.read_dir[1];
                            imghdr.col_dir[2] = acqhdr.read_dir[2];
                            imghdr.line_dir[0] = acqhdr.phase_dir[0];
                            imghdr.line_dir[1] = acqhdr.phase_dir[1];
                            imghdr.line_dir[2] = acqhdr.phase_dir[2];
                            imghdr.slice_dir[0] = acqhdr.slice_dir[0];
                            imghdr.slice_dir[1] = acqhdr.slice_dir[1];
                            imghdr.slice_dir[2] = acqhdr.slice_dir[2];
                            imghdr.patient_table_position[0] = acqhdr.patient_table_position[0];
                            imghdr.patient_table_position[1] = acqhdr.patient_table_position[1];
                            imghdr.patient_table_position[2] = acqhdr.patient_table_position[2];
                            imghdr.average = acqhdr.idx.average;
                            imghdr.slice = acqhdr.idx.slice;
                            imghdr.contrast = acqhdr.idx.contrast;
                            imghdr.phase = acqhdr.idx.phase;
                            imghdr.repetition = acqhdr.idx.repetition;
                            imghdr.set = acqhdr.idx.set;
                            imghdr.acquisition_time_stamp = acqhdr.acquisition_time_stamp;
                            imghdr.physiology_time_stamp = acqhdr.physiology_time_stamp;
                            imghdr.image_type = mrd::ImageType::kComplex;
                            imghdr.image_index = ++image_counter_;
                            imghdr.image_series_index = 0;

                            // Grab a wrapper around the relevant chunk of data [CHA,E2,E1,E0] for this loc, n, and s
                            // Each chunk will be [E0,E1,E2,CHA] big (hoNDArray dimension order)
                            std::vector<size_t> chunk_dims(4);
                            chunk_dims[0] = E0;
                            chunk_dims[1] = E1;
                            chunk_dims[2] = E2;
                            chunk_dims[3] = CHA;
                            // hoNDArray<std::complex<float>> chunk(chunk_dims, &dbuff.data(loc, s, n, 0, 0, 0, 0));
                            hoNDArray<std::complex<float>> chunk(chunk_dims, &dbuff.data(0, 0, 0, 0, n, s, loc));

                            // Do the FFTs in place
                            hoNDFFT<float>::instance()->ifft3c(chunk);

                            // Square root of the sum of squares
                            // Each image will be [E0,E1,E2,1] big
                            std::vector<size_t> img_dims(3);
                            img_dims[0] = E0;
                            img_dims[1] = E1;
                            img_dims[2] = E2;
                            // hoNDArray<std::complex<float>> output(img_dims, &imarray.data(loc, s, n, 0, 0, 0, 0));
                            hoNDArray<std::complex<float>> output(img_dims, &imarray.data(0, 0, 0, 0, n, s, loc));

                            // Compute d*d in place
                            multiplyConj(chunk, chunk, chunk);
                            // Add up
                            for (size_t c = 0; c < CHA; c++) {
                                output += hoNDArray<std::complex<float>>(img_dims, &chunk(0, 0, 0, c));
                            }
                            // Take the square root in place
                            sqrt_inplace(&output);
                        }
                    }
                }
                out.push(std::move(imarray));
            }
        }
    }
    GADGETRON_GADGET_EXPORT(SimpleReconGadget);

}
