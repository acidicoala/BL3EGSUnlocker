#include <unlocker/unlocker.hpp>

EXTERN_C [[maybe_unused]] BOOL WINAPI DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        unlocker::init(module);
    } else if (reason == DLL_PROCESS_DETACH) {
        unlocker::shutdown();
    }

    return TRUE;
}
