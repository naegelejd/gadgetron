find_package(PugiXML REQUIRED)

configure_file(gadgetron_config.in gadgetron_config.h)

add_executable(gadgetron
        main.cpp
        initialization.cpp
        initialization.h
        system_info.cpp

        connection/Core.h
        connection/Loader.cpp
        connection/Loader.h

        connection/config/Config.cpp
        connection/config/Config.h

        connection/core/Processable.h
        connection/core/Processable.cpp

        connection/nodes/Stream.cpp
        connection/nodes/Stream.h
        connection/nodes/Parallel.cpp
        connection/nodes/Parallel.h
        connection/nodes/ParallelProcess.cpp
        connection/nodes/ParallelProcess.h
        connection/nodes/PureStream.cpp
        connection/nodes/PureStream.h
        )

target_link_libraries(gadgetron
        gadgetron_core
        gadgetron_toolbox_log
        gadgetron_toolbox_mri_core
        Boost::system
        Boost::filesystem
        Boost::program_options
        ${PUGIXML_LIBRARIES}
        GTBLAS
        ${CMAKE_DL_LIBS})

target_include_directories(gadgetron
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR})


if (REQUIRE_SIGNED_CONFIG)
    target_link_libraries(gadgetron GTBabylon)
endif()

if (BUILD_PYTHON_SUPPORT)
    add_definitions("-DCOMPILING_WITH_PYTHON_SUPPORT")
endif ()

if (CUDA_FOUND)
    target_link_libraries(gadgetron ${CUDA_LIBRARIES})
endif ()

if (GPERFTOOLS_PROFILER)
    message("Adding gperftools cpu profiler to gadgetron link assemblage.")
    target_link_libraries(gadgetron ${GPERFTOOLS_PROFILER} ${GPERFTOOLS_TCMALLOC})
endif ()

install(TARGETS gadgetron DESTINATION bin COMPONENT main)
