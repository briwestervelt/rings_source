#include <pebble.h>

#define KEY_BACKGROUND_COLOR 0
#define KEY_FOREGROUND_COLOR 1
#define KEY_BATTERY_LINE 2
#define KEY_STATIC_LINE 3
#define KEY_NO_LINE 4
#define KEY_SHOW_DATE 5
#define KEY_BLUETOOTH_VIBES 6
#define KEY_RANDOM_COLOR 7

#define gap_width          1  //pixels
#define line_one_offset    25 //pixels (for day of week)
#define line_two_offset    3  //pixels (for day of month)

static Window *main_window;
static Layer *ring_layer, *battery_layer;
static TextLayer *date_line_one_layer, *date_line_two_layer;

static GColor background_color, foreground_color, bg_color_settings, fg_color_settings;
static int battery_level;
static bool battery_line_bool, static_line_bool, no_line_bool;
static bool show_date_bool;
static bool bluetooth_vibes_bool;
static bool random_color_bool;

static GColor random_color(void){
  //GColors take the form 0b11xxxxxx
  //I kind of stole this from a post by /u/chaos750 on /r/pebbledevelopers
  //thanks man!
   return (GColor8) { .argb = ((rand() % 0b00111111) + 0b11000000) };
}

static void set_colors(void){
  background_color = (random_color_bool) ? random_color() : bg_color_settings;
  foreground_color = (random_color_bool) ? gcolor_legible_over(background_color) : fg_color_settings;
}

static void update_colors(void){
  set_colors();

  //foreground
  layer_mark_dirty(battery_layer);
  text_layer_set_text_color(date_line_one_layer, foreground_color);
  text_layer_set_text_color(date_line_two_layer, foreground_color);
  
  //background
  window_set_background_color(main_window, background_color);
}

static void battery_callback(BatteryChargeState state){
  battery_level = state.charge_percent;
  layer_mark_dirty(battery_layer);
}

static void bluetooth_callback(bool connected){
  if((!connected) && (bluetooth_vibes_bool)){
    vibes_double_pulse();
  }
}

static void ring_update_proc(Layer *layer, GContext *ctx){
  GRect outer_bounds = layer_get_bounds(layer);
  GRect inner_bounds = grect_inset(outer_bounds, GEdgeInsets(outer_bounds.size.w/6));
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  int minute = t->tm_min;
  int hour = t->tm_hour;
  
  //corrects for 12 hour format
  if(hour >= 12) hour -= 12;
  
  int minute_angle = 360 * (minute / 60.0);
  //int hour_angle = 360 * ((hour / 12.0) + (0.1 * (minute / 60.0)));
  int hour_angle = 360 * (( hour + (minute / 60.0)) / 12.0); //better algorithm
  
  if((random_color_bool) && (minute == 0))
    update_colors();
  
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_fill_color(ctx, foreground_color);
  
  graphics_fill_radial(ctx, outer_bounds, GOvalScaleModeFitCircle, outer_bounds.size.w/6 - gap_width,
                       DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(minute_angle));
  graphics_fill_radial(ctx, inner_bounds, GOvalScaleModeFitCircle, outer_bounds.size.w/6 - gap_width,
                       DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(hour_angle));
    
}

static void battery_update_proc(Layer *layer, GContext *ctx){
  GRect bounds = layer_get_bounds(layer);
  GPoint center = GPoint(bounds.size.w/2, bounds.size.h/2);
  int half_bar_length = 0;
  
  if(!no_line_bool){
    if(battery_line_bool){
      half_bar_length = bounds.size.w/6 * (battery_level / 100.0);
    }
    else if(static_line_bool){
      half_bar_length = bounds.size.w/6;
    }
  
    GPoint left_point =  GPoint(center.x - half_bar_length, center.y);
    GPoint right_point = GPoint(center.x + half_bar_length, center.y);
  
    graphics_context_set_stroke_color(ctx, foreground_color);
    graphics_draw_line(ctx, left_point, right_point);
  }
}

static void update_date(void){  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  static char date_line_one_buffer[8];
  strftime(date_line_one_buffer, sizeof(date_line_one_buffer), (show_date_bool) ? "%a" : "", t);
  
  static char date_line_two_buffer[4];
  strftime(date_line_two_buffer, sizeof(date_line_two_buffer), (show_date_bool) ? "%d" : "", t);
  
  text_layer_set_text(date_line_one_layer, date_line_one_buffer);
  text_layer_set_text(date_line_two_layer, date_line_two_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  update_date();
  layer_mark_dirty(ring_layer);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context){
  Tuple *background_color_t = dict_find(iter, KEY_BACKGROUND_COLOR);
  Tuple *foreground_color_t = dict_find(iter, KEY_FOREGROUND_COLOR);
  Tuple *battery_line_t = dict_find(iter, KEY_BATTERY_LINE);
  Tuple *static_line_t = dict_find(iter, KEY_STATIC_LINE);
  Tuple *no_line_t = dict_find(iter, KEY_NO_LINE);
  Tuple *show_date_t = dict_find(iter, KEY_SHOW_DATE);
  Tuple *bluetooth_vibes_t = dict_find(iter, KEY_BLUETOOTH_VIBES);
  Tuple *random_color_t = dict_find(iter, KEY_RANDOM_COLOR);
  
  if(background_color_t){
    int background_color_HEX = background_color_t->value->int32;
    persist_write_int(KEY_BACKGROUND_COLOR, background_color_HEX);
    bg_color_settings = GColorFromHEX(background_color_HEX);
    update_colors();
    layer_mark_dirty(ring_layer);
  }
  if(foreground_color_t){
    int foreground_color_HEX = foreground_color_t->value->int32;
    persist_write_int(KEY_FOREGROUND_COLOR, foreground_color_HEX);
    fg_color_settings = GColorFromHEX(foreground_color_HEX);
    update_colors();
    layer_mark_dirty(ring_layer);
  }
  if(battery_line_t){
    battery_line_bool = battery_line_t->value->int8;
    persist_write_int(KEY_BATTERY_LINE, battery_line_bool);
    layer_mark_dirty(battery_layer);
  }
  if(static_line_t){
    static_line_bool = static_line_t->value->int8;
    persist_write_int(KEY_STATIC_LINE, static_line_bool);
    layer_mark_dirty(battery_layer);
  }
  if(no_line_t){
    no_line_bool = no_line_t->value->int8;
    persist_write_int(KEY_NO_LINE, no_line_bool);
    layer_mark_dirty(battery_layer);
  }
  if(show_date_t){
    show_date_bool = show_date_t->value->int8;
    persist_write_int(KEY_SHOW_DATE, show_date_bool);
    update_date();
  }
  if(bluetooth_vibes_t){
    bluetooth_vibes_bool = bluetooth_vibes_t->value->int8;
    persist_write_int(KEY_BLUETOOTH_VIBES, bluetooth_vibes_bool);
    persist_write_int(KEY_BLUETOOTH_VIBES, bluetooth_vibes_bool);
  }
  if(random_color_t){
    random_color_bool = random_color_t->value->int8;
    persist_write_int(KEY_RANDOM_COLOR, random_color_bool);
    update_colors();
    layer_mark_dirty(ring_layer);
  }
}

static void window_load(Window *window){
  if(persist_exists(KEY_BACKGROUND_COLOR)){
    int background_color_HEX = persist_read_int(KEY_BACKGROUND_COLOR);
    bg_color_settings = GColorFromHEX(background_color_HEX);
  }
  else{
    bg_color_settings = GColorBlue;
  }
  if(persist_exists(KEY_FOREGROUND_COLOR)){
    int foreground_color_HEX = persist_read_int(KEY_FOREGROUND_COLOR);
    fg_color_settings = GColorFromHEX(foreground_color_HEX);
  }
  else{
    fg_color_settings = GColorWhite;
  }
  
  battery_line_bool = (persist_exists(KEY_BATTERY_LINE)) ? persist_read_bool(KEY_BATTERY_LINE) : true;
  static_line_bool = (persist_exists(KEY_STATIC_LINE)) ? persist_read_bool(KEY_STATIC_LINE) : false;
  no_line_bool = (persist_exists(KEY_NO_LINE)) ? persist_read_bool(KEY_NO_LINE) : false;
  show_date_bool = (persist_exists(KEY_SHOW_DATE)) ? persist_read_bool(KEY_SHOW_DATE) : true;
  bluetooth_vibes_bool = (persist_exists(KEY_BLUETOOTH_VIBES)) ? persist_read_bool(KEY_BLUETOOTH_VIBES) : true;
  random_color_bool = (persist_exists(KEY_RANDOM_COLOR)) ? persist_read_bool(KEY_RANDOM_COLOR) : true;
  
  set_colors();
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = GPoint(bounds.size.w/2, bounds.size.h/2);
  
  window_set_background_color(main_window, background_color);
  
  ring_layer = layer_create(bounds);
  layer_set_update_proc(ring_layer, ring_update_proc);
  layer_add_child(window_layer, ring_layer);
  
  date_line_one_layer = text_layer_create(GRect(
    center.x - bounds.size.w/6,
    center.y - line_one_offset,
    bounds.size.w/3,
    line_one_offset
  ));
  text_layer_set_text_color(date_line_one_layer, foreground_color);
  text_layer_set_background_color(date_line_one_layer, GColorClear);
  text_layer_set_font(date_line_one_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(date_line_one_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(date_line_one_layer));
  
  date_line_two_layer = text_layer_create(GRect(
    center.x - bounds.size.w/6,
    center.y - line_two_offset,
    bounds.size.w/3,
    bounds.size.w/6
  ));
  text_layer_set_text_color(date_line_two_layer, foreground_color);
  text_layer_set_background_color(date_line_two_layer, GColorClear);
  text_layer_set_font(date_line_two_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
  text_layer_set_text_alignment(date_line_two_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(date_line_two_layer));
  
  battery_layer = layer_create(bounds);
  layer_set_update_proc(battery_layer, battery_update_proc);
  layer_add_child(window_layer, battery_layer);
}

static void window_unload(Window *window){
  layer_destroy(battery_layer);
  layer_destroy(ring_layer);
  text_layer_destroy(date_line_one_layer);
  text_layer_destroy(date_line_two_layer);
}

static void init(void){
  srand(time(NULL));
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_callback);
  connection_service_subscribe((ConnectionHandlers){
    .pebble_app_connection_handler = bluetooth_callback
  });
  
  main_window = window_create();
  
  window_set_window_handlers(main_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload
  });
  
  window_stack_push(main_window, true);
  
  battery_callback(battery_state_service_peek());
  update_date();
  
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(128, 0);
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "App is running %d", 0);
}

static void deinit(void){
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
  window_destroy(main_window);
}

int main(void){
  init();
  app_event_loop();
  deinit();
}