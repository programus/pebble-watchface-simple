#include <pebble.h>

enum {
  KEY_TEMPERATURE = 0,
  KEY_CONDITIONS
};
  
static Window* s_main_window;
static TextLayer* s_time_layer;
static GFont s_time_font;
static BitmapLayer* s_background_layer;
static GBitmap* s_background_bitmap;
static TextLayer* s_weather_layer;
static GFont s_weather_font;

static void inbox_received_callback(DictionaryIterator*, void*);
static void inbox_dropped_callback(AppMessageResult, void*);
static void outbox_failed_callback(DictionaryIterator*, AppMessageResult, void*);
static void outbox_sent_callback(DictionaryIterator*, void*);
static void init();
static void deinit();
static void main_window_load(Window*);
static void main_window_unload(Window*);
static void tick_handler(struct tm*, TimeUnits);
static void update_time();
static void request_weather();
int main(void);

static void inbox_received_callback(DictionaryIterator* iterator, void* context) {
  char temperature_buffer[8];
  char conditions_buffer[32];
  static char weather_layer_buffer[32];
  
  for (Tuple* t = dict_read_first(iterator); t != NULL; t = dict_read_next(iterator)) {
    switch (t->key) {
    case KEY_TEMPERATURE:
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)t->value->int32);
      break;
    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Unrecognized key: %d", (int) t->key);
      break;
    }
  }
  
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
  text_layer_set_text(s_weather_layer, weather_layer_buffer);
}
static void inbox_dropped_callback(AppMessageResult reason, void* context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped.");
}

static void outbox_failed_callback(DictionaryIterator* iterator, AppMessageResult reason, void* context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed.");
}

static void outbox_sent_callback(DictionaryIterator* iterator, void* context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success.");
}

static void init() {
  // initialize main window instance
  s_main_window = window_create();
  
  // set the window handlers
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // show window on the watch, with animated = true
  window_stack_push(s_main_window, true);
  
  // display time from the beginning
  update_time();
  
  // register the time tick service
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // open app message
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  window_destroy(s_main_window);
}

static void main_window_load(Window* window) {
  // Create background BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(s_main_window), bitmap_layer_get_layer(s_background_layer));
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(5, 52, 139, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  
  // Create custom font
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));
  
  // make text more like watchface
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // add to window
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_time_layer));
  
  // Create weather Layer
  s_weather_layer = text_layer_create(GRect(0, 10, 144, 25));
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Loading...");
  
  // Create font for weather display
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_20));
  text_layer_set_font(s_weather_layer, s_weather_font);
  
  // add to window
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_weather_layer));
}

static void main_window_unload(Window* window) {
  // unload GFont
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_weather_font);
  
  // destroy weather layer
  text_layer_destroy(s_weather_layer);
  
  // destroy time layer
  text_layer_destroy(s_time_layer);
  
  // destroy background layer
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
}

static void tick_handler(struct tm* tick_time, TimeUnits units_changed) {
  update_time();
}

static void update_time() {
  // Get a tm struct
  time_t temp = time(NULL);
  struct tm* tick_time = localtime(&temp);
  
  // create a long lived buffer
  static char buffer[] = "00:00";
  
  // write time into buffer
  char* fmt = NULL;
  if (clock_is_24h_style()) {
    fmt = "%H:%M";
  } else {
    fmt = "%I:%M";
  }
  strftime(buffer, sizeof(buffer), fmt, tick_time);
  
  // display the text
  text_layer_set_text(s_time_layer, buffer);
}

static void request_weather() {
  DictionaryIterator* iterator;
  app_message_outbox_begin(&iterator);
  
  dict_write_uint8(iterator, 0, 0);
  
  app_message_outbox_send();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}