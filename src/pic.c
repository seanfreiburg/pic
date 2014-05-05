#include <pebble.h>

static Window *window;
static RotBitmapLayer *icon_layer;
static GBitmap *icon_bitmap = NULL;
static Layer *draw_layer;

static AppSync sync;
static uint8_t sync_buffer[64];


enum ClockKey {
  ANGLE_KEY = 0x0, // TUPLE_INT
  SECONDS_KEY = 0x1
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


static uint32_t CLOCK_ICONS[] = {
  RESOURCE_ID_IMAGE_CLOCK
};


static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    case ANGLE_KEY:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "ANGLE_KEY");
      // App Sync keeps new_tuple in sync_buffer, so we may use it directly
      rot_bitmap_layer_increment_angle(icon_layer, (new_tuple->value->uint8)*ANGLE_MULTIPLIER);
      layer_mark_dirty((Layer*)icon_layer);
      break;
    case SECONDS_KEY:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "SECONDS_KEY");
      break;
  }
}


static void send_cmd(void) {
  Tuplet value = TupletInteger(1, 1);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_outbox_send();
}


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
  //text_layer_set_text(text_layer, "Penis");


}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  position = (position + 1) % PARTITIONS;
  //rot_bitmap_layer_increment_angle(icon_layer, 90*182);
  //layer_mark_dirty((Layer*)icon_layer);
  layer_mark_dirty(draw_layer);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Down");
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

    Tuplet initial_values[] = {
    TupletInteger(ANGLE_KEY, (uint8_t) 90),
    TupletInteger(SECONDS_KEY, (uint8_t) 0)
  };

  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);
}

static void window_unload(Window *window) {
  //text_layer_destroy(text_layer);
  app_sync_deinit(&sync);
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
