
/** \file   mri_core_utility.cpp
    \brief  Implementation useful utility functionalities for 2D and 3D MRI parallel imaging
    \author Hui Xue
*/

#include "mri_core_utility.h"
#include "hoNDArray_elemwise.h"
#include "hoNDArray_utils.h"
#include "hoNDFFT.h"
#include "mri_core_kspace_filter.h"
#include <ctime>

namespace Gadgetron
{
    template <typename T>
    hoNDArray<bool> detect_readout_sampling_status(const hoNDArray<T>& data)
    {
        try
        {
            typedef typename realType<T>::Type value_type;

            size_t RO = data.get_size(0);
            size_t E1 = data.get_size(1);
            size_t E2 = data.get_size(2);
            size_t CHA = data.get_size(3);
            size_t N = data.get_size(4);
            size_t S = data.get_size(5);
            size_t SLC = data.get_size(6);

            hoNDArray<bool> sampled(E1, E2, N, S, SLC);



            for (size_t slc = 0; slc < SLC; slc++)
            {
                for (size_t s = 0; s < S; s++)
                {
                    for (size_t n = 0; n < N; n++)
                    {
                        for (size_t e2 = 0; e2 < E2; e2++)
                        {
                            for (size_t e1 = 0; e1 < E1; e1++)
                            {
                                value_type v = 0;
                                const T* pData = &(data(0, e1, e2, 0, n, s, slc));


                                for (size_t ro = 0; ro < RO; ro++)
                                {
                                    v += std::abs(pData[ro]);
                                }

                                if (v > 0)
                                {
                                    sampled(e1, e2, n, s, slc) = true;
                                }
                                else
                                {
                                    sampled(e1, e2, n, s, slc) =false;
                                }
                            }
                        }
                    }
                }
            }

            return sampled;
        }
        catch (...)
        {
            GADGET_THROW("Errors in detect_readout_sampling_status(...) ... ");
        }
    }

    template EXPORTMRICORE hoNDArray<bool> detect_readout_sampling_status(const hoNDArray< float >& data);
    template EXPORTMRICORE hoNDArray<bool> detect_readout_sampling_status(const hoNDArray< double >& data);
    template EXPORTMRICORE hoNDArray<bool> detect_readout_sampling_status(const hoNDArray< std::complex<float> >& data);
    template EXPORTMRICORE hoNDArray<bool> detect_readout_sampling_status(const hoNDArray< std::complex<double> >& data);

    // ------------------------------------------------------------------------

    template <typename T>
    std::tuple<size_t, size_t> detect_sampled_region_E1(const hoNDArray<T>& data)
    {

        hoNDArray<bool> sampled = Gadgetron::detect_readout_sampling_status(data);

        size_t RO = data.get_size(0);
        size_t E1 = data.get_size(1);
        size_t E2 = data.get_size(2);
        size_t CHA = data.get_size(3);
        size_t N = data.get_size(4);
        size_t S = data.get_size(5);
        size_t SLC = data.get_size(6);

        size_t start_E1, end_E1;
        start_E1 = E1;
        end_E1 = 0;



        for (size_t slc = 0; slc < SLC; slc++)
        {
            for (size_t s = 0; s < S; s++)
            {
                for (size_t n = 0; n < N; n++)
                {
                    for (size_t e2 = 0; e2 < E2; e2++)
                    {
                        for (size_t e1 = 0; e1 < E1; e1++)
                        {
                            if (sampled(e1, e2, n, s, slc)==true)
                            {
                                if (e1 < start_E1) start_E1 = e1;
                                if (e1 > end_E1) end_E1 = e1;
                            }
                        }
                    }
                }
            }
        }

        return std::make_tuple(start_E1, end_E1);

    }

    template EXPORTMRICORE std::tuple<size_t, size_t> detect_sampled_region_E1(const hoNDArray<float>& data);
    template EXPORTMRICORE std::tuple<size_t, size_t> detect_sampled_region_E1(const hoNDArray<double>& data);
    template EXPORTMRICORE std::tuple<size_t, size_t> detect_sampled_region_E1(const hoNDArray< std::complex<float> >& data);
    template EXPORTMRICORE std::tuple<size_t, size_t> detect_sampled_region_E1(const hoNDArray< std::complex<double> >& data);

    // ------------------------------------------------------------------------

    template <typename T>
    std::tuple<size_t, size_t> detect_sampled_region_E2(const hoNDArray<T>& data)
    {
        try
        {
            hoNDArray<bool> sampled = Gadgetron::detect_readout_sampling_status(data);

            size_t RO = data.get_size(0);
            size_t E1 = data.get_size(1);
            size_t E2 = data.get_size(2);
            size_t CHA = data.get_size(3);
            size_t N = data.get_size(4);
            size_t S = data.get_size(5);
            size_t SLC = data.get_size(6);

            size_t start_E2, end_E2;
            start_E2 = E2;
            end_E2 = 0;

            size_t e1, e2, n, s, slc;

            for (slc = 0; slc < SLC; slc++)
            {
                for (s = 0; s < S; s++)
                {
                    for (n = 0; n < N; n++)
                    {
                        for (e2 = 0; e2 < E2; e2++)
                        {
                            for (e1 = 0; e1 < E1; e1++)
                            {
                                if (sampled(e1, e2, n, s, slc)==true)
                                {
                                    if (e2 < start_E2) start_E2 = e2;
                                    if (e2 > end_E2) end_E2 = e2;
                                }
                            }
                        }
                    }
                }
            }

            return std::make_tuple(start_E2, end_E2);
        }
        catch (...)
        {
            GADGET_THROW("Errors in detect_sampled_region_E2(...) ... ");
        }
    }

    template EXPORTMRICORE std::tuple<size_t, size_t> detect_sampled_region_E2(const hoNDArray<float>& data);
    template EXPORTMRICORE std::tuple<size_t, size_t> detect_sampled_region_E2(const hoNDArray<double>& data);
    template EXPORTMRICORE std::tuple<size_t, size_t> detect_sampled_region_E2(const hoNDArray< std::complex<float> >& data);
    template EXPORTMRICORE std::tuple<size_t, size_t> detect_sampled_region_E2(const hoNDArray< std::complex<double> >& data);


    // ------------------------------------------------------------------------

    template <typename T>
    void zero_pad_resize(const hoNDArray<T>& complexIm, size_t sizeRO, size_t sizeE1, size_t sizeE2, hoNDArray<T>& complexImResized)
    {
        try
        {
            std::vector<size_t> dim;
            complexIm.get_dimensions(dim);

            size_t RO = complexIm.get_size(0);
            size_t E1 = complexIm.get_size(1);
            size_t E2 = complexIm.get_size(2);

            GADGET_CHECK_THROW(sizeRO >= RO);
            GADGET_CHECK_THROW(sizeE1 >= E1);

            if (sizeE2 > 1)
            {
                if (RO == sizeRO && E1 == sizeE1 && E2 == sizeE2)
                {
                    complexImResized = complexIm;
                    return;
                }

                if (complexImResized.get_size(0) != sizeRO || complexImResized.get_size(1) != sizeE1 || complexImResized.get_size(2) != sizeE2)
                {
                    dim[0] = sizeRO;
                    dim[1] = sizeE1;
                    dim[2] = sizeE2;
                    complexImResized.create(dim);
                }

                Gadgetron::clear(&complexImResized);

                hoNDArray<T> kspace(complexIm);
                Gadgetron::hoNDFFT<typename realType<T>::Type>::instance()->fft3c(complexIm, kspace);

                Gadgetron::pad(sizeRO, sizeE1, sizeE2, kspace, complexImResized);

                Gadgetron::hoNDFFT<typename realType<T>::Type>::instance()->ifft3c(complexImResized);

                typename realType<T>::Type scaling = (typename realType<T>::Type)(std::sqrt((double)sizeRO*sizeE1*sizeE2) / std::sqrt((double)RO*E1));
                Gadgetron::scal(scaling, complexImResized);
            }
            else
            {
                if (RO == sizeRO && E1 == sizeE1)
                {
                    complexImResized = complexIm;
                    return;
                }

                if (complexImResized.get_size(0) != sizeRO || complexImResized.get_size(1) != sizeE1)
                {
                    dim[0] = sizeRO;
                    dim[1] = sizeE1;
                    complexImResized.create(dim);
                }

                Gadgetron::clear(&complexImResized);

                hoNDArray<T> kspace(complexIm);
                Gadgetron::hoNDFFT<typename realType<T>::Type>::instance()->fft2c(complexIm, kspace);

                Gadgetron::pad(sizeRO, sizeE1, kspace, complexImResized);
                Gadgetron::hoNDFFT<typename realType<T>::Type>::instance()->ifft2c(complexImResized);

                typename realType<T>::Type scaling = (typename realType<T>::Type)(std::sqrt((double)sizeRO*sizeE1) / std::sqrt((double)RO*E1));
                Gadgetron::scal(scaling, complexImResized);
            }
        }
        catch (const std::exception& e)
        {
            GADGET_THROW(e.what());
        }
        catch (...)
        {
            GADGET_THROW("Errors in zero_pad_resize(...) ... ");
        }
    }

    template EXPORTMRICORE void zero_pad_resize(const hoNDArray< std::complex<float> >& complexIm, size_t sizeRO, size_t sizeE1, size_t sizeE2, hoNDArray< std::complex<float> >& complexImResized);
    template EXPORTMRICORE void zero_pad_resize(const hoNDArray< std::complex<double> >& complexIm, size_t sizeRO, size_t sizeE1, size_t sizeE2, hoNDArray< std::complex<double> >& complexImResized);

    // ------------------------------------------------------------------------

    template <typename T> 
    void compute_averaged_data_N_S(const hoNDArray<T>& data, bool average_N, bool average_S, bool count_sampling_freq, hoNDArray<T>& res)
    {

        size_t RO = data.get_size(0);
        size_t E1 = data.get_size(1);
        size_t E2 = data.get_size(2);
        size_t CHA = data.get_size(3);
        size_t N = data.get_size(4);
        size_t S = data.get_size(5);
        size_t SLC = data.get_size(6);

        typedef typename realType<T>::Type value_type;

        if (average_N)
        {
            if (N > 1)
            {
                if (count_sampling_freq)
                {
                    hoNDArray<bool> sampled = Gadgetron::detect_readout_sampling_status(data);
                    Gadgetron::sum_over_dimension(data, res, 4);

                    // for every E1/E2 location, count how many times it is sampled for all N
                    size_t ro, e1, e2, cha, n, s, slc;
                    for (slc = 0; slc < SLC; slc++)
                    {
                        for (cha = 0; cha < CHA; cha++)
                        {
                            for (e2 = 0; e2 < E2; e2++)
                            {
                                for (e1 = 0; e1 < E1; e1++)
                                {
                                    float freq = 0;

                                    for (s = 0; s < S; s++)
                                    {
                                        for (n = 0; n < N; n++)
                                        {
                                            if (sampled(e1, e2, n, s, slc)) freq += 1;
                                        }

                                        if (freq > 1)
                                        {
                                            value_type freq_reciprocal = (value_type)(1.0 / freq);
                                            T* pAve = &(res(0, e1, e2, cha, 0, s, slc));
                                            for (ro = 0; ro < RO; ro++) pAve[ro] *= freq_reciprocal;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    Gadgetron::sum_over_dimension(data, res, (size_t)4);
                    Gadgetron::scal((value_type)(1.0 / N), res);
                }
            }
            else
            {
                res = data;
            }
        }
        else
        {
            res = data;
        }

        if (average_S)
        {
            if (S > 1)
            {
                hoNDArray<T> dataTmp;
                Gadgetron::sum_over_dimension(res, dataTmp, 5);
                Gadgetron::scal((value_type)(1.0 / S), dataTmp);
                res = dataTmp;
            }
        }

    }

    template EXPORTMRICORE void compute_averaged_data_N_S(const hoNDArray< std::complex<float> >& data, bool average_N, bool average_S, bool count_sampling_freq, hoNDArray< std::complex<float> >& res);
    template EXPORTMRICORE void compute_averaged_data_N_S(const hoNDArray< std::complex<double> >& data, bool average_N, bool average_S, bool count_sampling_freq, hoNDArray< std::complex<double> >& res);

    // ------------------------------------------------------------------------

    template <typename T> 
    void select_data_N_S(const hoNDArray<T>& data, bool select_N, size_t n, bool select_S, size_t s, hoNDArray<T>& res)
    {
        try
        {
            size_t RO = data.get_size(0);
            size_t E1 = data.get_size(1);
            size_t E2 = data.get_size(2);
            size_t CHA = data.get_size(3);
            size_t N = data.get_size(4);
            size_t S = data.get_size(5);
            size_t SLC = data.get_size(6);

            std::vector<size_t> dim;
            data.get_dimensions(dim);

            std::vector<size_t> dimR(dim);

            size_t slc, ss, nn;

            if(!select_N && !select_S)
            {
                res = data;
            }
            else if(select_N && !select_S)
            {
                if(n<N)
                {
                    dimR[4] = 1;
                    res.create(dimR);

                    for (slc = 0; slc < SLC; slc++)
                    {
                        for (ss = 0; ss < S; ss++)
                        {
                            const T* pData = &(data(0, 0, 0, 0, n, ss, slc));
                            T* pRes = &res(0, 0, 0, 0, 0, ss, slc);
                            memcpy(pRes, pData, sizeof(T)*RO*E1*E2*CHA);
                        }
                    }
                }
                else
                {
                    GADGET_THROW("select_data_N_S, n>=N");
                }
            }
            else if (!select_N && select_S)
            {
                if (s<S)
                {
                    dimR[5] = 1;
                    res.create(dimR);

                    for (slc = 0; slc < SLC; slc++)
                    {
                        for (nn = 0; nn < N; nn++)
                        {
                            const T* pData = &(data(0, 0, 0, 0, nn, s, slc));
                            T* pRes = &res(0, 0, 0, 0, nn, 0, slc);
                            memcpy(pRes, pData, sizeof(T)*RO*E1*E2*CHA);
                        }
                    }
                }
                else
                {
                    GADGET_THROW("select_data_N_S, s>=S");
                }
            }
            else
            {
                GADGET_CHECK_THROW(n < N);
                GADGET_CHECK_THROW(s < S);

                dimR[4] = 1;
                dimR[5] = 1;
                res.create(dimR);

                for (slc = 0; slc < SLC; slc++)
                {
                    const T* pData = &(data(0, 0, 0, 0, n, s, slc));
                    T* pRes = &res(0, 0, 0, 0, 0, 0, slc);
                    memcpy(pRes, pData, sizeof(T)*RO*E1*E2*CHA);
                }
            }
        }
        catch (...)
        {
            GADGET_THROW("Errors in select_data_N_S(...) ... ");
        }
    }

    template EXPORTMRICORE void select_data_N_S(const hoNDArray< std::complex<float> >& data, bool select_N, size_t n, bool select_S, size_t s, hoNDArray< std::complex<float> >& res);
    template EXPORTMRICORE void select_data_N_S(const hoNDArray< std::complex<double> >& data, bool select_N, size_t n, bool select_S, size_t s, hoNDArray< std::complex<double> >& res);

    // ------------------------------------------------------------------------

    template <typename T> 
    void compute_eigen_channel_coefficients(const hoNDArray<T>& data, bool average_N, bool average_S, bool count_sampling_freq, size_t N, size_t S, double coil_compression_thres, size_t compression_num_modesKept, std::vector< std::vector< std::vector< hoNDKLT<T> > > >& KLT)
    {
        
        size_t RO = data.get_size(0);
        size_t E1 = data.get_size(1);
        size_t E2 = data.get_size(2);
        size_t CHA = data.get_size(3);
        size_t dataN = data.get_size(4);
        size_t dataS = data.get_size(5);
        size_t SLC = data.get_size(6);

        typedef typename realType<T>::Type value_type;

        size_t n, s, slc;

        hoNDArray<T> dataAve;
        GADGET_CATCH_THROW( Gadgetron::compute_averaged_data_N_S(data, average_N, average_S, count_sampling_freq, dataAve) );

        size_t dataAveN = dataAve.get_size(4);
        size_t dataAveS = dataAve.get_size(5);

        if(KLT.size()!=SLC) KLT.resize(SLC);
        for (slc = 0; slc < SLC; slc++)
        {
            if (KLT[slc].size() != S) KLT[slc].resize(S);
            for (s = 0; s < S; s++)
            {
                if (KLT[slc][s].size() != N) KLT[slc][s].resize(N);
            }
        }

        for (slc = 0; slc < SLC; slc++)
        {
            for (s = 0; s < S; s++)
            {
                size_t s_used = s;
                if (s_used >= dataAveS) s_used = dataAveS - 1;

                for (n = 0; n < N; n++)
                {
                    size_t n_used = n;
                    if (n_used >= dataAveN) n_used = dataAveN - 1;

                    T* pDataAve = &(dataAve(0, 0, 0, 0, n_used, s_used, slc));
                    hoNDArray<T> dataUsed(RO, E1, E2, CHA, pDataAve);

                    if (slc == 0 && n == 0 && s == 0)
                    {
                        if (compression_num_modesKept > 0)
                        {
                            KLT[slc][s][n].prepare(dataUsed, 3, compression_num_modesKept);
                        }
                        else if (coil_compression_thres > 0)
                        {
                            KLT[slc][s][n].prepare(dataUsed, 3, (value_type)(coil_compression_thres));
                        }
                        else
                        {
                            KLT[slc][s][n].prepare(dataUsed, 3, (size_t)(0));
                        }
                    }
                    else
                    {
                        if(n>=dataAveN && s>=dataAveS)
                        {
                            KLT[slc][s][n] = KLT[slc][dataAveS - 1][dataAveN-1];
                        }
                        else
                        {
                            KLT[slc][s][n].prepare(dataUsed, 3, KLT[0][0][0].output_length());
                        }
                    }
                }
            }
        }
        
    }

    template EXPORTMRICORE void compute_eigen_channel_coefficients(const hoNDArray< std::complex<float> >& data, bool average_N, bool average_S, bool count_sampling_freq, size_t N, size_t S, double coil_compression_thres, size_t compression_num_modesKept, std::vector< std::vector< std::vector< hoNDKLT< std::complex<float> > > > >& KLT);
    template EXPORTMRICORE void compute_eigen_channel_coefficients(const hoNDArray< std::complex<double> >& data, bool average_N, bool average_S, bool count_sampling_freq, size_t N, size_t S, double coil_compression_thres, size_t compression_num_modesKept, std::vector< std::vector< std::vector< hoNDKLT< std::complex<double> > > > >& KLT);

    // ------------------------------------------------------------------------

    template <typename T> 
    void apply_eigen_channel_coefficients(const std::vector< std::vector< std::vector< hoNDKLT<T> > > >& KLT, hoNDArray<T>& data)
    {
        try
        {
            size_t RO = data.get_size(0);
            size_t E1 = data.get_size(1);
            size_t E2 = data.get_size(2);
            size_t CHA = data.get_size(3);
            size_t N = data.get_size(4);
            size_t S = data.get_size(5);
            size_t SLC = data.get_size(6);

            GADGET_CHECK_THROW(KLT.size() == SLC);

            size_t dstCHA = KLT[0][0][0].output_length();

            hoNDArray<T> dstData;
            dstData.create(RO, E1, E2, dstCHA, N, S, SLC);

            size_t n, s, slc;
            for (slc = 0; slc < SLC; slc++)
            {
                for (s = 0; s < S; s++)
                {
                    size_t s_KLT = s;
                    if (s_KLT >= KLT[slc].size()) s_KLT = KLT[slc].size()-1;

                    for (n = 0; n < N; n++)
                    {
                        size_t n_KLT = n;
                        if (n_KLT >= KLT[slc][s_KLT].size()) n_KLT = KLT[slc][s_KLT].size()-1;

                        T* pData = &(data(0, 0, 0, 0, n, s, slc));
                        hoNDArray<T> data_in(RO, E1, E2, CHA, pData);

                        T* pDstData = &(dstData(0, 0, 0, 0, n, s, slc));
                        hoNDArray<T> data_out(RO, E1, E2, dstCHA, pDstData);

                        KLT[slc][s_KLT][n_KLT].transform(data_in, data_out, 3);
                    }
                }
            }

            data = dstData;
        }
        catch (...)
        {
            GADGET_THROW("Errors in apply_eigen_channel_coefficients(...) ... ");
        }
    }

    template EXPORTMRICORE void apply_eigen_channel_coefficients(const std::vector< std::vector< std::vector< hoNDKLT< std::complex<float> > > > >& KLT, hoNDArray< std::complex<float> >& data);
    template EXPORTMRICORE void apply_eigen_channel_coefficients(const std::vector< std::vector< std::vector< hoNDKLT< std::complex<double> > > > >& KLT, hoNDArray< std::complex<double> >& data);

    // ------------------------------------------------------------------------

    void get_debug_folder_path(const std::string& debugFolder, std::string& debugFolderPath)
    {
        char* v = std::getenv("GADGETRON_DEBUG_FOLDER");
        if (v == NULL)
        {
#ifdef _WIN32
            debugFolderPath = "c:/temp/gadgetron";
#else
            debugFolderPath = "/tmp/gadgetron";
#endif // _WIN32
        }
        else
        {
            debugFolderPath = std::string(v);
        }

        debugFolderPath.append("/");
        debugFolderPath.append(debugFolder);
        debugFolderPath.append("/");
    }

    // ------------------------------------------------------------------------

    void find_encoding_limits(mrd::Header& h, mrd::EncodingCounters& meas_max_idx, bool verbose)
    {
        try
        {
            mrd::EncodingSpaceType e_space = h.encoding[0].encoded_space;
            mrd::EncodingSpaceType r_space = h.encoding[0].recon_space;
            mrd::EncodingLimitsType e_limits = h.encoding[0].encoding_limits;

            meas_max_idx.kspace_encode_step_1 = (uint16_t)e_space.matrix_size.y - 1;

            meas_max_idx.set = (e_limits.set && (e_limits.set->maximum>0)) ? e_limits.set->maximum : 0;
            meas_max_idx.phase = (e_limits.phase && (e_limits.phase->maximum>0)) ? e_limits.phase->maximum : 0;

            meas_max_idx.kspace_encode_step_2 = (uint16_t)e_space.matrix_size.z - 1;

            meas_max_idx.contrast = (e_limits.contrast && (e_limits.contrast->maximum > 0)) ? e_limits.contrast->maximum : 0;

            meas_max_idx.slice = (e_limits.slice && (e_limits.slice->maximum > 0)) ? e_limits.slice->maximum : 0;

            meas_max_idx.repetition = e_limits.repetition ? e_limits.repetition->maximum : 0;

            meas_max_idx.average = e_limits.average ? e_limits.average->maximum : 0;

            // always combine the SEG
            meas_max_idx.segment = 0;
        }
        catch (...)
        {
            GADGET_THROW("Error happened in findEncodingLimits(...) ... ");
        }
    }

    // ------------------------------------------------------------------------

    void find_matrix_size_encoding(mrd::Header& h, size_t matrix_size_encoding[3])
    {
        matrix_size_encoding[0] = h.encoding[0].encoded_space.matrix_size.x;
        matrix_size_encoding[1] = h.encoding[0].encoded_space.matrix_size.y;
        matrix_size_encoding[2] = h.encoding[0].encoded_space.matrix_size.z;
    }

    // ------------------------------------------------------------------------

    void find_FOV_encoding(mrd::Header& h, float field_of_view_encoding[3])
    {
        field_of_view_encoding[0] = h.encoding[0].encoded_space.field_of_view_mm.x;
        field_of_view_encoding[1] = h.encoding[0].encoded_space.field_of_view_mm.y;
        field_of_view_encoding[2] = h.encoding[0].encoded_space.field_of_view_mm.z;
    }

    // ------------------------------------------------------------------------

    void find_matrix_size_recon(mrd::Header& h, size_t matrix_size_recon[3])
    {
        matrix_size_recon[0] = h.encoding[0].recon_space.matrix_size.x;
        matrix_size_recon[1] = h.encoding[0].recon_space.matrix_size.y;
        matrix_size_recon[2] = h.encoding[0].recon_space.matrix_size.z;
    }

    // ------------------------------------------------------------------------

    void find_FOV_recon(mrd::Header& h, float field_of_view_recon[3])
    {
        field_of_view_recon[0] = h.encoding[0].recon_space.field_of_view_mm.x;
        field_of_view_recon[1] = h.encoding[0].recon_space.field_of_view_mm.y;
        field_of_view_recon[2] = h.encoding[0].recon_space.field_of_view_mm.z;
    }

    // ------------------------------------------------------------------------

    void get_current_moment(std::string& procTime)
    {
        char timestamp[100];
        time_t mytime;
        struct tm *mytm;
        mytime = time(NULL);
        mytm = localtime(&mytime);
        strftime(timestamp, sizeof(timestamp), "%a, %b %d %Y, %H:%M:%S", mytm);
        procTime = timestamp;
    }

    // ------------------------------------------------------------------------

    template <typename T>
    void get_mrd_meta_values(const mrd::ImageMeta& attrib, const std::string& name, std::vector<T>& v)
    {
        try
        {
            if (attrib.count(name) == 0)
            {
                v.clear();
                GWARN_STREAM("get_mrd_meta_values, can not find field : " << name);
                return;
            }

            auto vs = attrib.at(name);
            v.resize(vs.size());
            for (size_t i = 0; i < vs.size(); i++) {
                v[i] = std::get<T>(vs[i]);
            }
        }
        catch (...)
        {
            GADGET_THROW("Error happened in get_ismrmrd_meta_values(const ISMRMRD::MetaContainer& attrib, const std::string& name, std::vector<T>& v) ... ");
        }
    }

    template void get_mrd_meta_values(const mrd::ImageMeta& attrib, const std::string& name, std::vector<long>& v);
    template void get_mrd_meta_values(const mrd::ImageMeta& attrib, const std::string& name, std::vector<double>& v);
    template void get_mrd_meta_values(const mrd::ImageMeta& attrib, const std::string& name, std::vector<std::string>& v);

    // ------------------------------------------------------------------------

    template <typename T>
    void set_mrd_meta_values(mrd::ImageMeta& attrib, const std::string& name, const std::vector<T>& v)
    {
        try
        {
            attrib[name].clear();
            std::copy(v.begin(), v.end(), std::back_inserter(attrib[name]));
        }
        catch (...)
        {
            GADGET_THROW("Error happened in setISMRMRMetaValues(ISMRMRD::MetaContainer& attrib, const std::string& name, const std::vector<T>& v) ... ");
        }
    }

    template void set_mrd_meta_values(mrd::ImageMeta& attrib, const std::string& name, const std::vector<long>& v);
    template void set_mrd_meta_values(mrd::ImageMeta& attrib, const std::string& name, const std::vector<double>& v);
    template void set_mrd_meta_values(mrd::ImageMeta& attrib, const std::string& name, const std::vector<std::string>& v);

    // ------------------------------------------------------------------------

    template <typename T>
    void append_mrd_meta_values(mrd::ImageMeta& attrib, const std::string& name, const std::vector<T>& v)
    {
        try
        {
            std::copy(v.begin(), v.end(), std::back_inserter(attrib[name]));
        }
        catch (...)
        {
            GADGET_THROW("Error happened in append_ismrmrd_meta_values(ISMRMRD::MetaContainer& attrib, const std::string& name, const std::vector<T>& v) ... ");
        }
    }

    template void append_mrd_meta_values(mrd::ImageMeta& attrib, const std::string& name, const std::vector<long>& v);
    template void append_mrd_meta_values(mrd::ImageMeta& attrib, const std::string& name, const std::vector<double>& v);
    template void append_mrd_meta_values(mrd::ImageMeta& attrib, const std::string& name, const std::vector<std::string>& v);

    // ------------------------------------------------------------------------

    void PatientCoordinateSystem_to_DeviceCoordinateSystem(double& x, double& y, double& z, const std::string& position)
    {
        // this is following dicom tag (0020, 0037)

        if (position == "HFS") // Head-first supine (HFS)
        {
            y = -y;
            z = -z;
        }
        else if (position == "HFP") // Head-first prone (HFP)
        {
            x = -x;
            z = -z;
        }
        else if (position == "HFDR") // Head-first decubitus-right
        {
            double v = x;
            x = y;
            y = v;
            z = -z;
        }
        else if (position == "HFDL") // Head-first decubitus-left (HFDL)
        {
            double v = x;
            x = y;
            y = v;

            x = -x;
            y = -y;
            z = -z;
        }
        else if (position == "FFDR") // Feet-first decubitus-right (FFDR)
        {
            double v = x;
            x = y;
            y = v;

            x = -x;
        }
        else if (position == "FFDL") // Feet-first decubitus-left (FFDL)
        {
            double v = x;
            x = y;
            y = v;

            y = -y;
        }
        else if (position == "FFP") // Feet-first prone (FFP)
        {
        }
        else if (position == "FFS") // Feet-first supine (FFS)
        {
            x = -x;
            y = -y;
        }
        else
        {
            GADGET_THROW("Unknown position string ... ");
        }
    }

    void DeviceCoordinateSystem_to_PatientCoordinateSystem(double& x, double& y, double& z, const std::string& position)
    {
        if (position == "HFS") // Head-first supine (HFS)
        {
            y = -y;
            z = -z;
        }
        else if (position == "HFP") // Head-first prone (HFP)
        {
            x = -x;
            z = -z;
        }
        else if (position == "HFDR") // Head-first decubitus-right
        {
            double v = x;
            x = y;
            y = v;
            z = -z;
        }
        else if (position == "HFDL") // Head-first decubitus-left (HFDL)
        {
            double v = x;
            x = y;
            y = v;

            x = -x;
            y = -y;
            z = -z;
        }
        else if (position == "FFDR") // Feet-first decubitus-right (FFDR)
        {
            double v = x;
            x = y;
            y = v;

            y = -y;
        }
        else if (position == "FFDL") // Feet-first decubitus-left (FFDL)
        {
            double v = x;
            x = y;
            y = v;

            x = -x;
        }
        else if (position == "FFP") // Feet-first prone (FFP)
        {
        }
        else if (position == "FFS") // Feet-first supine (FFS)
        {
            x = -x;
            y = -y;
        }
        else
        {
            GADGET_THROW("Unknown position string ... ");
        }
    }

    // ------------------------------------------------------------------------

    bool check_idential_slice_prescription(mrd::ImageHeader a, mrd::ImageHeader b)
    {
        GADGET_CHECK_RETURN_FALSE(std::abs(a.field_of_view[0] - b.field_of_view[0])<0.1);
        GADGET_CHECK_RETURN_FALSE(std::abs(a.field_of_view[1] - b.field_of_view[1])<0.1);
        GADGET_CHECK_RETURN_FALSE(std::abs(a.field_of_view[2] - b.field_of_view[2])<0.1);

        GADGET_CHECK_RETURN_FALSE(std::abs(a.patient_table_position[0] - b.patient_table_position[0])<0.001);
        GADGET_CHECK_RETURN_FALSE(std::abs(a.patient_table_position[1] - b.patient_table_position[1])<0.001);
        GADGET_CHECK_RETURN_FALSE(std::abs(a.patient_table_position[2] - b.patient_table_position[2])<0.001);

        GADGET_CHECK_RETURN_FALSE(std::abs(a.position[0] - b.position[0])<0.001);
        GADGET_CHECK_RETURN_FALSE(std::abs(a.position[1] - b.position[1])<0.001);
        GADGET_CHECK_RETURN_FALSE(std::abs(a.position[2] - b.position[2])<0.001);

        GADGET_CHECK_RETURN_FALSE(std::abs(a.col_dir[0] - b.col_dir[0])<0.001);
        GADGET_CHECK_RETURN_FALSE(std::abs(a.col_dir[1] - b.col_dir[1])<0.001);
        GADGET_CHECK_RETURN_FALSE(std::abs(a.col_dir[2] - b.col_dir[2])<0.001);

        GADGET_CHECK_RETURN_FALSE(std::abs(a.line_dir[0] - b.line_dir[0])<0.001);
        GADGET_CHECK_RETURN_FALSE(std::abs(a.line_dir[1] - b.line_dir[1])<0.001);
        GADGET_CHECK_RETURN_FALSE(std::abs(a.line_dir[2] - b.line_dir[2])<0.001);

        return true;
    }

    // ------------------------------------------------------------------------

    namespace {
        template<class T> std::map<std::string, decltype(T::value)> to_map_internal(const std::vector<T>& userparameters){
            std::map<std::string, decltype(T::value)> output_map;
            for (auto element : userparameters){
                output_map.insert({element.name, element.value});
            }
            return output_map;
        }
    }

    std::map<std::string, std::int64_t> to_map(const std::vector<mrd::UserParameterLongType> & userparameters) {
        return to_map_internal(userparameters);
    }

    std::map<std::string, double> to_map(const std::vector<mrd::UserParameterDoubleType> & userparameters) {
        return to_map_internal(userparameters);
    }

    std::map<std::string, std::string> to_map(const std::vector<mrd::UserParameterStringType> & userparameters) {
        return to_map_internal(userparameters);
    }

    mrd::ImageHeader image_header_from_acquisition(const mrd::AcquisitionHeader& acq_header, const mrd::Header& header) {

        mrd::ImageHeader im_header;

        im_header.measurement_uid = acq_header.measurement_uid;

        im_header.field_of_view[0] = header.encoding[0].recon_space.field_of_view_mm.x;
        im_header.field_of_view[1] = header.encoding[0].recon_space.field_of_view_mm.y;
        im_header.field_of_view[2] = header.encoding[0].recon_space.field_of_view_mm.z;

        im_header.position = acq_header.position;
        im_header.col_dir = acq_header.read_dir;
        im_header.line_dir = acq_header.phase_dir;
        im_header.slice_dir = acq_header.slice_dir;
        im_header.patient_table_position = acq_header.patient_table_position;

        im_header.average    = acq_header.idx.average;
        im_header.slice      = acq_header.idx.slice;
        im_header.contrast   = acq_header.idx.contrast;
        im_header.phase      = acq_header.idx.phase;
        im_header.repetition = acq_header.idx.repetition;
        im_header.set        = acq_header.idx.set;

        im_header.acquisition_time_stamp = acq_header.acquisition_time_stamp;

        im_header.physiology_time_stamp = acq_header.physiology_time_stamp;

        im_header.image_type = mrd::ImageType::kComplex;
        im_header.image_index = 0;
        im_header.image_series_index = 0;

        im_header.user_int = acq_header.user_int;
        im_header.user_float = acq_header.user_float;

        return im_header;
    }


void initialize_bucket_stats(mrd::AcquisitionBucketStats& stats)
{
    stats.kspace_encode_step_1.minimum = std::numeric_limits<uint32_t>::max();
    stats.kspace_encode_step_2.minimum = std::numeric_limits<uint32_t>::max();
    stats.average.minimum = std::numeric_limits<uint32_t>::max();
    stats.slice.minimum = std::numeric_limits<uint32_t>::max();
    stats.contrast.minimum = std::numeric_limits<uint32_t>::max();
    stats.phase.minimum = std::numeric_limits<uint32_t>::max();
    stats.repetition.minimum = std::numeric_limits<uint32_t>::max();
    stats.set.minimum = std::numeric_limits<uint32_t>::max();
    stats.segment.minimum = std::numeric_limits<uint32_t>::max();
}

void add_stats_to_bucket(mrd::AcquisitionBucketStats& stats, const mrd::AcquisitionHeader& header)
{
    auto ke1 = header.idx.kspace_encode_step_1.value_or(0);
    if (ke1 < stats.kspace_encode_step_1.minimum) stats.kspace_encode_step_1.minimum = ke1;
    if (ke1 > stats.kspace_encode_step_1.maximum) stats.kspace_encode_step_1.maximum = ke1;
    auto ke2 = header.idx.kspace_encode_step_2.value_or(0);
    if (ke2 < stats.kspace_encode_step_2.minimum) stats.kspace_encode_step_2.minimum = ke2;
    if (ke2 > stats.kspace_encode_step_2.maximum) stats.kspace_encode_step_2.maximum = ke2;
    auto average = header.idx.average.value_or(0);
    if (average < stats.average.minimum) stats.average.minimum = average;
    if (average > stats.average.maximum) stats.average.maximum = average;
    auto slice = header.idx.slice.value_or(0);
    if (slice < stats.slice.minimum) stats.slice.minimum = slice;
    if (slice > stats.slice.maximum) stats.slice.maximum = slice;
    auto contrast = header.idx.contrast.value_or(0);
    if (contrast < stats.contrast.minimum) stats.contrast.minimum = contrast;
    if (contrast > stats.contrast.maximum) stats.contrast.maximum = contrast;
    auto phase = header.idx.phase.value_or(0);
    if (phase < stats.phase.minimum) stats.phase.minimum = phase;
    if (phase > stats.phase.maximum) stats.phase.maximum = phase;
    auto repetition = header.idx.repetition.value_or(0);
    if (repetition < stats.repetition.minimum) stats.repetition.minimum = repetition;
    if (repetition > stats.repetition.maximum) stats.repetition.maximum = repetition;
    auto set = header.idx.set.value_or(0);
    if (set < stats.set.minimum) stats.set.minimum = set;
    if (set > stats.set.maximum) stats.set.maximum = set;
    auto segment = header.idx.segment.value_or(0);
    if (segment < stats.segment.minimum) stats.segment.minimum = segment;
    if (segment > stats.segment.maximum) stats.segment.maximum = segment;
}

void add_acquisition_to_bucket(mrd::AcquisitionBucket& bucket, mrd::Acquisition acq)
{
    auto espace = size_t{acq.head.encoding_space_ref.value_or(0)};

    if (acq.head.flags.HasFlags(mrd::AcquisitionFlags::kIsParallelCalibration) ||
        acq.head.flags.HasFlags(mrd::AcquisitionFlags::kIsParallelCalibrationAndImaging)) {
        bucket.ref.push_back(acq);
        if (bucket.refstats.size() < (espace + 1)) {
            bucket.refstats.resize(espace + 1);
            initialize_bucket_stats(bucket.refstats[espace]);
        }
        add_stats_to_bucket(bucket.refstats[espace], acq.head);
    }
    if (!(acq.head.flags.HasFlags(mrd::AcquisitionFlags::kIsParallelCalibration) ||
          acq.head.flags.HasFlags(mrd::AcquisitionFlags::kIsPhasecorrData))) {
        if (bucket.datastats.size() < (espace + 1)) {
            bucket.datastats.resize(espace + 1);
            initialize_bucket_stats(bucket.datastats[espace]);
        }
        add_stats_to_bucket(bucket.datastats[espace], acq.head);
        bucket.data.emplace_back(std::move(acq));
    }
}
}
