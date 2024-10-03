#pragma once

#include "Gadget.h"
#include "Node.h"

namespace Gadgetron {

    class ImageFinishGadget : public Core::GenericChannelGadget {
    public:
        ImageFinishGadget(
                const Core::Context &context,
                const Core::GadgetProperties &properties
        ) : GenericChannelGadget(context,properties) {};

    protected:
        void process(Core::GenericInputChannel& in,
                    Core::OutputChannel& out) override;

    };
}

