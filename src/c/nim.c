#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_weather_layer;
static GFont s_font;
static char temperature_buffer[8];
static char conditions_buffer[32];
static char weather_layer_buffer[32];
static int s_battery_level;
static Layer *s_battery_layer;


static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int width = (s_battery_level * bounds.size.w) / 100;
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, s_battery_level > 30 ? GColorGreen : GColorRed);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void weather_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, 0, 0);

  // Send the message
  app_message_outbox_send();
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(GRect(0, 60, bounds.size.w, 48));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorCeleste);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));

  // weather
  s_weather_layer = text_layer_create(GRect(0, 21, bounds.size.w, 16));
  text_layer_set_background_color(s_weather_layer, GColorBlack);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Loading...");
  text_layer_set_font(s_weather_layer, s_font);

  // battery
  s_battery_layer = layer_create(GRect(0, 0, bounds.size.w, 5));
  layer_set_update_proc(s_battery_layer, battery_update_proc);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));
  layer_add_child(window_layer, s_battery_layer);

  // update meter
  layer_mark_dirty(s_battery_layer);
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);

  // If all data is available, use it
  if (temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d",
        (int)temp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s",
        conditions_tuple->value->cstring);

    // Assemble full string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s°C, %s",
        temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
  }
}

/*static void inbox_dropped_callback(AppMessageResult reason, void *context) {*/
  /*APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");*/
/*}*/

/*static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {*/
  /*APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");*/
/*}*/

/*static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {*/
  /*APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");*/
/*}*/

static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
}

static void init() {
  s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SOURCE_CODE_PRO_10));
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
      .load = main_window_load,
      .unload = main_window_unload
      });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  tick_timer_service_subscribe(HOUR_UNIT, weather_tick_handler);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  /*app_message_register_inbox_dropped(inbox_dropped_callback);*/
  /*app_message_register_outbox_failed(outbox_failed_callback);*/
  /*app_message_register_outbox_sent(outbox_sent_callback);*/
  battery_state_service_subscribe(battery_callback);
  battery_callback(battery_state_service_peek());

  // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
}

static void deinit() {
  layer_destroy(s_battery_layer);
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
