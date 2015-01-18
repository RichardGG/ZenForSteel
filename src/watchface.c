#include <pebble.h>

#define SHOW_SECOND_HAND 1

//WINDOW
Window* window;
//LAYER
Layer* layer;
//FONT
GFont raleway_font;
//BACKGROUND
GBitmap* background;

void update_layer(Layer *me, GContext* ctx) 
{
	char text[10];
	
	//draw watchface
	graphics_draw_bitmap_in_rect(ctx,background,GRect(0,0,144,168));
	
	//get tick_time
	time_t temp = time(NULL); 
	struct tm *tick_time = localtime(&temp);
	
	//get weekday
	strftime(text, 10, "%A", tick_time);
	//lowercase
	text[0] += 32;
	//draw weekday text
	graphics_context_set_text_color(ctx, GColorWhite);
	graphics_draw_text(ctx, text,  raleway_font, GRect(0,-6,144,100), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
	
	//draw hands
	GPoint center = GPoint(71,99);
	int16_t minuteHandLength = 54;
	int16_t hourHandLength = 34;
	GPoint minuteHand;
	GPoint hourHand;

	#if SHOW_SECOND_HAND==1
		int16_t secondHandLength = 64;
		GPoint secondHand;
	#endif
	
	graphics_context_set_stroke_color(ctx,GColorWhite);

	#if SHOW_SECOND_HAND==1
		int32_t second_angle = TRIG_MAX_ANGLE * tick_time->tm_sec / 60;
		secondHand.y = (int16_t)(-cos_lookup(second_angle) * (int32_t)secondHandLength / TRIG_MAX_RATIO) + center.y;
		secondHand.x = (int16_t)(sin_lookup(second_angle) * (int32_t)secondHandLength / TRIG_MAX_RATIO) + center.x;
		graphics_draw_line(ctx, center, secondHand);
	#endif
	
	int32_t minute_angle = TRIG_MAX_ANGLE * tick_time->tm_min / 60;
	minuteHand.y = (int16_t)(-cos_lookup(minute_angle) * (int32_t)minuteHandLength / TRIG_MAX_RATIO) + center.y;
	minuteHand.x = (int16_t)(sin_lookup(minute_angle) * (int32_t)minuteHandLength / TRIG_MAX_RATIO) + center.x;
	graphics_draw_line(ctx, center, minuteHand);
	
	int32_t hour_angle = (TRIG_MAX_ANGLE * (((tick_time->tm_hour % 12) * 6) + (tick_time->tm_min / 10))) / (12 * 6);
	hourHand.y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)hourHandLength / TRIG_MAX_RATIO) + center.y;
	hourHand.x = (int16_t)(sin_lookup(hour_angle) * (int32_t)hourHandLength / TRIG_MAX_RATIO) + center.x;
	graphics_draw_line(ctx, center, hourHand);
	
	
	//I didn't like how a 2px path rotated, so I'm using two lines next to each other
	//I need to move the pixels from vertically adjacent to horizontally adjacent based on the position
	bool addX = (tick_time->tm_hour > 20 && tick_time->tm_hour < 40) || tick_time->tm_hour < 10 || tick_time->tm_hour > 50;
	center.x+=addX?1:0;
	center.y+=!addX?1:0;
	minuteHand.x+=addX?1:0;
	minuteHand.y+=!addX?1:0;
	graphics_draw_line(ctx, center, minuteHand);
	
	center.x-=addX?1:0;
	center.y-=!addX?1:0;
	
	addX = (tick_time->tm_hour >= 4 && tick_time->tm_hour <= 8) || tick_time->tm_hour < 2 || tick_time->tm_hour > 10;
	center.x+=addX?1:0;
	center.y+=!addX?1:0;
	hourHand.x+=addX?1:0;
	hourHand.y+=!addX?1:0;
	graphics_draw_line(ctx, center, hourHand);

}

void tick(struct tm *tick_time, TimeUnits units_changed)
{
	//redraw every tick
	layer_mark_dirty(layer);
}

void init() 
{
	//create window
	window = window_create();
	window_set_background_color(window,GColorBlack);
	window_stack_push(window, true);
	Layer* window_layer = window_get_root_layer(window);	
	
	//load background
	background = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
	
	//load font
	raleway_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RALEWAY_21));
	
	//create layer
	layer = layer_create(GRect(0,0,144,168));
	layer_set_update_proc(layer, update_layer);
	layer_add_child(window_layer, layer);	
	
	//subscribe to tick event
	#if SHOW_SECOND_HAND==1
		tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) tick);
	#else
		tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick);
	#endif
}

void deinit() 
{
	layer_destroy(layer);
	fonts_unload_custom_font(raleway_font);
	gbitmap_destroy(background);
	window_destroy(window);
}

int main()
{
	init();
	app_event_loop();
	deinit();
}
