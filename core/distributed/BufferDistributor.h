#pragma once

#include "Context.h"
#include "Distributor.h"

#include "mri_core_data.h"

namespace Gadgetron::Core::Distributed {
    class BufferDistributor : public TypedDistributor<ReconData> {

    public:
        BufferDistributor(const Context &context, const GadgetProperties &props);

        void process(InputChannel<ReconData> &input, ChannelCreator &creator) override;

    private:
        const size_t encoding_spaces;
    };
}


