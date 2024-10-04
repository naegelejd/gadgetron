#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Types.h"

namespace Gadgetron::Server::Connection {

    struct Config {

        struct Gadget;
        struct Parallel;
        struct ParallelProcess;
        using Node = Core::variant<Gadget, Parallel, ParallelProcess>;

        template<class CONFIG>
        static std::string name(CONFIG config) {
            return config.name.empty() ? config.classname : config.name;
        }

        struct Stream {
            std::string key;
            std::vector<Node> nodes;
        };

        struct PureStream{
            std::vector<Gadget> gadgets;
        };

        struct Gadget {
            std::string name, dll, classname;
            std::unordered_map<std::string, std::string> properties;
            Gadget(std::string name, std::string dll, std::string classname, std::unordered_map<std::string, std::string> properties):
            name(std::move(name)), dll(std::move(dll)), classname(std::move(classname)), properties(std::move(properties))
            {
            }
        };

        struct Branch : Gadget { using Gadget::Gadget;};
        struct Merge : Gadget { using Gadget::Gadget;};

        struct Parallel {
            Branch branch;
            Merge merge;
            std::vector<Stream> streams;
        };

        struct ParallelProcess {
            size_t workers = 0;
            PureStream stream;
        };

        Stream stream;
    };

    Config parse_config(std::istream &stream);
    std::string serialize_config(const Config& config);
}
