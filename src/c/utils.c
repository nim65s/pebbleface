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

char * strtok(register char *s, register const char *delim) {
  register char *spanp;
  register int c, sc;
  char *tok;
  static char *last;

  if (s == NULL && (s = last) == NULL) return (NULL);

cont:
  c = *s++;
  for (spanp = (char *)delim; (sc = *spanp++) != 0;) if (c == sc) goto cont;

  if (c == 0) {    /* no non-delimiter characters */
    last = NULL;
    return (NULL);
  }
  tok = s - 1;

  for (;;) {
    c = *s++;
    spanp = (char *)delim;
    do {
      if ((sc = *spanp++) == c) {
        if (c == 0) s = NULL;
        else s[-1] = 0;
        last = s;
        return (tok);
      }
    } while (sc != 0);
  }
}


