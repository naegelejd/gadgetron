#include "IsmrmrdContextVariables.h"

#include <optional>
#include <regex>

#include "log.h"

namespace Gadgetron {

IsmrmrdContextVariables::IsmrmrdContextVariables(mrd::Header const& head) {
    if (head.acquisition_system_information && head.acquisition_system_information->device_id) {
        device_id_ = head.acquisition_system_information->device_id.value();
    }

    if (head.measurement_information && head.measurement_information->measurement_id) {
        measurement_id_ = head.measurement_information->measurement_id.value();
    }

    if (head.subject_information && head.subject_information->patient_id) {
        subject_id_ = head.subject_information->patient_id.value();
    }

    if (head.study_information && head.study_information->study_id) {
        session_id_ = head.study_information->study_id.value();
    }

    if (measurement_id_.empty() || (!session_id_.empty() && !device_id_.empty() && !subject_id_.empty())) {
        return;
    }

    // We were unable to find all that we need, we can attempt to extract from measurement id.
    update_from_measurement_id();
}

IsmrmrdContextVariables::IsmrmrdContextVariables(std::string measurement_id) : measurement_id_(measurement_id) {
    update_from_measurement_id();
}

void IsmrmrdContextVariables::update_from_measurement_id() {
    static std::regex reg("^([^_]*)_([^_]*)_([^_]*)_([^_]*)$");
    std::smatch match;

    if (!std::regex_match(measurement_id_, match, reg)) {
        GWARN_STREAM("WARNING: Attempted to extract context variables from measurement id, but failed.");
        return;
    }

    if (device_id_.empty()) {
        device_id_ = match[1];
    }

    if (subject_id_.empty()) {
        subject_id_ = match[2];
    }

    if (session_id_.empty()) {
        session_id_ = match[3];
    }
}

} // namespace Gadgetron
