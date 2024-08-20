#include <mrd/types.h>
#include <gtest/gtest.h>

#include <stdlib.h>

#include "IsmrmrdContextVariables.h"

using namespace Gadgetron;

using namespace std::chrono_literals;

mrd::SubjectInformationType subject = {.patient_id = "925046864"};
mrd::StudyInformationType study = {.study_id = "1076436037"};
mrd::MeasurementInformationType measurement = {
  .measurement_id = "LLJGHH888986",
  .series_date = yardl::Date(date::year{2012}/8/13),
  .series_time = 9h + 10min + 12s,
  .patient_position = mrd::PatientPosition::kHFS,
  .initial_series_number = 1,
  .protocol_name = "ExampleProt",
  .series_description = "MRIStudy1",
};
mrd::AcquisitionSystemInformationType acquisition = {
  .system_field_strength_t = 1.494,
  .relative_receiver_noise_bandwidth = 0.79,
  .device_id = "20434"
};

mrd::Header mrd_header_with_variables = {
  .subject_information = subject,
  .study_information = study,
  .measurement_information = measurement,
  .acquisition_system_information = acquisition
};


mrd::Header mrd_header_with_structured_measurement_id = {
  .study_information = std::optional<mrd::StudyInformationType>({.study_time = 16h + 13min + 23s}),
  .measurement_information = std::optional<mrd::MeasurementInformationType>({
    .measurement_id = "45387_925046864_1076436037_393",
    .patient_position = mrd::PatientPosition::kHFS,
    .protocol_name = "R1_PEFOV100_PERes100",
    .frame_of_reference_uid = "1.3.12.2.1107.5.2.19.45387.1.20160311172423043.0.0.0"
  }),
  .acquisition_system_information = std::optional<mrd::AcquisitionSystemInformationType>({
    .system_vendor = "SIEMENS",
    .system_model = "Skyra",
    .system_field_strength_t = 2.893620,
    .relative_receiver_noise_bandwidth = 0.793000,
    .receiver_channels = 26,
    .institution_name = "NIH"
  })
};

TEST(IsmrmrdContextVariablesTest, initialize_from_ismrmrd_header_with_all_elements) {

  IsmrmrdContextVariables ctx_vars(mrd_header_with_variables);

  EXPECT_EQ(ctx_vars.device_id(), "20434");
  EXPECT_EQ(ctx_vars.subject_id(), "925046864");
  EXPECT_EQ(ctx_vars.session_id(), "1076436037");
  EXPECT_EQ(ctx_vars.measurement_id(), "LLJGHH888986");
}

TEST(IsmrmrdContextVariablesTest, initialize_from_ismrmrd_header_with_structured_measurement_id) {

  IsmrmrdContextVariables ctx_vars(mrd_header_with_structured_measurement_id);
  
  EXPECT_EQ(ctx_vars.device_id(), "45387");
  EXPECT_EQ(ctx_vars.subject_id(), "925046864");
  EXPECT_EQ(ctx_vars.session_id(), "1076436037");
  EXPECT_EQ(ctx_vars.measurement_id(), "45387_925046864_1076436037_393");
}

TEST(IsmrmrdContextVariablesTest, malformed_structured_ids_handled) {
  auto measurements = std::vector<std::string> {
    "45387_925046864_925046864_1076436037_393",
    "453879250468649250468641076436037393",
    "45387__925046864_1076436037_393",
    };

  for (auto measurement_id : measurements) {
    auto head = mrd_header_with_structured_measurement_id;
    head.measurement_information->measurement_id = measurement_id;

    IsmrmrdContextVariables ctx_vars(head);
    EXPECT_EQ(ctx_vars.subject_id(), "");
    EXPECT_EQ(ctx_vars.device_id(), "");
    EXPECT_EQ(ctx_vars.session_id(), "");
    EXPECT_EQ(ctx_vars.measurement_id(), measurement_id);
  }
}

TEST(IsmrmrdContextVariablesTest, initialize_from_structured_measurement_id_string) {
  IsmrmrdContextVariables ctx_vars("45387_925046864_1076436037_393");
  
  EXPECT_EQ(ctx_vars.device_id(), "45387");
  EXPECT_EQ(ctx_vars.subject_id(), "925046864");
  EXPECT_EQ(ctx_vars.session_id(), "1076436037");
  EXPECT_EQ(ctx_vars.measurement_id(), "45387_925046864_1076436037_393");
}

TEST(IsmrmrdContextVariablesTest, initialize_from_unstructured_measurement_id_string) {
  IsmrmrdContextVariables ctx_vars("453879250468649250468641076436037393");
  
  EXPECT_EQ(ctx_vars.device_id(), "");
  EXPECT_EQ(ctx_vars.subject_id(), "");
  EXPECT_EQ(ctx_vars.session_id(), "");
  EXPECT_EQ(ctx_vars.measurement_id(), "453879250468649250468641076436037393");
}
