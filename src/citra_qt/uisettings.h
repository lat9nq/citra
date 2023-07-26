// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <string>
#include <utility>
#include <vector>
#include <QByteArray>
#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QVector>
#include "common/settings.h"
#include "common/settings_common.h"

namespace UISettings {

struct ContextualShortcut {
    QString keyseq;
    int context;
};

struct Shortcut {
    QString name;
    QString group;
    ContextualShortcut shortcut;
};

using Themes = std::array<std::pair<const char*, const char*>, 6>;
extern const Themes themes;

struct GameDir {
    QString path;
    bool deep_scan = false;
    bool expanded = false;
    bool operator==(const GameDir& rhs) const {
        return path == rhs.path;
    }
    bool operator!=(const GameDir& rhs) const {
        return !operator==(rhs);
    }
};

enum class GameListIconSize : u32 {
    NoIcon,    ///< Do not display icons
    SmallIcon, ///< Display a small (24x24) icon
    LargeIcon, ///< Display a large (48x48) icon
};

enum class GameListText : s32 {
    NoText = -1,   ///< No text
    FileName,      ///< Display the file name of the entry
    FullPath,      ///< Display the full path of the entry
    TitleName,     ///< Display the name of the title
    TitleID,       ///< Display the title ID
    LongTitleName, ///< Display the long name of the title
    ListEnd,       ///< Keep this at the end of the enum.
};

using Settings::Category;

struct Values {
    Settings::Linkage linkage{1000};

    QByteArray geometry;
    QByteArray state;

    QByteArray renderwindow_geometry;

    QByteArray gamelist_header_state;

    QByteArray microprofile_geometry;
    Settings::Setting<bool> microprofile_visible{linkage, false, "microProfileDialogVisible",
                                                 Category::UiLayout};

    Settings::Setting<bool> single_window_mode{linkage, true, "singleWindowMode", Category::Ui};
    Settings::Setting<bool> fullscreen{linkage, false, "fullscreen", Category::Ui};
    Settings::Setting<bool> display_titlebar{linkage, true, "displayTitleBars", Category::Ui};
    Settings::Setting<bool> show_filter_bar{linkage, true, "showFilterBar", Category::Ui};
    Settings::Setting<bool> show_status_bar{linkage, true, "showStatusBar", Category::Ui};

    Settings::Setting<bool> confirm_before_closing{linkage, true, "confirmClose",
                                                   Category::UiGeneral};
    Settings::Setting<bool> save_state_warning{linkage, true, "saveStateWarning", Category::Ui};
    Settings::Setting<bool> first_start{linkage, true, "firstStart", Category::Ui};
    Settings::Setting<bool> pause_when_in_background{linkage, false, "pauseWhenInBackground",
                                                     Category::UiGeneral};
    Settings::Setting<bool> hide_mouse{linkage, false, "hideInactiveMouse", Category::UiGeneral};

    bool updater_found;
    Settings::Setting<bool> update_on_close{linkage, false, "update_on_close", Category::Ui};
    Settings::Setting<bool> check_for_update_on_start{linkage, true, "check_for_update_on_start",
                                                      Category::Ui};

    // Discord RPC
    Settings::Setting<bool> enable_discord_presence{linkage, true, "enable_discord_presence",
                                                    Category::Ui};

    // Game List
    Settings::Setting<GameListIconSize> game_list_icon_size{linkage, GameListIconSize::LargeIcon,
                                                            "iconSize", Category::UiGameList};
    Settings::Setting<GameListText> game_list_row_1{linkage, GameListText::TitleName, "row1",
                                                    Category::UiGameList};
    Settings::Setting<GameListText> game_list_row_2{linkage, GameListText::FileName, "row2",
                                                    Category::UiGameList};
    Settings::Setting<bool> game_list_hide_no_icon{linkage, false, "hideNoIcon",
                                                   Category::UiGameList};
    Settings::Setting<bool> game_list_single_line_mode{linkage, false, "singleLineMode",
                                                       Category::UiGameList};

    // Compatibility List
    Settings::Setting<bool> show_compat_column{linkage, true, "show_compat_column",
                                               Category::UiGameList};
    Settings::Setting<bool> show_region_column{linkage, true, "show_region_column",
                                               Category::UiGameList};
    Settings::Setting<bool> show_type_column{linkage, true, "show_type_column",
                                             Category::UiGameList};
    Settings::Setting<bool> show_size_column{linkage, true, "show_size_column",
                                             Category::UiGameList};

    Settings::Setting<u16> screenshot_resolution_factor{linkage, 0, "screenshot_resolution_factor",
                                                        Category::Screenshots};
    Settings::SwitchableSetting<std::string> screenshot_path{linkage, "", "screenshotPath",
                                                             Category::Screenshots};

    QString roms_path;
    QString symbols_path;
    QString movie_record_path;
    QString movie_playback_path;
    QString video_dumping_path;
    QString game_dir_deprecated;
    bool game_dir_deprecated_deepscan;
    QVector<UISettings::GameDir> game_dirs;
    QStringList recent_files;
    QString language;

    QString theme;

    // Shortcut name <Shortcut, context>
    std::vector<Shortcut> shortcuts;

    Settings::Setting<u32> callout_flags{linkage, 0, "calloutFlags", Category::Ui};

    // multiplayer settings
    QString nickname;
    QString ip;
    QString port;
    QString room_nickname;
    QString room_name;
    quint32 max_player;
    QString room_port;
    uint host_type;
    qulonglong game_id;
    QString room_description;
    std::pair<std::vector<std::string>, std::vector<std::string>> ban_list;

    // logging
    Settings::Setting<bool> show_console{linkage, false, "showConsole", Category::Ui};
};

extern Values values;
} // namespace UISettings

Q_DECLARE_METATYPE(UISettings::GameDir*);
