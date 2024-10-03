/**
    \brief  Separates ReconData into separate images and performs FFT
    \test   Tested by: epi_2d.cfg
*/

#pragma once
#include "Node.h"
#include "hoNDArray.h"

#include "mri_core_data.h"
#include <complex>
#include "hoNDFFT.h"

namespace Gadgetron{

    class FFTGadget : public Core::ChannelGadget<ReconData> {
    public:
        FFTGadget(const Core::Context& context, const Core::GadgetProperties& props);
        void process(Core::InputChannel<ReconData>& input, Core::OutputChannel& out) override;

    protected:
        long long image_counter_;      

    };
}

