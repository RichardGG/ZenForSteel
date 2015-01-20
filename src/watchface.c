#include <pebble.h>

//WINDOW
Window* window;
//LAYER
Layer* layer;
//FONT
GFont raleway_font;
//BACKGROUND
GBitmap* background;
//SETTINGS
bool seconds = true;
bool weekday = true;
bool date = false;
bool month = false;

void update_layer(Layer *me, GContext* ctx) 
{
	//watchface drawing
	
	char text[10];
	
	//draw background
	graphics_draw_bitmap_in_rect(ctx,background,GRect(0,0,144,168));
	
	//get tick_time
	time_t temp = time(NULL); 
  	struct tm *tick_time = localtime(&temp);
	
	graphics_context_set_text_color(ctx, GColorWhite);
	
	//get weekday
	strftime(text, 10, "%A", tick_time);
	//lowercase
	text[0] += 32;
	
	if(weekday == 1)
		graphics_draw_text(ctx, text,  raleway_font, GRect(0,-6,144,100), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
	
	strftime(text, 10, "%d", tick_time);
	
	if(date == 1)
		graphics_draw_text(ctx, text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0,62,144,100), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
	
	strftime(text, 10, "%B", tick_time);
	//lowercase
	text[0] += 32;
	
	if(month == 1)
		graphics_draw_text(ctx, text,  fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(0,113,144,100), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
	
	//draw hands
	GPoint center = GPoint(71,99);
	int16_t secondHandLength = 64;
	int16_t minuteHandLength = 54;
	int16_t hourHandLength = 34;
	GPoint secondHand;
	GPoint minuteHand;
	GPoint hourHand;
	
	graphics_context_set_stroke_color(ctx,GColorWhite);

	int32_t second_angle = TRIG_MAX_ANGLE * tick_time->tm_sec / 60;
	secondHand.y = (int16_t)(-cos_lookup(second_angle) * (int32_t)secondHandLength / TRIG_MAX_RATIO) + center.y;
	secondHand.x = (int16_t)(sin_lookup(second_angle) * (int32_t)secondHandLength / TRIG_MAX_RATIO) + center.x;
	
	if(seconds == 1)
		graphics_draw_line(ctx, center, secondHand);
	
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
	bool addX = (tick_time->tm_min > 20 && tick_time->tm_min < 40) || tick_time->tm_min < 10 || tick_time->tm_min > 50;
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

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
	
	APP_LOG(APP_LOG_LEVEL_INFO, "inbox");
	
	//seconds
	Tuple *t = dict_find(iterator, 0);
	seconds = (int)t->value->int32 == 1;
		
	tick_timer_service_unsubscribe();	
	tick_timer_service_subscribe(seconds?SECOND_UNIT:MINUTE_UNIT, (TickHandler) tick);
	
	//weekday
	t = dict_find(iterator, 1);
	weekday = (int)t->value->int32 == 1;
	
	//date
	t = dict_find(iterator, 2);
	date = (int)t->value->int32 == 1;
	
	//month
	t = dict_find(iterator, 3);
	month = (int)t->value->int32 == 1;
	
	//redraw
	layer_mark_dirty(layer);
		
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {}


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
	
	//subscribe to seconds tick event
	tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) tick);
	
	// Register callbacks
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);

	// Open AppMessage
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
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
