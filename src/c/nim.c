#include "utils.h"

#define CAL_LINES 9
#define BOX_H 12
#define BOX_W 13
#define BOX_TOP 20
#define BOX_MID (BOX_TOP + BOX_H)
#define BOX_BOT (BOX_MID + BOX_H)
#define BOX_R (144 - BOX_W)

static Window *main_window;
static GFont source_code_pro;
static bool update = true;
static char *token;

// Layers
static TextLayer *time_layer, *weather_layer, *temp_layer, *rain_layer,
                 *wind_speed_layer, *wind_dir_layer;
static TextLayer *calendar_layers[CAL_LINES];
static Layer *battery_layer;

// Buffers
static char weather_buffer[32], calendar_buffer[CAL_LINES * 32];

static void weather_tick_handler() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_uint8(iter, 0, 0);
  app_message_outbox_send();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(time_layer);
  if (update || tick_time->tm_min == 0) weather_tick_handler();
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Time
  time_layer = text_layer_create(GRect(0, 10, bounds.size.w, 48));
  text_layer_set_background_color(time_layer, GColorBlack);
  text_layer_set_text_color(time_layer, GColorCeleste);
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_text(time_layer, "00:00");
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));

  // Temperature
  temp_layer = text_layer_create(GRect(0, BOX_TOP, BOX_W, BOX_H));
  text_layer_set_background_color(temp_layer, GColorBlack);
  text_layer_set_text_color(temp_layer, GColorGreen);
  text_layer_set_text_alignment(temp_layer, GTextAlignmentLeft);
  text_layer_set_text(temp_layer, "28");
  text_layer_set_font(temp_layer, source_code_pro);

  // Rain
  rain_layer = text_layer_create(GRect(0, BOX_MID, BOX_W, BOX_H));
  text_layer_set_background_color(rain_layer, GColorBlack);
  text_layer_set_text_color(rain_layer, GColorGreen);
  text_layer_set_text_alignment(rain_layer, GTextAlignmentLeft);
  text_layer_set_text(rain_layer, "0");
  text_layer_set_font(rain_layer, source_code_pro);

  // Wind Speed
  wind_speed_layer = text_layer_create(GRect(0, BOX_BOT, BOX_W, BOX_H));
  text_layer_set_background_color(wind_speed_layer, GColorBlack);
  text_layer_set_text_color(wind_speed_layer, GColorGreen);
  text_layer_set_text_alignment(wind_speed_layer, GTextAlignmentRight);
  text_layer_set_text(wind_speed_layer, "10");
  text_layer_set_font(wind_speed_layer, source_code_pro);

  // Wind Dir
  wind_dir_layer = text_layer_create(GRect(BOX_R, BOX_TOP, BOX_W, BOX_H));
  text_layer_set_background_color(wind_dir_layer, GColorBlack);
  text_layer_set_text_color(wind_dir_layer, GColorGreen);
  text_layer_set_text_alignment(wind_dir_layer, GTextAlignmentRight);
  text_layer_set_text(wind_dir_layer, "8");
  text_layer_set_font(wind_dir_layer, source_code_pro);

  // Weather
  weather_layer = text_layer_create(GRect(0, 4, bounds.size.w, 16));
  text_layer_set_background_color(weather_layer, GColorBlack);
  text_layer_set_text_color(weather_layer, GColorWhite);
  text_layer_set_text_alignment(weather_layer, GTextAlignmentCenter);
  text_layer_set_text(weather_layer, "123456789012345678901234");
  text_layer_set_font(weather_layer, source_code_pro);

  // Battery
  battery_layer = layer_create(GRect(0, 0, bounds.size.w, 5));
  layer_set_update_proc(battery_layer, battery_update_proc);

  // Calendar
  for (int i=0; i<CAL_LINES; i++) {
    calendar_layers[i] = text_layer_create(GRect(0, 58 + BOX_H * i, bounds.size.w, BOX_H));
    text_layer_set_background_color(calendar_layers[i], GColorBlack);
    text_layer_set_text_color(calendar_layers[i], GColorWhite);
    text_layer_set_text_alignment(calendar_layers[i], GTextAlignmentLeft);
    text_layer_set_font(calendar_layers[i], source_code_pro);
    text_layer_set_text(calendar_layers[i], "abcdefghijklmnopqrstuvwxyz");
    layer_add_child(window_layer, text_layer_get_layer(calendar_layers[i]));
  }

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  layer_add_child(window_layer, text_layer_get_layer(temp_layer));
  layer_add_child(window_layer, text_layer_get_layer(rain_layer));
  layer_add_child(window_layer, text_layer_get_layer(wind_speed_layer));
  layer_add_child(window_layer, text_layer_get_layer(wind_dir_layer));
  layer_add_child(window_layer, text_layer_get_layer(weather_layer));
  layer_add_child(window_layer, battery_layer);

  // update meter
  layer_mark_dirty(battery_layer);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(time_layer);
  text_layer_destroy(temp_layer);
  text_layer_destroy(rain_layer);
  text_layer_destroy(wind_speed_layer);
  text_layer_destroy(wind_dir_layer);
  text_layer_destroy(weather_layer);
  for (int i=0; i<CAL_LINES; i++) text_layer_destroy(calendar_layers[i]);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read tuples for data
  Tuple *calendar_tuple = dict_find(iterator, MESSAGE_KEY_C);

  // If all data is available, use it
  if (calendar_tuple) {
    snprintf(calendar_buffer, sizeof(calendar_buffer), "%s",
        calendar_tuple->value->cstring);
    token = strtok(calendar_buffer, "^");
    text_layer_set_text(weather_layer, token);
    token = strtok(NULL, "^");
    for (int i=0; i<CAL_LINES; i++) {
      if (token != NULL) {
        text_layer_set_text(calendar_layers[i], token);
        token = strtok(NULL, "^");
      } else {
        text_layer_set_text(calendar_layers[i], "");
      }
    }
    update = false;
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  snprintf(weather_buffer, sizeof(weather_buffer), "dropped %d", reason);
  text_layer_set_text(weather_layer, weather_buffer);
  update = true;
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  snprintf(weather_buffer, sizeof(weather_buffer), "failed %d", reason);
  text_layer_set_text(weather_layer, weather_buffer);
  update = true;
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  source_code_pro = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SOURCE_CODE_PRO_10));
  // Create main Window element and assign to pointer
  main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(main_window, (WindowHandlers) {
      .load = main_window_load,
      .unload = main_window_unload
      });

  // Show the Window on the watch, with animated=true
  window_stack_push(main_window, true);

  // Make sure the time is displayed from the start
  update_time(time_layer);

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  battery_state_service_subscribe(battery_callback);
  battery_callback(battery_state_service_peek());

  // Open AppMessage
  const int inbox_size = 256;
  const int outbox_size = 8;
  app_message_open(inbox_size, outbox_size);
}

static void deinit() {
  layer_destroy(battery_layer);
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
