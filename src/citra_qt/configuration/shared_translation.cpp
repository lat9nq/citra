// Copyright 2023 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "citra_qt/configuration/shared_translation.h"

#include <map>
#include <memory>
#include <tuple>
#include <utility>
#include <QWidget>
#include "citra_qt/uisettings.h"
#include "common/settings.h"
#include "common/settings_enums.h"
#include "common/settings_setting.h"

namespace ConfigurationShared {

std::unique_ptr<TranslationMap> InitializeTranslations(QWidget* parent) {
    std::unique_ptr<TranslationMap> translations = std::make_unique<TranslationMap>();
    [[maybe_unused]] const auto& tr = [parent](const char* text) -> QString {
        return parent->tr(text);
    };

#define INSERT(SETTINGS, ID, NAME, TOOLTIP)                                                        \
    translations->insert(std::pair{SETTINGS::values.ID.Id(), std::pair{tr((NAME)), tr((TOOLTIP))}})
#define BLANK(SETTINGS, ID)                                                                        \
    translations->insert(                                                                          \
        std::pair{SETTINGS::values.ID.Id(), std::pair{QStringLiteral(""), QStringLiteral("")}});

    INSERT(Settings, graphics_api, "Graphics API", "");
    INSERT(Settings, use_hw_shader, "Enable Hardware Shader",
           "Use OpenGL to accelerate shader emulation.\nRequires a relatively powerful GPU for "
           "better performance.");
    INSERT(Settings, shaders_accurate_mul, "Accurate Multiplication",
           "Correctly handle all edge cases in multiplication operation in shaders.\nSome games "
           "requires this to be enabled for the hardware shader to render properly.\nHowever this "
           "would reduce performance in most games.");
    INSERT(Settings, use_shader_jit, "Enable Shader JIT",
           "Use the JIT engine instead of the interpreter for software shader emulation.\nEnable "
           "this for better performance.");
    INSERT(Settings, use_disk_shader_cache, "Use Disk Shader Cache",
           "Reduce stuttering by storing and loading generated shaders to disk.");
    INSERT(Settings, use_vsync_new, "Enable VSync",
           "VSync prevents the screen from tearing, but some graphics cards have lower performance "
           "with VSync enabled. Keep it enabled if you don't notice a performance difference.");
    INSERT(Settings, async_presentation, "Enable Async Presentation",
           "Perform presentation on separate threads. Improves performance when using Vulkan in "
           "most games.");
    INSERT(Settings, async_shader_compilation, "Enable Async Shader Compilation",
           "Compile shaders using background threads to avoid shader compilation stutter. Expect "
           "temporary graphical glitches");
    INSERT(Settings, physical_device, "Physical Device", "");
    INSERT(Settings, spirv_shader_gen, "SPIR-V Shader Generation", "");

    BLANK(Settings, anaglyph_shader_name);
    INSERT(Settings, resolution_factor, "Internal Resolution", "");
    INSERT(Settings, filter_mode, "Enable Linear Filtering", "");
    INSERT(Settings, texture_filter, "Texture Filter", "");
    INSERT(Settings, pp_shader_name, "Post-Processing Shader", "");
    INSERT(Settings, render_3d, "Stereoscopic 3D Mode", "");
    INSERT(Settings, factor_3d, "Depth", "");
    INSERT(Settings, mono_render_option, "Eye to Render in Monoscopic Mode", "");
    INSERT(Settings, layout_option, "Screen Layout", "");
    INSERT(Settings, swap_screen, "Swap Screens", "");
    INSERT(Settings, upright_screen, "Rotate Screens Upright", "");
    INSERT(Settings, large_screen_proportion, "Large Screen Proportion", "");
    INSERT(Settings, custom_textures, "Use Custom Textures", "");
    INSERT(Settings, dump_textures, "Dump Textures", "");
    INSERT(Settings, preload_textures, "Preload Custom Texutres", "");
    INSERT(Settings, async_custom_loading, "Async Custom Texture Loading", "");
    BLANK(Settings, bg_red);
    BLANK(Settings, bg_green);
    BLANK(Settings, bg_blue);

#undef INSERT

    return translations;
}

std::unique_ptr<ComboboxTranslationMap> ComboboxEnumeration(QWidget* parent) {
    std::unique_ptr<ComboboxTranslationMap> translations =
        std::make_unique<ComboboxTranslationMap>();
    [[maybe_unused]] const auto& tr = [&](const char* text, const char* context = "") {
        return parent->tr(text, context);
    };

#define INDEX(ENUM) Settings::EnumMetadata<Settings::ENUM>::Index()
#define PAIR(ENUM, VALUE, TRANSLATION) {static_cast<u32>(Settings::ENUM::VALUE), tr(TRANSLATION)}
#define CTX_PAIR(ENUM, VALUE, TRANSLATION, CONTEXT)                                                \
    { static_cast<u32>(Settings::ENUM::VALUE), tr(TRANSLATION, CONTEXT) }

    translations->insert({Settings::EnumMetadata<Settings::GraphicsAPI>::Index(),
                          {
                              PAIR(GraphicsAPI, OpenGl, "OpenGL"),
                              PAIR(GraphicsAPI, Vulkan, "Vulkan"),
                              PAIR(GraphicsAPI, Software, "Software"),
                          }});
    translations->insert({INDEX(StereoRenderOption),
                          {
                              PAIR(StereoRenderOption, Off, "Off"),
                              PAIR(StereoRenderOption, SideBySide, "Side by Side"),
                              PAIR(StereoRenderOption, Anaglyph, "Anaglyph"),
                              PAIR(StereoRenderOption, Interlaced, "Interlaced"),
                              PAIR(StereoRenderOption, ReverseInterlaced, "Reverse Interlaced"),
                          }});
    translations->insert({INDEX(MonoRenderOption),
                          {
                              PAIR(MonoRenderOption, LeftEye, "Left Eye (default)"),
                              PAIR(MonoRenderOption, RightEye, "Right Eye"),
                          }});
    translations->insert({INDEX(LayoutOption),
                          {
                              PAIR(LayoutOption, Default, "Default"),
                              PAIR(LayoutOption, SingleScreen, "Single Screen"),
                              PAIR(LayoutOption, LargeScreen, "Large Screen"),
                              PAIR(LayoutOption, SideScreen, "Side by Side"),
                              PAIR(LayoutOption, SeparateWindows, "Separate Windows"),
                              PAIR(LayoutOption, HybridScreen, "Hybrid Screen"),
                          }});
    translations->insert({INDEX(TextureFilter),
                          {
                              PAIR(TextureFilter, None, "None"),
                              PAIR(TextureFilter, Anime4K, "Anime4K"),
                              PAIR(TextureFilter, Bicubic, "Bicubic"),
                              PAIR(TextureFilter, NearestNeighbor, "Nearest Neighbor"),
                              PAIR(TextureFilter, ScaleForce, "ScaleForce"),
                              PAIR(TextureFilter, Xbrz, "xBRZ"),
                              PAIR(TextureFilter, Mmpx, "MMPX"),
                          }});
    translations->insert({INDEX(ResolutionFactor),
                          {
                              PAIR(ResolutionFactor, Auto, "Auto (Window Size)"),
                              PAIR(ResolutionFactor, X1, "1x (400x240)"),
                              PAIR(ResolutionFactor, X2, "2x (800x480)"),
                              PAIR(ResolutionFactor, X3, "3x (1200x720)"),
                              PAIR(ResolutionFactor, X4, "4x (1600x960)"),
                              PAIR(ResolutionFactor, X5, "5x (2000x1200)"),
                              PAIR(ResolutionFactor, X6, "6x (2400x1440)"),
                              PAIR(ResolutionFactor, X7, "7x (2800x1680)"),
                              PAIR(ResolutionFactor, X8, "8x (3200x1920)"),
                              PAIR(ResolutionFactor, X9, "9x (3600x2160)"),
                              PAIR(ResolutionFactor, X10, "10x (4000x2400)"),
                          }});

#undef PAIR
#undef CTX_PAIR
#undef INDEX

    return translations;
}
} // namespace ConfigurationShared
