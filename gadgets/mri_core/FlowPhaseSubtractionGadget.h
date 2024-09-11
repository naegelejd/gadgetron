#pragma once
#include "Gadget.h"
#include "hoNDArray.h"

#include <complex>

namespace Gadgetron{
  
    class FlowPhaseSubtractionGadget : public Core::ChannelGadget<Core::Image<std::complex<float>>>
    {

    public:
        using Core::ChannelGadget<Core::Image<std::complex<float>>>::ChannelGadget;

        ~FlowPhaseSubtractionGadget() override = default;

        void process(Core::InputChannel<Core::Image<std::complex<float>>>& in, Core::OutputChannel& out) override;
    };
}

