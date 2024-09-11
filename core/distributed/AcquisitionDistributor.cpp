
#include <map>
#include "AcquisitionDistributor.h"

namespace {

    using Header = mrd::AcquisitionHeader;
    const std::unordered_map<std::string, std::function<uint32_t(const Header &)>> function_map = {
            {"kspace_encode_step_1", [](const Header &header) { return header.idx.kspace_encode_step_1.value_or(0); }},
            {"kspace_encode_step_2", [](const Header &header) { return header.idx.kspace_encode_step_2.value_or(0); }},
            {"average",              [](const Header &header) { return header.idx.average.value_or(0); }},
            {"slice",                [](const Header &header) { return header.idx.slice.value_or(0); }},
            {"contrast",             [](const Header &header) { return header.idx.contrast.value_or(0); }},
            {"phase",                [](const Header &header) { return header.idx.phase.value_or(0); }},
            {"repetition",           [](const Header &header) { return header.idx.repetition.value_or(0); }},
            {"set",                  [](const Header &header) { return header.idx.set.value_or(0); }},
            {"segment",              [](const Header &header) { return header.idx.segment.value_or(0); }},
            {"user_0",               [](const Header &header) { return header.idx.user[0]; }},
            {"user_1",               [](const Header &header) { return header.idx.user[1]; }},
            {"user_2",               [](const Header &header) { return header.idx.user[2]; }},
            {"user_3",               [](const Header &header) { return header.idx.user[3]; }},
            {"user_4",               [](const Header &header) { return header.idx.user[4]; }},
            {"user_5",               [](const Header &header) { return header.idx.user[5]; }},
            {"user_6",               [](const Header &header) { return header.idx.user[6]; }},
            {"user_7",               [](const Header &header) { return header.idx.user[7]; }}
    };

    const std::function<uint32_t(const mrd::AcquisitionHeader&)> &selector_function(const std::string &key) {
        return function_map.at(key);
    }
}


namespace Gadgetron::Core::Distributed {

    void AcquisitionDistributor::process(InputChannel<Acquisition> &input,
            ChannelCreator &creator
    ) {
        std::map<uint32_t, OutputChannel> channels{};

        for (Acquisition acq : input) {
            auto index = selector(acq.head);

            if (!channels.count(index)) channels.emplace(index, creator.create());

            channels.at(index).push(std::move(acq));
        }
    }

    AcquisitionDistributor::AcquisitionDistributor(
            const Gadgetron::Core::Context &context,
            const Gadgetron::Core::GadgetProperties &props
    ) : TypedDistributor(props), selector(selector_function(parallel_dimension)) {}

    GADGETRON_DISTRIBUTOR_EXPORT(AcquisitionDistributor);
}