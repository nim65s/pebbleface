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

// Layers
static TextLayer *desc_layer, *temp_layer, *rain_layer, *wind_layer,
                 *date_layer, *sunh_layer, *sunm_layer, *time_layer;
static TextLayer *calendar_layers[CAL_LINES];
static Layer *battery_layer;

// Buffers
static char desc_b[32], temp_b[8], rain_b[8], wind_b[8], date_b[8], sunm_b[8],
            sunh_b[8], cal_b[CAL_LINES][32];

static void weather_tick_handler() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_uint8(iter, 0, 0);
  app_message_outbox_send();
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(time_layer);
  if (update || tick_time->tm_min == 0) {
    weather_tick_handler();
    if (update || tick_time->tm_hour == 0) {
      snprintf(date_b, sizeof(date_b), "%d", tick_time->tm_day);
      text_layer_set_text(date_layer, date_b);
    }
  }
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Time
  time_layer = text_layer_create(GRect(0, 10, bounds.size.w, 50));
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
  text_layer_set_text(temp_layer, "");
  text_layer_set_font(temp_layer, source_code_pro);

  // Rain
  rain_layer = text_layer_create(GRect(0, BOX_MID, BOX_W, BOX_H));
  text_layer_set_background_color(rain_layer, GColorBlack);
  text_layer_set_text_color(rain_layer, GColorGreen);
  text_layer_set_text_alignment(rain_layer, GTextAlignmentLeft);
  text_layer_set_text(rain_layer, "");
  text_layer_set_font(rain_layer, source_code_pro);

  // Wind
  wind_layer = text_layer_create(GRect(0, BOX_BOT, BOX_W, BOX_H));
  text_layer_set_background_color(wind_layer, GColorBlack);
  text_layer_set_text_color(wind_layer, GColorGreen);
  text_layer_set_text_alignment(wind_layer, GTextAlignmentRight);
  text_layer_set_text(wind_layer, "");
  text_layer_set_font(wind_layer, source_code_pro);

  // Date
  date_layer = text_layer_create(GRect(BOX_R, BOX_TOP, BOX_W, BOX_H));
  text_layer_set_background_color(date_layer, GColorBlack);
  text_layer_set_text_color(date_layer, GColorGreen);
  text_layer_set_text_alignment(date_layer, GTextAlignmentLeft);
  text_layer_set_text(date_layer, "");
  text_layer_set_font(date_layer, source_code_pro);

  // Sun Hour
  sunh_layer = text_layer_create(GRect(BOX_R, BOX_MID, BOX_W, BOX_H));
  text_layer_set_background_color(sunh_layer, GColorBlack);
  text_layer_set_text_color(sunh_layer, GColorGreen);
  text_layer_set_text_alignment(sunh_layer, GTextAlignmentLeft);
  text_layer_set_text(sunh_layer, "");
  text_layer_set_font(sunh_layer, source_code_pro);

  // Sun Min
  sunm_layer = text_layer_create(GRect(BOX_R, BOX_BOT, BOX_W, BOX_H));
  text_layer_set_background_color(sunm_layer, GColorBlack);
  text_layer_set_text_color(sunm_layer, GColorGreen);
  text_layer_set_text_alignment(sunm_layer, GTextAlignmentRight);
  text_layer_set_text(sunm_layer, "");
  text_layer_set_font(sunm_layer, source_code_pro);

  // Description
  desc_layer = text_layer_create(GRect(0, 4, bounds.size.w, 16));
  text_layer_set_background_color(desc_layer, GColorBlack);
  text_layer_set_text_color(desc_layer, GColorWhite);
  text_layer_set_text_alignment(desc_layer, GTextAlignmentCenter);
  text_layer_set_text(desc_layer, "");
  text_layer_set_font(desc_layer, source_code_pro);

  // Battery
  battery_layer = layer_create(GRect(0, 0, bounds.size.w, 5));
  layer_set_update_proc(battery_layer, battery_update_proc);

  // Calendar
  for (int i=0; i<CAL_LINES; i++) {
    calendar_layers[i] = text_layer_create(GRect(0, 60 + BOX_H * i, bounds.size.w, BOX_H));
    text_layer_set_background_color(calendar_layers[i], GColorBlack);
    text_layer_set_text_color(calendar_layers[i], GColorWhite);
    text_layer_set_text_alignment(calendar_layers[i], GTextAlignmentLeft);
    text_layer_set_font(calendar_layers[i], source_code_pro);
    text_layer_set_text(calendar_layers[i], "");
    layer_add_child(window_layer, text_layer_get_layer(calendar_layers[i]));
  }

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  layer_add_child(window_layer, text_layer_get_layer(temp_layer));
  layer_add_child(window_layer, text_layer_get_layer(rain_layer));
  layer_add_child(window_layer, text_layer_get_layer(wind_layer));
  layer_add_child(window_layer, text_layer_get_layer(date_layer));
  layer_add_child(window_layer, text_layer_get_layer(sunh_layer));
  layer_add_child(window_layer, text_layer_get_layer(sunm_layer));
  layer_add_child(window_layer, text_layer_get_layer(desc_layer));
  layer_add_child(window_layer, battery_layer);

  // update meter
  layer_mark_dirty(battery_layer);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(time_layer);
  text_layer_destroy(temp_layer);
  text_layer_destroy(rain_layer);
  text_layer_destroy(wind_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(sunh_layer);
  text_layer_destroy(sunm_layer);
  text_layer_destroy(desc_layer);
  for (int i=0; i<CAL_LINES; i++) text_layer_destroy(calendar_layers[i]);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read tuples for data
  Tuple *temp_t = dict_find(iterator, MESSAGE_KEY_T);
  Tuple *rain_t = dict_find(iterator, MESSAGE_KEY_R);
  Tuple *wind_t = dict_find(iterator, MESSAGE_KEY_W);
  Tuple *sunh_t = dict_find(iterator, MESSAGE_KEY_H);
  Tuple *sunm_t = dict_find(iterator, MESSAGE_KEY_M);
  Tuple *desc_t = dict_find(iterator, MESSAGE_KEY_D);

  snprintf(temp_b, sizeof(temp_b), "%d", (int)temp_t->value->int32);
  text_layer_set_text(temp_layer, temp_b);
  snprintf(rain_b, sizeof(rain_b), "%d", (int)rain_t->value->int32);
  text_layer_set_text(rain_layer, rain_b);
  snprintf(wind_b, sizeof(wind_b), "%s", wind_t->value->cstring);
  text_layer_set_text(wind_layer, wind_b);
  snprintf(sunh_b, sizeof(sunh_b), "%d", (int)sunh_t->value->int32);
  text_layer_set_text(sunh_layer, sunh_b);
  snprintf(sunm_b, sizeof(sunm_b), "%d", (int)sunm_t->value->int32);
  text_layer_set_text(sunm_layer, sunm_b);
  snprintf(desc_b, sizeof(desc_b), "%s", desc_t->value->cstring);
  text_layer_set_text(desc_layer, desc_b);

  for (int i=0; i<CAL_LINES; i++) {
    Tuple *cal_t = dict_find(iterator, 10000 + i);
    if (cal_t) {
      snprintf(cal_b[i], sizeof(cal_b[i]), "%s", cal_t->value->cstring);
      text_layer_set_text(calendar_layers[i], cal_b[i]);
    } else {
      text_layer_set_text(calendar_layers[i], "");
    }
  }
  update = false;
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  snprintf(desc_b, sizeof(desc_b), "dropped %d", reason);
  text_layer_set_text(desc_layer, desc_b);
  update = true;
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  snprintf(desc_b, sizeof(desc_b), "failed %d", reason);
  text_layer_set_text(desc_layer, desc_b);
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
  const int inbox_size = 384;
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
