#include <pebble.h>

static Window *window;
static RotBitmapLayer *icon_layer;
static GBitmap *icon_bitmap = NULL;
static Layer *draw_layer;
static TextLayer *text_layer;



enum ClockKey {
  INIT_KEY = 0x0,
  ANGLE_KEY = 0x1, // TUPLE_INT
  SECONDS_KEY = 0x2
};

//144x168
#define SCREEN_WIDTH 144
#define SCREEN_HEIGHT 168
#define PARTITIONS 48
#define ANGLE_MULTIPLIER 182


int position = 0;
int current_side = 0;
int current_pixel = 0;
int start_side = 0;
int start_pixel = 0;
int init_seconds;
int init_angle = 0;
int text_layer_showing = 0;
char buf[15];

static uint32_t CLOCK_ICONS[] = {
  RESOURCE_ID_IMAGE_CLOCK
};



static void draw_section(Layer *layer, GContext *ctx){
  GPoint center = GPoint(SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
  int perimeter = (SCREEN_HEIGHT*2) + (SCREEN_WIDTH*2);
  int pixels =  (perimeter)/PARTITIONS;
  for (int i =0; i < pixels; i++){
    if (current_side == 0 && current_pixel == SCREEN_WIDTH){
        i--;
        current_side = 1;
        current_pixel = 0;
    }
    else if (current_side == 1 && current_pixel == SCREEN_HEIGHT){
        i--;
        current_side = 2;
        current_pixel = SCREEN_WIDTH;
    }
    else if (current_side == 2 && current_pixel == 0){
        i--;
        current_side = 3;
        current_pixel = SCREEN_HEIGHT;
    }
    else if (current_side == 3 && current_pixel == 0){
        i--;
        current_side = 0;
        current_pixel = 0;
    }

    else if (current_side == 0){
      graphics_draw_line(ctx,GPoint(current_pixel,0),center);
      current_pixel++;
    }
    else if (current_side == 1){
      graphics_draw_line(ctx,GPoint(SCREEN_WIDTH,current_pixel),center);
      current_pixel++;
    }
    else if (current_side == 2){
      graphics_draw_line(ctx,GPoint(current_pixel,SCREEN_HEIGHT),center);
      current_pixel--;
    }
    else if (current_side == 3){
      graphics_draw_line(ctx,GPoint(0,current_pixel),center);
      current_pixel--;
    }

  }
}


static void draw_up(Layer *layer, GContext *ctx, int start,int end){
  GPoint center = GPoint(SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
    for (int i = start; i <= end; i++){
    graphics_draw_line(ctx,GPoint(i,0),center);
   }

}

static void draw_down(Layer *layer, GContext *ctx, int start,int end){
  GPoint center = GPoint(SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
  for (int i = start; i <= end; i++){
    graphics_draw_line(ctx,GPoint(i,SCREEN_HEIGHT),center);
   }
}

static void draw_left(Layer *layer, GContext *ctx, int start,int end){
  GPoint center = GPoint(SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
  for (int i = start; i <= end; i++){
    graphics_draw_line(ctx,GPoint(0,i),center);
   }
}

static void draw_right(Layer *layer, GContext *ctx, int start,int end){
  GPoint center = GPoint(SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
   for (int i = start; i <= end; i++){
    graphics_draw_line(ctx,GPoint(SCREEN_WIDTH,i),center);
   }
}


static void draw_layer_draw(Layer *layer, GContext *ctx) {

  //graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  current_side = 0;
  current_pixel = 0;
  for (int i =0; i < position; i++){
    draw_section(layer,ctx);
  }
}


static void select_click_handler(ClickRecognizerRef recognizer, void *context) {


}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  position = (position + 1) % PARTITIONS;
  //rot_bitmap_layer_increment_angle(icon_layer, 90*182);
  //layer_mark_dirty((Layer*)icon_layer);
  layer_mark_dirty(draw_layer);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Down");
  DictionaryIterator **   iterator = NULL;
  app_message_outbox_begin(iterator);
  app_message_outbox_send();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Send");
}


static void in_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *init_tuple = dict_find(iter, INIT_KEY);
  Tuple *angle_tuple = dict_find(iter, ANGLE_KEY);
  Tuple *seconds_tuple = dict_find(iter, SECONDS_KEY);

  if (angle_tuple) {
    rot_bitmap_layer_set_angle(icon_layer, (angle_tuple->value->uint8)*182);
    layer_mark_dirty((Layer*)icon_layer);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Angle: %d",angle_tuple->value->uint8 );
    if (init_tuple){
      init_angle = angle_tuple->value->uint8;
    }
  }
  if (seconds_tuple) {
    if (init_tuple){

      init_seconds = seconds_tuple->value->int32;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "init seconds %d",init_seconds);
    }
    else {
        int seconds = (seconds_tuple->value->int32);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "seconds %d",seconds);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "init seconds %d",init_seconds);
        float ratio = (((float)seconds_tuple->value->int32) / ((float)init_seconds));
        position = (int)((1.0-ratio)*PARTITIONS);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Position: %i", position);
        layer_mark_dirty(draw_layer);
    }
  }
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped!");
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send!");
 }


static void app_message_init(void) {
  // Register message handlers
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_failed(out_failed_handler);
  // Init buffers
  app_message_open(64, 64);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  icon_bitmap = gbitmap_create_with_resource(CLOCK_ICONS[0]);
  icon_layer = rot_bitmap_layer_create(icon_bitmap);
  GRect bounds_b = layer_get_bounds((Layer*)icon_layer);
  const GPoint center = grect_center_point(&bounds_b);
  GRect image_frame = (GRect) { .origin = center, .size = bounds_b.size };
  image_frame.origin.x -= bounds_b.size.w/2+38; //rotlayer does something odd with positioning, and this is the only way I could correct it
  image_frame.origin.y -= bounds_b.size.h/2+26;
  layer_set_frame((Layer*)icon_layer,image_frame);
  rot_bitmap_set_compositing_mode(icon_layer, GCompOpAssign); // use whatever mode you need to
  bitmap_layer_set_background_color((BitmapLayer *) icon_layer, GColorBlack );
  layer_add_child(window_layer, (Layer*)icon_layer);


  draw_layer = layer_create(bounds);
  layer_add_child(window_layer, draw_layer);
  layer_set_update_proc(draw_layer, draw_layer_draw);

  text_layer = text_layer_create((GRect) { .origin = { 33, 72 }, .size = { 80, 20 } });
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_text_color( text_layer,GColorWhite);
  text_layer_set_background_color( text_layer,GColorBlack);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

}

static void window_unload(Window *window) {
  bitmap_layer_destroy((BitmapLayer*)icon_layer);
  layer_destroy(draw_layer);

}

static void init(void) {
  window = window_create();


  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_set_fullscreen(window, true);
  window_stack_push(window, animated);

  app_message_init();
}

static void deinit(void) {
  window_destroy(window);

}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
