#ifndef __utils__
#define __utils__

#include <pebble.h>

void battery_update_proc(Layer *layer, GContext *ctx);
void battery_callback(BatteryChargeState state);

void update_time(TextLayer *layer);

#endif
