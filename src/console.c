/*
----------------------------------------------------------------------------------------------------
  Console Layer - Basic
  Copyright (c) 2016 Rob Spiess
----------------------------------------------------------------------------------------------------
  This project is under the following license:

  The MIT License (MIT)

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
  modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  https://opensource.org/licenses/MIT
  
----------------------------------------------------------------------------------------------------
  Version History:
--------------------------------------------------
v2.1-20160703: Figured new console library is too complex
               Reduced options significantly, made easy to use
               Will possibly release old library as an advanced version of this one.
               Removed images, borders, headers, margins and other fluff
               Documented how-to guide (including TL;DR)

v2.0-20160618: Recreated Console Library
               Now stores everything in a circular buffer
               No longer wastes lots of space with lots of huge arrays and structs
               Fully configurable, saves text, image pointer, font, colors and alignments
               Layer has its own settings separate from rows settings
               Border thickness, color, size and enable settings
               Header size, text, alignment, color and font settings
               Dirty automatically settings
               Internal margin settings
               Colored backgrounds or clear
               Individually colored row settings, propogated to wordwrap lines.
               Use newline (\n) to start a new row

v1.0-20160529: Created Console Layer Library
               Improves upon old version which was basically a fixedwidth grid of text, each grid having color and character
               Each row has a struct that saves that row's font, colors, alignment and text
               Stores each row of text in a pre-allocated array
               Separate text, color and font data per row of text
               
----------------------------------------------------------------------------------------------------
 Library Sizes
--------------------------------------------------
Compiled Size: (slightly smaller than this)
  APLITE Total footprint in RAM:         1814 bytes / 24KB
  BASALT Total footprint in RAM:         1814 bytes / 64KB
  CHALK  Total footprint in RAM:         1814 bytes / 64KB
--------------------------------------------------
Running Size:
  Create Console Layer (500 byte buffer) : 588 heap bytes used
  Allocates 2nd buffer on the heap (same size as layer buffer, e.g. 500 bytes) temporarily when rendering
  Doesn't use too much stack when rendering or writing, maybe a couple dozen bytes?

----------------------------------------------------------------------------------------------------
 Data Structure and Buffer Description
--------------------------------------------------
         | |      First Chunk      |     Second Chunk      |      Third Chunk      |
 Buffer: |0|SBCFONTstring...string0|SBCFONTstring...string0|SBCFONTstring...string0|0000000---til end of buffer
          ^=BOF/EOF(pos points here)                      ^=0 terminated EndOfString (EOS)
          
       0 = 1 byte:  Circular Buffer Begin/End of file (BOF/EOF) split point (must = 0)
  string = x bytes: The 0-termindated string displayed  (required, though can = 0 for empty string)
    FONT = 4 bytes: Font Pointer          (optional, if bit b=1 in Settings Byte)
       C = 1 byte:  Text Color            (optional, if bit c=1 in Settings Byte)
       B = 1 byte:  Text Background Color (optional, if bit d=1 in Settings Byte)
       S = 1 byte:  Settings Byte
       0babcdefgh = Settings Byte
         a        1 bit:  Settings                    [Must = 1]
          b       1 bit:  Background Color Specified? [0 = no (inherit from console_layer), 1 = yes]
           c      1 bit:  Text Color Specified?       [0 = no (inherit from console_layer), 1 = yes]
            d     1 bit:  Font Specified?             [0 = no (inherit from console_layer), 1 = yes]
             ef   2 bits: Alignment                   [00=left, 01=center, 10=right,   11=inherit]
               gh 2 bits: Word Wrap                   [00=no,   01=yes,    10=inherit, 11=inherit]
               g  1 bit:  Inherit Word Wrap?          [0 = no (change), 1 = yes (inherit)]
                h 1 bit:  bit g = 1: Unused. bit g = 0: Word Wrap? (0 = no, 1 = yes)
                          "Word Wrap no" means one line of text displayed only (ends in "..." if too long)
                          "Word Wrap yes" means wrap long (and \n inside string) text to multiple lines


--------------------------------------------------
 Note that the data can wrap around the buffer:
--------------------------------------------------

        Third|Fourth|0|     First Chunk       |     Second Chunk      | Third Chunk (wraps around end of buffer)
Buffer: "ing0|SBCFON|0|SBCFONTstring...string0|SBCFONTstring...string0|SBCFONTstring...str"
                     ^ = BOF/EOF

In this example, the Fourth Chunk is invalid since it's been partially overwritten by the First Chunk (First = Most recently written, bottom of the text)

----------------------------------------------------------------------------------------------------
*/

#include "console.h"

typedef struct console_data_struct {
  bool           word_wrap;
  GColor         background_color;
  GColor         text_color;
  GFont          font;
  GTextAlignment alignment;

  size_t         buffer_size;
  uintptr_t      pos;
  char          *buffer;
} console_data_struct;
                                          // 0bABCDEFGH = Settings Byte
#define          SETTINGS_BIT  0b10000000 //   A        1 bit:  Always = 1 (Makes sure settings byte isn't 0, which would signify EOF)
#define  BACKGROUND_COLOR_BIT  0b01000000 //    B       1 bit:  Background Color Specified? [0 = no (inherit from console_layer), 1 = yes]
#define        TEXT_COLOR_BIT  0b00100000 //     C      1 bit:  Text Color Specified?       [0 = no (inherit from console_layer), 1 = yes]
#define              FONT_BIT  0b00010000 //      D     1 bit:  Font Specified?             [0 = no (inherit from console_layer), 1 = yes]
#define         ALIGNMENT_BITS 0b00001100 //       EF   2 bits: Alignment                   [00=left, 01=center, 10=right,   11=inherit]
#define         WORD_WRAP_BITS 0b00000011 //         GH 2 bits: Word Wrap                   [00=no,   01=yes,    10=inherit, 11=inherit]
#define WORD_WRAP_INHERIT_BIT  0b00000010 //         G  1 bit:  Inherit Word Wrap?          [0 = no (change), 1 = yes (inherit)]
#define         WORD_WRAP_BIT  0b00000001 //          H 1 bit:  WORD_WRAP_INHERIT_BIT = 1: Unused. WORD_WRAP_INHERIT_BIT = 0: Word Wrap
// Word Wrap: 0 = One line of text displayed only (ends in "..." if too long), 1 = Wrap Long (and \n) Text to multiple lines

#define DEFAULT_BUFFER_SIZE 500      // Size (in bytes) of text buffer -- per layer

#if (dirty_console_layer_automatically)
  #define MARK_DIRTY layer_mark_dirty(console_layer)
#else
  #define MARK_DIRTY
#endif

//------------------------------------------------------------------------------------------------//
// Gets
//----------------------------------------------//

GColor         console_layer_get_background_color(Layer *console_layer) {return ((console_data_struct*)layer_get_data(console_layer))->background_color;}
GColor         console_layer_get_text_color      (Layer *console_layer) {return ((console_data_struct*)layer_get_data(console_layer))->text_color;}
GTextAlignment console_layer_get_alignment       (Layer *console_layer) {return ((console_data_struct*)layer_get_data(console_layer))->alignment;}
bool           console_layer_get_word_wrap       (Layer *console_layer) {return ((console_data_struct*)layer_get_data(console_layer))->word_wrap;}
GFont          console_layer_get_font            (Layer *console_layer) {return ((console_data_struct*)layer_get_data(console_layer))->font;}


//------------------------------------------------------------------------------------------------//
// Sets
//----------------------------------------------//

void console_layer_set_background_color(Layer *console_layer, GColor         background_color){((console_data_struct*)layer_get_data(console_layer))->background_color = background_color; MARK_DIRTY;}
void console_layer_set_alignment       (Layer *console_layer, GTextAlignment alignment)       {((console_data_struct*)layer_get_data(console_layer))->alignment        = alignment;        MARK_DIRTY;}
void console_layer_set_text_color      (Layer *console_layer, GColor         text_color)      {((console_data_struct*)layer_get_data(console_layer))->text_color       = text_color;       MARK_DIRTY;}
void console_layer_set_word_wrap       (Layer *console_layer, bool           word_wrap)       {((console_data_struct*)layer_get_data(console_layer))->word_wrap        = word_wrap;        MARK_DIRTY;}
void console_layer_set_font            (Layer *console_layer, GFont          font)            {((console_data_struct*)layer_get_data(console_layer))->font             = font;             MARK_DIRTY;}


//----------------------------------------------//

void console_layer_set_style(Layer *console_layer, GColor text_color, GColor background_color, GFont font, GTextAlignment alignment, bool word_wrap) {
  console_data_struct *console_data = (console_data_struct*)layer_get_data(console_layer);
  console_data->text_color       = text_color;
  console_data->background_color = background_color;
  console_data->font             = font;
  console_data->word_wrap        = word_wrap;
  console_data->alignment        = alignment;
  MARK_DIRTY;
}

//------------------------------------------------------------------------------------------------//






//------------------------------------------------------------------------------------------------//
// Write Layer
//----------------------------------------------//

void console_layer_clear(Layer *console_layer) {
  console_data_struct *console_data = (console_data_struct*)layer_get_data(console_layer);
  console_data->pos = 0;
  console_data->buffer[0] = 0;
  console_data->buffer[1] = 0;
  MARK_DIRTY;
}

//----------------------------------------------//

void console_layer_write_text_styled(Layer *console_layer, char *text, GColor text_color, GColor background_color, GFont font, GTextAlignment alignment, int word_wrap) {
  console_data_struct *console_data = (console_data_struct*)layer_get_data(console_layer);

  console_data->pos += ((UINTPTR_MAX - (UINTPTR_MAX % console_data->buffer_size)) - console_data->buffer_size);

  uint8_t settings = SETTINGS_BIT;
  
  // Copy text (forwards in memory, but from last char to first char) to buffer
  char *begin = text;
  while(*text) text++;   // ends on 0
  do {
    console_data->buffer[console_data->pos-- % console_data->buffer_size] = *text;
  } while((text--)!=begin);
    

  // Assemble settings
  // Settings: If a Font is specified, copy to Buffer and flag it in Settings
  if (font) {
    for (uintptr_t i=0; i<sizeof(font); i++)
      console_data->buffer[console_data->pos-- % console_data->buffer_size] = ((uint8_t*)&font)[i];
    settings |= FONT_BIT;
  }

  //Settings: If Text Color is specified, copy to Buffer and flag it in Settings
  if (text_color.argb!=GColorClear.argb) {
    console_data->buffer[console_data->pos-- % console_data->buffer_size] = text_color.argb;
    settings |= TEXT_COLOR_BIT;
  }

  //Settings: If Background Color is specified, copy to Buffer and flag it in Settings
  if (background_color.argb!=GColorClear.argb) {
    console_data->buffer[console_data->pos-- % console_data->buffer_size] = background_color.argb;
    settings |= BACKGROUND_COLOR_BIT;
  }

  // Settings: Flag Word Wrap in Settings, even it if it's "inherit from console_layer"
  settings |= (word_wrap & WORD_WRAP_BITS);
  
  // Settings: Flag Alignment in Settings, even if it's "inherit from console_layer"
  settings |= (alignment==GTextAlignmentLeft?0b0000 : alignment==GTextAlignmentCenter?0b0100 : alignment==GTextAlignmentRight?0b1000 : 0b1100);

  // Save settings
  console_data->buffer[console_data->pos-- % console_data->buffer_size] = settings;
  
  // EOF -- Head/Tail buffer transition point
  console_data->buffer[console_data->pos % console_data->buffer_size] = 0;

  // Fix pos
  console_data->pos %= console_data->buffer_size;
  
  MARK_DIRTY;
}

//----------------------------------------------//

void console_layer_write_text(Layer *console_layer, char *text) {
  // Write text and inherit all settings from the console_layer
  console_layer_write_text_styled(console_layer, text, GColorInherit, GColorInherit, GFontInherit, GTextAlignmentInherit, WordWrapInherit);
}





//------------------------------------------------------------------------------------------------//
// Draw Layer
//----------------------------------------------//
// Modifies Graphics Context: Fill Color, Text Color
static void console_layer_update(Layer *console_layer, GContext *ctx) {
  console_data_struct *console_data = (console_data_struct*)layer_get_data(console_layer);
  GRect bounds = layer_get_bounds(console_layer);

  // Layer Background
  if (console_data->background_color.argb!=GColorClear.argb) {
    graphics_context_set_fill_color(ctx, console_data->background_color);
    graphics_fill_rect(ctx, (GRect){.origin = GPoint(0, 0), .size = bounds.size}, 0, GCornerNone);
  }

  // Display Text
  //char text[console_data->buffer_size + 1];         // allocate on stack (Locks up when using DictationAPI and a large buffer)
  char *text = malloc(console_data->buffer_size + 1); // allocate on heap
  if (text) {  // verify heap allocation was successful
    int16_t y = bounds.size.h; // Start at the bottom of layer
    uint8_t settings;
    intptr_t cursor = console_data->pos + 1;  // Get past the EOF 0

    // While text is within visible bounds && not at EOF (also, if not EOF, copy byte to settings)
    while (y>bounds.origin.y && (settings = console_data->buffer[cursor % console_data->buffer_size])) {

      // Extract word_wrap from settings
      bool word_wrap = settings&WORD_WRAP_INHERIT_BIT ? console_data->word_wrap : settings&WORD_WRAP_BIT;

      // Extract Alignment from Settings (and if it's inherit, get it from layer, unless THAT's also inherit then default to left)
      // This could be quicker if I could just assume the enum: GTextAlignmentLeft=0, Center=1 and Right=2 (which it does),
      // but I can't cause it'd lose abstraction.  Not that I already didn't lose it by adding "GTextAlignmentInherit"
      GTextAlignment alignment;
      switch (settings & ALIGNMENT_BITS) {
        case 0b0000: alignment = GTextAlignmentLeft;   break;
        case 0b0100: alignment = GTextAlignmentCenter; break;
        case 0b1000: alignment = GTextAlignmentRight;  break;
        default:  // 0b1100 = Inherit from layer
        //Method 1:
//         alignment = console_data->alignment==GTextAlignmentInherit ? GTextAlignmentLeft : console_data->alignment;
        //Method 2:
        switch(console_data->alignment) {
         case GTextAlignmentLeft:
         case GTextAlignmentCenter:
         case GTextAlignmentRight:
           alignment = console_data->alignment;  // Copy alignment from layer
         break;
         default:  // Default: Layer is set to Inherit, which doesn't make sense.  Setting alignment to Left as a fallback.
           alignment = GTextAlignmentLeft;
        }
      }

      // If background color is specified in settings, copy from buffer
      GColor background_color = console_data->background_color;  // Assume inherit from layer
      if (settings&BACKGROUND_COLOR_BIT)
        background_color = (GColor){.argb=console_data->buffer[++cursor % console_data->buffer_size]};

      // If text color is specified in settings, copy from buffer
      GColor text_color = console_data->text_color;  // Assume inherit from layer
      if (settings&TEXT_COLOR_BIT)
        text_color = (GColor){.argb=console_data->buffer[++cursor % console_data->buffer_size]};
      graphics_context_set_text_color(ctx, text_color);

      // If font is specified in settings, copy from buffer
      GFont font = console_data->font;  // Assume inherit from layer
      if (settings&FONT_BIT)
        for (uintptr_t i=0; i<sizeof(GFont); i++) {
        ((uint8_t*)&font)[(sizeof(GFont)-1)-i] = console_data->buffer[++cursor % console_data->buffer_size];
      }

      // Copy the 0-terminated string into a temp buffer (because pebble's text functions can't wrap around end of buffer)
      // Sure I could just manipulate the string itself every write, but... uhh...
      intptr_t i = -1;
      do {
        if (i < (intptr_t)(console_data->buffer_size)) i++;     // Stop it from maxing out
        text[i] = console_data->buffer[++cursor % console_data->buffer_size];
      } while (text[i]);  // Stop at 0 (EndOfString)

      // If we have gone beyond the buffer_size, data is invalid.  Since buffer[cursor%size] is pointing at 0 (EOS), while loop will exit
      if (cursor - console_data->pos < console_data->buffer_size) {
        cursor++;  // Get past the string terminating 0 so the while can loop (unless we're now at the EOF 0)

        int16_t text_height = graphics_text_layout_get_content_size(word_wrap?text:" ", font, GRect(0, 0, bounds.size.w, 0x7FFF), GTextOverflowModeTrailingEllipsis, alignment).h;
        y -= text_height;
        if (text_height>0 && background_color.argb!=GColorClear.argb) {
          graphics_context_set_fill_color(ctx, background_color);
          graphics_fill_rect(ctx, GRect(bounds.origin.x, bounds.origin.y + y, bounds.size.w, text_height), 0, GCornerNone);  // fill background
        }

        // Render Text (y-3 because Pebble's text rendering is dumb and goes outside rect)
        if (text_color.argb!=GColorClear.argb)   // Pebble renders clear text as black
          graphics_draw_text(ctx, text, font, GRect(bounds.origin.x, bounds.origin.y + (y-3), bounds.size.w, text_height), GTextOverflowModeTrailingEllipsis, alignment, NULL);
      } // END if data valid
    } // END While
    free(text);     // free up heap (remove this line if allocating on stack)
  } else {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Unable to display console_layer text: Out of space (%d bytes needed, %d heap bytes free)", (int)console_data->buffer_size + 1, (int)heap_bytes_free());
  }
}




//------------------------------------------------------------------------------------------------//
// Create Layer
//----------------------------------------------//

Layer* console_layer_create_with_buffer_size(GRect frame, int buffer_size) {
  Layer *console_layer;
  size_t data_size = sizeof (console_data_struct) + buffer_size;

  if ((console_layer = layer_create_with_data(frame, data_size))) {
    console_data_struct *console_data = (console_data_struct*)layer_get_data(console_layer);
    // Point buffer to memory allocated just after the struct.  Assumes buffer is the last type inside struct.
    // Sure, this could be malloc'd separately instead of complicated pointer math, but hey, this works.
    console_data->buffer = (char*)(&(console_data->buffer) + sizeof(console_data->buffer));
    console_data->buffer_size = buffer_size;

    layer_set_clips(console_layer, true);
    console_layer_clear(console_layer);
    console_layer_set_style(console_layer, GColorBlack, GColorClear, fonts_get_system_font(FONT_KEY_GOTHIC_14), GTextAlignmentLeft, true);
    layer_set_update_proc(console_layer, console_layer_update);
  }
  return console_layer;
}

//----------------------------------------------//

Layer* console_layer_create(GRect frame) {
  return console_layer_create_with_buffer_size(frame, DEFAULT_BUFFER_SIZE);
}



//------------------------------------------------------------------------------------------------//
//  Debugging and Experimental Junk
//----------------------------------------------//



/*
// Log the console layer buffer's raw data: for debugging
void log_buffer(Layer *console_layer) {
  console_data_struct *console_data = (console_data_struct*)layer_get_data(console_layer);
  printf("Head/Tail Separator Position: %d", (int)console_data->pos);
  //for(int i=console_data->pos; i<console_data->buffer_size; i++)  // Log from current (EOF head/tail) position to the end of the buffer
  for(int i=0; i<console_data->buffer_size; i++)                    // Log the whole buffer
    if (console_data->buffer[i]<=127 && console_data->buffer[i]>=32)
      printf("buffer[%d] = %d (%x) '%c'", i, console_data->buffer[i], console_data->buffer[i], console_data->buffer[i]);
    else
      printf("buffer[%d] = %d (%x)", i, console_data->buffer[i], console_data->buffer[i]);
}
*/


//----------------------------------------------//


/*
// Experimental Idea:
// Extracts all strings from the console_layer and logs them via printf()
// Currently logs everything from most newest to oldest (with newest at the top of the output -- opposite of the console_layer)
void console_layer_log_contents(Layer *console_layer) {
  console_data_struct *console_data = (console_data_struct*)layer_get_data(console_layer);
  char *text = malloc(console_data->buffer_size + 1); // allocate on heap
  if (text) {  // verify heap allocation was successful
    uint8_t settings;
    intptr_t cursor = console_data->pos + 1;  // Get past the EOF 0

    // While text is within visible bounds && not at EOF (also, if not EOF, copy byte to settings)
    while ((settings = console_data->buffer[cursor % console_data->buffer_size])) {
      // If background color is specified in settings, copy from buffer
      if (settings&BACKGROUND_COLOR_BIT)
        ++cursor;

      // If text color is specified in settings, copy from buffer
      if (settings&TEXT_COLOR_BIT)
        ++cursor;

      // If font is specified in settings, copy from buffer
      if (settings&FONT_BIT)
        for (uintptr_t i=0; i<sizeof(GFont); i++)
          ++cursor;


      // Copy the 0-terminated string into a temp buffer (because pebble's text functions can't wrap around end of buffer)
      intptr_t i = -1;
      do {
        if (i < (intptr_t)(console_data->buffer_size)) i++;     // Stop it from maxing out
        text[i] = console_data->buffer[++cursor % console_data->buffer_size];
      } while (text[i]);

      // If we've not gone beyond the size of the buffer, then the data should be valid
      // If we HAVE gone beyond the buffer_size, buffer[cursor%size] is pointing at 0, exiting the while loop
      if (cursor - console_data->pos < console_data->buffer_size) {
        cursor++;  // Get past the string terminating 0 so the while can loop (unless we're now at the EOF 0)
        printf("%s", text);
      } // END if data valid
    } // END While
    free(text);
  } else {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Unable to display console_layer text: Out of space (%d bytes needed, %d heap bytes free)", (int)console_data->buffer_size + 1, (int)heap_bytes_free());
  }
}
*/
