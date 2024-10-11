
#include "mri_core_stream.h"

namespace Gadgetron
{
    GenericReconMrdStreamer::GenericReconMrdStreamer() : verbose_(false)
    {
    }

    GenericReconMrdStreamer::GenericReconMrdStreamer(const std::map<std::string, std::string>& parameters) : verbose_(false)
    {
        this->initialize_stream_name_buffer(parameters);
    }

    GenericReconMrdStreamer::~GenericReconMrdStreamer()
    {
    }

    void GenericReconMrdStreamer::initialize_stream_name_buffer(const std::map<std::string, std::string>& parameters)
    {
        this->initialize_stream_name_buffer(parameters, GENERIC_RECON_STREAM_MRD_HEADER);
        this->initialize_stream_name_buffer(parameters, GENERIC_RECON_STREAM_UNDERSAMPLED_KSPACE);
        this->initialize_stream_name_buffer(parameters, GENERIC_RECON_STREAM_REF_KSPACE);
        this->initialize_stream_name_buffer(parameters, GENERIC_RECON_STREAM_REF_KSPACE_FOR_COILMAP);
        this->initialize_stream_name_buffer(parameters, GENERIC_RECON_STREAM_COILMAP);
        this->initialize_stream_name_buffer(parameters, GENERIC_RECON_STREAM_GFACTOR_MAP);
        this->initialize_stream_name_buffer(parameters, GENERIC_RECON_STREAM_RECONED_KSPACE);
        this->initialize_stream_name_buffer(parameters, GENERIC_RECON_STREAM_RECONED_COMPLEX_IMAGE);
        this->initialize_stream_name_buffer(parameters, GENERIC_RECON_STREAM_RECONED_COMPLEX_IMAGE_AFTER_POSTPROCESSING);
    }

    void GenericReconMrdStreamer::initialize_stream_name_buffer(const std::map<std::string, std::string>& parameters, const std::string& name)
    {
        if (parameters.find(name) != parameters.end())
        {
            buffer_names_[name].first = parameters.at(name);
            GDEBUG_CONDITION_STREAM(this->verbose_, "Buffer to store " << name << " is " << this->buffer_names_[name].first);
        }
    }

    void GenericReconMrdStreamer::close_stream_buffer()
    {
        for (auto const& x : this->buffer_names_)
        {
            GDEBUG_CONDITION_STREAM(this->verbose_, "GenericReconMrdStreamer::close_stream_buffer, stream is for " << x.first << " - " << this->buffer_names_[x.first].first);

            if(this->buffer_names_[x.first].second)
            {
                auto& writer = *this->buffer_names_[x.first].second;
                GDEBUG_CONDITION_STREAM(this->verbose_, "GenericReconMrdStreamer::close_stream_buffer, stream is open for " << x.first << "; flushing it ... ");
                writer.EndData();
                writer.Flush();
            }
        }
    }

    void GenericReconMrdStreamer::stream_mrd_header(const mrd::Header& hdr)
    {
        if (this->buffer_names_.find(GENERIC_RECON_STREAM_MRD_HEADER) != this->buffer_names_.end())
        {
            std::string buf_name = this->buffer_names_[GENERIC_RECON_STREAM_MRD_HEADER].first;
            if (!this->buffer_names_[GENERIC_RECON_STREAM_MRD_HEADER].second)
            {
                this->buffer_names_[GENERIC_RECON_STREAM_MRD_HEADER].second = std::make_shared<mrd::binary::MrdWriter>(buf_name);
            }

            auto& writer = *this->buffer_names_[GENERIC_RECON_STREAM_MRD_HEADER].second;
            GDEBUG_STREAM("GenericReconMrdStreamer, stream the mrd header to the array buffer " << buf_name);
            writer.WriteHeader(hdr);
        }
        else
        {
            GWARN_CONDITION_STREAM(this->verbose_, "GenericReconMrdStreamer, the pre-set buffer names do not include " << GENERIC_RECON_STREAM_MRD_HEADER << "; the header will not be saved into the buffer ...");
        }
    }
}