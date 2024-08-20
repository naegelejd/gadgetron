#pragma once

#include <optional>

#include <mrd/types.h>

#include <boost/process.hpp>
#include <boost/program_options.hpp>

#include "StorageSetup.h"

namespace Gadgetron::Server {
std::tuple<std::string, std::optional<boost::process::child>>
ensure_storage_server(const boost::program_options::variables_map& args);

StorageSpaces setup_storage_spaces(const std::string& address, const mrd::Header& header);
} // namespace Gadgetron::Server
