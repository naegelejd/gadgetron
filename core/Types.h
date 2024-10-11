#pragma once

#include <mrd/types.h>

namespace Gadgetron::Core {
    using AnyImage =
            std::variant<
                    mrd::Image<short>,
                    mrd::Image<unsigned short>,
                    mrd::Image<float>,
                    mrd::Image<double>,
                    mrd::Image<std::complex<float>>,
                    mrd::Image<std::complex<double>>
            >;
}