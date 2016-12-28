#include "utils.h"

int s_battery_level;

void update_time(TextLayer *layer) {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), "%H:%M", tick_time);
  text_layer_set_text(layer, s_buffer);
}

void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int width = (s_battery_level * bounds.size.w) / 100;
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, s_battery_level > 20 ? GColorGreen : GColorRed);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}

void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
}
