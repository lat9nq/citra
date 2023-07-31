// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <algorithm>
#include <array>
#include <QKeySequence>
#include <QSettings>
#include "citra_qt/configuration/config.h"
#include "common/file_util.h"
#include "common/settings.h"
#include "common/settings_common.h"
#include "core/hle/service/cam/cam_params.h"
#include "core/hle/service/service.h"
#include "input_common/main.h"
#include "input_common/udp/client.h"
#include "network/network.h"
#include "network/network_settings.h"

Config::Config(const std::string& config_name, ConfigType config_type) : type{config_type} {
    global = config_type == ConfigType::GlobalConfig;
    Initialize(config_name);
}

Config::~Config() {
    if (global) {
        Save();
    }
}

const std::array<int, Settings::NativeButton::NumButtons> Config::default_buttons = {
    Qt::Key_A, Qt::Key_S, Qt::Key_Z, Qt::Key_X, Qt::Key_T, Qt::Key_G,
    Qt::Key_F, Qt::Key_H, Qt::Key_Q, Qt::Key_W, Qt::Key_M, Qt::Key_N,
    Qt::Key_O, Qt::Key_P, Qt::Key_1, Qt::Key_2, Qt::Key_B, Qt::Key_V,
};

const std::array<std::array<int, 5>, Settings::NativeAnalog::NumAnalogs> Config::default_analogs{{
    {
        Qt::Key_Up,
        Qt::Key_Down,
        Qt::Key_Left,
        Qt::Key_Right,
        Qt::Key_D,
    },
    {
        Qt::Key_I,
        Qt::Key_K,
        Qt::Key_J,
        Qt::Key_L,
        Qt::Key_D,
    },
}};

// This shouldn't have anything except static initializers (no functions). So
// QKeySequence(...).toString() is NOT ALLOWED HERE.
// This must be in alphabetical order according to action name as it must have the same order as
// UISetting::values.shortcuts, which is alphabetically ordered.
// clang-format off
const std::array<UISettings::Shortcut, 28> Config::default_hotkeys {{
     {QStringLiteral("Advance Frame"),            QStringLiteral("Main Window"), {QStringLiteral(""),     Qt::ApplicationShortcut}},
     {QStringLiteral("Capture Screenshot"),       QStringLiteral("Main Window"), {QStringLiteral("Ctrl+P"), Qt::WidgetWithChildrenShortcut}},
     {QStringLiteral("Continue/Pause Emulation"), QStringLiteral("Main Window"), {QStringLiteral("F4"),     Qt::WindowShortcut}},
     {QStringLiteral("Decrease 3D Factor"),       QStringLiteral("Main Window"), {QStringLiteral("Ctrl+-"), Qt::ApplicationShortcut}},
     {QStringLiteral("Decrease Speed Limit"),     QStringLiteral("Main Window"), {QStringLiteral("-"),      Qt::ApplicationShortcut}},
     {QStringLiteral("Exit Citra"),               QStringLiteral("Main Window"), {QStringLiteral("Ctrl+Q"), Qt::WindowShortcut}},
     {QStringLiteral("Exit Fullscreen"),          QStringLiteral("Main Window"), {QStringLiteral("Esc"),    Qt::WindowShortcut}},
     {QStringLiteral("Fullscreen"),               QStringLiteral("Main Window"), {QStringLiteral("F11"),    Qt::WindowShortcut}},
     {QStringLiteral("Increase 3D Factor"),       QStringLiteral("Main Window"), {QStringLiteral("Ctrl++"), Qt::ApplicationShortcut}},
     {QStringLiteral("Increase Speed Limit"),     QStringLiteral("Main Window"), {QStringLiteral("+"),      Qt::ApplicationShortcut}},
     {QStringLiteral("Load Amiibo"),              QStringLiteral("Main Window"), {QStringLiteral("F2"),     Qt::WidgetWithChildrenShortcut}},
     {QStringLiteral("Load File"),                QStringLiteral("Main Window"), {QStringLiteral("Ctrl+O"), Qt::WidgetWithChildrenShortcut}},
     {QStringLiteral("Load from Newest Slot"),    QStringLiteral("Main Window"), {QStringLiteral("Ctrl+V"), Qt::WindowShortcut}},
     {QStringLiteral("Mute Audio"),               QStringLiteral("Main Window"), {QStringLiteral("Ctrl+M"), Qt::WindowShortcut}},
     {QStringLiteral("Remove Amiibo"),            QStringLiteral("Main Window"), {QStringLiteral("F3"),     Qt::ApplicationShortcut}},
     {QStringLiteral("Restart Emulation"),        QStringLiteral("Main Window"), {QStringLiteral("F6"),     Qt::WindowShortcut}},
     {QStringLiteral("Rotate Screens Upright"),   QStringLiteral("Main Window"), {QStringLiteral("F8"),     Qt::WindowShortcut}},
     {QStringLiteral("Save to Oldest Slot"),      QStringLiteral("Main Window"), {QStringLiteral("Ctrl+C"), Qt::WindowShortcut}},
     {QStringLiteral("Stop Emulation"),           QStringLiteral("Main Window"), {QStringLiteral("F5"),     Qt::WindowShortcut}},
     {QStringLiteral("Swap Screens"),             QStringLiteral("Main Window"), {QStringLiteral("F9"),     Qt::WindowShortcut}},
     {QStringLiteral("Toggle 3D"),                QStringLiteral("Main Window"), {QStringLiteral("Ctrl+3"), Qt::ApplicationShortcut}},
     {QStringLiteral("Toggle Per-Game Speed"),    QStringLiteral("Main Window"), {QStringLiteral("Ctrl+Z"), Qt::ApplicationShortcut}},
     {QStringLiteral("Toggle Filter Bar"),        QStringLiteral("Main Window"), {QStringLiteral("Ctrl+F"), Qt::WindowShortcut}},
     {QStringLiteral("Toggle Frame Advancing"),   QStringLiteral("Main Window"), {QStringLiteral("Ctrl+A"), Qt::ApplicationShortcut}},
     {QStringLiteral("Toggle Screen Layout"),     QStringLiteral("Main Window"), {QStringLiteral("F10"),    Qt::WindowShortcut}},
     {QStringLiteral("Toggle Status Bar"),        QStringLiteral("Main Window"), {QStringLiteral("Ctrl+S"), Qt::WindowShortcut}},
     {QStringLiteral("Toggle Texture Dumping"),   QStringLiteral("Main Window"), {QStringLiteral(""),       Qt::ApplicationShortcut}},
     {QStringLiteral("Toggle Custom Textures"),   QStringLiteral("Main Window"), {QStringLiteral("F7"),     Qt::ApplicationShortcut}},
    }};
// clang-format on

void Config::Initialize(const std::string& config_name) {
    const std::string fs_config_loc = FileUtil::GetUserPath(FileUtil::UserPath::ConfigDir);
    const std::string config_file = fmt::format("{}.ini", config_name);

    switch (type) {
    case ConfigType::GlobalConfig:
        qt_config_loc = fmt::format("{}/{}", fs_config_loc, config_file);
        break;
    case ConfigType::PerGameConfig:
        qt_config_loc = fmt::format("{}/custom/{}", fs_config_loc, config_file);
        break;
    }

    FileUtil::CreateFullPath(qt_config_loc);
    qt_config =
        std::make_unique<QSettings>(QString::fromStdString(qt_config_loc), QSettings::IniFormat);
    Reload();
}

/* {Read,Write}BasicSetting and WriteGlobalSetting templates must be defined here before their
 * usages later in this file. This allows explicit definition of some types that don't work
 * nicely with the general version.
 */

// Explicit std::string definition: Qt can't implicitly convert a std::string to a QVariant, nor
// can it implicitly convert a QVariant back to a {std::,Q}string
template <>
void Config::ReadBasicSetting(Settings::Setting<std::string>& setting) {
    const QString name = QString::fromStdString(setting.GetLabel());
    const auto default_value = QString::fromStdString(setting.GetDefault());
    if (qt_config->value(name + QStringLiteral("/default"), false).toBool()) {
        setting.SetValue(default_value.toStdString());
    } else {
        setting.SetValue(qt_config->value(name, default_value).toString().toStdString());
    }
}

template <typename Type, bool ranged>
void Config::ReadBasicSetting(Settings::Setting<Type, ranged>& setting) {
    const QString name = QString::fromStdString(setting.GetLabel());
    const Type default_value = setting.GetDefault();
    if (qt_config->value(name + QStringLiteral("/default"), false).toBool()) {
        setting.SetValue(default_value);
    } else {
        QVariant value{};
        if constexpr (std::is_enum_v<Type>) {
            using TypeU = std::underlying_type_t<Type>;
            value = qt_config->value(name, static_cast<TypeU>(default_value));
            setting.SetValue(static_cast<Type>(value.value<TypeU>()));
        } else {
            value = qt_config->value(name, QVariant::fromValue(default_value));
            setting.SetValue(value.value<Type>());
        }
    }
}

template <typename Type, bool ranged>
void Config::ReadGlobalSetting(Settings::SwitchableSetting<Type, ranged>& setting) {
    QString name = QString::fromStdString(setting.GetLabel());
    const bool use_global = qt_config->value(name + QStringLiteral("/use_global"), true).toBool();
    setting.SetGlobal(use_global);
    if (global || !use_global) {
        QVariant default_value{};
        if constexpr (std::is_enum_v<Type>) {
            using TypeU = std::underlying_type_t<Type>;
            default_value = QVariant::fromValue<TypeU>(static_cast<TypeU>(setting.GetDefault()));
            setting.SetValue(static_cast<Type>(ReadSetting(name, default_value).value<TypeU>()));
        } else {
            default_value = QVariant::fromValue<Type>(setting.GetDefault());
            setting.SetValue(ReadSetting(name, default_value).value<Type>());
        }
    }
}

template <>
void Config::ReadGlobalSetting(Settings::SwitchableSetting<std::string>& setting) {
    QString name = QString::fromStdString(setting.GetLabel());
    const bool use_global = qt_config->value(name + QStringLiteral("/use_global"), true).toBool();
    setting.SetGlobal(use_global);
    if (global || !use_global) {
        const QString default_value = QString::fromStdString(setting.GetDefault());
        setting.SetValue(
            ReadSetting(name, QVariant::fromValue(default_value)).toString().toStdString());
    }
}

// Explicit std::string definition: Qt can't implicitly convert a std::string to a QVariant
template <>
void Config::WriteBasicSetting(const Settings::Setting<std::string>& setting) {
    const QString name = QString::fromStdString(setting.GetLabel());
    const std::string& value = setting.GetValue();
    qt_config->setValue(name + QStringLiteral("/default"), value == setting.GetDefault());
    qt_config->setValue(name, QString::fromStdString(value));
}

// Explicit u16 definition: Qt would store it as QMetaType otherwise, which is not human-readable
template <>
void Config::WriteBasicSetting(const Settings::Setting<u16>& setting) {
    const QString name = QString::fromStdString(setting.GetLabel());
    const u16& value = setting.GetValue();
    qt_config->setValue(name + QStringLiteral("/default"), value == setting.GetDefault());
    qt_config->setValue(name, static_cast<u32>(value));
}

template <typename Type, bool ranged>
void Config::WriteBasicSetting(const Settings::Setting<Type, ranged>& setting) {
    const QString name = QString::fromStdString(setting.GetLabel());
    const Type value = setting.GetValue();
    qt_config->setValue(name + QStringLiteral("/default"), value == setting.GetDefault());
    if constexpr (std::is_enum_v<Type>) {
        qt_config->setValue(name, static_cast<std::underlying_type_t<Type>>(value));
    } else {
        qt_config->setValue(name, QVariant::fromValue(value));
    }
}

template <typename Type, bool ranged>
void Config::WriteGlobalSetting(const Settings::SwitchableSetting<Type, ranged>& setting) {
    const QString name = QString::fromStdString(setting.GetLabel());
    const Type& value = setting.GetValue(global);
    if (!global) {
        qt_config->setValue(name + QStringLiteral("/use_global"), setting.UsingGlobal());
    }
    if (global || !setting.UsingGlobal()) {
        qt_config->setValue(name + QStringLiteral("/default"), value == setting.GetDefault());
        if constexpr (std::is_enum_v<Type>) {
            qt_config->setValue(name, static_cast<std::underlying_type_t<Type>>(value));
        } else {
            qt_config->setValue(name, QVariant::fromValue(value));
        }
    }
}

template <>
void Config::WriteGlobalSetting(const Settings::SwitchableSetting<std::string>& setting) {
    const QString name = QString::fromStdString(setting.GetLabel());
    const std::string& value = setting.GetValue(global);
    if (!global) {
        qt_config->setValue(name + QStringLiteral("/use_global"), setting.UsingGlobal());
    }
    if (global || !setting.UsingGlobal()) {
        qt_config->setValue(name + QStringLiteral("/default"), value == setting.GetDefault());
        qt_config->setValue(name, QString::fromStdString(value));
    }
}

// Explicit u16 definition: Qt would store it as QMetaType otherwise, which is not human-readable
template <>
void Config::WriteGlobalSetting(const Settings::SwitchableSetting<u16, true>& setting) {
    const QString name = QString::fromStdString(setting.GetLabel());
    const u16& value = setting.GetValue(global);
    if (!global) {
        qt_config->setValue(name + QStringLiteral("/use_global"), setting.UsingGlobal());
    }
    if (global || !setting.UsingGlobal()) {
        qt_config->setValue(name + QStringLiteral("/default"), value == setting.GetDefault());
        qt_config->setValue(name, static_cast<u32>(value));
    }
}

void Config::ReadValues() {
    if (global) {
        ReadControlValues();
        ReadCameraValues();
        ReadDataStorageValues();
        ReadMiscellaneousValues();
        ReadDebuggingValues();
        ReadWebServiceValues();
        ReadVideoDumpingValues();
    }

    ReadUIValues();
    ReadCoreValues();
    ReadRendererValues();
    ReadLayoutValues();
    ReadAudioValues();
    ReadSystemValues();
    ReadUtilityValues();
}

void Config::ReadAudioValues() {
    qt_config->beginGroup(QStringLiteral("Audio"));

    ReadCategory(Settings::Category::Audio);

    qt_config->endGroup();
}

void Config::ReadCameraValues() {
    using namespace Service::CAM;
    qt_config->beginGroup(QStringLiteral("Camera"));

    Settings::values.camera_name[OuterRightCamera] =
        ReadSetting(QStringLiteral("camera_outer_right_name"), QStringLiteral("blank"))
            .toString()
            .toStdString();
    Settings::values.camera_config[OuterRightCamera] =
        ReadSetting(QStringLiteral("camera_outer_right_config"), QString{})
            .toString()
            .toStdString();
    Settings::values.camera_flip[OuterRightCamera] =
        ReadSetting(QStringLiteral("camera_outer_right_flip"), 0).toInt();
    Settings::values.camera_name[InnerCamera] =
        ReadSetting(QStringLiteral("camera_inner_name"), QStringLiteral("blank"))
            .toString()
            .toStdString();
    Settings::values.camera_config[InnerCamera] =
        ReadSetting(QStringLiteral("camera_inner_config"), QString{}).toString().toStdString();
    Settings::values.camera_flip[InnerCamera] =
        ReadSetting(QStringLiteral("camera_inner_flip"), 0).toInt();
    Settings::values.camera_name[OuterLeftCamera] =
        ReadSetting(QStringLiteral("camera_outer_left_name"), QStringLiteral("blank"))
            .toString()
            .toStdString();
    Settings::values.camera_config[OuterLeftCamera] =
        ReadSetting(QStringLiteral("camera_outer_left_config"), QString{}).toString().toStdString();
    Settings::values.camera_flip[OuterLeftCamera] =
        ReadSetting(QStringLiteral("camera_outer_left_flip"), 0).toInt();

    qt_config->endGroup();
}

void Config::ReadControlValues() {
    qt_config->beginGroup(QStringLiteral("Controls"));

    int num_touch_from_button_maps =
        qt_config->beginReadArray(QStringLiteral("touch_from_button_maps"));

    if (num_touch_from_button_maps > 0) {
        const auto append_touch_from_button_map = [this] {
            Settings::TouchFromButtonMap map;
            map.name = ReadSetting(QStringLiteral("name"), QStringLiteral("default"))
                           .toString()
                           .toStdString();
            const int num_touch_maps = qt_config->beginReadArray(QStringLiteral("entries"));
            map.buttons.reserve(num_touch_maps);
            for (int i = 0; i < num_touch_maps; i++) {
                qt_config->setArrayIndex(i);
                std::string touch_mapping =
                    ReadSetting(QStringLiteral("bind")).toString().toStdString();
                map.buttons.emplace_back(std::move(touch_mapping));
            }
            qt_config->endArray(); // entries
            Settings::values.touch_from_button_maps.emplace_back(std::move(map));
        };

        for (int i = 0; i < num_touch_from_button_maps; ++i) {
            qt_config->setArrayIndex(i);
            append_touch_from_button_map();
        }
    } else {
        Settings::values.touch_from_button_maps.emplace_back(
            Settings::TouchFromButtonMap{"default", {}});
        num_touch_from_button_maps = 1;
    }
    qt_config->endArray();

    Settings::values.current_input_profile_index =
        ReadSetting(QStringLiteral("profile"), 0).toInt();

    const auto append_profile = [this, num_touch_from_button_maps] {
        Settings::InputProfile profile;
        profile.name =
            ReadSetting(QStringLiteral("name"), QStringLiteral("default")).toString().toStdString();
        for (int i = 0; i < Settings::NativeButton::NumButtons; ++i) {
            std::string default_param = InputCommon::GenerateKeyboardParam(default_buttons[i]);
            profile.buttons[i] = ReadSetting(QString::fromUtf8(Settings::NativeButton::mapping[i]),
                                             QString::fromStdString(default_param))
                                     .toString()
                                     .toStdString();
            if (profile.buttons[i].empty())
                profile.buttons[i] = default_param;
        }
        for (int i = 0; i < Settings::NativeAnalog::NumAnalogs; ++i) {
            std::string default_param = InputCommon::GenerateAnalogParamFromKeys(
                default_analogs[i][0], default_analogs[i][1], default_analogs[i][2],
                default_analogs[i][3], default_analogs[i][4], 0.5f);
            profile.analogs[i] = ReadSetting(QString::fromUtf8(Settings::NativeAnalog::mapping[i]),
                                             QString::fromStdString(default_param))
                                     .toString()
                                     .toStdString();
            if (profile.analogs[i].empty())
                profile.analogs[i] = default_param;
        }
        profile.motion_device =
            ReadSetting(QStringLiteral("motion_device"),
                        QStringLiteral(
                            "engine:motion_emu,update_period:100,sensitivity:0.01,tilt_clamp:90.0"))
                .toString()
                .toStdString();
        profile.touch_device =
            ReadSetting(QStringLiteral("touch_device"), QStringLiteral("engine:emu_window"))
                .toString()
                .toStdString();
        profile.use_touch_from_button =
            ReadSetting(QStringLiteral("use_touch_from_button"), false).toBool();
        profile.touch_from_button_map_index =
            ReadSetting(QStringLiteral("touch_from_button_map"), 0).toInt();
        profile.touch_from_button_map_index =
            std::clamp(profile.touch_from_button_map_index, 0, num_touch_from_button_maps - 1);
        profile.udp_input_address =
            ReadSetting(QStringLiteral("udp_input_address"),
                        QString::fromUtf8(InputCommon::CemuhookUDP::DEFAULT_ADDR))
                .toString()
                .toStdString();
        profile.udp_input_port = static_cast<u16>(
            ReadSetting(QStringLiteral("udp_input_port"), InputCommon::CemuhookUDP::DEFAULT_PORT)
                .toInt());
        profile.udp_pad_index =
            static_cast<u8>(ReadSetting(QStringLiteral("udp_pad_index"), 0).toUInt());
        Settings::values.input_profiles.emplace_back(std::move(profile));
    };

    int num_input_profiles = qt_config->beginReadArray(QStringLiteral("profiles"));

    for (int i = 0; i < num_input_profiles; ++i) {
        qt_config->setArrayIndex(i);
        append_profile();
    }

    qt_config->endArray();

    // create a input profile if no input profiles exist, with the default or old settings
    if (num_input_profiles == 0) {
        append_profile();
        num_input_profiles = 1;
    }

    // ensure that the current input profile index is valid.
    Settings::values.current_input_profile_index =
        std::clamp(Settings::values.current_input_profile_index, 0, num_input_profiles - 1);

    Settings::LoadProfile(Settings::values.current_input_profile_index);

    qt_config->endGroup();
}

void Config::ReadUtilityValues() {
    qt_config->beginGroup(QStringLiteral("Utility"));

    ReadCategory(Settings::Category::Utility);

    qt_config->endGroup();
}

void Config::ReadCoreValues() {
    qt_config->beginGroup(QStringLiteral("Core"));

    ReadCategory(Settings::Category::Core);

    qt_config->endGroup();
}

void Config::ReadDataStorageValues() {
    qt_config->beginGroup(QStringLiteral("Data Storage"));

    ReadCategory(Settings::Category::DataStorage);

    const std::string nand_dir =
        ReadSetting(QStringLiteral("nand_directory"), QStringLiteral("")).toString().toStdString();
    const std::string sdmc_dir =
        ReadSetting(QStringLiteral("sdmc_directory"), QStringLiteral("")).toString().toStdString();

    if (Settings::values.use_custom_storage) {
        FileUtil::UpdateUserPath(FileUtil::UserPath::NANDDir, nand_dir);
        FileUtil::UpdateUserPath(FileUtil::UserPath::SDMCDir, sdmc_dir);
    }

    qt_config->endGroup();
}

void Config::ReadDebuggingValues() {
    qt_config->beginGroup(QStringLiteral("Debugging"));

    // Intentionally not using the QT default setting as this is intended to be changed in the ini
    Settings::values.record_frame_times =
        qt_config->value(QStringLiteral("record_frame_times"), false).toBool();

    ReadCategory(Settings::Category::Debugging);

    qt_config->beginGroup(QStringLiteral("LLE"));
    for (const auto& service_module : Service::service_module_map) {
        bool use_lle = ReadSetting(QString::fromStdString(service_module.name), false).toBool();
        Settings::values.lle_modules.emplace(service_module.name, use_lle);
    }
    qt_config->endGroup();
    qt_config->endGroup();
}

void Config::ReadLayoutValues() {
    qt_config->beginGroup(QStringLiteral("Layout"));

    ReadCategory(Settings::Category::Layout);

    qt_config->endGroup();
}

void Config::ReadMiscellaneousValues() {
    qt_config->beginGroup(QStringLiteral("Miscellaneous"));

    ReadCategory(Settings::Category::Miscellaneous);

    qt_config->endGroup();
}

void Config::ReadMultiplayerValues() {
    qt_config->beginGroup(QStringLiteral("Multiplayer"));

    ReadCategory(Settings::Category::Multiplayer);

    // Read ban list back
    int size = qt_config->beginReadArray(QStringLiteral("username_ban_list"));
    UISettings::values.ban_list.first.resize(size);
    for (int i = 0; i < size; ++i) {
        qt_config->setArrayIndex(i);
        UISettings::values.ban_list.first[i] =
            ReadSetting(QStringLiteral("username")).toString().toStdString();
    }
    qt_config->endArray();
    size = qt_config->beginReadArray(QStringLiteral("ip_ban_list"));
    UISettings::values.ban_list.second.resize(size);
    for (int i = 0; i < size; ++i) {
        qt_config->setArrayIndex(i);
        UISettings::values.ban_list.second[i] =
            ReadSetting(QStringLiteral("ip")).toString().toStdString();
    }
    qt_config->endArray();

    qt_config->endGroup();
}

void Config::ReadPathValues() {
    qt_config->beginGroup(QStringLiteral("Paths"));

    ReadCategory(Settings::Category::Screenshots);
    ReadCategory(Settings::Category::Paths);

    if (global) {
        UISettings::values.roms_path = ReadSetting(QStringLiteral("romsPath")).toString();
        UISettings::values.symbols_path = ReadSetting(QStringLiteral("symbolsPath")).toString();
        UISettings::values.movie_record_path =
            ReadSetting(QStringLiteral("movieRecordPath")).toString();
        UISettings::values.movie_playback_path =
            ReadSetting(QStringLiteral("moviePlaybackPath")).toString();
        UISettings::values.video_dumping_path =
            ReadSetting(QStringLiteral("videoDumpingPath")).toString();
        UISettings::values.game_dir_deprecated =
            ReadSetting(QStringLiteral("gameListRootDir"), QStringLiteral(".")).toString();
        UISettings::values.game_dir_deprecated_deepscan =
            ReadSetting(QStringLiteral("gameListDeepScan"), false).toBool();
        int size = qt_config->beginReadArray(QStringLiteral("gamedirs"));
        for (int i = 0; i < size; ++i) {
            qt_config->setArrayIndex(i);
            UISettings::GameDir game_dir;
            game_dir.path = ReadSetting(QStringLiteral("path")).toString();
            game_dir.deep_scan = ReadSetting(QStringLiteral("deep_scan"), false).toBool();
            game_dir.expanded = ReadSetting(QStringLiteral("expanded"), true).toBool();
            UISettings::values.game_dirs.append(game_dir);
        }
        qt_config->endArray();
        // create NAND and SD card directories if empty, these are not removable through the UI,
        // also carries over old game list settings if present
        if (UISettings::values.game_dirs.isEmpty()) {
            UISettings::GameDir game_dir;
            game_dir.path = QStringLiteral("INSTALLED");
            game_dir.expanded = true;
            UISettings::values.game_dirs.append(game_dir);
            game_dir.path = QStringLiteral("SYSTEM");
            UISettings::values.game_dirs.append(game_dir);
            if (UISettings::values.game_dir_deprecated != QStringLiteral(".")) {
                game_dir.path = UISettings::values.game_dir_deprecated;
                game_dir.deep_scan = UISettings::values.game_dir_deprecated_deepscan;
                UISettings::values.game_dirs.append(game_dir);
            }
        }
        UISettings::values.recent_files = ReadSetting(QStringLiteral("recentFiles")).toStringList();
        UISettings::values.language = ReadSetting(QStringLiteral("language"), QString{}).toString();
    }

    qt_config->endGroup();
}

void Config::ReadRendererValues() {
    qt_config->beginGroup(QStringLiteral("Renderer"));

    ReadCategory(Settings::Category::Renderer);

    qt_config->endGroup();
}

void Config::ReadShortcutValues() {
    qt_config->beginGroup(QStringLiteral("Shortcuts"));

    for (const auto& [name, group, shortcut] : default_hotkeys) {
        qt_config->beginGroup(group);
        qt_config->beginGroup(name);
        // No longer using ReadSetting for shortcut.second as it innacurately returns a value of 1
        // for WidgetWithChildrenShortcut which is a value of 3. Needed to fix shortcuts the open
        // a file dialog in windowed mode
        UISettings::values.shortcuts.push_back(
            {name,
             group,
             {ReadSetting(QStringLiteral("KeySeq"), shortcut.keyseq).toString(),
              shortcut.context}});
        qt_config->endGroup();
        qt_config->endGroup();
    }

    qt_config->endGroup();
}

void Config::ReadSystemValues() {
    qt_config->beginGroup(QStringLiteral("System"));

    ReadCategory(Settings::Category::System);

    qt_config->endGroup();
}

void Config::ReadVideoDumpingValues() {
    qt_config->beginGroup(QStringLiteral("VideoDumping"));

    ReadCategory(Settings::Category::VideoDumping);

    qt_config->endGroup();
}

void Config::ReadUIValues() {
    qt_config->beginGroup(QStringLiteral("UI"));

    ReadPathValues();

    ReadCategory(Settings::Category::Ui);
    ReadCategory(Settings::Category::UiGeneral);

    if (global) {
        UISettings::values.theme =
            ReadSetting(QStringLiteral("theme"), QString::fromUtf8(UISettings::themes[0].second))
                .toString();

        ReadUpdaterValues();
        ReadUILayoutValues();
        ReadUIGameListValues();
        ReadShortcutValues();
        ReadMultiplayerValues();
    }

    qt_config->endGroup();
}

void Config::ReadUIGameListValues() {
    qt_config->beginGroup(QStringLiteral("GameList"));

    ReadCategory(Settings::Category::UiUpdater);

    qt_config->endGroup();
}

void Config::ReadUILayoutValues() {
    qt_config->beginGroup(QStringLiteral("UILayout"));

    UISettings::values.geometry = ReadSetting(QStringLiteral("geometry")).toByteArray();
    UISettings::values.state = ReadSetting(QStringLiteral("state")).toByteArray();
    UISettings::values.renderwindow_geometry =
        ReadSetting(QStringLiteral("geometryRenderWindow")).toByteArray();
    UISettings::values.gamelist_header_state =
        ReadSetting(QStringLiteral("gameListHeaderState")).toByteArray();
    UISettings::values.microprofile_geometry =
        ReadSetting(QStringLiteral("microProfileDialogGeometry")).toByteArray();

    ReadCategory(Settings::Category::UiLayout);

    qt_config->endGroup();
}

void Config::ReadUpdaterValues() {
    qt_config->beginGroup(QStringLiteral("Updater"));

    ReadCategory(Settings::Category::UiUpdater);

    qt_config->endGroup();
}

void Config::ReadWebServiceValues() {
    qt_config->beginGroup(QStringLiteral("WebService"));

    NetSettings::values.enable_telemetry =
        ReadSetting(QStringLiteral("enable_telemetry"), false).toBool();
    NetSettings::values.web_api_url =
        ReadSetting(QStringLiteral("web_api_url"), QStringLiteral("https://api.citra-emu.org"))
            .toString()
            .toStdString();
    NetSettings::values.citra_username =
        ReadSetting(QStringLiteral("citra_username")).toString().toStdString();
    NetSettings::values.citra_token =
        ReadSetting(QStringLiteral("citra_token")).toString().toStdString();

    qt_config->endGroup();
}

void Config::SaveValues() {
    if (global) {
        SaveControlValues();
        SaveCameraValues();
        SaveDataStorageValues();
        SaveMiscellaneousValues();
        SaveDebuggingValues();
        SaveWebServiceValues();
        SaveVideoDumpingValues();
    }

    SaveUIValues();
    SaveCoreValues();
    SaveRendererValues();
    SaveLayoutValues();
    SaveAudioValues();
    SaveSystemValues();
    SaveUtilityValues();
    qt_config->sync();
}

void Config::SaveAudioValues() {
    qt_config->beginGroup(QStringLiteral("Audio"));

    WriteCategory(Settings::Category::Audio);

    qt_config->endGroup();
}

void Config::SaveCameraValues() {
    using namespace Service::CAM;
    qt_config->beginGroup(QStringLiteral("Camera"));

    WriteSetting(QStringLiteral("camera_outer_right_name"),
                 QString::fromStdString(Settings::values.camera_name[OuterRightCamera]),
                 QStringLiteral("blank"));
    WriteSetting(QStringLiteral("camera_outer_right_config"),
                 QString::fromStdString(Settings::values.camera_config[OuterRightCamera]),
                 QString{});
    WriteSetting(QStringLiteral("camera_outer_right_flip"),
                 Settings::values.camera_flip[OuterRightCamera], 0);
    WriteSetting(QStringLiteral("camera_inner_name"),
                 QString::fromStdString(Settings::values.camera_name[InnerCamera]),
                 QStringLiteral("blank"));
    WriteSetting(QStringLiteral("camera_inner_config"),
                 QString::fromStdString(Settings::values.camera_config[InnerCamera]), QString{});
    WriteSetting(QStringLiteral("camera_inner_flip"), Settings::values.camera_flip[InnerCamera], 0);
    WriteSetting(QStringLiteral("camera_outer_left_name"),
                 QString::fromStdString(Settings::values.camera_name[OuterLeftCamera]),
                 QStringLiteral("blank"));
    WriteSetting(QStringLiteral("camera_outer_left_config"),
                 QString::fromStdString(Settings::values.camera_config[OuterLeftCamera]),
                 QString{});
    WriteSetting(QStringLiteral("camera_outer_left_flip"),
                 Settings::values.camera_flip[OuterLeftCamera], 0);

    qt_config->endGroup();
}

void Config::SaveControlValues() {
    qt_config->beginGroup(QStringLiteral("Controls"));

    WriteSetting(QStringLiteral("profile"), Settings::values.current_input_profile_index, 0);
    qt_config->beginWriteArray(QStringLiteral("profiles"));
    for (std::size_t p = 0; p < Settings::values.input_profiles.size(); ++p) {
        qt_config->setArrayIndex(static_cast<int>(p));
        const auto& profile = Settings::values.input_profiles[p];
        WriteSetting(QStringLiteral("name"), QString::fromStdString(profile.name),
                     QStringLiteral("default"));
        for (int i = 0; i < Settings::NativeButton::NumButtons; ++i) {
            std::string default_param = InputCommon::GenerateKeyboardParam(default_buttons[i]);
            WriteSetting(QString::fromStdString(Settings::NativeButton::mapping[i]),
                         QString::fromStdString(profile.buttons[i]),
                         QString::fromStdString(default_param));
        }
        for (int i = 0; i < Settings::NativeAnalog::NumAnalogs; ++i) {
            std::string default_param = InputCommon::GenerateAnalogParamFromKeys(
                default_analogs[i][0], default_analogs[i][1], default_analogs[i][2],
                default_analogs[i][3], default_analogs[i][4], 0.5f);
            WriteSetting(QString::fromStdString(Settings::NativeAnalog::mapping[i]),
                         QString::fromStdString(profile.analogs[i]),
                         QString::fromStdString(default_param));
        }
        WriteSetting(
            QStringLiteral("motion_device"), QString::fromStdString(profile.motion_device),
            QStringLiteral("engine:motion_emu,update_period:100,sensitivity:0.01,tilt_clamp:90.0"));
        WriteSetting(QStringLiteral("touch_device"), QString::fromStdString(profile.touch_device),
                     QStringLiteral("engine:emu_window"));
        WriteSetting(QStringLiteral("use_touch_from_button"), profile.use_touch_from_button, false);
        WriteSetting(QStringLiteral("touch_from_button_map"), profile.touch_from_button_map_index,
                     0);
        WriteSetting(QStringLiteral("udp_input_address"),
                     QString::fromStdString(profile.udp_input_address),
                     QString::fromUtf8(InputCommon::CemuhookUDP::DEFAULT_ADDR));
        WriteSetting(QStringLiteral("udp_input_port"), profile.udp_input_port,
                     InputCommon::CemuhookUDP::DEFAULT_PORT);
        WriteSetting(QStringLiteral("udp_pad_index"), profile.udp_pad_index, 0);
    }
    qt_config->endArray();

    qt_config->beginWriteArray(QStringLiteral("touch_from_button_maps"));
    for (std::size_t p = 0; p < Settings::values.touch_from_button_maps.size(); ++p) {
        qt_config->setArrayIndex(static_cast<int>(p));
        const auto& map = Settings::values.touch_from_button_maps[p];
        WriteSetting(QStringLiteral("name"), QString::fromStdString(map.name),
                     QStringLiteral("default"));
        qt_config->beginWriteArray(QStringLiteral("entries"));
        for (std::size_t q = 0; q < map.buttons.size(); ++q) {
            qt_config->setArrayIndex(static_cast<int>(q));
            WriteSetting(QStringLiteral("bind"), QString::fromStdString(map.buttons[q]));
        }
        qt_config->endArray();
    }
    qt_config->endArray();

    qt_config->endGroup();
}

void Config::SaveUtilityValues() {
    qt_config->beginGroup(QStringLiteral("Utility"));

    WriteCategory(Settings::Category::Utility);

    qt_config->endGroup();
}

void Config::SaveCoreValues() {
    qt_config->beginGroup(QStringLiteral("Core"));

    WriteCategory(Settings::Category::Core);

    qt_config->endGroup();
}

void Config::SaveDataStorageValues() {
    qt_config->beginGroup(QStringLiteral("Data Storage"));

    WriteCategory(Settings::Category::DataStorage);

    WriteSetting(QStringLiteral("nand_directory"),
                 QString::fromStdString(FileUtil::GetUserPath(FileUtil::UserPath::NANDDir)),
                 QStringLiteral(""));
    WriteSetting(QStringLiteral("sdmc_directory"),
                 QString::fromStdString(FileUtil::GetUserPath(FileUtil::UserPath::SDMCDir)),
                 QStringLiteral(""));

    qt_config->endGroup();
}

void Config::SaveDebuggingValues() {
    qt_config->beginGroup(QStringLiteral("Debugging"));

    // Intentionally not using the QT default setting as this is intended to be changed in the ini
    qt_config->setValue(QStringLiteral("record_frame_times"), Settings::values.record_frame_times);

    WriteCategory(Settings::Category::Debugging);

    qt_config->beginGroup(QStringLiteral("LLE"));
    for (const auto& service_module : Settings::values.lle_modules) {
        WriteSetting(QString::fromStdString(service_module.first), service_module.second, false);
    }
    qt_config->endGroup();

    qt_config->endGroup();
}

void Config::SaveLayoutValues() {
    qt_config->beginGroup(QStringLiteral("Layout"));

    WriteCategory(Settings::Category::Layout);

    qt_config->endGroup();
}

void Config::SaveMiscellaneousValues() {
    qt_config->beginGroup(QStringLiteral("Miscellaneous"));

    WriteCategory(Settings::Category::Miscellaneous);

    qt_config->endGroup();
}

void Config::SaveMultiplayerValues() {
    qt_config->beginGroup(QStringLiteral("Multiplayer"));

    WriteCategory(Settings::Category::Multiplayer);

    // Write ban list
    qt_config->beginWriteArray(QStringLiteral("username_ban_list"));
    for (std::size_t i = 0; i < UISettings::values.ban_list.first.size(); ++i) {
        qt_config->setArrayIndex(static_cast<int>(i));
        WriteSetting(QStringLiteral("username"),
                     QString::fromStdString(UISettings::values.ban_list.first[i]));
    }
    qt_config->endArray();
    qt_config->beginWriteArray(QStringLiteral("ip_ban_list"));
    for (std::size_t i = 0; i < UISettings::values.ban_list.second.size(); ++i) {
        qt_config->setArrayIndex(static_cast<int>(i));
        WriteSetting(QStringLiteral("ip"),
                     QString::fromStdString(UISettings::values.ban_list.second[i]));
    }
    qt_config->endArray();

    qt_config->endGroup();
}

void Config::SavePathValues() {
    qt_config->beginGroup(QStringLiteral("Paths"));

    WriteCategory(Settings::Category::Screenshots);
    WriteCategory(Settings::Category::Paths);

    if (global) {
        WriteSetting(QStringLiteral("romsPath"), UISettings::values.roms_path);
        WriteSetting(QStringLiteral("symbolsPath"), UISettings::values.symbols_path);
        WriteSetting(QStringLiteral("movieRecordPath"), UISettings::values.movie_record_path);
        WriteSetting(QStringLiteral("moviePlaybackPath"), UISettings::values.movie_playback_path);
        WriteSetting(QStringLiteral("videoDumpingPath"), UISettings::values.video_dumping_path);
        qt_config->beginWriteArray(QStringLiteral("gamedirs"));
        for (int i = 0; i < UISettings::values.game_dirs.size(); ++i) {
            qt_config->setArrayIndex(i);
            const auto& game_dir = UISettings::values.game_dirs[i];
            WriteSetting(QStringLiteral("path"), game_dir.path);
            WriteSetting(QStringLiteral("deep_scan"), game_dir.deep_scan, false);
            WriteSetting(QStringLiteral("expanded"), game_dir.expanded, true);
        }
        qt_config->endArray();
        WriteSetting(QStringLiteral("recentFiles"), UISettings::values.recent_files);
        WriteSetting(QStringLiteral("language"), UISettings::values.language, QString{});
    }

    qt_config->endGroup();
}

void Config::SaveRendererValues() {
    qt_config->beginGroup(QStringLiteral("Renderer"));

    WriteCategory(Settings::Category::Renderer);

    if (global) {
        WriteSetting(QStringLiteral("use_shader_jit"), Settings::values.use_shader_jit.GetValue(),
                     true);
    }

    qt_config->endGroup();
}

void Config::SaveShortcutValues() {
    qt_config->beginGroup(QStringLiteral("Shortcuts"));

    // Lengths of UISettings::values.shortcuts & default_hotkeys are same.
    // However, their ordering must also be the same.
    for (std::size_t i = 0; i < default_hotkeys.size(); i++) {
        const auto& [name, group, shortcut] = UISettings::values.shortcuts[i];
        const auto& default_hotkey = default_hotkeys[i].shortcut;

        qt_config->beginGroup(group);
        qt_config->beginGroup(name);
        WriteSetting(QStringLiteral("KeySeq"), shortcut.keyseq, default_hotkey.keyseq);
        WriteSetting(QStringLiteral("Context"), shortcut.context, default_hotkey.context);
        qt_config->endGroup();
        qt_config->endGroup();
    }

    qt_config->endGroup();
}

void Config::SaveSystemValues() {
    qt_config->beginGroup(QStringLiteral("System"));

    WriteCategory(Settings::Category::System);

    qt_config->endGroup();
}

void Config::SaveVideoDumpingValues() {
    qt_config->beginGroup(QStringLiteral("VideoDumping"));

    WriteCategory(Settings::Category::VideoDumping);

    qt_config->endGroup();
}

void Config::SaveUIValues() {
    qt_config->beginGroup(QStringLiteral("UI"));

    SavePathValues();

    if (global) {
        WriteSetting(QStringLiteral("theme"), UISettings::values.theme,
                     QString::fromUtf8(UISettings::themes[0].second));

        SaveUpdaterValues();
        SaveUILayoutValues();
        SaveUIGameListValues();
        SaveShortcutValues();
        SaveMultiplayerValues();

        WriteCategory(Settings::Category::Ui);
        WriteCategory(Settings::Category::UiGeneral);
    }

    qt_config->endGroup();
}

void Config::SaveUIGameListValues() {
    qt_config->beginGroup(QStringLiteral("GameList"));

    WriteCategory(Settings::Category::UiGameList);

    qt_config->endGroup();
}

void Config::SaveUILayoutValues() {
    qt_config->beginGroup(QStringLiteral("UILayout"));

    WriteSetting(QStringLiteral("geometry"), UISettings::values.geometry);
    WriteSetting(QStringLiteral("state"), UISettings::values.state);
    WriteSetting(QStringLiteral("geometryRenderWindow"), UISettings::values.renderwindow_geometry);
    WriteSetting(QStringLiteral("gameListHeaderState"), UISettings::values.gamelist_header_state);
    WriteSetting(QStringLiteral("microProfileDialogGeometry"),
                 UISettings::values.microprofile_geometry);

    WriteCategory(Settings::Category::UiLayout);

    qt_config->endGroup();
}

void Config::SaveUpdaterValues() {
    qt_config->beginGroup(QStringLiteral("Updater"));

    WriteCategory(Settings::Category::UiUpdater);

    qt_config->endGroup();
}

void Config::SaveWebServiceValues() {
    qt_config->beginGroup(QStringLiteral("WebService"));

    WriteSetting(QStringLiteral("enable_telemetry"), NetSettings::values.enable_telemetry, false);
    WriteSetting(QStringLiteral("web_api_url"),
                 QString::fromStdString(NetSettings::values.web_api_url),
                 QStringLiteral("https://api.citra-emu.org"));
    WriteSetting(QStringLiteral("citra_username"),
                 QString::fromStdString(NetSettings::values.citra_username));
    WriteSetting(QStringLiteral("citra_token"),
                 QString::fromStdString(NetSettings::values.citra_token));

    qt_config->endGroup();
}

static auto FindRelevantList(Settings::Category category) {
    auto& map = Settings::values.linkage.by_category;
    if (map.contains(category)) {
        return Settings::values.linkage.by_category[category];
    }
    return UISettings::values.linkage.by_category[category];
}

void Config::ReadCategory(Settings::Category category) {
    const auto& settings = FindRelevantList(category);
    std::for_each(settings.begin(), settings.end(),
                  [&](const auto& setting) { ReadSettingGeneric(setting); });
}

void Config::WriteCategory(Settings::Category category) {
    const auto& settings = FindRelevantList(category);
    std::for_each(settings.begin(), settings.end(),
                  [&](const auto& setting) { WriteSettingGeneric(setting); });
}

void Config::ReadSettingGeneric(Settings::BasicSetting* const setting) {
    if (!setting->Save() || (!setting->Switchable() && !global)) {
        return;
    }
    const QString name = QString::fromStdString(setting->GetLabel());
    const auto default_value =
        QVariant::fromValue<QString>(QString::fromStdString(setting->DefaultToString()));

    bool use_global = true;
    if (setting->Switchable() && !global) {
        use_global = qt_config->value(name + QStringLiteral("/use_global"), true).value<bool>();
        setting->SetGlobal(use_global);
    }

    if (global || !use_global) {
        const bool is_default = ReadSetting(name + QStringLiteral("/default"), true).value<bool>();
        if (!is_default) {
            setting->LoadString(ReadSetting(name, default_value).value<QString>().toStdString());
        } else {
            // Empty string resets the Setting to default
            setting->LoadString("");
        }
    }
}

void Config::WriteSettingGeneric(Settings::BasicSetting* const setting) const {
    if (!setting->Save()) {
        return;
    }
    const QVariant value = QVariant::fromValue(QString::fromStdString(setting->ToString()));
    const QVariant default_value =
        QVariant::fromValue(QString::fromStdString(setting->DefaultToString()));
    const QString label = QString::fromStdString(setting->GetLabel());
    if (setting->Switchable()) {
        if (!global) {
            qt_config->setValue(label + QStringLiteral("/use_global"), setting->UsingGlobal());
        }
        if (global || !setting->UsingGlobal()) {
            qt_config->setValue(label + QStringLiteral("/default"), value == default_value);
            qt_config->setValue(label, value);
        }
    } else if (global) {
        qt_config->setValue(label + QStringLiteral("/default"), value == default_value);
        qt_config->setValue(label, value);
    }
}

QVariant Config::ReadSetting(const QString& name) const {
    return qt_config->value(name);
}

QVariant Config::ReadSetting(const QString& name, const QVariant& default_value) const {
    QVariant result;
    if (qt_config->value(name + QStringLiteral("/default"), false).toBool()) {
        result = default_value;
    } else {
        result = qt_config->value(name, default_value);
    }
    return result;
}

void Config::WriteSetting(const QString& name, const QVariant& value) {
    qt_config->setValue(name, value);
}

void Config::WriteSetting(const QString& name, const QVariant& value,
                          const QVariant& default_value) {
    qt_config->setValue(name + QStringLiteral("/default"), value == default_value);
    qt_config->setValue(name, value);
}

void Config::Reload() {
    ReadValues();
    // To apply default value changes
    SaveValues();
}

void Config::Save() {
    SaveValues();
}
