#pragma once

// Keep glyphs as string literals so they can be concatenated at compile time.
// Use GLYPH(...) when a single-byte character value is needed instead.

#define GLYPH(s) ((s)[0])

#define char_battery_left_s "\x80"
#define char_battery_full_s "\x81"
#define char_battery_half_s "\x82"
#define char_battery_empty_s "\x83"
#define char_symbol_charging_s "\xC6"
#define char_battery_charging1_s "\x84"
#define char_battery_charging2_s "\x85"
#define char_battery_right_s "\x86"

#define char_symbol_loop_s "\x87"
#define char_symbol_bpm_s "\x88"
#define char_symbol_volume_s "\x89"
#define char_symbol_muted_s "\x8A"
#define char_symbol_indicatorEmpty_s "\x8B"
#define char_symbol_indicatorFull_s "\x8C"

#define char_playback_pause_s "\x8C"
#define char_playback_play_s "\x90"

#define char_indicator_position_s "\x90"
#define char_indicator_positionMuted_s "\x91"
#define char_indicator_up_s "\x92"
#define char_indicator_down_s "\x93"
#define char_indicator_rightNoLeft_s "\x94"
#define char_indicator_leftNoRight_s "\x95"
#define char_indicator_leftRight_s "\x96"
#define char_indicator_load_s "\xED"
#define char_indicator_save_s "\xEE"
#define char_indicator_ellipsis_s "\xEF"

#define char_bargraph_bar0_s " "
#define char_bargraph_bar1_s "\xA1"
#define char_bargraph_bar2_s "\xA2"
#define char_bargraph_bar3_s "\xA3"
#define char_bargraph_bar4_s "\xA4"
#define char_bargraph_bar5_s "\xA5"
#define char_bargraph_bar6_s "\xA6"
#define char_bargraph_bar7_s "\xA7"
#define char_bargraph_bar8_s "\xA8"
#define char_bargraph_bar9_s "\xA8"
#define char_bargraph_bar10_s "\xDB"

#define char_key_right_s "\x97"
#define char_key_up_s "\x98"
#define char_key_down_s "\x99"
#define char_key_left_s "\x9A"
#define char_key_play_s "\x9B"
#define char_key_enter_s "\x9C"
#define char_key_alt_s "\x9D"
#define char_key_nav_s "\x9E"
#define char_key_edit_s "\x9F"

#define char_block_light_s "\xB0"
#define char_block_medium_s "\xB1"
#define char_block_dark_s "\xB2"
#define char_block_top_s "\xDF"
#define char_block_bottom_s "\xDC"
#define char_block_left_s "\xDE"
#define char_block_right_s "\xDD"
#define char_block_full_s "\xDB"
#define char_block_cutTriangleTop_s "\x9C"
#define char_block_cutTriangleRight_s "\x9D"
#define char_block_cutTriangleBottom_s "\x9E"
#define char_block_cutTriangleLeft_s "\x9F"
#define char_block_triangleTopRight_s "\xFC"
#define char_block_triangleTopLeft_s "\xFD"
#define char_block_triangleBottomRight_s "\xFE"
#define char_block_triangleBottomLeft_s "\xFF"
#define char_block_topLeftCorner_s "\xE4"
#define char_block_topRightCorner_s "\xE7"
#define char_block_bottomLeftCorner_s "\xE5"
#define char_block_bottomRightCorner_s "\xE6"

#define char_button_border_left_s "\x8E"
#define char_button_border_right_s "\x8F"

#define char_button_right_s "\x97"
#define char_button_up_s "\x98"
#define char_button_down_s "\x99"
#define char_button_left_s "\x9A"
#define char_button_play_s "\x9B"
#define char_button_enter_s "\x9C"
#define char_button_alt_s "\x9D"
#define char_button_nav_s "\x9E"
#define char_button_edit_s "\x9F"

#define char_filledBorder_bottom_s "\xDF"
#define char_filledBorder_top_s "\xDC"
#define char_filledBorder_right_s "\xDE"
#define char_filledBorder_left_s "\xDD"
#define char_filledBorder_topLeftCorner_s "\xE0"
#define char_filledBorder_topRightCorner_s "\xE3"
#define char_filledBorder_bottomLeftCorner_s "\xE1"
#define char_filledBorder_bottomRightCorner_s "\xE2"

#define char_waveform_sine1_s "\xF0"
#define char_waveform_sine2_s "\xF1"
#define char_waveform_saw1_s "\xF2"
#define char_waveform_saw2_s "\xF3"
#define char_waveform_pulse1_s "\xF4"
#define char_waveform_pulse2_s "\xF5"
#define char_waveform_tri1_s "\xF6"
#define char_waveform_tri2_s "\xF7"
#define char_waveform_noise1_s "\xF8"
#define char_waveform_noise2_s "\xF9"

#define char_waveform_tri_s char_waveform_tri1_s char_waveform_tri2_s
#define char_waveform_saw_s char_waveform_saw1_s char_waveform_saw2_s
#define char_waveform_pulse_s char_waveform_pulse1_s char_waveform_pulse2_s
#define char_waveform_noise_s char_waveform_noise1_s char_waveform_noise2_s

#define char_border_single_topLeft_s "\xDA"
#define char_border_single_topRight_s "\xBF"
#define char_border_single_bottomLeft_s "\xC0"
#define char_border_single_bottomRight_s "\xD9"
#define char_border_single_horizontal_s "\xC4"
#define char_border_single_vertical_s "\xB3"
#define char_border_single_horizontalUp_s "\xC1"
#define char_border_single_horizontalDown_s "\xC2"
#define char_border_single_verticalLeft_s "\xB4"
#define char_border_single_verticalRight_s "\xC3"
#define char_border_single_cross_s "\xC5"

#define char_line_2_s                                                          \
  char_border_single_horizontal_s char_border_single_horizontal_s
#define char_line_3_s char_line_2_s char_border_single_horizontal_s
#define char_line_4_s char_line_3_s char_border_single_horizontal_s
#define char_line_5_s char_line_4_s char_border_single_horizontal_s
#define char_line_6_s char_line_5_s char_border_single_horizontal_s
#define char_line_7_s char_line_6_s char_border_single_horizontal_s
#define char_line_8_s char_line_7_s char_border_single_horizontal_s
#define char_line_9_s char_line_8_s char_border_single_horizontal_s
#define char_line_10_s char_line_9_s char_border_single_horizontal_s
#define char_line_11_s char_line_10_s char_border_single_horizontal_s

#define char_border_double_topLeft_s "\xC9"
#define char_border_double_topRight_s "\xBB"
#define char_border_double_bottomLeft_s "\xC8"
#define char_border_double_bottomRight_s "\xBC"
#define char_border_double_horizontal_s "\xCD"
#define char_border_double_vertical_s "\xBA"
#define char_border_double_horizontalUp_s "\xCA"
#define char_border_double_horizontalDown_s "\xCB"
#define char_border_double_verticalLeft_s "\xB9"
#define char_border_double_verticalRight_s "\xCC"
#define char_border_double_cross_s "\xCE"

#define char_logo_x_s "\xD0"
#define char_logo_xiponics_0_s "\xD1"
#define char_logo_xiponics_1_s "\xD2"
#define char_logo_xiponics_2_s "\xD3"
#define char_logo_xiponics_3_s "\xD4"
#define char_logo_xiponics_4_s "\xD5"
#define char_logo_xiponics_5_s "\xD6"
#define char_logo_xiponics_6_s "\xD7"

#define char_logo_string_s                                                     \
  char_logo_x_s char_logo_xiponics_0_s char_logo_xiponics_1_s                  \
      char_logo_xiponics_2_s char_logo_xiponics_3_s char_logo_xiponics_4_s     \
          char_logo_xiponics_5_s char_logo_xiponics_6_s

#define string_battery_charging                                                \
  char_battery_left_s char_battery_charging1_s char_battery_charging2_s        \
      char_battery_right_s
#define string_battery_100_percent                                             \
  char_battery_left_s char_battery_full_s char_battery_full_s                  \
      char_battery_right_s
#define string_battery_75_percent                                              \
  char_battery_left_s char_battery_full_s char_battery_half_s                  \
      char_battery_right_s
#define string_battery_50_percent                                              \
  char_battery_left_s char_battery_full_s char_battery_empty_s                 \
      char_battery_right_s
#define string_battery_25_percent                                              \
  char_battery_left_s char_battery_half_s char_battery_empty_s                 \
      char_battery_right_s
#define string_battery_0_percent                                               \
  char_battery_left_s char_battery_empty_s char_battery_empty_s                \
      char_battery_right_s

// Array of bargraph characters for fast lookup
static const char *const char_bargraph_lookup[] = {
    char_bargraph_bar0_s, // 0 - comments just to keep format linter happy
    char_bargraph_bar1_s, // 1
    char_bargraph_bar2_s, // 2
    char_bargraph_bar3_s, // 3
    char_bargraph_bar4_s, // 4
    char_bargraph_bar5_s, // 5
    char_bargraph_bar6_s, // 6
    char_bargraph_bar7_s, // 7
    char_bargraph_bar8_s, // 8
    char_bargraph_bar9_s, // 9
    char_bargraph_bar10_s // 10
};

#define char_bargraph_s(x)                                                     \
  (char_bargraph_lookup[(x) < 0 ? 0 : ((x) > 10 ? 10 : (x))])

// progress bar parts
#define char_propgress_bar_0_s "\xEC"
#define char_propgress_bar_1_s "\xE8"
#define char_propgress_bar_2_s "\xE9"
#define char_propgress_bar_3_s "\xEA"
#define char_propgress_bar_4_s "\xEB"
