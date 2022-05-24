#include <unlocker/unlocker.hpp>
#include <build_config.h>

#include <koalabox/config_parser.hpp>
#include <koalabox/file_logger.hpp>
#include <koalabox/hook.hpp>
#include <koalabox/loader.hpp>
#include <koalabox/patcher.hpp>
#include <koalabox/win_util.hpp>

namespace unlocker {

    struct EntitlementsContainer {
        [[maybe_unused]] uint8_t pad10[0x10];
        char* entitlements_json;
        uint32_t json_length;
    };

    Config config;

    void* parseEntitlements(void* RCX, void* RDX, EntitlementsContainer** R8, bool R9B) {
        auto* container = *R8;

        const auto original_entitlements_json = container->entitlements_json;
        const auto original_json_length = container->json_length;

        const String json_string(original_entitlements_json, original_json_length);

        logger->debug("Original json:\n{}", json_string);

        auto json = nlohmann::json::parse(json_string, nullptr, true, true);

        logger->debug("Original entitlement IDs:", json_string);
        for (const auto& entitlement: json) {
            logger->debug("  '{}'", entitlement["catalogItemId"]);
        }

        for (const auto& id: config.entitlements) {
            nlohmann::json entitlement = {
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

            json.insert(json.end(), entitlement);
        }

        String entitlements_json = json.dump(2);

        // Replace original entitlements with injected
        container->entitlements_json = entitlements_json.data();
        container->json_length = entitlements_json.size();

        logger->debug("Injected entitlements:\n{}", container->entitlements_json);

        static const auto parseEntitlements_o = hook::get_original_function(true, nullptr, __func__, parseEntitlements);
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
