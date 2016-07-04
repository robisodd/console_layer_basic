#include <pebble.h>
#include "console.h"

/*
----------------------------------------------------------------------------------------------------
 TL;DR: How to use example
--------------------------------------------------
Create:
    Layer *root_layer = window_get_root_layer(my_window);
    Layer *my_console_layer = console_layer_create(layer_get_frame(root_layer));
    layer_add_child(root_layer, my_console_layer);

Write:
    console_layer_write_text(my_console_layer, "Hello\nWorld!");  // Word Wrap must be on for \n

Destroy:
    layer_destroy(my_console_layer);

TL;DR Notes:
  You can have more than 1 console layer at the same time, each has a separate text buffer & style.
  Uses a standard Layer pointer, so most standard Pebble layer functions work.
  You can change the layer style after text is written (doesn't affect console_layer_write_text_styled)
  Layer will automatically dirty after writing, unless you turn that setting off.



----------------------------------------------------------------------------------------------------
 TL;DR: Addendum
--------------------------------------------------
Configure:
    console_layer_set_style(my_console_layer,                           // Pointer to which console layer
                            GColorBlack,                                // The layer's text's color
                            GColorClear,                                // The layer's background color
                            fonts_get_system_font(FONT_KEY_GOTHIC_14),  // The layer's text's font
                            GTextAlignmentLeft,                         // The layer's text's alignment
                            true);                                      // The layer's word wrap
                            
    Note that those settings above is what console_layer_create() sets by default.


Write Fancy:
    console_layer_write_text_styled(my_console_layer,                          // Pointer to which console layer
                                    "Hello\nWorld!",                           // Text is deep copied to layer so can be from a temporary source
                                    GColorGreen,                               // Text Color       (GColorInherit to use layer's text color)
                                    GColorBlack,                               // Background Color (GColorInherit to use layer's background color)
                                    fonts_get_system_font(FONT_KEY_GOTHIC_28), // Font             (GFontInherit to use layer's setting)
                                    GTextAlignmentCenter,                      // Text Alignment   (GTextAlignmentInherit to use layer's setting)
                                    true);                                     // Word Wrap        (WordWrapInherit to use layer's setting)
    
    If you use an inherit setting, changing layer's style affects text, even after being written.



----------------------------------------------------------------------------------------------------
 How to use "Console Layer" (Full Instructions)
--------------------------------------------------
1) Create the console layer:

Create the console layer and add it as a child to the window or another layer, just as you would creating a normal layer:
    Layer *my_console_layer;
    Layer *root_layer = window_get_root_layer(my_window);
    GRect window_frame = layer_get_frame(root_layer);
    my_console_layer = console_layer_create(window_frame);
    layer_add_child(root_layer, my_console_layer);
  
  You can create multiple layers:
    Layer *my_second_console_layer = console_layer_create(GRect(x, y, w, h));
    layer_add_child(root_layer, my_second_console_layer);

  If you have more than one console layer, each will have its own buffer which, by default, is 500 bytes.
  With a 500 byte buffer, the whole layer takes up 588 bytes.  If that is too much, or if the layer is small or
    only needs to display a small amount of text, you can create a layer with a smaller buffer:
    
    Layer *my_console_layer = console_layer_create_with_buffer_size(Grect(x, y, w, h), 100);  // 100 byte buffer
    
  All text written to the layer is deep copied to the layer's buffer, so it can be from a temporary source.
  
  If console_layer_write_text() is used, the buffer only fills with text:
    1 byte  for each Standard ASCII Character
    4 bytes for each Emoji and UNICODE character
    1 byte  for the string-terminating null character
    
  If console_layer_write_text_styled() is used, for each setting that isn't set to inherit:
    1 byte  for the text color (0 bytes for GColorInherit or GColorClear)
    1 byte  for the background color (0 bytes for GColorInherit or GColorClear)
    4 bytes for the font pointer (0 bytes for GFontInherit)
    0 bytes for text alignment and wordwrap settings
  
  
2) Set up your console layer style:
  Each console layer has a set way to display its text -- its style.  The text written to it
    does not have to use this style, but most of the time it does.
    
  When you create a new console layer, the default style settings will be:
      Text Color: GColorBlack
      Background: GColorClear
            Font: FONT_KEY_GOTHIC_14
       Alignment: GTextAlignmentLeft
       Word Wrap: true
      
  If you wish to change these default settings, you can set them individually using the following functions:
   console_layer_set_background_color(my_console_layer, GColorYellow);
   console_layer_set_text_color      (my_console_layer, GColorBlack);
   console_layer_set_alignment       (my_console_layer, GTextAlignmentRight);
   console_layer_set_word_wrap       (my_console_layer, true);
   console_layer_set_font            (my_console_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MY_FONT));

  Though if you are changing the whole layer's style, it is more efficient and preferred to use the function:
  console_layer_set_style()
  
  If you are changing most of the layer's settings but not all of them, you can either call each setting individually like above,
  or you can use the console_layer_get_*() functions:
    console_layer_set_style(my_console_layer,                                // Pointer to which console layer
                            GColorBlack,                                     // The layer's text's color
                            GColorClear,                                     // The layer's background color
                            fonts_get_system_font(FONT_KEY_GOTHIC_09),       // The layer's text's font
                            console_layer_get_alignment(my_console_layer),   // Don't change the layer's text's alignment
                            console_layer_get_word_wrap(my_console_layer));  // Don't change the layer's word wrap
  
                          
                          
                          
3) Writing to the console layer

3a) Write simple text
  Write with the function:
    console_layer_write_text(my_console_layer, "Hello World");
  
  You can write any text the Pebble supports, including Emoji and UNICODE.  Writing text will deep copy the text to the layer's buffer,
  and so therefore can be from a temporary source.  The text will appear on the layer with the layer's font, colors and other settings.
  If you call any console_layer_set_*() functions after the text is written, it will reflect the new changes.
  
  The write_text function does not support displaying variables and advanced text.  To display these, use the snprintf() function:
    char text[10];
    snprintf(text, sizeof(text), "Answer: %d", answer);
    console_layer_write_text(my_console_layer, text);

  You can also include newline (\n) characters, but any text after the newline won't be displayed unless you set word wrap to true.


3b) Write in fancy text
  You can also write styled text with a different font, color, word wrap and/or alignment using the function:
  
    console_layer_write_text_styled(my_console_layer,                          // Pointer to which console layer
                                    "Hello\nWorld!",                           // Text is deep copied to layer so can be from a temporary source
                                    GColorGreen,                               // Text Color       (GColorInherit to use layer's text color)
                                    GColorBlack,                               // Background Color (GColorInherit to use layer's background color)
                                    fonts_get_system_font(FONT_KEY_GOTHIC_28), // Font             (GFontInherit to use layer's setting)
                                    GTextAlignmentCenter,                      // Text Alignment   (GTextAlignmentInherit to use layer's setting)
                                    true);                                     // Word Wrap        (WordWrapInherit to use layer's setting)
  
  The inherit values mentioned are if you only want to change some of the settings.
  For instance, say you only want to change the text color, you can use the following Inherit settings:
    console_layer_write_text_styled(my_console_layer, "Gray text!", GColorLightGray, GColorInherit, GFontInherit, GTextAlignmentInherit, WordWrapInherit);
  
  Any inherited settings will take their settings from the console layer.
  If the console layer changes its settings, the written text will also show these changes, even long after the text has been written.
  
  You can set all settings as Inherited, but it would be exactly like calling console_layer_write_text().
  In fact, this is exactly how console_layer_write_text() works.


3c) Clear the layer of text

You can clear your layer using the function:
  console_layer_clear(my_console_layer);
  
This will clear the text from the layer and any specially formatted lines, but leaves the layer's font, alignment, word wrap and colors.


4) Optional: Mark as dirty
  Any changes performed in steps 2 and 3 won't be displayed until the next time the layer is drawn.
  The layer will be drawn during the next scheduled redraw session which won't be called until the layer is marked dirty.
  You can mark the console layer dirty the same way as any other layer:
  
    layer_mark_dirty(my_console_layer);
    
  However, by default, the layer will be dirtied anytime any text is written to the layer or the layer's settings change.

  To change this, in the console.h file is a line that sets true or false to the following setting:

    dirty_console_layer_automatically
    
  If set to true, layer_mark_dirty() will be called any time the console_layer changes
  If set to false, you'll have to manually call "layer_mark_dirty(my_console_layer)" any time you want to see changes.
  It's just one setting for all layers controlled at compile-time.  For more control, see Console Layer Plus below.



5) Destroy the layer
  Once you are done with the layer, you can destroy it with the standard Pebble layer_destroy function:
    layer_destroy(my_console_layer);
  This will free up the space on the heap taken by the the layer struct and its text buffer.
  You can also call console_layer_destroy(my_console_layer), but this is just a macro for the standard layer_destroy function



6) Other Layer Functions
Console Layers use a standard Layer pointer, so therefore you can use
most standard Pebble layer functions with your console layer including (but not limited to):
      layer_get_window, layer_set_hidden, layer_get_frame, and layer_add_child
  Don't use layer_set_update_proc
  



----------------------------------------------------------------------------------------------------
 Console Layer Plus
--------------------------------------------------
  A different version of this library is avilable at:
    http://github.com/robisodd/console_layer_2
    
  It has more features, including:
    Multiple fonts, colors and alignments on the same line
    Writing images inline or behind text
    Writing text without advancing, allowing separate write statements to write to the same line.
    Border around the layer
    Header above the layer
    A settable internal Margin so text doesn't touch sides/top/bottom
    Dirty automatically settable per layer
    and more!
*/

static Window *main_window;
static Layer *console_layer, *mini_console_layer;

// ---------------------------------------------------------------------------------------------- //
//  Button Functions
// ---------------------------------------------------------------------------------------------- //
static void up_hold_click_handler(ClickRecognizerRef recognizer, void *context) { //  UP  button held
  // Write some text but color it blue (use layer's settings for all other properties)
  // Notice the blue stays blue despite pressing down and changing the layer's style.  All Inherited properties change, though.
  console_layer_write_text_styled(console_layer, "Gray area!", GColorInherit, GColorLightGray, GFontInherit, GTextAlignmentInherit, WordWrapInherit);
  console_layer_write_text(mini_console_layer, "Up Held");
}


static void up_click_handler(ClickRecognizerRef recognizer, void *context) {      //   UP   button
  // Write some text using the layer's settings
  console_layer_write_text(console_layer, "Hello World!");
  console_layer_write_text(mini_console_layer, "Up Pressed");
}


static void sl_hold_click_handler(ClickRecognizerRef recognizer, void *context) { // SELECT button held
  // Toggle mini console layer (as an example of hiding your log unless you want to see it)
  layer_set_hidden(mini_console_layer, !layer_get_hidden(mini_console_layer));
}


static void sl_click_handler(ClickRecognizerRef recognizer, void *context) {      // SELECT button pressed briefly
  // Emoji and color test. Notice the this text doesn't change when pressing down as no properties are set to Inherit from the layer
  console_layer_write_text_styled(console_layer, "ERROR: \U0001F4A9 Detected", PBL_IF_COLOR_ELSE(GColorRed, GColorWhite), GColorBlack, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, true);
  console_layer_write_text_styled(mini_console_layer, "ERROR!", PBL_IF_COLOR_ELSE(GColorRed, GColorWhite), GColorInherit, GFontInherit, GTextAlignmentInherit, WordWrapInherit);
}


static void dn_click_handler(ClickRecognizerRef recognizer, void *context) {      //  DOWN  button pressed briefly
  // Randomly change the layer's text alignment text color and font
  // Notice that font, colors, word wrap and alignment can change (even after written) when using
  //   console_layer_write_text() or when using the function
  //   console_layer_write_text_styled() with "inherit" settings, but that text written using
  //   console_layer_write_text_styled() with specific settings retain those settings.
  switch(rand()%3) {
    case 0: console_layer_set_alignment(console_layer, GTextAlignmentLeft); break;
    case 1: console_layer_set_alignment(console_layer, GTextAlignmentCenter); break;
    case 2: console_layer_set_alignment(console_layer, GTextAlignmentRight); break;
  }
  switch(rand()%4) {
    case 0: console_layer_set_font(console_layer, fonts_get_system_font(FONT_KEY_GOTHIC_09));      break;
    case 1: console_layer_set_font(console_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));      break;
    case 2: console_layer_set_font(console_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD)); break;
    case 3: console_layer_set_font(console_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));break;
  }
  switch(rand()%4) {
    case 0: console_layer_set_text_color(console_layer, GColorBlack);  break;
    case 1: console_layer_set_text_color(console_layer, GColorGreen); break;
    case 2: console_layer_set_text_color(console_layer, GColorOrange); break;
    case 3: console_layer_set_text_color(console_layer, GColorPurple);  break;
  }
  console_layer_write_text(mini_console_layer, "Changed Style");
}


static void dn_hold_click_handler(ClickRecognizerRef recognizer, void *context) { //  DOWN  button held
  console_layer_clear(console_layer);
  console_layer_write_text(mini_console_layer, "Layer Cleared");
}


static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP,     up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, sl_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN,   dn_click_handler);
  window_long_click_subscribe  (BUTTON_ID_UP,     0, up_hold_click_handler, NULL);
  window_long_click_subscribe  (BUTTON_ID_SELECT, 0, sl_hold_click_handler, NULL);
  window_long_click_subscribe  (BUTTON_ID_DOWN,   0, dn_hold_click_handler, NULL);
}


// ---------------------------------------------------------------------------------------------- //
//  Main Functions
// ---------------------------------------------------------------------------------------------- //
static void main_window_load(Window *window) {
  window_set_click_config_provider(window, click_config_provider);
  
  // Create the console layer
  Layer *root_layer = window_get_root_layer(window);
  GRect rect = grect_inset(layer_get_frame(root_layer), GEdgeInsets(PBL_IF_ROUND_ELSE(26, 5)));  // Bring it in from the edge a bit
  console_layer = console_layer_create(rect);                          // Create layer with 500 byte buffer
  //console_layer = console_layer_create_with_buffer_size(rect, 100);  // Create layer with 100 byte buffer (try watching text disappear!)
  layer_add_child(root_layer, console_layer);
    
  // Change background color (default is clear background)
  console_layer_set_background_color(console_layer, GColorWhite);
  
  // Enable word wrap (default has word wrap off)
  console_layer_set_word_wrap(console_layer, true);
  

  // Create a second mini console layer
  int16_t width = 80, height = 50;
  rect = GRect((layer_get_frame(root_layer).size.w - width) / 2, 10, width, height);
  mini_console_layer = console_layer_create_with_buffer_size(rect, 100);  // Create layer with 100 byte buffer (it's a tiny layer)
  layer_add_child(root_layer, mini_console_layer);
  
  // Configure mini console layer with white on black word-wrapped text using a tiny centered font
  console_layer_set_style(mini_console_layer, GColorWhite, GColorBlack, fonts_get_system_font(FONT_KEY_GOTHIC_09), GTextAlignmentCenter, true);
  
  // Hiding the mini console layer
  // Note: uses the standard Pebble set hidden function -- most standard Layer functions will work on Console Layers
  layer_set_hidden(mini_console_layer, true);
  
  // Write some text
  console_layer_write_text(mini_console_layer, "Program Started");
  console_layer_write_text(console_layer, "Welcome!\nTry pressing and holding some buttons!");
}


static void main_window_unload(Window *window) {
  // Destroy the layers with the standard Pebble layer destroy function
  layer_destroy(mini_console_layer);
  layer_destroy(console_layer);
}


static void init() {
  // Create main Window
  main_window = window_create();
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_set_background_color(main_window, GColorDarkGray);
  window_stack_push(main_window, true);
}


static void deinit() {
  window_destroy(main_window);  // Destroy main Window
}


int main(void) {
  init();
  app_event_loop();
  deinit();
}