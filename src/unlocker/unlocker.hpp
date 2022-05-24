#pragma once

#include <koalabox/koalabox.hpp>

#include <nlohmann/json.hpp>

namespace unlocker {
    using namespace koalabox;

    struct Config {
        uint32_t $version = 1;
        bool logging = false;
        String entitlements_pattern;
        String name_space;
        Vector<String> entitlements;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
            Config, $version,
            logging,
            entitlements_pattern,
            name_space,
            entitlements
        )
    };


    void init(const HMODULE& self_module);

    void shutdown();

}
