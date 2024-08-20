#include <filesystem>
#include <future>
#include <memory>
#include <sstream>
#include <thread>

#include <boost/asio.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/program_options/variables_map.hpp>

#include <mrd/binary/protocols.h>
#include <mri_core_data.h>

#include "Channel.h"
#include "connection/Core.h"
#include "connection/Loader.h"

using namespace Gadgetron::Core;
using namespace Gadgetron::Server;

namespace
{

class ErrorThrower : public Connection::ErrorReporter
{
  public:
    void operator()(const std::string& location, const std::string& message) override {
        throw std::runtime_error(("[" + location + "] ERROR: " + message));
    }
};

std::filesystem::path find_config_path(const std::string& home_dir, const std::string& config_xml)
{
    auto config_path = std::filesystem::path(home_dir) / std::filesystem::path("share/gadgetron/config") /
                       std::filesystem::path(config_xml);

    if (!std::filesystem::is_regular_file(config_path)) {
        throw std::runtime_error("Failed to find gadgetron configuration at the expected path: " +
                                 config_path.string());
    }

    return config_path;
}

} // namespace

class StreamConsumer
{
public:
    StreamConsumer(const boost::program_options::variables_map& args, std::string storage_address)
        : args_(args), storage_address_(storage_address) {}
    ~StreamConsumer() {}

    void consume(std::istream& input_stream, std::ostream& output_stream, std::string config_xml_name)
    {
        Context::Paths paths{
            args_["home"].as<boost::filesystem::path>().string(),
            args_["dir"].as<boost::filesystem::path>().string()};

        mrd::binary::MrdReader mrd_reader(input_stream);
        mrd::binary::MrdWriter mrd_writer(output_stream);

        mrd::Header hdr = consume_mrd_header(mrd_reader, mrd_writer);
        auto storage_spaces = setup_storage_spaces(storage_address_, hdr);

        auto context = StreamContext(hdr, paths, args_, storage_address_, storage_spaces);
        auto loader = Connection::Loader(context);
        auto config_path = find_config_path(args_["home"].as<boost::filesystem::path>().string(), config_xml_name);

        GINFO_STREAM("Loading configuration from: " << config_path.string());
        std::ifstream file(config_path, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file at path: " + config_path.string());
        }

        auto config = Connection::parse_config(file);
        file.close();

        auto stream = loader.load(config.stream);
        auto input_channel = make_channel<MessageChannel>();
        auto output_channel = make_channel<MessageChannel>();
        std::atomic<bool> processing = true;

        auto process_future = std::async(std::launch::async, [&]() {
            try
            {
                ErrorThrower error_thrower;
                Connection::ErrorHandler error_handler(error_thrower, std::string(__FILE__));
                stream->process(std::move(input_channel.input), std::move(output_channel.output), error_handler);
                processing = false;
            }
            catch (const std::exception& exc)
            {
                {
                    // Induce a ChannelClosed exception upon readers of the channel.
                    auto destruct_me = std::move(output_channel.output);
                }
                processing = false;
                throw;
            }
        });

        auto input_future = std::async(std::launch::async, [&]() {
            try
            {
                consume_input_stream(mrd_reader, input_channel);
            }
            catch(std::ios_base::failure& exc)
            {
                // Induce a ChannelClosed exception upon readers of the channel.
                auto destruct_me = std::move(input_channel.output);
            }
        });

        auto output_future = std::async(std::launch::async, [&]()
        {
            process_output_stream(output_channel, mrd_writer);
        });

        // Clean up and propagate exceptions if they occurred
        input_future.get();
        output_future.get();
        process_future.get();
    }

  private:

    mrd::Header consume_mrd_header(mrd::binary::MrdReader& mrd_reader, mrd::binary::MrdWriter& mrd_writer)
    {
        std::optional<mrd::Header> hdr;

        mrd_reader.ReadHeader(hdr);
        mrd_writer.WriteHeader(hdr);

        if (!hdr.has_value()) {
            throw std::runtime_error("Failed to read ISMRMRD header");
        }
        return hdr.value();
    }

    void consume_input_stream(mrd::binary::MrdReader& mrd_reader, ChannelPair& input_channel)
    {
        mrd::StreamItem stream_item;
        while (mrd_reader.ReadData(stream_item)) {
            std::visit([&](auto&& arg) {
                Message msg(std::move(arg));
                input_channel.output.push_message(std::move(msg));
            }, stream_item);

            /** TODO Joe: If we want to "check" that we're sending valid data types, we might need
             *  to exhaust the variants of mrd::StreamItem... e.g.
             * 
            if (auto* v = std::get_if<mrd::Acquisition>(&stream_item)) {
                Message msg(std::move(*v));
                input_channel.output.push_message(std::move(msg));
            } else if (...) {}
             * 
             */
        }

        mrd_reader.Close();
        auto destruct_me = std::move(input_channel.output);
    }

    void process_output_stream(ChannelPair& output_channel, mrd::binary::MrdWriter& mrd_writer)
    {
        while (true) {
            try {
                auto message = output_channel.input.pop();

                if (convertible_to<Gadgetron::AcquisitionBucket>(message) ) {
                    GERROR_STREAM("AcquisitionBucket not yet supported by MrdWriter");
                } else if (convertible_to<Gadgetron::IsmrmrdImageArray>(message) ) {
                    GERROR_STREAM("ImageArray not yet supported by MrdWriter");
                } else if (convertible_to<mrd::ImageUint16>(message) ) {
                    mrd_writer.WriteData(force_unpack<mrd::ImageUint16>(std::move(message)));
                } else if (convertible_to<mrd::ImageInt16>(message) ) {
                    mrd_writer.WriteData(force_unpack<mrd::ImageInt16>(std::move(message)));
                } else if (convertible_to<mrd::ImageUint>(message) ) {
                    mrd_writer.WriteData(force_unpack<mrd::ImageUint>(std::move(message)));
                } else if (convertible_to<mrd::ImageInt>(message) ) {
                    mrd_writer.WriteData(force_unpack<mrd::ImageInt>(std::move(message)));
                } else if (convertible_to<mrd::ImageFloat>(message) ) {
                    mrd_writer.WriteData(force_unpack<mrd::ImageFloat>(std::move(message)));
                } else if (convertible_to<mrd::ImageDouble>(message) ) {
                    mrd_writer.WriteData(force_unpack<mrd::ImageDouble>(std::move(message)));
                } else if (convertible_to<mrd::ImageComplexFloat>(message) ) {
                    mrd_writer.WriteData(force_unpack<mrd::ImageComplexFloat>(std::move(message)));
                } else if (convertible_to<mrd::ImageComplexDouble>(message) ) {
                    mrd_writer.WriteData(force_unpack<mrd::ImageComplexDouble>(std::move(message)));
                } else {
                    GERROR_STREAM("Unsupported Message type for MrdWriter");
                }
            } catch (const ChannelClosed& exc) {
                break;
            }
        }

        mrd_writer.EndData();
        mrd_writer.Close();
    }

    boost::program_options::variables_map args_;
    std::string storage_address_;
};
