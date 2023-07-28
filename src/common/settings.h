// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <string>
#include <unordered_map>
#include <vector>
#include "common/common_types.h"
#include "common/settings_common.h"
#include "common/settings_enums.h"
#include "common/settings_setting.h"

namespace Settings {

namespace NativeButton {

enum Values {
    A,
    B,
    X,
    Y,
    Up,
    Down,
    Left,
    Right,
    L,
    R,
    Start,
    Select,
    Debug,
    Gpio14,

    ZL,
    ZR,

    Home,
    Power,

    NumButtons,
};

constexpr int BUTTON_HID_BEGIN = A;
constexpr int BUTTON_IR_BEGIN = ZL;
constexpr int BUTTON_NS_BEGIN = Power;

constexpr int BUTTON_HID_END = BUTTON_IR_BEGIN;
constexpr int BUTTON_IR_END = BUTTON_NS_BEGIN;
constexpr int BUTTON_NS_END = NumButtons;

constexpr int NUM_BUTTONS_HID = BUTTON_HID_END - BUTTON_HID_BEGIN;
constexpr int NUM_BUTTONS_IR = BUTTON_IR_END - BUTTON_IR_BEGIN;
constexpr int NUM_BUTTONS_NS = BUTTON_NS_END - BUTTON_NS_BEGIN;

static const std::array<const char*, NumButtons> mapping = {{
    "button_a",
    "button_b",
    "button_x",
    "button_y",
    "button_up",
    "button_down",
    "button_left",
    "button_right",
    "button_l",
    "button_r",
    "button_start",
    "button_select",
    "button_debug",
    "button_gpio14",
    "button_zl",
    "button_zr",
    "button_home",
    "button_power",
}};

} // namespace NativeButton

namespace NativeAnalog {
enum Values {
    CirclePad,
    CStick,
    NumAnalogs,
};

constexpr std::array<const char*, NumAnalogs> mapping = {{
    "circle_pad",
    "c_stick",
}};
} // namespace NativeAnalog

struct InputProfile {
    std::string name;
    std::array<std::string, NativeButton::NumButtons> buttons;
    std::array<std::string, NativeAnalog::NumAnalogs> analogs;
    std::string motion_device;
    std::string touch_device;
    bool use_touch_from_button;
    int touch_from_button_map_index;
    std::string udp_input_address;
    u16 udp_input_port;
    u8 udp_pad_index;
};

struct TouchFromButtonMap {
    std::string name;
    std::vector<std::string> buttons;
};

/// A special region value indicating that citra will automatically select a region
/// value to fit the region lockout info of the game
static constexpr s32 REGION_VALUE_AUTO_SELECT = -1;

static constexpr u32 CAMERA_COUNT = 3;

struct Values {
    Linkage linkage{};

    // Controls
    InputProfile current_input_profile;       ///< The current input profile
    int current_input_profile_index;          ///< The current input profile index
    std::vector<InputProfile> input_profiles; ///< The list of input profiles
    std::vector<TouchFromButtonMap> touch_from_button_maps;

    // Core
    Setting<bool> use_cpu_jit{linkage, true, "use_cpu_jit", Category::Core};
    SwitchableSetting<s32, true> cpu_clock_percentage{
        linkage, 100, 5, 400, "cpu_clock_percentage", Category::Core};
    SwitchableSetting<bool> is_new_3ds{linkage, true, "is_new_3ds", Category::Core};

    // Data Storage
    Setting<bool> use_virtual_sd{linkage, true, "use_virtual_sd", Category::DataStorage};
    Setting<bool> use_custom_storage{linkage, false, "use_custom_storage", Category::DataStorage};

    // System
    SwitchableSetting<s32> region_value{linkage, REGION_VALUE_AUTO_SELECT, "region_value",
                                        Category::System};
    Setting<InitClock> init_clock{linkage, InitClock::SystemTime, "init_clock", Category::System};
    Setting<u64> init_time{linkage, 946681277ULL, "init_time", Category::System};
    Setting<s64> init_time_offset{linkage, 0, "init_time_offset", Category::System};
    Setting<bool> plugin_loader_enabled{linkage, false, "plugin_loader", Category::System};
    Setting<bool> allow_plugin_loader{linkage, true, "allow_plugin_loader", Category::System};

    // Renderer
    SwitchableSetting<GraphicsAPI, true> graphics_api{
        linkage,
        GraphicsAPI::OpenGl,
        GraphicsAPI::Software,
        static_cast<GraphicsAPI>(static_cast<u32>(GraphicsAPI::ApiCount) - 1),
        "graphics_api",
        Category::Renderer};
    SwitchableSetting<u32> physical_device{linkage, 0, "physical_device", Category::Renderer};
    Setting<bool> use_gles{linkage, false, "use_gles", Category::Renderer};
    Setting<bool> renderer_debug{linkage, false, "renderer_debug", Category::Renderer};
    Setting<bool> dump_command_buffers{linkage, false, "dump_command_buffers", Category::Renderer};
    SwitchableSetting<bool> spirv_shader_gen{linkage, true, "spirv_shader_gen", Category::Renderer};
    SwitchableSetting<bool> async_shader_compilation{linkage, false, "async_shader_compilation",
                                                     Category::Renderer};
    SwitchableSetting<bool> async_presentation{linkage, true, "async_presentation",
                                               Category::Renderer};
    SwitchableSetting<bool> use_hw_shader{linkage, true, "use_hw_shader", Category::Renderer};
    SwitchableSetting<bool> use_disk_shader_cache{linkage, true, "use_disk_shader_cache",
                                                  Category::Renderer};
    SwitchableSetting<bool> shaders_accurate_mul{linkage, true, "shaders_accurate_mul",
                                                 Category::Renderer};
    SwitchableSetting<bool> use_vsync_new{linkage, true, "use_vsync_new", Category::Renderer};
    Setting<bool> use_shader_jit{linkage, true, "use_shader_jit", Category::Renderer};
    SwitchableSetting<u32, true> resolution_factor{linkage,           1, 0, 10, "resolution_factor",
                                                   Category::Renderer};
    SwitchableSetting<u16, true> frame_limit{linkage, 100,           0,
                                             1000,    "frame_limit", Category::Renderer};
    SwitchableSetting<TextureFilter> texture_filter{linkage, TextureFilter::None, "texture_filter",
                                                    Category::Renderer};

    SwitchableSetting<LayoutOption> layout_option{linkage, LayoutOption::Default, "layout_option",
                                                  Category::Layout};
    SwitchableSetting<bool> swap_screen{linkage, false, "swap_screen", Category::Layout};
    SwitchableSetting<bool> upright_screen{linkage, false, "upright_screen", Category::Layout};
    SwitchableSetting<float, true> large_screen_proportion{
        linkage, 4.f, 1.f, 16.f, "large_screen_proportion", Category::Layout};
    Setting<bool> custom_layout{linkage, false, "custom_layout", Category::Layout};
    Setting<u16> custom_top_left{linkage, 0, "custom_top_left", Category::Layout};
    Setting<u16> custom_top_top{linkage, 0, "custom_top_top", Category::Layout};
    Setting<u16> custom_top_right{linkage, 400, "custom_top_right", Category::Layout};
    Setting<u16> custom_top_bottom{linkage, 240, "custom_top_bottom", Category::Layout};
    Setting<u16> custom_bottom_left{linkage, 40, "custom_bottom_left", Category::Layout};
    Setting<u16> custom_bottom_top{linkage, 240, "custom_bottom_top", Category::Layout};
    Setting<u16> custom_bottom_right{linkage, 360, "custom_bottom_right", Category::Layout};
    Setting<u16> custom_bottom_bottom{linkage, 480, "custom_bottom_bottom", Category::Layout};
    Setting<u16> custom_second_layer_opacity{linkage, 100, "custom_second_layer_opacity",
                                             Category::Layout};

    SwitchableSetting<float> bg_red{linkage, 0.f, "bg_red", Category::Renderer};
    SwitchableSetting<float> bg_green{linkage, 0.f, "bg_green", Category::Renderer};
    SwitchableSetting<float> bg_blue{linkage, 0.f, "bg_blue", Category::Renderer};

    SwitchableSetting<StereoRenderOption> render_3d{linkage, StereoRenderOption::Off, "render_3d",
                                                    Category::Layout};
    SwitchableSetting<u32> factor_3d{linkage, 0, "factor_3d", Category::Layout};
    SwitchableSetting<MonoRenderOption> mono_render_option{linkage, MonoRenderOption::LeftEye,
                                                           "mono_render_option", Category::Layout};

    Setting<u32> cardboard_screen_size{linkage, 85, "cardboard_screen_size", Category::Renderer};
    Setting<s32> cardboard_x_shift{linkage, 0, "cardboard_x_shift", Category::Renderer};
    Setting<s32> cardboard_y_shift{linkage, 0, "cardboard_y_shift", Category::Renderer};

    SwitchableSetting<bool> filter_mode{linkage, true, "filter_mode", Category::Layout};
    SwitchableSetting<std::string> pp_shader_name{linkage, "none (builtin)", "pp_shader_name",
                                                  Category::Layout};
    SwitchableSetting<std::string> anaglyph_shader_name{linkage, "dubois (builtin)",
                                                        "anaglyph_shader_name", Category::Layout};

    SwitchableSetting<bool> dump_textures{linkage, false, "dump_textures", Category::Utility};
    SwitchableSetting<bool> custom_textures{linkage, false, "custom_textures", Category::Utility};
    SwitchableSetting<bool> preload_textures{linkage, false, "preload_textures", Category::Utility};
    SwitchableSetting<bool> async_custom_loading{linkage, true, "async_custom_loading",
                                                 Category::Utility};

    // Audio
    bool audio_muted;
    SwitchableSetting<AudioEmulation> audio_emulation{linkage, AudioEmulation::Hle,
                                                      "audio_emulation", Category::Audio};
    SwitchableSetting<bool> enable_audio_stretching{linkage, true, "enable_audio_stretching",
                                                    Category::Audio};
    SwitchableSetting<float, true> volume{linkage, 1.f, 0.f, 1.f, "volume", Category::Audio};
    Setting<AudioEngine> output_type{linkage, AudioEngine::Auto, "output_type", Category::Audio};
    Setting<std::string> output_device{linkage, "auto", "output_device", Category::Audio};
    Setting<AudioInputType> input_type{linkage, AudioInputType::Auto, "input_type",
                                       Category::Audio};
    Setting<std::string> input_device{linkage, "auto", "input_device", Category::Audio};

    // Camera
    std::array<std::string, CAMERA_COUNT> camera_name;
    std::array<std::string, CAMERA_COUNT> camera_config;
    std::array<int, CAMERA_COUNT> camera_flip;

    // Debugging
    bool record_frame_times;
    std::unordered_map<std::string, bool> lle_modules;
    Setting<bool> use_gdbstub{linkage, false, "use_gdbstub", Category::Debugging};
    Setting<u16> gdbstub_port{linkage, 24689, "gdbstub_port", Category::Debugging};

    // Miscellaneous
    Setting<std::string> log_filter{linkage, "*:Info", "log_filter", Category::Miscellaneous};

    // Video Dumping
    std::string output_format;
    std::string format_options;

    std::string video_encoder;
    std::string video_encoder_options;
    u64 video_bitrate;

    std::string audio_encoder;
    std::string audio_encoder_options;
    u64 audio_bitrate;
};

extern Values values;

const char* TranslateCategory(Category category);

bool IsConfiguringGlobal();
void SetConfiguringGlobal(bool is_global);

float Volume();

void LogSettings();

// Restore the global state of all applicable settings in the Values struct
void RestoreGlobalState(bool is_powered_on);

// Input profiles
void LoadProfile(int index);
void SaveProfile(int index);
void CreateProfile(std::string name);
void DeleteProfile(int index);
void RenameCurrentProfile(std::string new_name);

} // namespace Settings
