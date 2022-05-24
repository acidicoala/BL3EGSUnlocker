# 🐨 BL3 EGS Unlocker

**Legit DLC Unlocker for Epic Game Store version of Borderlands 3**

It works by hooking a function that parses json string of entitlements.
The unlocker automatically fetches DLC IDs and adds them to the original list.
Users can use blacklist option in config to disable automatically fetched DLCs.
Users can also inject additional DLC IDs in the config file.
Config file is entirely optional. All fields within the config file are optional too.

## Installation

* Setup the Koaloader
  1. Download the [latest release](https://github.com/acidicoala/Koaloader/releases/latest) of Koaloader archive
  2. From this archive, unzip the `winmm-64/winmm.dll` file into the `Borderlands3\OakGame\Binaries\Win64` directory
* Setup the Unlocker
  1. Download the [latest release](https://github.com/acidicoala/BL3EGSUnlocker/releases/latest) archive of this unlocker
  2. From this archive, unzip the `Unlocker.dll` file into the `Borderlands3\OakGame\Binaries\Win64` directory

## 👋 Acknowledgements

This unlocker makes use of the following open source projects:

- [JSON for Modern C++](https://github.com/nlohmann/json)
- [PolyHook 2](https://github.com/stevemk14ebr/PolyHook_2_0)
- [spdlog](https://github.com/gabime/spdlog)

## 📄 License

This software is licensed under [BSD Zero Clause  License], terms of which are available in [LICENSE.txt]

[BSD Zero Clause  License]: https://choosealicense.com/licenses/0bsd/

[LICENSE.txt]: LICENSE.txt
