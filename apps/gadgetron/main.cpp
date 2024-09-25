#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "log.h"
#include "gadgetron_paths.h"
#include "initialization.h"

#include "system_info.h"
#include "gadgetron_config.h"

#include "StreamConsumer.h"


using namespace boost::filesystem;
using namespace boost::program_options;
using namespace Gadgetron::Server;

using gadget_parameter = std::pair<std::string, std::string>;

std::istream& operator>>(std::istream& in, gadget_parameter& param) {
    std::string token;
    in >> token;
    // parse <key>=<value> into a gadget_parameter
    auto pos = token.find('=');
    if (pos == std::string::npos) {
        throw std::runtime_error("Invalid gadget parameter: " + token);
    }
    param.first = token.substr(0, pos);
    param.second = token.substr(pos + 1);
    return in;
}

std::ostream& operator<<(std::ostream& out, const gadget_parameter& param) {
    out << param.first << "=" << param.second;
    return out;
}

int main(int argc, char *argv[]) {
    options_description gadgetron_options("Allowed options:");
    gadgetron_options.add_options()
            ("help,h", "Prints this help message.")
            ("info", "Prints build info about the Gadgetron.")
            ("dir,W",
                value<path>()->default_value(default_working_folder()),
                "Set the Gadgetron working directory.")
            ("home,G",
                value<path>()->default_value(default_gadgetron_home()),
                "Set the Gadgetron home directory.")
            ("input,i",
                value<std::string>(),
                "Input file for binary data to perform a local reconstruction with")
            ("output,o",
                value<std::string>(),
                "Output file for binary data as a result of a local reconstruction")
            ("config,c",
                value<std::string>(),
                "Filename of the desired gadgetron reconstruction config.")
            ("parameter",
                value<std::vector<gadget_parameter>>(),
                "Parameter to be passed to the gadgetron reconstruction config. Multiple parameters can be passed."
                "Format: --parameter <name>=<value> --parameter <name>=<value> ...");

    options_description desc;
    desc.add(gadgetron_options);

    variables_map args;
    store(parse_command_line(argc, argv, desc), args);
    notify(args);

    try {
        check_environment_variables();
        configure_blas_libraries();
        set_locale();

        if (args.count("help")) {
            GINFO_STREAM(desc);
            return 0;
        }

        if (args.count("info")) {
            std::stringstream str;
            Info::print_system_information(str);
            GINFO(str.str().c_str());
            return 0;
        }

        GINFO("Gadgetron %s [%s]\n", GADGETRON_VERSION_STRING, GADGETRON_GIT_SHA1_HASH);

        // Ensure working directory exists.
        create_directories(args["dir"].as<path>());

        if (!args.count("config"))
        {
            GERROR_STREAM("No config file provided. Use --config/-c");
            return 1;
        }

        auto cfg = args["config"].as<std::string>();
        StreamConsumer consumer(args);

        if(args.count("input") && args.count("output"))
        {
            auto input_stream = std::ifstream(args["input"].as<std::string>());
            if (!input_stream) {
                GERROR_STREAM("Could not open input file: " << args["input"].as<std::string>());
                return 1;
            }
            auto output_stream = std::ofstream(args["output"].as<std::string>());
            if (!output_stream) {
                GERROR_STREAM("Could not open output file: " << args["output"].as<std::string>());
                return 1;
            }
            consumer.consume(input_stream, output_stream, cfg);
            output_stream.close();
        }
        else if(args.count("input"))
        {
            auto input_stream = std::ifstream(args["input"].as<std::string>());
            if (!input_stream) {
                GERROR_STREAM("Could not open input file: " << args["input"].as<std::string>());
                return 1;
            }
            consumer.consume(input_stream, std::cout, cfg);
            std::flush(std::cout);
        }
        else if(args.count("output"))
        {
            auto output_stream = std::ofstream(args["output"].as<std::string>());
            if (!output_stream) {
                GERROR_STREAM("Could not open output file: " << args["output"].as<std::string>());
                return 1;
            }
            consumer.consume(std::cin, output_stream, cfg);
            output_stream.close();
        }
        else
        {
            consumer.consume(std::cin, std::cout, cfg);
            std::flush(std::cout);
        }
        GDEBUG_STREAM("Finished consuming stream");
    }
    catch (std::exception &e)
    {
        GERROR_STREAM(e.what() << std::endl);
        std::exit(EXIT_FAILURE);
    }
    catch(...)
    {
        GERROR_STREAM("Unhandled exception, exiting" << std::endl);
        std::exit(EXIT_FAILURE);
    }

    return 0;
}
