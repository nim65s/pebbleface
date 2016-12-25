#include <pebble.h>

static Window *s_main_window;
static GFont s_font;

// Time
static TextLayer *s_time_layer;

// Weather
static TextLayer *s_weather_layer;
static char weather_buffer[32];

// Battery
static int s_battery_level;
static Layer *s_battery_layer;

// Calendar
static TextLayer* s_calendar_layers[7];
static char calendar_buffer[7 * 32];
static char *token;


char * strtok(s, delim)
  register char *s;
  register const char *delim;
{
  register char *spanp;
  register int c, sc;
  char *tok;
  static char *last;


  if (s == NULL && (s = last) == NULL) return (NULL);

cont:
  c = *s++;
  for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
    if (c == sc) goto cont;
  }

  if (c == 0) {    /* no non-delimiter characters */
    last = NULL;
    return (NULL);
  }
  tok = s - 1;

  /*
   * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
   * Note that delim must have one NUL; we stop if we see that, too.
   */
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
  /* NOTREACHED */
}


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
  strftime(s_buffer, sizeof(s_buffer), "%H:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void weather_tick_handler() {
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, 0, 0);

  // Send the message
  app_message_outbox_send();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  weather_tick_handler();
  /*if (tick_time->tm_min == 0) weather_tick_handler();*/
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Time
  s_time_layer = text_layer_create(GRect(0, 20, bounds.size.w, 52));
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorCeleste);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));

  // Weather
  s_weather_layer = text_layer_create(GRect(0, 4, bounds.size.w, 16));
  text_layer_set_background_color(s_weather_layer, GColorBlack);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "");
  text_layer_set_font(s_weather_layer, s_font);

  // Battery
  s_battery_layer = layer_create(GRect(0, 0, bounds.size.w, 5));
  layer_set_update_proc(s_battery_layer, battery_update_proc);

  // Calendar
  for (int i=0; i<6; i++) {
    s_calendar_layers[i] = text_layer_create(GRect(0, 72 + 16 * i, bounds.size.w, 16));
    text_layer_set_background_color(s_calendar_layers[i], GColorBlack);
    text_layer_set_text_color(s_calendar_layers[i], GColorWhite);
    text_layer_set_text_alignment(s_calendar_layers[i], GTextAlignmentLeft);
    text_layer_set_font(s_calendar_layers[i], s_font);
    text_layer_set_text(s_calendar_layers[i], "");
    layer_add_child(window_layer, text_layer_get_layer(s_calendar_layers[i]));
  }

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
  /*Tuple *weather_tuple = dict_find(iterator, MESSAGE_KEY_W);*/
  Tuple *calendar_tuple = dict_find(iterator, MESSAGE_KEY_C);

  // If all data is available, use it
  if (calendar_tuple) {
    snprintf(calendar_buffer, sizeof(calendar_buffer), "%s",
        calendar_tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_ERROR, calendar_buffer);
    token = strtok(calendar_buffer, "^");
    text_layer_set_text(s_weather_layer, token);
    token = strtok(NULL, "^");
    int i = 0;
    while(token != NULL && i < 6) {
      text_layer_set_text(s_calendar_layers[i++], token);
      token = strtok(NULL, "^");
    }
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
  APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)reason);
  text_layer_set_text(s_weather_layer, "raté");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

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

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
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
