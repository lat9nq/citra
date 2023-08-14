// SPDX-FileCopyrightText: Copyright 2023 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

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

#undef INSERT

    return translations;
}

std::unique_ptr<ComboboxTranslationMap> ComboboxEnumeration(QWidget* parent) {
    std::unique_ptr<ComboboxTranslationMap> translations =
        std::make_unique<ComboboxTranslationMap>();
    [[maybe_unused]] const auto& tr = [&](const char* text, const char* context = "") {
        return parent->tr(text, context);
    };

#define PAIR(ENUM, VALUE, TRANSLATION) {static_cast<u32>(Settings::ENUM::VALUE), tr(TRANSLATION)}
#define CTX_PAIR(ENUM, VALUE, TRANSLATION, CONTEXT)                                                \
    { static_cast<u32>(Settings::ENUM::VALUE), tr(TRANSLATION, CONTEXT) }

    translations->insert({Settings::EnumMetadata<Settings::GraphicsAPI>::Index(),
                          {
                              PAIR(GraphicsAPI, OpenGl, "OpenGL"),
                              PAIR(GraphicsAPI, Vulkan, "Vulkan"),
                              PAIR(GraphicsAPI, Software, "Software"),
                          }});

#undef PAIR
#undef CTX_PAIR

    return translations;
}
} // namespace ConfigurationShared
