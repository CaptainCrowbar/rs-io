#pragma once

#include <string>
#include <array>

namespace RS::Channel {

    inline std::array<int, 3> version() noexcept {
        return {{ 0, 1, 5 }};
    }

    inline std::string version_string() {
        return "0.1.5";
    }

}
