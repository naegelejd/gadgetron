#include "AugmentImageMetadataGadget.h"

#include "log.h"

namespace Gadgetron{

Gadgetron::Core::Image<std::complex<float>> Gadgetron::AugmentImageMetadataGadget::process_function(
    Gadgetron::Core::Image<std::complex<float>> input_image) const
{
    if (input_image.meta["ImageRowDir"].size() != 3) {
        input_image.meta["ImageRowDir"].push_back(input_image.head.col_dir[0]);
        input_image.meta["ImageRowDir"].push_back(input_image.head.col_dir[1]);
        input_image.meta["ImageRowDir"].push_back(input_image.head.col_dir[2]);
    }


    if (input_image.meta["ImageColumnDir"].size() != 3) {
        input_image.meta["ImageColumnDir"].push_back(input_image.head.line_dir[0]);
        input_image.meta["ImageColumnDir"].push_back(input_image.head.line_dir[1]);
        input_image.meta["ImageColumnDir"].push_back(input_image.head.line_dir[2]);
    }
    
    return input_image;
}

GADGETRON_GADGET_EXPORT(AugmentImageMetadataGadget)
}