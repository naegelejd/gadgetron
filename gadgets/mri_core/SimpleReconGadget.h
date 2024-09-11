/**
    \brief  Performs basic FFT reconstruction on IsmrmrdReconData and passes along as IsmrmrdReconData
    \test   Tested by: simple_gre.cfg, simple_gre_python_image_array_recon.cfg, and others
*/

#pragma once

#include "Node.h"
#include "hoNDArray.h"

#include "mri_core_data.h"
#include "hoNDArray_math.h"
#include "hoNDFFT.h"
#include <complex>

namespace Gadgetron {

    class SimpleReconGadget : public Core::ChannelGadget<ReconData> {
    public:
        SimpleReconGadget(const Core::Context& context, const Core::GadgetProperties& props);
        void process(Core::InputChannel<ReconData>& input, Core::OutputChannel& out) override;

    protected:
        mrd::Header header;
        long long image_counter_;      
    };
}