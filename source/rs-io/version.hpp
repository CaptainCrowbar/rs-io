// This file is generated by the rs-bump-version script

#pragma once

#include <string>
#include <array>

namespace RS::IO {

    inline std::array<int, 3> version() noexcept {
        return {{ 0, 2, 6 }};
    }

    inline std::string version_string() {
        return "0.2.6";
    }

}
