#include <unlocker/unlocker.hpp>
#include <build_config.h>

#include <koalabox/config_parser.hpp>
#include <koalabox/file_logger.hpp>
#include <koalabox/hook.hpp>
#include <koalabox/loader.hpp>
#include <koalabox/patcher.hpp>
#include <koalabox/win_util.hpp>

#include <cpr/cpr.h>

namespace unlocker {

    struct EntitlementsContainer {
        [[maybe_unused]] uint8_t pad10[0x10];
        char* entitlements_json;
        uint32_t json_length;
    };

    Config config; // NOLINT(cert-err58-cpp)

    auto make_entitlement(const String& id) {
        return nlohmann::json{
            {"id",                id},
            {"entitlementName",   id},
            {"namespace",         config.name_space},
            {"catalogItemId",     id},
            {"entitlementType",   "AUDIENCE"},
            {"grantDate",         "2022-01-01T00:00:00.000Z"},
            {"consumable",        false},
            {"status",            "ACTIVE"},
            {"useCount",          0},
            {"entitlementSource", "eos"},
        };
    }

    void* parseEntitlements(void* RCX, void* RDX, EntitlementsContainer** R8, bool R9B) {
        static const auto parseEntitlements_o = hook::get_original_function(true, nullptr, __func__, parseEntitlements);
        logger->debug("{} -> R8: {}", __func__, fmt::ptr(R8));

        if (not R8 or not*R8 or not(*R8)->entitlements_json) {
            logger->warn("Incomplete R8 argument");
            return parseEntitlements_o(RCX, RDX, R8, R9B);
        }

        auto* container = *R8;

        const auto original_entitlements_json = container->entitlements_json;
        const auto original_json_length = container->json_length;

        const String json_string(original_entitlements_json, original_json_length);

        logger->debug("Original json:\n{}", json_string);

        auto json = nlohmann::json::parse(json_string, nullptr, true, true);

        auto json_contains_entitlement_id = [&](const String& id) {
            return std::ranges::any_of(json, [&](const auto& entitlement) {
                const auto& catalogItemId = entitlement["catalogItemId"];
                return util::strings_are_equal(catalogItemId, id);
            });
        };

        logger->debug("Original entitlement IDs:", json_string);
        for (const auto& entitlement: json) {
            logger->debug("  '{}'", entitlement["catalogItemId"]);
        }

        if (config.auto_fetch_entitlements) {
            nlohmann::json payload = {
                {"query",     R"(query($namespace: String!) {
                        Catalog {
                            catalogOffers(
                                namespace: $namespace
                                params: {
                                    count: 1000,
                                }
                            ) {
                                elements {
                                    items {
                                        id,
                                        title
                                    }
                                }
                            }
                        }
                    })"
                },
                {"variables", {{"namespace", config.name_space}}}
            };

            const auto res = cpr::Post(
                cpr::Url{"https://graphql.epicgames.com/graphql"},
                cpr::Header{{"content-type", "application/json"}},
                cpr::Body{payload.dump()}
            );

            if (res.status_code == cpr::status::HTTP_OK) {
                const auto res_json = nlohmann::json::parse(res.text);

                // logger->debug("Web API entitlements response json:\n{}", res_json.dump(2));

                const auto& elements = res_json["data"]["Catalog"]["catalogOffers"]["elements"];

                for (const auto& element: elements) {
                    for (const auto& items: element) {
                        for (const auto& item: items) {
                            const auto& id = item["id"];
                            const auto& name = item["title"];

                            // Skip entitlement injection if it is blacklisted or already present
                            if (config.blacklist.contains(id) or json_contains_entitlement_id(id)) {
                                continue;
                            }

                            logger->debug("Adding auto-fetched entitlement: '{}' — '{}'", id, name);
                            json.insert(json.end(), make_entitlement(id));
                        }
                    }
                }
            } else {
                logger->error(
                    "Failed to automatically fetch entitlement ids. "
                    "Status code: {}. Text: {}", res.status_code, res.text
                );
            }
        }

        for (const auto& id: config.entitlements) {
            json.insert(json.end(), make_entitlement(id));
        }

        String entitlements_json = json.dump(2);

        // Replace original entitlements with injected
        container->entitlements_json = entitlements_json.data();
        container->json_length = entitlements_json.size();

        logger->info("Injected entitlements:\n{}", container->entitlements_json);

        const auto result = parseEntitlements_o(RCX, RDX, R8, R9B);

        // Restore original entitlements to avoid memory relocation crashes
        container->entitlements_json = original_entitlements_json;
        container->json_length = original_json_length;

        return result;
    }

    void init(const HMODULE& self_module) {
        try {
            DisableThreadLibraryCalls(self_module);

            koalabox::project_name = PROJECT_NAME;

            const auto self_directory = loader::get_module_dir(self_module);

            config = config_parser::parse<Config>(self_directory / PROJECT_NAME".jsonc", true);

            if (config.logging) {
                logger = file_logger::create(self_directory / fmt::format("{}.log", PROJECT_NAME));
            }

            logger->info("🐨 {} v{}", PROJECT_NAME, PROJECT_VERSION);

            const auto process_handle = win_util::get_module_handle(nullptr);
            const auto process_info = win_util::get_module_info(process_handle);
            const auto address = (uint64_t) patcher::find_pattern_address(
                process_info, "parseEntitlements", config.entitlements_pattern
            );

            hook::init(true);
            hook::detour_or_throw(address, "parseEntitlements", (FunctionAddress) parseEntitlements);

            logger->info("🚀 Initialization complete");
        } catch (const Exception& ex) {
            util::panic(fmt::format("Initialization error: {}", ex.what()));
        }
    }

    void shutdown() {
        logger->info("💀 Shutdown complete");
    }

}
