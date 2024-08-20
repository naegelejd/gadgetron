#pragma once


#include "TypeTraits.h"
#include <mrd/types.h>
#include <tuple>
#include <optional>
#include <variant>

namespace Gadgetron::Core {

    /** TODO Joe: Going forward there's no need to alias these std types */
    template<class T>
    using optional = std::optional<T>;
    constexpr auto none = std::nullopt;

    template<class... TYPES>
    using variant = std::variant<TYPES...>;
    using std::visit;
    using std::holds_alternative;
    using std::get;

    template<class... ARGS>
    using tuple = std::tuple<ARGS...>;

    using Acquisition = mrd::Acquisition;

    /** TODO Joe - No need to restrict Waveform to uint32_t going forward */
    using Waveform = mrd::Waveform<uint32_t>;

    template<class T>
    using Image = mrd::Image<T>;

    using AnyImage =
            variant<
                    Image<short>,
                    Image<unsigned short>,
                    Image<float>,
                    Image<double>,
                    Image<std::complex<float>>,
                    Image<std::complex<double>>
            >;
}


#include "Types.hpp"