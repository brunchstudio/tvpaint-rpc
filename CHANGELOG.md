# Changelog

## [1.2.0] - 2026-07-10

## Migration to TVPaint 12.1

* Updated the code to build against the TVPaint 12.1 SDK.
* Replaced all deprecated SDK symbols with their current equivalents.
* Fixed the plugin showing up as an empty panel by hiding the requester window explicitly with `TVDisplayReq` (the `bPIRequesterFlags_Hidden` flag no longer hides it under TVPaint 12's new GUI).

**How to install :**
Just unzip the file in your tvpaint installation folder `$TVP_INSTALL_FOLDER/Resources/plugins/`.

---

## [1.2.0b] - 2026-05-13

## This is a beta release for the new TVPaint 12.1.0 version and new SDK. Though the plugin is working properley, this has not been thoroughly tested and we do not currently recommend using this in production for now.

## Migration to TVPaint SDK 12

Updated the plugin from the legacy TVPaint SDK to the new TVPaint Animation SDK 12 for TVPaint 12.1 and up. The new SDK ships as a proper CMake package with compiled libraries, instead of the old headers-plus-loose `dllx.c` layout.

### `CMakeLists.txt`

- **Replaced the manual SDK wiring with `find_package(TVPaintAnimationSDK REQUIRED)`.**
  The old SDK required the caller to pass `TVPAINT_SDK_FOLDER`, manually add  its `include/` directory, and compile `lib/ dllx.c` as part of the plugin sources. 
The new SDK ships `TVPaintAnimationSDKConfig.cmake` under `TVPaint_SDK_12/lib/cmake/`, so `find_package` + linking the `TVPaintAnimationSDK::TVPaintAnimationSDK` target replaces all of that. The  `dllx.c` file no longer exists in the SDK; its code is compiled into the `TVPaintAnimationSDK.lib` static library that the imported target links in  automatically.

- **Removed the `-DWIN32` / `-DWIN64` `add_definitions` block.** 
  The old SDK's  `plugdllx.h` branched on those macros to pick integer widths; the new SDK  uses the standard `_WIN64` predefined macro instead, so we no longer need to set them ourselves.

- **Added `create_plugin_bundle(${PROJECT_NAME})`.** 
  This is a helper function  shipped with the new SDK (via `TVPaintAnimationSDKUtilities.cmake`, included  automatically by `find_package`). It post-processes the built DLL into the `tvpaint-rpc-Windows.plugin/Contents/Windows/` directory layout that TVPaint 12 expects when loading plugins.

### `src/main.cpp`, `src/server.cpp`, `src/server.hpp`

- **Header include changed from `plugdllx.h` / `plugx.h` to `TVPaintAnimationSDK/TVPaintSDK.h`.** The new SDK splits its API across many `pi-*.h` headers; `TVPaintSDK.h` is the umbrella header that pulls them  all in, matching what the two old headers covered.

- **`PIFilter *` → `PIPlugin *`.** The plugin handle type was renamed.
  `PIFilter` still exists as a deprecated typedef, but using `PIPlugin`  directly avoids `[[deprecated]]` warnings and matches the signatures of  every SDK function we call.

- **`FAR PASCAL` → `STDCALL` on all `PI_*` entry points.** 
 The new SDK defines its own `STDCALL` macro (`__stdcall` on Win64, empty elsewhere) and declares all
  `PI_*` callbacks with it, so we match that convention.

- **`DWORD req` → `INTPTR req`.** 
  `TVOpenFilterReqEx` now returns `INTPTR` (unsigned 64-bit on Win64) instead of `DWORD`. Storing its return value in
  a narrower type would truncate.

- **Replaced every deprecated enumerator in this file with its `ePI*` / `k*` / `bPI*` counterpart.** 
   All of the old names still exist as deprecated aliases, but they now live in separate `*_DEPRECATED` enum
  types and trigger `[[deprecated]]` warnings. In the `TVGrabTicks` case  MSVC actually refuses the implicit conversion between enum types, so the  call only compiles with the new name. The mapping used:
    - `PITICKS_FLAG_ON` → `kPITicks_On` (third argument to `TVGrabTicks`)
    - `PIRF_HIDDEN_REQ` → `bPIRequesterFlags_Hidden` (flags to `TVOpenFilterReqEx`)
    - `FILTERREQ_NO_TBAR` → `bPIFilterFlags_NoTopBar` (menu flags to `TVOpenFilterReqEx`)
    - `PICBREQ_TICKS` → `kPIEvents_Ticks` (event in `PI_Msg` switch)
    - `PICBREQ_CLOSE` → `kPIEvents_WindowClose` (event in `PI_Msg` switch)

- **`PI_Parameters` signature: `char *iArg` → `const char *iArg`.** 
  The SDK  tightened the parameter type; matching it keeps the symbol from being  rejected as a mismatched declaration.

---

## [1.1.0] - 2025-05-13
## Migration to TVPaint 12.0

* Updated the code to run with TVPaint 12.0.X versions 

---

## [1.0.0] - 2024-03-11
### Fixed
* FIX save_dependencies functions #14