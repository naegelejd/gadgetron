#pragma once

#include "GriddingReconGadgetBase.h"
#include "cuNDArray.h"
#include "cuNDArray_math.h"

namespace Gadgetron {
    class GriddingReconGadget : public GriddingReconGadgetBase<cuNDArray> {
    public:
        GriddingReconGadget();
        ~GriddingReconGadget();
    };


}


