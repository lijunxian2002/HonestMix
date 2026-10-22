// ═══════════════════════════════════════════════════════════
// DesignTokens.h —— 由 tools/gen_tokens.py 自动生成，勿手改！
// 来源: design-tokens.md
// 修改方式: 改 design-tokens.md → 重跑生成器
// ═══════════════════════════════════════════════════════════
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace hm::tokens
{
    // ── 颜色令牌 ──
    inline juce::Colour bg_plugin_top() { return juce::Colour::fromRGB(17, 18, 24); }
    inline juce::Colour bg_plugin_mid() { return juce::Colour::fromRGB(16, 17, 22); }
    inline juce::Colour bg_plugin_bottom() { return juce::Colour::fromRGB(14, 15, 19); }
    inline juce::Colour bg_body() { return juce::Colour::fromRGB(26, 26, 24); }
    inline juce::Colour bg_vu_panel() { return juce::Colour::fromRGB(17, 18, 21); }
    inline juce::Colour bg_vu_track() { return juce::Colour::fromRGB(13, 14, 18); }
    inline juce::Colour bg_curve_panel() { return juce::Colour::fromRGBA(0, 0, 0, 107); }
    inline juce::Colour bg_chip() { return juce::Colour::fromRGBA(0, 0, 0, 71); }
    inline juce::Colour bg_btn() { return juce::Colour::fromRGBA(0, 0, 0, 51); }
    inline juce::Colour bg_overlay() { return juce::Colour::fromRGBA(14, 16, 20, 247); }
    inline juce::Colour bg_dropdown() { return juce::Colour::fromRGBA(18, 20, 26, 252); }
    inline juce::Colour bg_tooltip() { return juce::Colour::fromRGBA(26, 26, 24, 245); }
    inline juce::Colour bg_overlay_btn() { return juce::Colour::fromRGBA(255, 255, 255, 8); }
    inline juce::Colour bg_overlay_btn_hover() { return juce::Colour::fromRGBA(255, 255, 255, 15); }
    inline juce::Colour bg_overlay_close() { return juce::Colour::fromRGBA(255, 255, 255, 10); }
    inline juce::Colour bg_overlay_close_hover() { return juce::Colour::fromRGBA(255, 255, 255, 20); }
    inline juce::Colour bg_tap_active() { return juce::Colour::fromRGBA(200, 160, 80, 38); }
    inline juce::Colour bg_drop_item_hover() { return juce::Colour::fromRGBA(255, 255, 255, 10); }
    inline juce::Colour text_brand() { return juce::Colour::fromRGBA(245, 245, 240, 153); }
    inline juce::Colour text_chip_input() { return juce::Colour::fromRGBA(240, 240, 235, 173); }
    inline juce::Colour text_chip_input_focus() { return juce::Colour::fromRGBA(230, 230, 225, 178); }
    inline juce::Colour text_chip_placeholder() { return juce::Colour::fromRGBA(210, 210, 205, 76); }
    inline juce::Colour text_chip_label() { return juce::Colour::fromRGBA(235, 235, 230, 115); }
    inline juce::Colour text_drop_item() { return juce::Colour::fromRGBA(235, 235, 230, 158); }
    inline juce::Colour text_drop_item_hover() { return juce::Colour::fromRGBA(230, 230, 225, 204); }
    inline juce::Colour text_drop_brand() { return juce::Colour::fromRGBA(160, 160, 160, 51); }
    inline juce::Colour text_btn() { return juce::Colour::fromRGBA(235, 235, 230, 97); }
    inline juce::Colour text_btn_accent() { return juce::Colour::fromRGBA(255, 255, 250, 128); }
    inline juce::Colour text_overlay_title() { return juce::Colour::fromRGBA(200, 200, 200, 89); }
    inline juce::Colour text_overlay_body() { return juce::Colour::fromRGBA(200, 200, 200, 64); }
    inline juce::Colour text_overlay_btn() { return juce::Colour::fromRGBA(200, 200, 200, 76); }
    inline juce::Colour text_overlay_btn_hover() { return juce::Colour::fromRGBA(255, 255, 255, 128); }
    inline juce::Colour text_overlay_close() { return juce::Colour::fromRGBA(200, 200, 200, 76); }
    inline juce::Colour text_overlay_close_hover() { return juce::Colour::fromRGBA(255, 255, 255, 128); }
    inline juce::Colour text_bpm_big() { return juce::Colour::fromRGBA(220, 220, 215, 178); }
    inline juce::Colour text_bpm_btn() { return juce::Colour::fromRGBA(200, 200, 200, 76); }
    inline juce::Colour text_bpm_btn_hover() { return juce::Colour::fromRGBA(255, 255, 255, 128); }
    inline juce::Colour text_tap() { return juce::Colour::fromRGBA(200, 200, 200, 76); }
    inline juce::Colour text_tap_hover() { return juce::Colour::fromRGBA(255, 255, 255, 128); }
    inline juce::Colour text_tap_active() { return juce::Colour::fromRGBA(220, 180, 100, 153); }
    inline juce::Colour text_table_cell() { return juce::Colour::fromRGBA(200, 200, 200, 89); }
    inline juce::Colour text_table_header() { return juce::Colour::fromRGBA(200, 200, 200, 128); }
    inline juce::Colour text_table_value() { return juce::Colour::fromRGBA(200, 180, 150, 102); }
    inline juce::Colour text_tooltip_tag() { return juce::Colour::fromRGBA(200, 200, 200, 89); }
    inline juce::Colour text_tooltip_hz() { return juce::Colour::fromRGBA(220, 220, 215, 217); }
    inline juce::Colour text_tooltip_db() { return juce::Colour::fromRGBA(235, 235, 230, 158); }
    inline juce::Colour text_curve_label() { return juce::Colour::fromRGBA(210, 225, 248, 191); }
    inline juce::Colour text_fader_label() { return juce::Colour::fromRGBA(200, 190, 160, 102); }
    inline juce::Colour border_plugin() { return juce::Colour::fromRGBA(20, 22, 26, 242); }
    inline juce::Colour border_cockpit() { return juce::Colour::fromRGBA(16, 18, 22, 230); }
    inline juce::Colour border_topbar_bottom() { return juce::Colour::fromRGBA(255, 255, 255, 10); }
    inline juce::Colour border_bottom_row_top() { return juce::Colour::fromRGBA(255, 255, 255, 8); }
    inline juce::Colour border_vu() { return juce::Colour::fromRGBA(0, 0, 0, 128); }
    inline juce::Colour border_curve() { return juce::Colour::fromRGBA(80, 78, 74, 128); }
    inline juce::Colour border_curve_dim() { return juce::Colour::fromRGBA(80, 78, 74, 38); }
    inline juce::Colour border_chip() { return juce::Colour::fromRGBA(0, 0, 0, 71); }
    inline juce::Colour border_fader_track() { return juce::Colour::fromRGBA(0, 0, 0, 107); }
    inline juce::Colour border_fader_cap() { return juce::Colour::fromRGBA(20, 18, 15, 128); }
    inline juce::Colour border_btn() { return juce::Colour::fromRGBA(0, 0, 0, 107); }
    inline juce::Colour border_dropdown() { return juce::Colour::fromRGBA(255, 255, 255, 20); }
    inline juce::Colour border_tooltip() { return juce::Colour::fromRGBA(255, 255, 255, 26); }
    inline juce::Colour border_overlay_btn() { return juce::Colour::fromRGBA(255, 255, 255, 20); }
    inline juce::Colour border_overlay_btn_sm() { return juce::Colour::fromRGBA(255, 255, 255, 15); }
    inline juce::Colour border_overlay_btn_hover() { return juce::Colour::fromRGBA(255, 255, 255, 20); }
    inline juce::Colour border_cockpit_inner() { return juce::Colour::fromRGBA(255, 255, 255, 2); }
    inline juce::Colour border_table_row() { return juce::Colour::fromRGBA(255, 255, 255, 8); }
    inline juce::Colour bolt_grad_center() { return juce::Colour::fromRGBA(150, 155, 165, 102); }
    inline juce::Colour bolt_grad_edge() { return juce::Colour::fromRGBA(45, 50, 55, 191); }
    inline juce::Colour bolt_mid_grad_center() { return juce::Colour::fromRGBA(130, 135, 145, 76); }
    inline juce::Colour bolt_mid_grad_edge() { return juce::Colour::fromRGBA(40, 45, 50, 166); }
    inline juce::Colour bolt_shadow_dark() { return juce::Colour::fromRGBA(0, 0, 0, 107); }
    inline juce::Colour bolt_shadow_ring() { return juce::Colour::fromRGBA(0, 0, 0, 51); }
    inline juce::Colour deco_centerline() { return juce::Colour::fromRGBA(255, 255, 255, 2); }
    inline juce::Colour deco_hline() { return juce::Colour::fromRGBA(255, 255, 255, 1); }
    inline juce::Colour breath_color() { return juce::Colour::fromRGBA(255, 255, 255, 230); }
    inline juce::Colour breath_glow() { return juce::Colour::fromRGBA(200, 220, 255, 153); }
    inline juce::Colour chase_star_color() { return juce::Colour::fromRGBA(180, 220, 255, 230); }
    inline juce::Colour chase_star_glow_1() { return juce::Colour::fromRGBA(150, 200, 255, 153); }
    inline juce::Colour chase_star_glow_2() { return juce::Colour::fromRGBA(120, 180, 240, 76); }
    inline juce::Colour st_dot_default() { return juce::Colour::fromRGBA(100, 98, 94, 178); }
    inline juce::Colour st_dot_glow_default() { return juce::Colour::fromRGBA(80, 78, 74, 76); }
    inline juce::Colour vu_fill_glow_base() { return juce::Colour::fromRGBA(80, 200, 110, 51); }

    // ── 数值令牌（间距/圆角/字号）──
    inline constexpr int spacing_xxs = 2;
    inline constexpr int spacing_xs = 3;
    inline constexpr int spacing_sm = 6;
    inline constexpr int spacing_md = 8;
    inline constexpr int spacing_lg = 10;
    inline constexpr int spacing_xl = 12;
    inline constexpr int spacing_xxl = 18;
    inline constexpr int pad_cockpit = 8;
    inline constexpr int radius_xs = 2;
    inline constexpr int radius_sm = 3;
    inline constexpr int radius_s = 4;
    inline constexpr int radius_m = 5;
    inline constexpr int radius_md = 6;
    inline constexpr int radius_ml = 8;
    inline constexpr int radius_lg = 10;
    inline constexpr int design_width = 600;

    // ── 内边距令牌（每方向独立常量）──
    inline constexpr int pad_plugin_top = 22;
    inline constexpr int pad_plugin_right = 20;
    inline constexpr int pad_plugin_bottom = 16;
    inline constexpr int pad_plugin_left = 20;
    inline constexpr int pad_vu_top = 6;
    inline constexpr int pad_vu_right = 10;
    inline constexpr int pad_vu_bottom = 10;
    inline constexpr int pad_vu_left = 10;
    inline constexpr int pad_chip_top = 9;
    inline constexpr int pad_chip_right = 6;
    inline constexpr int pad_chip_bottom = 7;
    inline constexpr int pad_chip_left = 6;
    inline constexpr int pad_btn_top = 9;
    inline constexpr int pad_btn_right = 6;
    inline constexpr int pad_btn_bottom = 6;
    inline constexpr int pad_btn_left = 6;
    inline constexpr int pad_overlay_top = 20;
    inline constexpr int pad_overlay_right = 24;
    inline constexpr int pad_overlay_bottom = 24;
    inline constexpr int pad_overlay_left = 24;
    inline constexpr int pad_overlay_btn_lg_top = 10;
    inline constexpr int pad_overlay_btn_lg_right = 16;
    inline constexpr int pad_overlay_btn_lg_bottom = 16;
    inline constexpr int pad_overlay_btn_lg_left = 16;
    inline constexpr int pad_overlay_btn_sm_top = 8;
    inline constexpr int pad_overlay_btn_sm_right = 6;
    inline constexpr int pad_overlay_btn_sm_bottom = 6;
    inline constexpr int pad_overlay_btn_sm_left = 6;
    inline constexpr int pad_drop_item_top = 4;
    inline constexpr int pad_drop_item_right = 10;
    inline constexpr int pad_drop_item_bottom = 4;
    inline constexpr int pad_drop_item_left = 16;
    inline constexpr int pad_drop_item_flat_top = 4;
    inline constexpr int pad_drop_item_flat_right = 10;
    inline constexpr int pad_drop_item_flat_bottom = 10;
    inline constexpr int pad_drop_item_flat_left = 10;
    inline constexpr int pad_drop_brand_top = 4;
    inline constexpr int pad_drop_brand_right = 10;
    inline constexpr int pad_drop_brand_bottom = 1;
    inline constexpr int pad_drop_brand_left = 10;
    inline constexpr int pad_table_cell_top = 5;
    inline constexpr int pad_table_cell_right = 8;
    inline constexpr int pad_table_cell_bottom = 8;
    inline constexpr int pad_table_cell_left = 8;
    inline constexpr int pad_tooltip_top = 3;
    inline constexpr int pad_tooltip_right = 7;
    inline constexpr int pad_tooltip_bottom = 7;
    inline constexpr int pad_tooltip_left = 7;

    // ── 字号令牌（来自字号与字距表）──
    inline constexpr float fs_text_brand = 9.0f;
    inline constexpr float fs_text_chip_label = 6.0f;
    inline constexpr float fs_text_chip_input = 9.0f;
    inline constexpr float fs_text_drop_brand = 7.0f;
    inline constexpr float fs_text_drop_item = 8.0f;
    inline constexpr float fs_text_btn = 7.0f;
    inline constexpr float fs_text_overlay_title = 11.0f;
    inline constexpr float fs_text_overlay_body = 9.0f;
    inline constexpr float fs_text_overlay_btn = 9.0f;
    inline constexpr float fs_text_bpm_big = 36.0f;
    inline constexpr float fs_text_bpm_btn = 16.0f;
    inline constexpr float fs_text_table_cell = 9.0f;
    inline constexpr float fs_text_table_header = 9.0f;
    inline constexpr float fs_text_table_value = 9.0f;
    inline constexpr float fs_text_curve_label = 5.0f;
    inline constexpr float fs_text_tooltip_tag = 7.0f;
    inline constexpr float fs_text_tooltip_hz = 9.0f;
    inline constexpr float fs_text_tooltip_db = 9.0f;
    inline constexpr float fs_text_fader_label = 12.0f;

    // ── 过渡时长令牌（ms + 60fps lerp 因子）──
    // lerp 因子公式: 1-exp(ln(0.05)/(秒*60)) —— CSS transition → 帧循环
    inline constexpr int trans_instant_ms = 80;
    inline constexpr float trans_instant_lerp = 0.4643f;
    inline constexpr int trans_fast_ms = 150;
    inline constexpr float trans_fast_lerp = 0.2831f;
    inline constexpr int trans_normal_ms = 200;
    inline constexpr float trans_normal_lerp = 0.2209f;
    inline constexpr int trans_slow_ms = 300;
    inline constexpr float trans_slow_lerp = 0.1533f;
    inline constexpr int trans_curve_border_ms = 800;
    inline constexpr float trans_curve_border_lerp = 0.0605f;
    inline constexpr int trans_stars_enter_ms = 800;
    inline constexpr float trans_stars_enter_lerp = 0.0605f;
    inline constexpr int trans_stars_exit_ms = 1500;
    inline constexpr float trans_stars_exit_lerp = 0.0327f;
    inline constexpr int trans_star_form_ms = 3500;
    inline constexpr float trans_star_form_lerp = 0.0142f;
    inline constexpr int trans_curve_label_ms = 400;
    inline constexpr float trans_curve_label_lerp = 0.1173f;
    inline constexpr int trans_curve_label_delay_ms = 618;
    inline constexpr float trans_curve_label_delay_lerp = 0.0776f;

    // ── 星空粒子参数（§8.3 程序生成）──
    inline constexpr int star_count = 45;
    inline constexpr float star_size_min = 0.3f /* px */;
    inline constexpr float star_size_max = 1.6f /* px */;
    inline constexpr float star_opacity_min = 0.05f;
    inline constexpr float star_opacity_max = 0.23f;
    inline juce::Colour star_color() { return juce::Colours::white; }
    inline constexpr float breath_period = 1978.0f /* ms */;
    inline constexpr float breath_rise_ratio = 0.382f;
    inline constexpr float breath_min_brightness = 0.44f;
    inline constexpr float breath_max_brightness = 1.44f;
    inline constexpr float chase_speed = 8e-05f;
    inline constexpr float chase_speed_dist = 0.02f;
    inline constexpr float chase_damping = 0.95f;
    inline constexpr float chase_size = 3.0f /* px */;
    inline constexpr int heart_count = 15;
    inline constexpr float heart_cooldown = 10000.0f /* ms */;
    inline constexpr float heart_still_time = 1200.0f /* ms */;
    inline constexpr float parallax_x = 6.0f /* px */;
    inline constexpr float parallax_y = 4.0f /* px */;
    inline constexpr float parallax_enter_duration = 800.0f /* ms */;
    inline constexpr float parallax_exit_duration = 1500.0f /* ms */;

    // ── 阴影组（box-shadow 逐层）──
    struct ShadowLayer { float dx, dy, blur, spread; juce::Colour col; bool inset; };
    inline const ShadowLayer plugin_shadow[6] = {
        {0.0f, 0.0f, 0.0f, 5.0f, juce::Colour::fromRGBA(45, 50, 58, 20), false},
        {0.0f, 0.0f, 0.0f, 4.0f, juce::Colour::fromRGBA(14, 15, 18, 178), false},
        {0.0f, 0.0f, 0.0f, 1.0f, juce::Colour::fromRGBA(55, 60, 68, 38), false},
        {0.0f, 0.0f, 0.0f, 1.0f, juce::Colour::fromRGBA(50, 55, 62, 38), false},
        {0.0f, 8.0f, 21.0f, 0.0f, juce::Colour::fromRGBA(0, 0, 0, 107), false},
        {0.0f, 3.0f, 5.0f, 0.0f, juce::Colour::fromRGBA(0, 0, 0, 107), false},
    };
    inline const ShadowLayer cockpit_shadow[6] = {
        {0.0f, 2.0f, 0.0f, 0.0f, juce::Colour::fromRGBA(255, 255, 255, 2), true},
        {0.0f, 0.0f, 0.0f, 6.0f, juce::Colour::fromRGBA(40, 45, 52, 26), false},
        {0.0f, 0.0f, 0.0f, 5.0f, juce::Colour::fromRGBA(12, 13, 16, 204), false},
        {0.0f, 0.0f, 0.0f, 1.0f, juce::Colour::fromRGBA(55, 60, 70, 51), false},
        {0.0f, 0.0f, 0.0f, 1.0f, juce::Colour::fromRGBA(255, 255, 255, 3), true},
        {0.0f, 8.0f, 21.0f, 0.0f, juce::Colour::fromRGBA(0, 0, 0, 107), false},
    };
    inline const ShadowLayer vu_panel_shadow[2] = {
        {0.0f, 1.0f, 0.0f, 0.0f, juce::Colour::fromRGBA(255, 255, 255, 3), true},
        {0.0f, 2.0f, 4.0f, 0.0f, juce::Colour::fromRGBA(0, 0, 0, 102), true},
    };
    inline const ShadowLayer curve_panel_shadow[2] = {
        {0.0f, 5.0f, 18.0f, 0.0f, juce::Colour::fromRGBA(0, 0, 0, 153), true},
        {0.0f, 2.0f, 0.0f, 0.0f, juce::Colour::fromRGBA(255, 255, 255, 3), false},
    };
    inline const ShadowLayer chip_shadow[2] = {
        {0.0f, 1.0f, 0.0f, 0.0f, juce::Colour::fromRGBA(255, 255, 255, 3), true},
        {0.0f, 2.0f, 4.0f, 0.0f, juce::Colour::fromRGBA(0, 0, 0, 107), true},
    };
    inline const ShadowLayer fader_cap_shadow[4] = {
        {0.0f, 1.0f, 0.0f, 0.0f, juce::Colour::fromRGBA(255, 255, 255, 10), false},
        {0.0f, 2.0f, 3.0f, 0.0f, juce::Colour::fromRGBA(0, 0, 0, 107), false},
        {0.0f, 5.0f, 8.0f, 0.0f, juce::Colour::fromRGBA(0, 0, 0, 107), false},
        {0.0f, 8.0f, 16.0f, 0.0f, juce::Colour::fromRGBA(0, 0, 0, 107), false},
    };
    inline const ShadowLayer bottom_btn_shadow[2] = {
        {0.0f, 1.0f, 0.0f, 0.0f, juce::Colour::fromRGBA(255, 255, 255, 3), true},
        {0.0f, 1.0f, 2.0f, 0.0f, juce::Colour::fromRGBA(0, 0, 0, 51), false},
    };
    inline const ShadowLayer dropdown_shadow[1] = {
        {0.0f, 8.0f, 24.0f, 0.0f, juce::Colour::fromRGBA(0, 0, 0, 178), false},
    };
    inline const ShadowLayer curve_label_glow_shadow[3] = {
        {0.0f, 0.0f, 2.0f, 0.0f, juce::Colour::fromRGBA(190, 215, 248, 153), false},
        {0.0f, 0.0f, 6.0f, 0.0f, juce::Colour::fromRGBA(150, 190, 235, 89), false},
        {0.0f, 0.0f, 12.0f, 0.0f, juce::Colour::fromRGBA(110, 160, 220, 38), false},
    };
}
