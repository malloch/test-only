//
// "$Id$"
//
// Input widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

// This is the "user interface", it decodes user actions into what to
// do to the text.  See also fltk3::Input_.cxx, where the text is actually
// manipulated (and some ui, in particular the mouse, is done...).
// In theory you can replace this code with another subclass to change
// the keybindings.

#include <stdio.h>
#include <stdlib.h>
#include <fltk3/run.h>
#include <fltk3/Window.h>
#include <fltk3/Input.h>
#include <fltk3/draw.h>
#include <fltk3/ask.h>
#include "flstring.h"

#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif


void fltk3::Input::draw() {
  if (input_type() == FL_HIDDEN_INPUT) return;
  fltk3::Boxtype b = box();
  if (damage() & FL_DAMAGE_ALL) draw_box(b, color());
  fltk3::Input_::drawtext(x()+fltk3::box_dx(b), y()+fltk3::box_dy(b),
		      w()-fltk3::box_dw(b), h()-fltk3::box_dh(b));
}

// kludge so shift causes selection to extend:
int fltk3::Input::shift_position(int p) {
  return position(p, fltk3::event_state(fltk3::SHIFT) ? mark() : p);
}

int fltk3::Input::shift_up_down_position(int p) {
  return up_down_position(p, fltk3::event_state(fltk3::SHIFT));
}

// If you define this symbol as zero you will get the peculiar fltk
// behavior where moving off the end of an input field will move the
// cursor into the next field:
// define it as 1 to prevent cursor movement from going to next field:
#define NORMAL_INPUT_MOVE 0

#define ctrl(x) ((x)^0x40)

// List of characters that are legal in a floating point input field.
// This text string is created at run-time to take the current locale
// into account (for example, continental Europe uses a comma instead
// of a decimal point). For back compatibility reasons, we always 
// allow the decimal point.
#ifdef HAVE_LOCALECONV
static const char *standard_fp_chars = ".eE+-"; 
static const char *legal_fp_chars = 0L;
#else
static const char *legal_fp_chars = ".eE+-"; 
#endif

int fltk3::Input::handle_key() {
  
  char ascii = fltk3::event_text()[0];
  
  int repeat_num=1;
  
  int del;
  if (fltk3::compose(del)) {
    
    // Insert characters into numeric fields after checking for legality:
    if (input_type() == FL_FLOAT_INPUT || input_type() == FL_INT_INPUT) {
      fltk3::compose_reset(); // ignore any foreign letters...
      
      // initialize the list of legal characters inside a floating point number
#ifdef HAVE_LOCALECONV
      if (!legal_fp_chars) {
        int len = strlen(standard_fp_chars);
        struct lconv *lc = localeconv();
        if (lc) {
          if (lc->decimal_point) len += strlen(lc->decimal_point);
          if (lc->mon_decimal_point) len += strlen(lc->mon_decimal_point);
          if (lc->positive_sign) len += strlen(lc->positive_sign);
          if (lc->negative_sign) len += strlen(lc->negative_sign);
        }
        // the following line is not a true memory leak because the array is only
        // allocated once if required, and automatically freed when the program quits
        char *chars = (char*)malloc(len+1);
	legal_fp_chars = chars;
        strcpy(chars, standard_fp_chars);
        if (lc) {
          if (lc->decimal_point) strcat(chars, lc->decimal_point);
          if (lc->mon_decimal_point) strcat(chars, lc->mon_decimal_point);
          if (lc->positive_sign) strcat(chars, lc->positive_sign);
          if (lc->negative_sign) strcat(chars, lc->negative_sign);
        }
      }
#endif // HAVE_LOCALECONV
      
      // find the insert position
      int ip = position()<mark() ? position() : mark();
      // This is complex to allow "0xff12" hex to be typed:
      if (!ip && (ascii == '+' || ascii == '-') 
          || (ascii >= '0' && ascii <= '9') 
          || (ip==1 && index(0)=='0' && (ascii=='x' || ascii == 'X')) 
          || (ip>1 && index(0)=='0' && (index(1)=='x'||index(1)=='X')
              && (ascii>='A'&& ascii<='F' || ascii>='a'&& ascii<='f')) 
          || input_type()==FL_FLOAT_INPUT && ascii && strchr(legal_fp_chars, ascii)) 
      {
	if (readonly()) fltk3::beep();
	else replace(position(), mark(), &ascii, 1);
      }
      return 1;
    }
    
    if (del || fltk3::event_length()) {
      if (readonly()) fltk3::beep();
      else replace(position(), del ? position()-del : mark(),
	           fltk3::event_text(), fltk3::event_length());
    }
    return 1;
  }
  
  unsigned int mods = fltk3::event_state() & (fltk3::META|fltk3::CTRL|fltk3::ALT);
  switch (fltk3::event_key()) {
    case fltk3::InsertKey:
      if (fltk3::event_state() & fltk3::CTRL) ascii = ctrl('C');
      else if (fltk3::event_state() & fltk3::SHIFT) ascii = ctrl('V');
      break;
    case fltk3::DeleteKey:
#ifdef __APPLE__
      if (mods==0 || mods==fltk3::CTRL) { // delete next char
        ascii = ctrl('D');
      } else if (mods==fltk3::ALT) { // delete next word
        if (mark() != position()) return cut();
        cut(position(), word_end(position()));
        return 1;
      } else if (mods==fltk3::META) { // delete to the end of the line
        if (mark() != position()) return cut();
        cut(position(), line_end(position()));
        return 1;
      } else return 1;
#else
      if (mods==0) {
        ascii = ctrl('D'); 
      } else if (mods==fltk3::SHIFT) {
        ascii = ctrl('X');
      } else return 1;
#endif
      break;
    case fltk3::LeftKey:
#ifdef __APPLE__
      if (mods==0) { // char left
        ascii = ctrl('B'); 
      } else if (mods==fltk3::ALT) { // word left
        shift_position(word_start(position()));
        return 1; 
      } else if (mods==fltk3::CTRL || mods==fltk3::META) { // start of line
        shift_position(line_start(position()));
        return 1;
      } else return 1;
#else
      if (mods==0) { // char left
        ascii = ctrl('B'); 
      } else if (mods==fltk3::CTRL) { // word left
        shift_position(word_start(position()));
        return 1;
      } else return 1;
#endif
      break;
    case fltk3::RightKey:
#ifdef __APPLE__
      if (mods==0) { // char right
        ascii = ctrl('F'); 
      } else if (mods==fltk3::ALT) { // word right
        shift_position(word_end(position()));
        return 1;
      } else if (mods==fltk3::CTRL || mods==fltk3::META) { // end of line
        shift_position(line_end(position()));
        return 1;
      } else return 1;
#else
      if (mods==0) { // char right
        ascii = ctrl('F'); 
      } else if (mods==fltk3::CTRL) { // word right
        shift_position(word_end(position()));
        return 1;
      } else return 1;
#endif // __APPLE__
      break;
    case fltk3::PageUpKey:
#ifdef __APPLE__
      if (mods==0) { // scroll text one page
                     // OS X scrolls the view, but does not move the cursor
                     // fltk3::Input has no scroll control, so instead we move the cursor by one page
        repeat_num = linesPerPage();
        ascii = ctrl('P');
      } else if (mods==fltk3::ALT) { // move cursor one page
        repeat_num = linesPerPage();
        ascii = ctrl('P');
      } else return 1;
      break;
#else
      repeat_num = linesPerPage();
      // fall through
#endif
    case fltk3::UpKey:
#ifdef __APPLE__
      if (mods==0) { // line up
        ascii = ctrl('P');
      } else if (mods==fltk3::CTRL) { // scroll text down one page
                                  // OS X scrolls the view, but does not move the cursor
                                  // fltk3::Input has no scroll control, so instead we move the cursor by one page
        repeat_num = linesPerPage();
        ascii = ctrl('P');
      } else if (mods==fltk3::ALT) { // line start and up
        if (line_start(position())==position() && position()>0)
          return shift_position(line_start(position()-1)) + NORMAL_INPUT_MOVE;
        else
          return shift_position(line_start(position())) + NORMAL_INPUT_MOVE;
      } else if (mods==fltk3::META) { // start of document
        shift_position(0);
        return 1;
      } else return 1;
#else
      if (mods==0) { // line up
        ascii = ctrl('P');
      } else if (mods==fltk3::CTRL) { // scroll text down one line
                                  // fltk3::Input has no scroll control, so instead we move the cursor by one page
        ascii = ctrl('P');
      } else return 1;
#endif
      break;
    case fltk3::PageDownKey:
#ifdef __APPLE__
      if (mods==0) { // scroll text one page
                     // OS X scrolls the view, but does not move the cursor
                     // fltk3::Input has no scroll control, so instead we move the cursor by one page
        repeat_num = linesPerPage();
        ascii = ctrl('N');
      } else if (mods==fltk3::ALT) { // move cursor one page
        repeat_num = linesPerPage();
        ascii = ctrl('N');
      } else return 1;
      break;
#else
      repeat_num = linesPerPage();
      // fall through
#endif
    case fltk3::DownKey:
#ifdef __APPLE__
      if (mods==0) { // line down
        ascii = ctrl('N');
      } else if (mods==fltk3::CTRL) {
        // OS X scrolls the view, but does not move the cursor
        // fltk3::Input has no scroll control, so instead we move the cursor by one page
        repeat_num = linesPerPage();
        ascii = ctrl('N');
      } else if (mods==fltk3::ALT) { // line end and down
        if (line_end(position())==position() && position()<size())
          return shift_position(line_end(position()+1)) + NORMAL_INPUT_MOVE;
        else
          return shift_position(line_end(position())) + NORMAL_INPUT_MOVE;
      } else if (mods==fltk3::META) { // end of document
        shift_position(size());
        return 1;
      } else return 1;
#else
      if (mods==0) { // line down
        ascii = ctrl('N');
      } else if (mods==fltk3::CTRL) { // scroll text up one line
                                  // fltk3::Input has no scroll control, so instead we move the cursor by one page
        ascii = ctrl('N');
      } else return 1;
#endif
      break;
    case fltk3::HomeKey:
#ifdef __APPLE__
      if (mods==0) { // scroll display to the top
                     // OS X scrolls the view, but does not move the cursor
                     // fltk3::Input has no scroll control, so instead we move the cursor by one page
        shift_position(0);
        return 1;
      } else return 1;
#else
      if (mods==0) {
        ascii = ctrl('A');
      } else if (mods==fltk3::CTRL) {
        shift_position(0);
        return 1;
      }
#endif
      break;
    case fltk3::EndKey:
#ifdef __APPLE__
      if (mods==0) { // scroll display to the bottom
                     // OS X scrolls the view, but does not move the cursor
                     // fltk3::Input has no scroll control, so instead we move the cursor by one page
        shift_position(size());
        return 1; 
      } else return 1;
#else
      if (mods==0) {
        ascii = ctrl('E');
      } else if (mods==fltk3::CTRL) {
        shift_position(size());
        return 1;
      } else return 1;
#endif
      break;
    case fltk3::BackSpaceKey:
#ifdef __APPLE__
      if (mods==0 || mods==fltk3::CTRL) { // delete previous char
        ascii = ctrl('H');
      } else if (mods==fltk3::ALT) { // delete previous word
        if (mark() != position()) return cut();
        cut(word_start(position()), position());
        return 1;
      } else if (mods==fltk3::META) { // delete to the beginning of the line
        if (mark() != position()) return cut();
        cut(line_start(position()), position());
        return 1;
      } else return 1;
#else
      ascii = ctrl('H'); 
#endif
      break;
    case fltk3::EnterKey:
    case fltk3::KeypadEnter:
      if (when() & fltk3::WHEN_ENTER_KEY) {
        position(size(), 0);
        maybe_do_callback();
        return 1;
      } else if (input_type() == FL_MULTILINE_INPUT && !readonly())
        return replace(position(), mark(), "\n", 1);
      else 
        return 0;	// reserved for shortcuts
    case fltk3::TabKey:
      if (fltk3::event_state(fltk3::CTRL|fltk3::SHIFT) || input_type()!=FL_MULTILINE_INPUT || readonly()) return 0;
      return replace(position(), mark(), &ascii, 1);
#ifdef __APPLE__
    case 'c' :
    case 'v' :
    case 'x' :
    case 'z' :
      //    printf("'%c' (0x%02x) pressed with%s%s%s%s\n", ascii, ascii,
      //           fltk3::event_state(fltk3::SHIFT) ? " fltk3::SHIFT" : "",
      //           fltk3::event_state(fltk3::CTRL) ? " fltk3::CTRL" : "",
      //           fltk3::event_state(fltk3::ALT) ? " fltk3::ALT" : "",
      //           fltk3::event_state(fltk3::META) ? " fltk3::META" : "");
      if (fltk3::event_state(fltk3::META)) ascii -= 0x60;
      //    printf("using '%c' (0x%02x)...\n", ascii, ascii);
      break;
#endif // __APPLE__
  }
  
  int i;
  switch (ascii) {
    case ctrl('A'): // go to the beginning of the current line
      return shift_position(line_start(position())) + NORMAL_INPUT_MOVE;
    case ctrl('B'): // go one character backward
      return shift_position(position()-1) + NORMAL_INPUT_MOVE;
    case ctrl('C'): // copy
      return copy(1);
    case ctrl('D'): // cut the next character
    case ctrl('?'):
      if (readonly()) {
        fltk3::beep();
        return 1;
      }
      if (mark() != position()) return cut();
      else return cut(1);
    case ctrl('E'): // go to the end of the line
      return shift_position(line_end(position())) + NORMAL_INPUT_MOVE;
    case ctrl('F'): // go to the next character
      return shift_position(position()+1) + NORMAL_INPUT_MOVE;
    case ctrl('H'): // cut the previous character
      if (readonly()) {
        fltk3::beep();
        return 1;
      }
      if (mark() != position()) cut();
      else cut(-1);
      return 1;
    case ctrl('K'): // cut to the end of the line
      if (readonly()) {
        fltk3::beep();
        return 1;
      }
      if (position()>=size()) return 0;
      i = line_end(position());
      if (i == position() && i < size()) i++;
      cut(position(), i);
      return copy_cuts();
    case ctrl('N'): // go down one line
      i = position();
      if (line_end(i) >= size()) return NORMAL_INPUT_MOVE;
      while (repeat_num--) {  
        i = line_end(i);
        if (i >= size()) break;
        i++;
      }
      shift_up_down_position(i);
      return 1;
    case ctrl('P'): // go up one line
      i = position();
      if (!line_start(i)) return NORMAL_INPUT_MOVE;
      while(repeat_num--) {
        i = line_start(i);
        if (!i) break;
        i--;
      }
      shift_up_down_position(line_start(i));
      return 1;
    case ctrl('U'): // clear the whole document? 
      if (readonly()) {
        fltk3::beep();
        return 1;
      }
      return cut(0, size());
    case ctrl('V'): // paste text
    case ctrl('Y'):
      if (readonly()) {
        fltk3::beep();
        return 1;
      }
      fltk3::paste(*this, 1);
      return 1;
    case ctrl('X'): // cut the selected text
    case ctrl('W'):
      if (readonly()) {
        fltk3::beep();
        return 1;
      }
      copy(1);
      return cut();
    case ctrl('Z'): // undo
    case ctrl('_'):
      if (readonly()) {
        fltk3::beep();
        return 1;
      }
      return undo();
    case ctrl('I'): // insert literal
    case ctrl('J'):
    case ctrl('L'):
    case ctrl('M'):
      if (readonly()) {
        fltk3::beep();
        return 1;
      }
      // insert a few selected control characters literally:
      if (input_type() != FL_FLOAT_INPUT && input_type() != FL_INT_INPUT)
        return replace(position(), mark(), &ascii, 1);
  }
  
  return 0;
}

int fltk3::Input::handle(int event) {
  static int dnd_save_position, dnd_save_mark, drag_start = -1, newpos;
  static fltk3::Widget *dnd_save_focus;
  switch (event) {
    case fltk3::FOCUS:
      switch (fltk3::event_key()) {
        case fltk3::RightKey:
          position(0);
          break;
        case fltk3::LeftKey:
          position(size());
          break;
        case fltk3::DownKey:
          up_down_position(0);
          break;
        case fltk3::UpKey:
          up_down_position(line_start(size()));
          break;
        case fltk3::TabKey:
        case 0xfe20: // XK_ISO_Left_Tab
          position(size(),0);
          break;
        default:
          position(position(),mark());// turns off the saved up/down arrow position
          break;
      }
      break;
      
    case fltk3::KEY:
      if (fltk3::event_key() == fltk3::TabKey && mark() != position()) {
        // Set the current cursor position to the end of the selection...
        if (mark() > position())
          position(mark());
        else
          position(position());
        return (1);
      } else {
        if (active_r() && window() && this == fltk3::belowmouse()) 
          window()->cursor(FL_CURSOR_NONE);
        return handle_key();
      }
      
    case fltk3::PUSH:
      if (fltk3::dnd_text_ops()) {
        int oldpos = position(), oldmark = mark();
        fltk3::Boxtype b = box();
        fltk3::Input_::handle_mouse(
                                x()+fltk3::box_dx(b), y()+fltk3::box_dy(b),
                                w()-fltk3::box_dw(b), h()-fltk3::box_dh(b), 0);
        newpos = position(); 
        position( oldpos, oldmark );
        if (fltk3::focus()==this && !fltk3::event_state(fltk3::SHIFT) && input_type()!=FL_SECRET_INPUT &&
            (newpos >= mark() && newpos < position() ||
             newpos >= position() && newpos < mark())) {
              // user clicked in the selection, may be trying to drag
              drag_start = newpos;
              return 1;
            }
        drag_start = -1;
      }
      
      if (fltk3::focus() != this) {
        fltk3::focus(this);
        handle(fltk3::FOCUS);
      }
      break;
      
    case fltk3::DRAG:
      if (fltk3::dnd_text_ops()) {
        if (drag_start >= 0) {
          if (fltk3::event_is_click()) return 1; // debounce the mouse
                                              // save the position because sometimes we don't get DND_ENTER:
          dnd_save_position = position();
          dnd_save_mark = mark();
          // drag the data:
          copy(0); fltk3::dnd();
          return 1;
        }
      }
      break;
      
    case fltk3::RELEASE:
      if (fltk3::event_button() == 2) {
        fltk3::event_is_click(0); // stop double click from picking a word
        fltk3::paste(*this, 0);
      } else if (!fltk3::event_is_click()) {
        // copy drag-selected text to the clipboard.
        copy(0);
      } else if (fltk3::event_is_click() && drag_start >= 0) {
        // user clicked in the field and wants to reset the cursor position...
        position(drag_start, drag_start);
        drag_start = -1;
      } else if (fltk3::event_clicks()) {
        // user double or triple clicked to select word or whole text
        copy(0);
      }
      
      // For output widgets, do the callback so the app knows the user
      // did something with the mouse...
      if (readonly()) do_callback();
      
      return 1;
      
    case fltk3::DND_ENTER:
      fltk3::belowmouse(this); // send the leave events first
      dnd_save_position = position();
      dnd_save_mark = mark();
      dnd_save_focus = fltk3::focus();
      if (dnd_save_focus != this) {
        fltk3::focus(this);
        handle(fltk3::FOCUS);
      }
      // fall through:
    case fltk3::DND_DRAG: 
      //int p = mouse_position(X, Y, W, H);
#if DND_OUT_XXXX
      if (fltk3::focus()==this && (p>=dnd_save_position && p<=dnd_save_mark ||
                                p>=dnd_save_mark && p<=dnd_save_position)) {
        position(dnd_save_position, dnd_save_mark);
        return 0;
      }
#endif
    {
      fltk3::Boxtype b = box();
      fltk3::Input_::handle_mouse(
                              x()+fltk3::box_dx(b), y()+fltk3::box_dy(b),
                              w()-fltk3::box_dw(b), h()-fltk3::box_dh(b), 0);
    }
      return 1;
      
    case fltk3::DND_LEAVE:
      position(dnd_save_position, dnd_save_mark);
#if DND_OUT_XXXX
      if (!focused())
#endif
        if (dnd_save_focus != this) {
          fltk3::focus(dnd_save_focus);
          handle(fltk3::UNFOCUS);
        }
      return 1;
      
    case fltk3::DND_RELEASE:
      take_focus();
      return 1;
      
      /* TODO: this will scroll the area, but stop if the cursor would become invisible.
       That clipping happens in drawtext(). Do we change the clipping or should 
       we move the cursor (ouch)?
       case fltk3::MOUSEWHEEL:
       if (fltk3::e_dy > 0) {
       yscroll( yscroll() - fltk3::e_dy*15 );
       } else if (fltk3::e_dy < 0) {
       yscroll( yscroll() - fltk3::e_dy*15 );
       }
       return 1;
       */
      
  }
  fltk3::Boxtype b = box();
  return fltk3::Input_::handletext(event,
                               x()+fltk3::box_dx(b), y()+fltk3::box_dy(b),
                               w()-fltk3::box_dw(b), h()-fltk3::box_dh(b));
}

/**
 Creates a new fltk3::Input widget using the given position, size,
 and label string. The default boxtype is fltk3::DOWN_BOX.
 */
fltk3::Input::Input(int X, int Y, int W, int H, const char *l)
: fltk3::Input_(X, Y, W, H, l) {
}

//
// End of "$Id$".
//
