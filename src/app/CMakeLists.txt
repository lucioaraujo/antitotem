# ICON_BIG/ICON_SMALL: JUCE's own juceaide tool embeds a real icon resource
# (.ico on Windows, .icns on macOS) in the executable itself - without this
# the exe carries no icon at all, so Explorer/Finder, the taskbar and any
# shortcut pointing at it (including the Start Menu shortcut CPack creates
# below) fall back to a generic icon. Same fix as Navalha 2's, same day
# (26 ago. 2026) - see docs/TAREFAS.md.
juce_add_gui_app(AntitotemSimpleSequencerApp
    PRODUCT_NAME "Antitotem - Objeto Sonoro"
    COMPANY_NAME "RASGO"
    BUNDLE_ID "org.rasgo.antitotem.simplesequencer"
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/../../assets/identity/icons/antitotem-icon-256.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/../../assets/identity/icons/antitotem-icon-32.png")
target_sources(AntitotemSimpleSequencerApp PRIVATE Main.cpp)

# Typography is an application asset, not a dependency on whichever font a
# particular desktop happens to provide. The same OFL family is already used
# by Rasgo Synth; embedding it here preserves Portuguese glyphs and makes the
# interface reproducible on the studio machine and in future packages.
set(ANTITOTEM_BINARY_ASSETS
    "${CMAKE_CURRENT_SOURCE_DIR}/../../assets/fonts/IBMPlexMono-Regular.ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../assets/fonts/IBMPlexMono-Bold.ttf")
if (EXISTS "${ANTITOTEM_LOGO_PATH}")
    list(APPEND ANTITOTEM_BINARY_ASSETS "${ANTITOTEM_LOGO_PATH}")
    target_compile_definitions(AntitotemSimpleSequencerApp PRIVATE ANTITOTEM_HAS_LOGO=1)
else()
    target_compile_definitions(AntitotemSimpleSequencerApp PRIVATE ANTITOTEM_HAS_LOGO=0)
endif()
juce_add_binary_data(AntitotemBinaryData SOURCES ${ANTITOTEM_BINARY_ASSETS})
target_link_libraries(AntitotemSimpleSequencerApp PRIVATE AntitotemBinaryData)
target_compile_features(AntitotemSimpleSequencerApp PRIVATE cxx_std_20)
if (MSVC)
    target_compile_options(AntitotemSimpleSequencerApp PRIVATE /utf-8)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(AntitotemSimpleSequencerApp PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
target_link_libraries(AntitotemSimpleSequencerApp PRIVATE
    antitotem_simple_sequencer_core
    juce::juce_audio_utils
    juce::juce_audio_formats
    juce::juce_data_structures
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
target_compile_definitions(AntitotemSimpleSequencerApp PRIVATE
    JUCE_APPLICATION_NAME_STRING="Antitotem - Objeto Sonoro"
    JUCE_APPLICATION_VERSION_STRING="${PROJECT_VERSION}"
    JUCE_USE_XRANDR=0
    JUCE_USE_XINERAMA=0
    JUCE_USE_XCURSOR=0
    JUCE_USE_XINPUT=0)

# Packaging (.deb via CPack, 25 ago. 2026 - see docs/TAREFAS.md gate de
# publicação). Binary installs without the display PRODUCT_NAME's spaces -
# `antitotem` is the conventional Debian binary name; the window title
# keeps saying "Antitotem - Objeto Sonoro" regardless, that's a separate
# string (JUCE_APPLICATION_NAME_STRING above).
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    install(PROGRAMS "$<TARGET_FILE:AntitotemSimpleSequencerApp>"
        DESTINATION bin RENAME antitotem)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../../packaging/linux/antitotem.desktop"
        DESTINATION share/applications)
    # Icon is optional on purpose - ANTITOTEM_LOGO_PATH is a local path on
    # the author's machine today (same limitation ANTITOTEM_HAS_LOGO above
    # already works around for the in-app logo), not yet a portable asset
    # committed to the repo. Packaging without it is a valid .deb, just
    # without a custom icon in the launcher; installing one would need the
    # SVG to exist first. XDG scalable theme accepts SVG directly, no
    # raster conversion needed once that asset is portable.
    if (EXISTS "${ANTITOTEM_LOGO_PATH}")
        install(FILES "${ANTITOTEM_LOGO_PATH}"
            DESTINATION share/icons/hicolor/scalable/apps
            RENAME antitotem.svg)
    endif()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Spaces in the exe name are normal on Windows (unlike Linux's
    # convention) - no RENAME needed, keep the JUCE PRODUCT_NAME as-is.
    install(TARGETS AntitotemSimpleSequencerApp RUNTIME DESTINATION .)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # juce_add_gui_app already produces a real .app bundle on macOS
    # (BUNDLE_ID set above) - install it as one, not as a bare binary.
    install(TARGETS AntitotemSimpleSequencerApp BUNDLE DESTINATION .)
endif()
