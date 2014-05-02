#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static BitmapLayer *icon_layer;
static GBitmap *icon_bitmap = NULL;
static Layer *draw_layer;
//144x168
#define SCREEN_WIDTH 144
#define SCREEN_HEIGHT 168

int position = 0;

static uint32_t WEATHER_ICONS[] = {
  RESOURCE_ID_IMAGE_CLOCK
};

static void draw_lines(Layer *layer, GContext *ctx, int start_loop, int end_loop, int positive, int horizontal, int fixed_point) {
  GPoint center = GPoint(SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
  if (horizontal){
    if (positive){
     for (int i = start_loop; i <= end_loop; i++){
        graphics_draw_line(ctx,GPoint(i,fixed_point),center);
      }
    }
    else{
      for (int i = start_loop; i <= end_loop; i--){
        graphics_draw_line(ctx,GPoint(i,fixed_point),center);
      }
    }
  }
  else {
    if (positive){
      for (int i = start_loop; i <= end_loop; i++){
        graphics_draw_line(ctx,GPoint(fixed_point,i),center);
      }
    }
    else{
      for (int i = start_loop; i <= end_loop; i--){
        graphics_draw_line(ctx,GPoint(fixed_point,i),center);
      }
    }
  }

}

static void draw_one(Layer *layer, GContext *ctx) {
   draw_lines(layer,ctx,SCREEN_WIDTH/2, 7*SCREEN_WIDTH/10, true,true,0);
}

static void draw_two(Layer *layer, GContext *ctx) {

  //draw_lines(layer,ctx, 7*SCREEN_WIDTH/10,SCREEN_WIDTH, true,true,0);
  //draw_lines(layer,ctx, 0, 3*SCREEN_HEIGHT/10,false,true,SCREEN_WIDTH );

}

static void draw_three(Layer *layer, GContext *ctx) {
GPoint center = GPoint(SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
   for (int i = SCREEN_HEIGHT/4; i <= SCREEN_HEIGHT/2; i++){
    graphics_draw_line(ctx,GPoint(SCREEN_WIDTH,i),center);
   }
}

static void draw_four(Layer *layer, GContext *ctx) {
GPoint center = GPoint(SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
   for (int i = SCREEN_HEIGHT/2; i <= 3*SCREEN_HEIGHT/4; i++){
    graphics_draw_line(ctx,GPoint(SCREEN_WIDTH,i),center);
   }
}

static void draw_five(Layer *layer, GContext *ctx) {
GPoint center = GPoint(SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
   for (int i = 3*SCREEN_HEIGHT/4; i <= SCREEN_HEIGHT; i++){
    graphics_draw_line(ctx,GPoint(SCREEN_WIDTH,i),center);
   }
}

static void draw_six(Layer *layer, GContext *ctx) {
GPoint center = GPoint(SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
   for (int i = SCREEN_WIDTH; i >= SCREEN_WIDTH/2; i--){
    graphics_draw_line(ctx,GPoint(i,SCREEN_HEIGHT),center);
   }
}

static void draw_seven(Layer *layer, GContext *ctx) {
GPoint center = GPoint(SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
   for (int i = SCREEN_WIDTH/2; i >= SCREEN_WIDTH/4; i--){
    graphics_draw_line(ctx,GPoint(i,SCREEN_HEIGHT),center);
   }
}

static void draw_eight(Layer *layer, GContext *ctx) {
GPoint center = GPoint(SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
   for (int i = 3*SCREEN_WIDTH/4; i >= 0; i--){
    graphics_draw_line(ctx,GPoint(i,SCREEN_HEIGHT),center);
   }
}

static void draw_nine(Layer *layer, GContext *ctx) {
GPoint center = GPoint(SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
   for (int i = SCREEN_HEIGHT; i >= SCREEN_HEIGHT/2; i--){
    graphics_draw_line(ctx,GPoint(0,i),center);
   }
}

static void draw_ten(Layer *layer, GContext *ctx) {
GPoint center = GPoint(SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
   for (int i = SCREEN_WIDTH/2; i >= SCREEN_WIDTH/4; i--){
    graphics_draw_line(ctx,GPoint(0,i),center);
   }
}

static void draw_eleven(Layer *layer, GContext *ctx) {
GPoint center = GPoint(SCREEN_WIDTH /2, SCREEN_HEIGHT /2);
   for (int i = 0; i <= SCREEN_WIDTH/2; i--){
    graphics_draw_line(ctx,GPoint(i,0),center);
   }
}

static void draw_layer_draw(Layer *layer, GContext *ctx) {

   // position can be between 0 and 11
   // 0 is empty
   // 11 is full
   //GRect bounds = layer_get_bounds(layer);
   //GPoint start = GPoint(SCREEN_WIDTH /2,0);
    graphics_context_set_stroke_color(ctx, GColorWhite);
   if (position >= 1 ){
      draw_one(layer,ctx);
   }
   if (position >= 2 ){
      draw_two(layer,ctx);
   }
   if (position >= 3 ){
      draw_three(layer,ctx);
   }
   if (position >= 4 ){
    draw_four(layer,ctx);
   }
   if (position >= 5 ){
    draw_five(layer,ctx);
   }
   if (position >= 6 ){
    draw_six(layer,ctx);
   }
   if (position >= 7 ){
    draw_seven(layer,ctx);
   }
   if (position >= 8 ){
    draw_eight(layer,ctx);
   }
   if (position >= 9 ){
    draw_nine(layer,ctx);
   }
   if (position >= 10 ){
    draw_ten(layer,ctx);
   }
   if (position >= 11 ){
    draw_eleven(layer,ctx);
   }




}




static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Penis");


}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  position = (position + 1) %11;
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


  icon_layer = bitmap_layer_create(bounds);
  layer_add_child(window_layer, bitmap_layer_get_layer(icon_layer));
  icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[0]);
  bitmap_layer_set_bitmap(icon_layer, icon_bitmap);
  /*
  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Press a button");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  */
  draw_layer = layer_create(bounds);
  layer_add_child(window_layer, draw_layer);
  layer_set_update_proc(draw_layer, draw_layer_draw);
}

static void window_unload(Window *window) {
  //text_layer_destroy(text_layer);
  bitmap_layer_destroy(icon_layer);

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
