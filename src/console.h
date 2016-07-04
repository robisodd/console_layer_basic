#pragma once
#include <pebble.h>
//------------------------------------------------------------------------------------------------//
// Beware that redrawing console layers modifies the following graphics context settings:
//   Fill Color
//   Text Color


//------------------------------------------------------------------------------------------------//
// Inherit Values
//------------------------------------------------------------------------------------------------//
// Use these Inherit values with console_layer_write_text_styled() to specify
//   if you want the text to inherit styling settings from the console_layer.

// Also, GTextAlignmentInherit = 50 is arbitrary and can be changed to anything that
//   doesn't equal: GTextAlignmentLeft, GTextAlignmentCenter or GTextAlignmentRight.

#define GTextAlignmentInherit  50
#define GColorInherit          GColorClear
#define GFontInherit           NULL
#define WordWrapFalse          false
#define WordWrapTrue           true
#define WordWrapInherit        2


//------------------------------------------------------------------------------------------------//
// Dirty Automatically
//------------------------------------------------------------------------------------------------//
// Set to true to have console layers self-dirty anytime they are updated, cleared or written to.
// Set to false and you'll have to manually schedule a redraw by calling layer_mark_dirty().

#define dirty_console_layer_automatically true


//------------------------------------------------------------------------------------------------//
// Create and Destroy Layers
//------------------------------------------------------------------------------------------------//
Layer* console_layer_create_with_buffer_size(GRect frame, int buffer_size);
Layer* console_layer_create                 (GRect frame);   // Creates layer with 500 byte buffer

// You can just use the standard layer_destroy to destroy a console_layer.  Adding for consistency
#define console_layer_destroy(console_layer) layer_destroy(console_layer)

//------------------------------------------------------------------------------------------------//
// Gets
//------------------------------------------------------------------------------------------------//
GColor         console_layer_get_background_color(Layer *console_layer);
GColor         console_layer_get_text_color      (Layer *console_layer);
GTextAlignment console_layer_get_alignment       (Layer *console_layer);
bool           console_layer_get_word_wrap       (Layer *console_layer);
GFont          console_layer_get_font            (Layer *console_layer);


//------------------------------------------------------------------------------------------------//
// Sets
//------------------------------------------------------------------------------------------------//
void console_layer_set_background_color(Layer *console_layer, GColor         background_color);
void console_layer_set_text_color      (Layer *console_layer, GColor         text_color);
void console_layer_set_alignment       (Layer *console_layer, GTextAlignment alignment);
void console_layer_set_word_wrap       (Layer *console_layer, bool           word_wrap);
void console_layer_set_font            (Layer *console_layer, GFont          font);

void console_layer_set_style           (Layer *console_layer,
                                        GColor text_color,
                                        GColor background_color,
                                        GFont font,
                                        GTextAlignment alignment,
                                        bool word_wrap);


//------------------------------------------------------------------------------------------------//
// Write Text
// Note: The function deep copies the source text into the console_layer's buffer
//       so it can be from a temporary source.
//------------------------------------------------------------------------------------------------//
void console_layer_clear            (Layer *console_layer);
void console_layer_write_text       (Layer *console_layer, char *text);

void console_layer_write_text_styled(Layer *console_layer,
                                     char *text,
                                     GColor text_color,
                                     GColor background_color,
                                     GFont font,
                                     GTextAlignment alignment,
                                     int word_wrap);
