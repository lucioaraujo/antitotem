cmake_minimum_required(VERSION 3.22)

# Must be set before project() to take effect (CMake/Apple requirement).
# Same bug found and fixed on Navalha 2's macOS build the same day (26 ago.
# 2026): the .dmg refused to open on the author's own Mac ("Impossible
# d'utiliser cette version de l'application... avec cette version de
# macOS") - confirmed via the shipped binary's LC_BUILD_VERSION: minos
# 26.0 (the GitHub macos-latest runner's own OS, since no deployment
# target was ever set) and a single arm64 slice (the runner's own native
# arch, since no architecture list was set either). 10.13 (High Sierra,
# 2017) is a broadly-supported JUCE deployment target that doesn't
# require dropping any API this project actually uses; x86_64;arm64
# produces one Universal 2 binary instead of choosing between two
# downloads.
if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version" FORCE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build a Universal 2 binary" FORCE)
endif()

project(AntitotemObjetoSonoro VERSION 0.1.0 LANGUAGES C CXX)

option(ANTITOTEM_BUILD_APP "Build the JUCE standalone instrument" ON)
option(ANTITOTEM_BUILD_TESTS "Build deterministic core tests" ON)
option(ANTITOTEM_BUILD_TOOLS "Build offline dev tools (CPU benchmark, listening-validation renders)" OFF)
set(ANTITOTEM_JUCE_PATH "" CACHE PATH "Path to a JUCE source checkout")
# 25 ago. 2026 - was an absolute path on the author's own machine, breaking
# clean-checkout builds elsewhere. Now lives in the repo at
# assets/identity/ (shared by the app icon, the .deb icon, and the
# website - not website-only, so it doesn't live under website/). Still
# overridable via -DANTITOTEM_LOGO_PATH=... for anyone using a different
# asset. Simplified/lower-node-count redraw, 25 ago. 2026 (author).
set(ANTITOTEM_LOGO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/identity/logo_antitotem_novo.svg"
    CACHE FILEPATH "Path to the authorial Antitotem SVG logo")

add_library(antitotem_simple_sequencer_core STATIC
    src/core/SimpleSequencer.cpp
    src/core/DualObjectEngine.cpp
    src/core/MelodicInterpreter.cpp)
target_include_directories(antitotem_simple_sequencer_core PUBLIC src)
target_compile_features(antitotem_simple_sequencer_core PUBLIC cxx_std_20)
if (MSVC)
    target_compile_options(antitotem_simple_sequencer_core PRIVATE /W4 /utf-8)
else()
    target_compile_options(antitotem_simple_sequencer_core PRIVATE
        -Wall -Wextra -Wpedantic -Werror -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

if (ANTITOTEM_BUILD_TESTS)
    enable_testing()
    add_executable(antitotem_simple_sequencer_tests tests/SimpleSequencerTests.cpp)
    target_link_libraries(antitotem_simple_sequencer_tests PRIVATE antitotem_simple_sequencer_core)
    add_test(NAME antitotem_simple_sequencer_tests COMMAND antitotem_simple_sequencer_tests)
endif()

if (ANTITOTEM_BUILD_TOOLS)
    add_executable(antitotem_cpu_benchmark tools/CpuBenchmark.cpp)
    target_link_libraries(antitotem_cpu_benchmark PRIVATE antitotem_simple_sequencer_core)
    if (NOT MSVC)
        target_compile_options(antitotem_cpu_benchmark PRIVATE -O3)
    endif()

    # Headless listening-validation render (no JUCE/GUI/audio device needed) -
    # see tools/RenderPanDemo.cpp and docs/TAREFAS.md, 17 ago. 2026.
    add_executable(antitotem_render_pan_demo tools/RenderPanDemo.cpp)
    target_link_libraries(antitotem_render_pan_demo PRIVATE antitotem_simple_sequencer_core)

    # Listening-validation render for the new prototype-stage modules
    # (MaterialFilter, ChaosField, WanderSource) - header-only, no link
    # against antitotem_simple_sequencer_core needed.
    add_executable(antitotem_render_new_modules_demo tools/RenderNewModulesDemo.cpp)
    target_include_directories(antitotem_render_new_modules_demo PRIVATE src)
    target_compile_features(antitotem_render_new_modules_demo PRIVATE cxx_std_20)
endif()

if (ANTITOTEM_BUILD_APP)
    if (ANTITOTEM_JUCE_PATH AND EXISTS "${ANTITOTEM_JUCE_PATH}/CMakeLists.txt")
        add_subdirectory("${ANTITOTEM_JUCE_PATH}" "${CMAKE_BINARY_DIR}/JUCE" EXCLUDE_FROM_ALL)
        add_subdirectory(src/app)
    else()
        message(WARNING "Set ANTITOTEM_JUCE_PATH to build the JUCE standalone app")
    endif()
endif()

# Packaging via CPack (25 ago. 2026 Linux/.deb; 25 ago. 2026 (same day,
# later) generalized to Windows/NSIS and macOS/DragNDrop - see
# docs/TAREFAS.md for what's actually been built/tested vs. only
# configured here). Only meaningful when the app itself was actually
# configured above - a tests-only or tools-only build has nothing to
# package. Neither Windows nor macOS can be built or tested from this
# development machine (Linux, no MSVC/Xcode) - this block only prepares
# the CMake side; GitHub Actions (.github/workflows/antitotem-package.yml)
# is what actually runs it on real Windows/macOS runners.
if (ANTITOTEM_BUILD_APP AND ANTITOTEM_JUCE_PATH)
    set(CPACK_PACKAGE_NAME "antitotem")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_VENDOR "RASGO")
    set(CPACK_PACKAGE_CONTACT "Lúcio Araújo <rasgo.instruments@gmail.com>")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
        "Antitotem - Objeto Sonoro: standalone modular sound object instrument")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
        set(CPACK_GENERATOR "DEB")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Lúcio Araújo <rasgo.instruments@gmail.com>")
        set(CPACK_DEBIAN_PACKAGE_SECTION "sound")
        set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/lucioaraujo/antitotem/tree/main")
        set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
        # libx11-6/libxext6/libxss1/libgio-2.0-0 are dlopen()'d by JUCE at
        # runtime (confirmed via `strings` on the built binary, 25 ago.
        # 2026 - they don't show up as ELF NEEDED entries, so
        # CPACK_DEBIAN_PACKAGE_SHLIBDEPS above won't catch them on its
        # own) - without them installed the window never opens. Declared
        # explicitly alongside the shlibdeps-detected ones.
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libx11-6, libxext6, libxss1, libgio-2.0-0")
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        # NSIS chosen over WiX/MSI: ships preinstalled on GitHub's
        # windows-latest runner, no extra toolchain to install in CI.
        set(CPACK_GENERATOR "NSIS")
        set(CPACK_NSIS_PACKAGE_NAME "Antitotem - Objeto Sonoro")
        set(CPACK_NSIS_DISPLAY_NAME "Antitotem - Objeto Sonoro")
        set(CPACK_NSIS_CONTACT "rasgo.instruments@gmail.com")
        set(CPACK_NSIS_MODIFY_PATH OFF)
        set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
        set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/lucioaraujo/antitotem/tree/main")
        # Same bug found and fixed on Navalha 2's installer the same day
        # (26 ago. 2026): NSIS's CPack template only registers the uninstall
        # entry in "Programs and Features" by default - without
        # CPACK_PACKAGE_EXECUTABLES it has no notion of which installed file
        # is a launchable application, so it creates zero shortcuts. This is
        # the actual fix for a Start Menu entry existing at all; the exe's
        # own embedded icon (ICON_BIG/ICON_SMALL on juce_add_gui_app, see
        # src/app/CMakeLists.txt) carries over to the shortcut automatically.
        set(CPACK_PACKAGE_EXECUTABLES "Antitotem - Objeto Sonoro" "Antitotem - Objeto Sonoro")
        set(CPACK_CREATE_DESKTOP_LINKS "Antitotem - Objeto Sonoro")
        set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/antitotem.ico")
        set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/antitotem.ico")
        set(CPACK_NSIS_INSTALLED_ICON_NAME "Antitotem - Objeto Sonoro.exe")
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        # DragNDrop (.dmg) needs no extra tooling beyond macOS's own
        # hdiutil, unlike the Bundle generator's productbuild dependency.
        set(CPACK_GENERATOR "DragNDrop")
        set(CPACK_DMG_VOLUME_NAME "Antitotem - Objeto Sonoro")
    endif()

    include(CPack)
endif()
