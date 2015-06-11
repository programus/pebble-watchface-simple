#include <pebble.h>
  
static Window* s_main_window;
static TextLayer* s_time_layer;
static GFont s_time_font;

static void init();
static void deinit();
static void main_window_load(Window*);
static void main_window_unload(Window*);
static void tick_handler(struct tm*, TimeUnits);
static void update_time();
int main(void);

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
}

static void deinit() {
  window_destroy(s_main_window);
}

static void main_window_load(Window* window) {
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
}

static void main_window_unload(Window* window) {
  // unload GFont
  fonts_unload_custom_font(s_time_font);
  
  // destroy time layer
  text_layer_destroy(s_time_layer);
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

int main(void) {
  init();
  app_event_loop();
  deinit();
}