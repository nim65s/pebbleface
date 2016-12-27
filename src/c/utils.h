#ifndef __utils__
#define __utils__

#include <pebble.h>

char * strtok(char *s, const char *delim);

void battery_update_proc(Layer *layer, GContext *ctx);
void battery_callback(BatteryChargeState state);

void update_time(TextLayer *layer);

#endif
