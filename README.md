#Console Layer for Pebble
  Creates a layer allowing you to APP_LOG to the screen or perform any other scrolling text output.

##TL;DR How to use:
#####Create:  
    Layer *root_layer = window_get_root_layer(my_window);
    Layer *my_console_layer = console_layer_create(layer_get_frame(root_layer));
    layer_add_child(root_layer, my_console_layer);

#####Write:  
    console_layer_write_text(my_console_layer, "Hello\nWorld!");

#####Destroy:
    layer_destroy(my_console_layer);

#####TL;DR Notes:  
- The console layer has 500 byte buffer (by default) which written text is deep copied into.  
- You can have more than one console layer at the same time, each has a separate text buffer & style.  
- Uses a standard Layer pointer, so most standard Pebble layer functions work.  
- You can change the layer style which can affect text even after text is written.  
- Layer will automatically dirty after writing, unless you turn that setting off.  


----------------------------------------

###TL;DR: Addendum
#####Configure:
    console_layer_set_style(my_console_layer,                           // Pointer to which console layer
                            GColorBlack,                                // The layer's text's color
                            GColorClear,                                // The layer's background color
                            fonts_get_system_font(FONT_KEY_GOTHIC_14),  // The layer's text's font
                            GTextAlignmentLeft,                         // The layer's text's alignment
                            true);                                      // The layer's word wrap
                            
Note that those settings above is what console_layer_create() sets by default.


#####Write Fancy:
    console_layer_write_text_styled(my_console_layer,                          // Pointer to which console layer
                                    "Hello\nWorld!",                           // Text is deep copied to layer so can be from a temporary source
                                    GColorGreen,                               // Text Color       (GColorInherit to use layer's text color)
                                    GColorBlack,                               // Background Color (GColorInherit to use layer's background color)
                                    fonts_get_system_font(FONT_KEY_GOTHIC_28), // Font             (GFontInherit to use layer's setting)
                                    GTextAlignmentCenter,                      // Text Alignment   (GTextAlignmentInherit to use layer's setting)
                                    true);                                     // Word Wrap        (WordWrapInherit to use layer's setting)
    
If you use an inherit setting, changing layer's style affects text, even after being written.  


----------------------------------------

##List of Functions

Beware that redrawing console layers modifies the following graphics context settings:  
- Fill Color  
- Text Color  

#####Inherit Values

Use these Inherit values with `console_layer_write_text_styled()` to specify if you want the text to inherit styling settings from the console_layer.

    GTextAlignmentInherit
    GColorInherit
    GFontInherit
    WordWrapInherit

#####Create and Destroy Layers

Create a layer (defaults to 500 byte buffer)

    Layer* console_layer_create(GRect frame)

Create layer with a specified buffer size

    Layer* console_layer_create_with_buffer_size(GRect frame, int buffer_size)

You can just use the standard `layer_destroy` to destroy a console layer, however the library provides this macro for consistency.

    console_layer_destroy(Layer *console_layer)

#####Gets

Functions to get the console layer's current style settings

    GColor         console_layer_get_background_color(Layer *console_layer)
    GColor         console_layer_get_text_color      (Layer *console_layer)
    GTextAlignment console_layer_get_alignment       (Layer *console_layer)
    bool           console_layer_get_word_wrap       (Layer *console_layer)
    GFont          console_layer_get_font            (Layer *console_layer)

#####Sets  

Functions to change the console layer's style settings

    void console_layer_set_background_color(Layer *console_layer, GColor         background_color)
    void console_layer_set_text_color      (Layer *console_layer, GColor         text_color)
    void console_layer_set_alignment       (Layer *console_layer, GTextAlignment alignment)
    void console_layer_set_word_wrap       (Layer *console_layer, bool           word_wrap)
    void console_layer_set_font            (Layer *console_layer, GFont          font)

Or set the console layer's style with one function  

    void console_layer_set_style(Layer         *console_layer,
                                 GColor         text_color,
                                 GColor         background_color,
                                 GFont          font,
                                 GTextAlignment alignment,
                                 bool           word_wrap)


#####Write Text

Write text to the layer using layer's style  

    void console_layer_write_text(Layer *console_layer, char *text)
    
Write styled text to the buffer  

    void console_layer_write_text_styled(Layer *console_layer,
                                         char *text,
                                         GColor text_color,
                                         GColor background_color,
                                         GFont font,
                                         GTextAlignment alignment,
                                         int word_wrap)

Clear the layer of its text (leaves the style alone)  

    void console_layer_clear(Layer *console_layer)
    
----------------------------------------

##How to use Console Layer (Full Instructions)

#####1) Create the console layer:  

Create the console layer and add it as a child to the window or another layer, just as you would creating a normal layer:  

    Layer *my_console_layer;
    Layer *root_layer = window_get_root_layer(my_window);
    GRect window_frame = layer_get_frame(root_layer);
    my_console_layer = console_layer_create(window_frame);
    layer_add_child(root_layer, my_console_layer);
  
You can create multiple layers:

    Layer *my_second_console_layer = console_layer_create(GRect(x, y, w, h));
    layer_add_child(root_layer, my_second_console_layer);

If you have more than one console layer, each will have its own buffer which, by default, is 500 bytes. With a 500 byte buffer, the whole layer takes up 588 bytes.  If that is too much, or if the layer is small or only needs to display a small amount of text, you can create a layer with a smaller buffer:
    
    // Create a console layer with a 100 byte buffer
    Layer *my_console_layer = console_layer_create_with_buffer_size(Grect(x, y, w, h), 100);
    
All text written to the layer is deep copied to the layer's buffer, so it can be from a temporary source.
  
If `console_layer_write_text()` is used, the buffer only fills with text:  
- 1 byte  for each Standard ASCII Character  
- 4 bytes for each Emoji and Unicode character  
- 1 byte  for the string-terminating null character  
    
If `console_layer_write_text_styled()` is used, for each setting that isn't set to inherit:  
- 1 byte  for the text color (0 bytes for GColorInherit or GColorClear)  
- 1 byte  for the background color (0 bytes for GColorInherit or GColorClear)  
- 4 bytes for the font pointer (0 bytes for GFontInherit)  
- 0 bytes for text alignment and wordwrap settings  
  
  
#####2) Set up your console layer style:  

Each console layer has a set way to display its text -- its style.  The text written to it does not have to use this style, but most of the time it does.  When you create a new console layer, the default style settings will be:  

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

Though if you are changing the whole layer's style, it is more efficient and preferred to use the function   `console_layer_set_style()`.  If you are changing most of the layer's settings but not all of them, you can either call each setting individually like above, or you can use the console_layer_get_*() functions:

    console_layer_set_style(my_console_layer,                                // Pointer to which console layer
                            GColorBlack,                                     // The layer's text's color
                            GColorClear,                                     // The layer's background color
                            fonts_get_system_font(FONT_KEY_GOTHIC_09),       // The layer's text's font
                            console_layer_get_alignment(my_console_layer),   // Don't change the layer's text's alignment
                            console_layer_get_word_wrap(my_console_layer));  // Don't change the layer's word wrap
  
#####3) Writing to the console layer
#####3a) Write simple text  

To write to the console layer with the layer's styling, use the function:  

    console_layer_write_text(my_console_layer, "Hello World");
  
You can write any text the Pebble supports, including emoji and in Unicode.  You can also include newline (\n) characters, but any text after the newline won't be displayed unless you set word wrap to true.  Writing text will deep copy the text to the layer's buffer, and so therefore can be from a temporary source.  The text will appear on the layer with the layer's font, colors and other settings.  If you call any `console_layer_set_*()` functions after the text is written, it will reflect the new changes.  The `console_layer_write_text` function does not support displaying variables and advanced text.  To display these, use the snprintf() function:  

    char text[10];
    snprintf(text, sizeof(text), "Answer: %d", answer);
    console_layer_write_text(my_console_layer, text);



#####3b) Write in fancy text

You can also write styled text with a different font, color, word wrap and/or alignment using the function:

    console_layer_write_text_styled(my_console_layer,                          // Pointer to which console layer
                                    "Hello\nWorld!",                           // Text is deep copied to layer so can be from a temporary source
                                    GColorGreen,                               // Text Color       (GColorInherit to use layer's text color)
                                    GColorBlack,                               // Background Color (GColorInherit to use layer's background color)
                                    fonts_get_system_font(FONT_KEY_GOTHIC_28), // Font             (GFontInherit to use layer's setting)
                                    GTextAlignmentCenter,                      // Text Alignment   (GTextAlignmentInherit to use layer's setting)
                                    true);                                     // Word Wrap        (WordWrapInherit to use layer's setting)
  
The inherit values mentioned are if you only want to change some of the settings. For instance, say you only want to change the text color, you can use the following inherit settings:

    console_layer_write_text_styled(my_console_layer, "Gray text!", GColorLightGray, GColorInherit, GFontInherit, GTextAlignmentInherit, WordWrapInherit);
  
Any inherited settings will take their settings from the console layer.  If the console layer changes its settings, the written text will also show these changes, even long after the text has been written.  You can set all settings as inherited, but it would be exactly like calling `console_layer_write_text()`.  In fact, this is exactly how `console_layer_write_text()` works.


#####3c) Clear the layer of text

You can clear your layer using the function:  

    console_layer_clear(my_console_layer);
  
This will clear the text from the layer and any specially formatted lines, but leaves the layer's font, alignment, word wrap and colors.


#####4) Optional: Mark as dirty  

Any changes performed in steps 2 and 3 won't be displayed until the next time the layer is drawn. The layer will be drawn during the next scheduled redraw session which won't be called until the layer is marked dirty. You can mark the console layer dirty the same way as any other layer:  
  
    layer_mark_dirty(my_console_layer);

However, by default, the layer will be dirtied anytime any text is written to the layer or the layer's settings change.  To change this, in the console.h file is a line that sets true or false to the following setting:  

    dirty_console_layer_automatically true
    
- If set to true, layer_mark_dirty() will be called any time the console_layer changes  
- If set to false, you'll have to manually call "layer_mark_dirty(my_console_layer)" any time you want to see changes.  
It's just one setting for all layers controlled at compile-time.  For more control, see Console Layer Plus below.



#####5) Destroy the layer
Once you are done with the layer, you can destroy it with the standard Pebble layer_destroy function:

    layer_destroy(my_console_layer);
    
This will free up the space on the heap taken by the the layer struct and its text buffer.  You can also call:  

    console_layer_destroy(my_console_layer)

but this is just a macro for the standard layer_destroy function.



#####6) Other Layer Functions

Console layers use a standard Pebble Layer pointer, therefore you can use most standard Pebble layer functions with your console layer including (but not limited to):

      layer_get_window
      layer_set_hidden
      layer_get_frame
      layer_insert_below_sibling
      
  And more.  Do *not* use `layer_set_update_proc` or else the layer will no longer properly draw.

-----------------------------

###Console Layer Plus
A different version of this library is avilable at:  
  http://github.com/robisodd/console_layer_2
    
It has more features, including:  
- Multiple fonts, colors and alignments on the same line  
- Writing images inline or behind text  
- Writing text without advancing, allowing separate write statements to write to the same line.  
- Border around the layer  
- Header above the layer  
- A settable internal Margin so text doesn't touch sides/top/bottom  
- Dirty automatically settable per layer  
and more!


Licensing information
---------------------

This project is under the following license:

The MIT License (MIT)

Copyright (c) 2016 Rob Spiess

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

https://opensource.org/licenses/MIT
