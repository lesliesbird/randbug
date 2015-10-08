#include <pebble.h>

#define ANIM_SPEED 30
static Window *window;
static TextLayer *TimeLayer;
static TextLayer *AMPMLayer;
static TextLayer *DayLayer;
static Layer *RandLayer;
static char TimeText[] = "00:00";
static char AMPM[] = "XX";
static char DayText[] = "Xxx";
AppTimer *timer_handle;

uint8_t fb_data[18][138], pixel_x, pixel_y, pixel_byte, pixel_bit;
uint8_t powers[8] = {1,2,4,8,16,32,64,128};
int x, y;

void Draw_Pixel() {
    
    pixel_byte = x / 8;
    pixel_bit = x % 8;
    fb_data[pixel_byte][y] = fb_data[pixel_byte][y] | powers[pixel_bit];

}

void randlayer_update_callback(Layer *layer, GContext* ctx) {
    

    graphics_context_set_stroke_color(ctx, GColorWhite);

    
    for (pixel_x=0; pixel_x <144; pixel_x++) {
        for (pixel_y=0; pixel_y <138; pixel_y++) {

            pixel_byte = pixel_x / 8;
            pixel_bit = pixel_x % 8;
            
            if ((fb_data[pixel_byte][pixel_y] & powers[pixel_bit]) > 0) {

                    graphics_draw_pixel(ctx, GPoint(pixel_x, pixel_y));
                }
            }
        }
    
    x = rand() % 144;
    y = rand() % 138;
    Draw_Pixel();

    }
    
    void handle_timer() {
        
        layer_mark_dirty(RandLayer);
        timer_handle = app_timer_register(ANIM_SPEED, handle_timer, NULL);
        
    }

        void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
            
            char *TimeFormat;
            
            if (clock_is_24h_style()) {
                TimeFormat = "%R";
            } else {
                
                TimeFormat = "%I:%M";
            }
            
            strftime(TimeText, sizeof(TimeText), TimeFormat, tick_time);
            if (!clock_is_24h_style() && (TimeText[0] == '0')) {
                memmove(TimeText, &TimeText[1], sizeof(TimeText) - 1);
            }
            
            text_layer_set_text(TimeLayer, TimeText);
            
            if (!clock_is_24h_style()) {
                strftime(AMPM, sizeof(AMPM), "%p", tick_time);
                text_layer_set_text(AMPMLayer, AMPM);
            }
            
            strftime(DayText, sizeof(DayText), "%a", tick_time);
            text_layer_set_text(DayLayer, DayText);
            
            
        }
        
        void handle_init(void) {
            
            window = window_create();
            window_stack_push(window, true /* Animated */);
            Layer *window_layer = window_get_root_layer(window);
            window_set_background_color(window, GColorBlack);
            RandLayer = layer_create(GRect(0,0,144,138));
            layer_set_update_proc(RandLayer, randlayer_update_callback);
            layer_add_child(window_layer, RandLayer);
            
            TimeLayer = text_layer_create(GRect(0, 138, 144, 33));
            text_layer_set_text_color(TimeLayer, GColorWhite);
            text_layer_set_background_color(TimeLayer, GColorBlack);
            text_layer_set_text_alignment(TimeLayer, GTextAlignmentCenter);
            text_layer_set_font(TimeLayer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_REGULAR_30)));
            layer_add_child(window_layer, text_layer_get_layer(TimeLayer));
            
            if (!clock_is_24h_style()) {
                AMPMLayer = text_layer_create(GRect(106, 142, 30, 14));
                text_layer_set_text_color(AMPMLayer, GColorWhite);
                text_layer_set_background_color(AMPMLayer, GColorBlack);
                text_layer_set_text_alignment(AMPMLayer, GTextAlignmentCenter);
                text_layer_set_font(AMPMLayer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_REGULAR_14)));
                layer_add_child(window_layer, text_layer_get_layer(AMPMLayer));
            }
            
            DayLayer = text_layer_create(GRect(0, 142, 50, 14));
            text_layer_set_text_color(DayLayer, GColorWhite);
            text_layer_set_background_color(DayLayer, GColorClear);
            text_layer_set_text_alignment(DayLayer, GTextAlignmentCenter);
            text_layer_set_font(DayLayer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_REGULAR_14)));
            layer_add_child(window_layer, text_layer_get_layer(DayLayer));
            
            
            srand(time(NULL));
            
            time_t now = time(NULL);
            struct tm *current_time = localtime(&now);
            handle_minute_tick(current_time, MINUTE_UNIT);
            timer_handle = app_timer_register(ANIM_SPEED, handle_timer, NULL);
            tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
            
        }
        
        void handle_deinit(void) {
            
            tick_timer_service_unsubscribe();
            text_layer_destroy(TimeLayer);
            text_layer_destroy(AMPMLayer);
            text_layer_destroy(DayLayer);
            layer_destroy(RandLayer);
            window_destroy(window);
            
        }
        
        int main(void) {
            
            handle_init();
            app_event_loop();
            handle_deinit();
            
        }
