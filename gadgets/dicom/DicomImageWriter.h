#ifndef DICOMIMAGEWRITER_H
#define DICOMIMAGEWRITER_H

#include <dcmtk/dcmdata/dctk.h>
#include "ismrmrd/ismrmrd.h"
#include "Writer.h"

namespace Gadgetron {

    class DicomImageWriter
        : public Core::TypedWriter<DcmFileFormat, std::optional<std::string>, std::optional<ISMRMRD::MetaContainer>> {
    protected:
        void serialize(std::ostream& stream, const DcmFileFormat&,
            const std::optional<std::string>&,
            const std::optional<ISMRMRD::MetaContainer>& args) override;

    protected:
    public:
    };

} /* namespace Gadgetron */

#endif
