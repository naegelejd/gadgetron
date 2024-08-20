#include "mri_core_data.h"


using namespace Gadgetron;

/** TODO Joe: Remove these vvvvv */
// #include <unordered_map>
// static const std::unordered_map<std::string,Gadgetron::IsmrmrdCondition> condition_map = {
//     { "kspace_encode_step_1",IsmrmrdCONDITION::KSPACE_ENCODE_STEP_1},
//     { "kspace_encode_step_2",IsmrmrdCONDITION::KSPACE_ENCODE_STEP_2},
//     {"average",IsmrmrdCONDITION::AVERAGE},
//     {"slice",IsmrmrdCONDITION::SLICE},
//     {"contrast",IsmrmrdCONDITION::CONTRAST},
//     {"phase",IsmrmrdCONDITION::PHASE},
//     {"repetition",IsmrmrdCONDITION::REPETITION},
//     {"set",IsmrmrdCONDITION::SET},
//     {"segment",IsmrmrdCONDITION::SEGMENT},
//     {"user_0",IsmrmrdCONDITION::USER_0},
//     {"user_1",IsmrmrdCONDITION::USER_1},
//     {"user_2",IsmrmrdCONDITION::USER_2},
//     {"user_3",IsmrmrdCONDITION::USER_3},
//     {"user_4",IsmrmrdCONDITION::USER_4},
//     {"user_5",IsmrmrdCONDITION::USER_5},
//     {"user_6",IsmrmrdCONDITION::USER_6},
//     {"user_7",IsmrmrdCONDITION::USER_7},
//     {"n_acquisitions",IsmrmrdCONDITION::N_ACQUISITIONS},
//     {"none",IsmrmrdCONDITION::NONE},
//     {"",IsmrmrdCONDITION::NONE}

// };

// void Gadgetron::from_string(const std::string& stringval, IsmrmrdCondition& condition){
//     std::string stringval_lower = stringval;
//     for(auto& s : stringval_lower) s = std::to_lower(s);
//     if (!condition_map.count(stringval_lower)) throw std::runtime_error("Could not convert string to IsmrmrdCondition");
//     return condition_map[stringval_lower];
// }


namespace Gadgetron {
void add_stats_to_bucket(mrd::AcquisitionBucketStats& stats, const mrd::AcquisitionHeader& header) {
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

void add_acquisition_to_bucket(mrd::AcquisitionBucket& bucket, mrd::Acquisition acq) {
    auto espace = size_t{acq.head.encoding_space_ref.value_or(0)};

    if (acq.head.flags.HasFlags(mrd::AcquisitionFlags::kIsParallelCalibration) ||
        acq.head.flags.HasFlags(mrd::AcquisitionFlags::kIsParallelCalibrationAndImaging)) {
        bucket.ref.push_back(acq);
        if (bucket.refstats.size() < (espace + 1)) {
            bucket.refstats.resize(espace + 1);
        }
        add_stats_to_bucket(bucket.refstats[espace], acq.head);
    }
    if (!(acq.head.flags.HasFlags(mrd::AcquisitionFlags::kIsParallelCalibration) ||
          acq.head.flags.HasFlags(mrd::AcquisitionFlags::kIsPhasecorrData))) {
        if (bucket.datastats.size() < (espace + 1)) {
            bucket.datastats.resize(espace + 1);
        }
        add_stats_to_bucket(bucket.datastats[espace], acq.head);
        bucket.data.emplace_back(std::move(acq));
    }
}

}