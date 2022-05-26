#pragma once

#include <koalabox/koalabox.hpp>

#include <nlohmann/json.hpp>

namespace unlocker {
    using namespace koalabox;

    struct Config {
        uint32_t $version = 2;
        bool logging = false;
        String entitlements_pattern = "4C 89 44 24 18 48 89 54 24 10 48 89 4C 24 08 55"
                                      "53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 F8"
                                      "48 81 EC ?? ?? ?? ?? 48 8B 5A 08 45 0F B6 F1";
        String name_space = "catnip";
        bool auto_fetch_entitlements = true;
        Set<String> blacklist;
        Vector<String> entitlements;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
            Config, $version,
            logging,
            entitlements_pattern,
            name_space,
            auto_fetch_entitlements,
            blacklist,
            entitlements
        )
    };


    void init(const HMODULE& self_module);

    void shutdown();

}
