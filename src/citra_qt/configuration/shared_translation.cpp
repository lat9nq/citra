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

#undef INSERT

    return translations;
}

std::unique_ptr<ComboboxTranslationMap> ComboboxEnumeration(QWidget* parent) {
    std::unique_ptr<ComboboxTranslationMap> translations =
        std::make_unique<ComboboxTranslationMap>();
    [[maybe_unused]] const auto& tr = [&](const char* text, const char* context = "") {
        return parent->tr(text, context);
    };

#define PAIR(ENUM, VALUE, TRANSLATION)                                                             \
    { static_cast<u32>(Settings::ENUM::VALUE), tr(TRANSLATION) }
#define CTX_PAIR(ENUM, VALUE, TRANSLATION, CONTEXT)                                                \
    { static_cast<u32>(Settings::ENUM::VALUE), tr(TRANSLATION, CONTEXT) }

#undef PAIR
#undef CTX_PAIR

    return translations;
}
} // namespace ConfigurationShared
