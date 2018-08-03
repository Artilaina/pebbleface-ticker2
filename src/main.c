
/*

Copyright (C) 2016 Mark Reed

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

-------------------------------------------------------------------

*/

#include <pebble.h>
#include "main.h"
#define ANTIALIASING true

static Window *s_window;

Layer *layer;
static TextLayer *s_time_layer, *s_hour_layer;
TextLayer *battery_text_layer;

static GBitmap *background_image;
static BitmapLayer *background_layer;
static GBitmap *background_image2;
static BitmapLayer *background_layer2;

static GFont custom_font;
static GFont custom_font1;

int charge_percent = 0;

bool text_Displayed = false;

GBitmap *img_battery_100;
GBitmap *img_battery_90;
GBitmap *img_battery_80;
GBitmap *img_battery_70;
GBitmap *img_battery_60;
GBitmap *img_battery_50;
GBitmap *img_battery_40;
GBitmap *img_battery_30;
GBitmap *img_battery_20;
GBitmap *img_battery_10;
GBitmap *img_battery_charge;
BitmapLayer *layer_batt_img;

#ifdef PBL_PLATFORM_CHALK

#define WINDOW_HEIGHT 180
#define WINDOW_WIDTH 180
#define TEXTBOX_HEIGHT 60
#define TEXTBOX_WIDTH 1400

#else

#define WINDOW_HEIGHT 168
#define WINDOW_WIDTH 144
#define TEXTBOX_HEIGHT 60
#define TEXTBOX_WIDTH 1400

#endif

static Layer *s_canvas_layer;

// animates layer by number of pixels
PropertyAnimation *s_box_animation;

// prototype so anim_stopped_handler can compile (implementation below)
void animate_text();

// A struct for our specific settings (see main.h)
ClaySettings settings;

// when watch is shaken or tapped
static void accel_tap_handler(AccelAxisType axis, int32_t direction)
{

  layer_set_hidden(layer, true);

  layer_set_hidden(bitmap_layer_get_layer(background_layer2), false);
  layer_set_hidden(bitmap_layer_get_layer(layer_batt_img), true);
  layer_set_hidden(s_canvas_layer, true);

  if (text_Displayed)
  {
    animate_text();
    text_Displayed = false;
  }
  else
  {
    animate_text();
    text_Displayed = true;
  }
}

// ------- load settings --------- //

// Initialize the default settings
static void prv_default_settings()
{
  settings.BackgroundColor = GColorBlack;
  settings.ForegroundColor = GColorWhite;
  settings.HourHand = GColorRed;
  settings.MinuteHand = GColorLightGray;
  settings.SecondHand = GColorLightGray;
  settings.SecondTick = false;
  settings.Animations = true;
  settings.Duration = 1500;
  settings.Bluetoothvibe = false;
  settings.Bg = false;
}
// Read settings from persistent storage
static void prv_load_settings()
{
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void prv_save_settings()
{
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  // Update the display based on new settings
  prv_update_display();
}

// Update the display elements
static void prv_update_display()
{
  // Background color
  window_set_background_color(s_window, settings.BackgroundColor);

  // Foreground Color
  text_layer_set_text_color(s_time_layer, settings.ForegroundColor);
  text_layer_set_text_color(battery_text_layer, settings.ForegroundColor);
  text_layer_set_text_color(s_hour_layer, settings.HourHand;

  if (settings.Animations)
  {
    accel_tap_service_subscribe(&accel_tap_handler);
  }
  else
  {
    accel_tap_service_unsubscribe();
  }

  // TODO Hay un fallo en la configuración: si se activa "Show dot pattern on analog screen" lo que hace es ocultar el patrón en vez de mostrarlo.
  if (settings.Bg)
  {
    layer_set_hidden(bitmap_layer_get_layer(background_layer), false);
  }
  else
  {
    layer_set_hidden(bitmap_layer_get_layer(background_layer), true);
  }
}

// Handle the response from AppMessage
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context)
{
  // Background Color
  Tuple *bg_color_t = dict_find(iter, MESSAGE_KEY_BackgroundColor);
  if (bg_color_t)
  {
    settings.BackgroundColor = GColorFromHEX(bg_color_t->value->int32);
  }

  // Foreground Color
  Tuple *fg_color_t = dict_find(iter, MESSAGE_KEY_ForegroundColor);
  if (fg_color_t)
  {
    settings.ForegroundColor = GColorFromHEX(fg_color_t->value->int32);
  }

  // Hour hand Color
  Tuple *hourhand_color_t = dict_find(iter, MESSAGE_KEY_HourHand);
  if (hourhand_color_t)
  {
    settings.HourHand = GColorFromHEX(hourhand_color_t->value->int32);
  }

  // Minute hand Color
  Tuple *minutehand_color_t = dict_find(iter, MESSAGE_KEY_MinuteHand);
  if (minutehand_color_t)
  {
    settings.MinuteHand = GColorFromHEX(minutehand_color_t->value->int32);
  }

  // Second hand Color
  Tuple *secondhand_color_t = dict_find(iter, MESSAGE_KEY_SecondHand);
  if (secondhand_color_t)
  {
    settings.SecondHand = GColorFromHEX(secondhand_color_t->value->int32);
  }

  // Second Tick
  Tuple *second_tick_t = dict_find(iter, MESSAGE_KEY_SecondTick);
  if (second_tick_t)
  {
    settings.SecondTick = second_tick_t->value->int32 == 1;
  }

  // Animation
  Tuple *animations_t = dict_find(iter, MESSAGE_KEY_Animations);
  if (animations_t)
  {
    settings.Animations = animations_t->value->int32 == 1;
  }

  // Duration
  Tuple *duration_t = dict_find(iter, MESSAGE_KEY_Duration);
  if (duration_t)
  {
    settings.Duration = duration_t->value->int32;
  }

  // Bluetoothvibe
  Tuple *bluetoothvibe_t = dict_find(iter, MESSAGE_KEY_Bluetoothvibe);
  if (bluetoothvibe_t)
  {
    settings.Bluetoothvibe = bluetoothvibe_t->value->int32 == 1;
  }

  // Bluetoothvibe
  Tuple *bg_t = dict_find(iter, MESSAGE_KEY_Bg);
  if (bg_t)
  {
    settings.Bg = bg_t->value->int32 == 1;
  }
  // Save the new settings to persistent storage
  prv_save_settings();
}

// -----------end settings------------- //

static void canvas_update_proc(Layer *this_layer, GContext *ctx)
{
  GRect bounds = layer_get_bounds(this_layer);

  // Get the center of the screen (non full-screen)
  GPoint center = GPoint(bounds.size.w / 2, (bounds.size.h / 2));

  //  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, settings.BackgroundColor);
  graphics_fill_circle(ctx, center, 16);
}

void handle_tick(struct tm *tick_time, TimeUnits units_changed)
{

  //redraw every tick
  layer_mark_dirty(layer);

  static char s_time_text[] = "Wednesday 00 September 2015, 00:00pm";
  char *time_format;

  if (clock_is_24h_style())
  {
    time_format = "%A %d %B %Y, %R";
  }
  else
  {
    time_format = "%A %d %B %Y, %l:%M%P";
  }
  strftime(s_time_text, sizeof(s_time_text), time_format, tick_time);
  text_layer_set_text(s_time_layer, s_time_text);
}

void update_layer(Layer *me, GContext *ctx)
{

  //watchface drawing

  //get tick_time
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  static char s_hour_text[2];

  //draw hands

  GRect bounds_main = layer_get_bounds(me);

  // Get the center of the screen (non full-screen)
  GPoint center = GPoint(bounds_main.size.w / 2, (bounds_main.size.h / 2));
  GPoint center_small = GPoint(bounds_main.size.w / 2, (bounds_main.size.h / 20));

  graphics_context_set_fill_color(ctx, settings.HourHand);
  graphics_fill_circle(ctx, center_small, 3);

#ifdef PBL_PLATFORM_CHALK
  int16_t secondHandLength = 84;
  int16_t minuteHandLength = 84;
  int16_t hourHandLength = 42;
#else
  int16_t secondHandLength = 70;
  int16_t minuteHandLength = 70;
  int16_t hourHandLength = 40;
#endif
  GPoint secondHand;
  GPoint minuteHand;
  GPoint hourHand;

  int32_t minute_angle = TRIG_MAX_ANGLE * tick_time->tm_min / 60;
  minuteHand.y = (int16_t)(-cos_lookup(minute_angle) * (int32_t)minuteHandLength / TRIG_MAX_RATIO) + center.y;
  minuteHand.x = (int16_t)(sin_lookup(minute_angle) * (int32_t)minuteHandLength / TRIG_MAX_RATIO) + center.x;

  int32_t hour_angle = (TRIG_MAX_ANGLE * (((tick_time->tm_hour % 12) * 6) + (tick_time->tm_min / 10))) / (12 * 6);
  hourHand.y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)hourHandLength / TRIG_MAX_RATIO) + center.y;
  hourHand.x = (int16_t)(sin_lookup(hour_angle) * (int32_t)hourHandLength / TRIG_MAX_RATIO) + center.x;

  //------------------------------------------------------------------------------------------------------------//
  // SECOND HAND
  if (settings.SecondTick == 1)
  {
    int32_t second_angle = TRIG_MAX_ANGLE * tick_time->tm_sec / 60;
    secondHand.y = (int16_t)(-cos_lookup(second_angle) * (int32_t)secondHandLength / TRIG_MAX_RATIO) + center.y;
    secondHand.x = (int16_t)(sin_lookup(second_angle) * (int32_t)secondHandLength / TRIG_MAX_RATIO) + center.x;

    tick_timer_service_subscribe(SECOND_UNIT, &handle_tick);
    graphics_context_set_stroke_color(ctx, settings.SecondHand);
    graphics_draw_line(ctx, center, secondHand);
  }
  else
  {
    tick_timer_service_subscribe(MINUTE_UNIT, &handle_tick);
  }

  //------------------------------------------------------------------------------------------------------------//
  // MINUTE HAND

  //I didn't like how a 2px path rotated, so I'm using two lines next to each other
  //I need to move the pixels from vertically adjacent to horizontally adjacent based on the position
  bool addX = (tick_time->tm_min > 20 && tick_time->tm_min < 40) || tick_time->tm_min < 10 || tick_time->tm_min > 50;
  center.x += addX ? 2 : 0;
  center.y += !addX ? 2 : 0;
  minuteHand.x += addX ? 2 : 0;
  minuteHand.y += !addX ? 2 : 0;

  graphics_context_set_stroke_width(ctx, 4);
  graphics_context_set_antialiased(ctx, ANTIALIASING);
  graphics_context_set_stroke_color(ctx, GColorWhite);

#ifdef PBL_COLOR
  graphics_context_set_stroke_color(ctx, settings.MinuteHand);
#endif

  graphics_draw_line(ctx, center, minuteHand);

  //------------------------------------------------------------------------------------------------------------//
  // HOUR HAND
  center.x -= addX ? 1 : 0;
  center.y -= !addX ? 1 : 0;

  addX = (tick_time->tm_hour >= 4 && tick_time->tm_hour <= 8) || tick_time->tm_hour < 2 || tick_time->tm_hour > 10;
  center.x += addX ? 1 : 0;
  center.y += !addX ? 1 : 0;
  hourHand.x += addX ? 1 : 0;
  hourHand.y += !addX ? 1 : 0;
  graphics_context_set_stroke_color(ctx, GColorWhite);
#ifdef PBL_COLOR
  graphics_context_set_stroke_color(ctx, settings.HourHand);
#endif
  graphics_draw_line(ctx, center, hourHand);

  strftime(s_hour_text, sizeof(s_hour_text), %s, tick_time->tm_hour);
  text_layer_set_text(s_hour_layer, s_hour_text);
}

void anim_stopped_handler(Animation *animation, bool finished, void *context)
{
  if (finished)
  {
    layer_set_hidden(bitmap_layer_get_layer(background_layer2), true);
    layer_set_hidden(bitmap_layer_get_layer(layer_batt_img), false);
    layer_set_hidden(s_canvas_layer, false);
    layer_set_hidden(layer, false);
    property_animation_destroy(s_box_animation);
  }
}

void animate_text()
{
#ifdef PBL_PLATFORM_CHALK
  GRect start_frame = GRect(190, 69, WINDOW_WIDTH, TEXTBOX_HEIGHT);
  GRect finish_frame = GRect(-700, 69, TEXTBOX_WIDTH, TEXTBOX_HEIGHT);
#else
  GRect start_frame = GRect(190, 59, WINDOW_WIDTH, TEXTBOX_HEIGHT);
  GRect finish_frame = GRect(-700, 59, TEXTBOX_WIDTH, TEXTBOX_HEIGHT);
#endif
  s_box_animation = property_animation_create_layer_frame(text_layer_get_layer(s_time_layer), &start_frame, &finish_frame);
  animation_set_handlers((Animation *)s_box_animation, (AnimationHandlers){.stopped = anim_stopped_handler}, NULL);
  animation_set_duration((Animation *)s_box_animation, settings.Duration);
  animation_set_curve((Animation *)s_box_animation, AnimationCurveLinear);
  animation_set_delay((Animation *)s_box_animation, 0);
  animation_schedule((Animation *)s_box_animation);
}

void update_battery_state(BatteryChargeState charge_state)
{
  static char battery_text[] = "    ";

  if (charge_state.is_charging)
  {
    bitmap_layer_set_bitmap(layer_batt_img, img_battery_charge);
    snprintf(battery_text, sizeof(battery_text), "+%d", charge_state.charge_percent);
    layer_set_hidden(text_layer_get_layer(battery_text_layer), false);
  }
  else
  {
    layer_set_hidden(text_layer_get_layer(battery_text_layer), true);

    if (charge_state.charge_percent <= 10)
    {
      bitmap_layer_set_bitmap(layer_batt_img, img_battery_10);
    }
    else if (charge_state.charge_percent <= 20)
    {
      bitmap_layer_set_bitmap(layer_batt_img, img_battery_20);
    }
    else if (charge_state.charge_percent <= 30)
    {
      bitmap_layer_set_bitmap(layer_batt_img, img_battery_30);
    }
    else if (charge_state.charge_percent <= 40)
    {
      bitmap_layer_set_bitmap(layer_batt_img, img_battery_40);
    }
    else if (charge_state.charge_percent <= 50)
    {
      bitmap_layer_set_bitmap(layer_batt_img, img_battery_50);
    }
    else if (charge_state.charge_percent <= 60)
    {
      bitmap_layer_set_bitmap(layer_batt_img, img_battery_60);
    }
    else if (charge_state.charge_percent <= 70)
    {
      bitmap_layer_set_bitmap(layer_batt_img, img_battery_70);
    }
    else if (charge_state.charge_percent <= 80)
    {
      bitmap_layer_set_bitmap(layer_batt_img, img_battery_80);
    }
    else if (charge_state.charge_percent <= 90)
    {
      bitmap_layer_set_bitmap(layer_batt_img, img_battery_90);
    }
    else if (charge_state.charge_percent <= 100)
    {
      bitmap_layer_set_bitmap(layer_batt_img, img_battery_100);
    }
    else
    {
      bitmap_layer_set_bitmap(layer_batt_img, img_battery_100);
    }
  }

  charge_percent = charge_state.charge_percent;
  text_layer_set_text(battery_text_layer, battery_text);
}

static void handle_bluetooth(bool connected)
{

  if (!connected && settings.Bluetoothvibe)
  {
    vibes_short_pulse();
  }
}

void force_update(void)
{
  update_battery_state(battery_state_service_peek());
  handle_bluetooth(bluetooth_connection_service_peek());

  struct tm *t;
  time_t temp;
  temp = time(NULL);
  t = localtime(&temp);

  handle_tick(t, MINUTE_UNIT);
}

// Window Load event
static void prv_window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  custom_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CUSTOM_40));
  custom_font1 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CUSTOM_22));

  //load background image [ticker]
  background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACK);
  background_layer = bitmap_layer_create(GRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT));

  bitmap_layer_set_bitmap(background_layer, background_image);
  GCompOp compositing_mod_back = GCompOpSet;
  bitmap_layer_set_compositing_mode(background_layer, compositing_mod_back);
  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));
  layer_set_hidden(bitmap_layer_get_layer(background_layer), true);

  //load background image [analog]
  background_image2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACK);
  background_layer2 = bitmap_layer_create(GRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT));

  bitmap_layer_set_bitmap(background_layer2, background_image2);
  GCompOp compositing_mod_back2 = GCompOpSet;
  bitmap_layer_set_compositing_mode(background_layer2, compositing_mod_back2);
  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer2));
  layer_set_hidden(bitmap_layer_get_layer(background_layer2), true);

  // create hands layer
  layer = layer_create(GRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT));

  layer_set_update_proc(layer, update_layer);
  layer_add_child(window_get_root_layer(window), layer);

  // create other layers

#ifdef PBL_PLATFORM_CHALK
  s_time_layer = text_layer_create(GRect(185, 79, 700, 60));
#else
  s_time_layer = text_layer_create(GRect(145, 59, 700, 60));
#endif

#ifdef PBL_COLOR
  text_layer_set_text_color(s_time_layer, settings.MinuteHand);
#else
  text_layer_set_text_color(s_time_layer, GColorWhite);
#endif
  
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentLeft);
  text_layer_set_font(s_time_layer, custom_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

  // Create Layer
  s_canvas_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  layer_add_child(window_layer, s_canvas_layer);

  // Set the update_proc
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);

  // prepare battery layer

  img_battery_100 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_100);
  img_battery_90 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_90);
  img_battery_80 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_80);
  img_battery_70 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_70);
  img_battery_60 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_60);
  img_battery_50 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_50);
  img_battery_40 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_40);
  img_battery_30 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_30);
  img_battery_20 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_20);
  img_battery_10 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_10);
  img_battery_charge = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_CHARGING);

#ifdef PBL_PLATFORM_CHALK
  layer_batt_img = bitmap_layer_create(GRect(73, 73, 35, 35));
#else
  layer_batt_img = bitmap_layer_create(GRect(56, 68, 35, 35));
#endif

  bitmap_layer_set_bitmap(layer_batt_img, img_battery_100);
  GCompOp compositing_mode_batt = GCompOpSet;
  bitmap_layer_set_compositing_mode(layer_batt_img, compositing_mode_batt);
  layer_add_child(window_layer, bitmap_layer_get_layer(layer_batt_img));

#ifdef PBL_PLATFORM_CHALK
  battery_text_layer = text_layer_create(GRect(0, 73, 182, 26));
#else
  battery_text_layer = text_layer_create(GRect(0, 68, 144, 26));
#endif

#ifdef PBL_COLOR
  text_layer_set_text_color(battery_text_layer, settings.ForegroundColor);
#else
  text_layer_set_text_color(battery_text_layer, GColorWhite);
#endif
  text_layer_set_background_color(battery_text_layer, GColorClear);
  text_layer_set_text_alignment(battery_text_layer, GTextAlignmentCenter);
  text_layer_set_font(battery_text_layer, custom_font1);
  layer_add_child(window_layer, text_layer_get_layer(battery_text_layer));

  //TODO Colocar las horas para pebble round lo dejo para cuando alguien me regale uno y pueda probarlo...
  s_hour_layer = text_layer_create(GRect(0, (int)(bounds.size.w/2), 5, 10));

#ifdef PBL_COLOR
  text_layer_set_text_color(s_hour_layer, settings.HourHand);
#else
  text_layer_set_text_color(s_hour_layer, GColorWhite);
#endif
  
  text_layer_set_background_color(s_hour_layer, GColorClear);
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentCenter);
  text_layer_set_font(s_hour_layer, custom_font1);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_hour_layer));


  // draw first frame
  force_update();

  prv_update_display();
}

// Window Unload event
static void prv_window_unload(Window *window)
{

  layer_remove_from_parent(bitmap_layer_get_layer(layer_batt_img));
  bitmap_layer_destroy(layer_batt_img);

  gbitmap_destroy(img_battery_100);
  gbitmap_destroy(img_battery_90);
  gbitmap_destroy(img_battery_80);
  gbitmap_destroy(img_battery_70);
  gbitmap_destroy(img_battery_60);
  gbitmap_destroy(img_battery_50);
  gbitmap_destroy(img_battery_40);
  gbitmap_destroy(img_battery_30);
  gbitmap_destroy(img_battery_20);
  gbitmap_destroy(img_battery_10);
  gbitmap_destroy(img_battery_charge);

  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_hour_layer);
  text_layer_destroy(battery_text_layer);

  layer_remove_from_parent(bitmap_layer_get_layer(background_layer));
  bitmap_layer_destroy(background_layer);
  gbitmap_destroy(background_image);
  background_image = NULL;

  layer_remove_from_parent(bitmap_layer_get_layer(background_layer2));
  bitmap_layer_destroy(background_layer2);
  gbitmap_destroy(background_image2);
  background_image2 = NULL;

  layer_destroy(layer);

  layer_destroy(s_canvas_layer);
}

static void prv_init(void)
{
  prv_load_settings();

  // Listen for AppMessages
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(128, 128);

  // international support
  setlocale(LC_ALL, "");

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
                                           .load = prv_window_load,
                                           .unload = prv_window_unload,
                                       });

  window_stack_push(s_window, true);

  // subscribe on time tick

  bluetooth_connection_service_subscribe(&handle_bluetooth);
  battery_state_service_subscribe(update_battery_state);
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_tick);
}

static void prv_deinit(void)
{

  battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();

  fonts_unload_custom_font(custom_font);
  fonts_unload_custom_font(custom_font1);

  if (s_window)
  {
    window_destroy(s_window);
  }
}

int main(void)
{
  prv_init();
  app_event_loop();
  prv_deinit();
}
