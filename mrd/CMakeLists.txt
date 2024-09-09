
set(HOWARD_HINNANT_DATE_MINIMUM_VERSION "3.0.0")
find_package(date ${HOWARD_HINNANT_DATE_MINIMUM_VERSION} REQUIRED)

if(VCPKG_TARGET_TRIPLET)
  set(HDF5_CXX_LIBRARIES hdf5::hdf5_cpp-shared)
else()
  set(HDF5_CXX_LIBRARIES hdf5::hdf5_cpp)
endif()

# set(HDF5_MINIMUM_VERSION "1.10.5")
# find_package(HDF5 ${HDF5_MINIMUM_VERSION} REQUIRED COMPONENTS C CXX)

set(XTENSOR_MINIMUM_VERSION "0.21.10")
find_package(xtensor ${XTENSOR_MINIMUM_VERSION} REQUIRED)

set(NLOHMANN_JSON_MINIMUM_VERSION "3.11.1")
find_package(nlohmann_json ${NLOHMANN_JSON_MINIMUM_VERSION} REQUIRED)

# add_library(mrd_generated OBJECT
#   protocols.cc
#   types.cc
#   ndjson/protocols.cc
#   binary/protocols.cc
#   hdf5/protocols.cc
# )

# target_link_libraries(mrd_generated
#   PUBLIC ${HDF5_C_LIBRARIES}
#   PUBLIC ${HDF5_CXX_LIBRARIES}
#   PUBLIC xtensor
#   PUBLIC date::date
#   PUBLIC nlohmann_json::nlohmann_json
# )

set(mrd_sources
    ndarray/ndarray_custom.h
    mrd/cpp/mrd/protocols.cc
    mrd/cpp/mrd/types.cc
    mrd/cpp/mrd/binary/protocols.cc
)

add_library(gadgetron_mrd SHARED ${mrd_sources})
add_library(mrd::mrd ALIAS gadgetron_mrd)

target_include_directories(gadgetron_mrd
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ndarray/>"
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/mrd/cpp/>"
)

target_link_libraries(gadgetron_mrd
    PUBLIC xtensor
    PUBLIC date::date
    PUBLIC nlohmann_json::nlohmann_json
    gadgetron_toolbox_cpucore
)

install(TARGETS gadgetron_mrd
		EXPORT gadgetron-export
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
	COMPONENT main
)

add_executable(
  ismrmrd_to_mrd
  mrd/cpp/mrd-tools/ismrmrd_to_mrd.cc
  mrd/cpp/mrd-tools/converters.cc
    mrd/cpp/mrd/protocols.cc
    mrd/cpp/mrd/types.cc
    mrd/cpp/mrd/binary/protocols.cc
)

target_include_directories(ismrmrd_to_mrd
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/mrd/cpp/>"
)

target_link_libraries(
  ismrmrd_to_mrd
#   mrd::mrd
  ISMRMRD::ISMRMRD
)

add_executable(
  mrd_to_ismrmrd
  mrd/cpp/mrd-tools/mrd_to_ismrmrd.cc
  mrd/cpp/mrd-tools/converters.cc
    mrd/cpp/mrd/protocols.cc
    mrd/cpp/mrd/types.cc
    mrd/cpp/mrd/binary/protocols.cc

)

target_include_directories(mrd_to_ismrmrd
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/mrd/cpp/>"
)

target_link_libraries(
  mrd_to_ismrmrd
#   mrd::mrd
  ISMRMRD::ISMRMRD
)

install(TARGETS
    ismrmrd_to_mrd
    mrd_to_ismrmrd
    DESTINATION bin)