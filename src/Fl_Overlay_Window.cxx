//
// "$Id$"
//
// Overlay window code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

/** \fn virtual void Fl_Overlay_Window::draw_overlay() = 0
  You must subclass Fl_Overlay_Window and provide this method.
  It is just like a draw() method, except it draws the overlay.
  The overlay will have already been "cleared" when this is called.  You
  can use any of the routines described in &lt;FL/draw.h&gt;.
*/

// A window using double-buffering and able to draw an overlay
// on top of that.  Uses the hardware to draw the overlay if
// possible, otherwise it just draws in the front buffer.

#include <config.h>
#include <fltk3/run.h>
#include <fltk3/OverlayWindow.h>
#include <fltk3/draw.h>
#include <fltk3/x.h>

void Fl_Overlay_Window::show() {
  fltk3::DoubleWindow::show();
  if (overlay_ && overlay_ != this) overlay_->show();
}

void Fl_Overlay_Window::hide() {
  fltk3::DoubleWindow::hide();
}

void Fl_Overlay_Window::flush() {
#ifdef BOXX_BUGS
  if (overlay_ && overlay_ != this && overlay_->shown()) {
    // all drawing to windows hidden by overlay windows is ignored, fix this
    XUnmapWindow(fl_display, fl_xid(overlay_));
    fltk3::DoubleWindow::flush(0);
    XMapWindow(fl_display, fl_xid(overlay_));
    return;
  }
#endif
  int erase_overlay = (damage()&fltk3::DAMAGE_OVERLAY) | (overlay_ == this);
  clear_damage((uchar)(damage()&~fltk3::DAMAGE_OVERLAY));
  fltk3::DoubleWindow::flush(erase_overlay);
  if (overlay_ == this) draw_overlay();
}

void Fl_Overlay_Window::resize(int X, int Y, int W, int H) {
  fltk3::DoubleWindow::resize(X,Y,W,H);
  if (overlay_ && overlay_!=this) overlay_->resize(0,0,w(),h());
}

/**
  Destroys the window and all child widgets.
*/
Fl_Overlay_Window::~Fl_Overlay_Window() {
  hide();
//  delete overlay; this is done by ~Fl_Group
}

#if !HAVE_OVERLAY

int Fl_Overlay_Window::can_do_overlay() {return 0;}

/**
  Call this to indicate that the overlay data has changed and needs to
  be redrawn.  The overlay will be clear until the first time this is
  called, so if you want an initial display you must call this after
  calling show().
*/
void Fl_Overlay_Window::redraw_overlay() {
  overlay_ = this;
  clear_damage((uchar)(damage()|fltk3::DAMAGE_OVERLAY));
  fltk3::damage(fltk3::DAMAGE_CHILD);
}

#else

extern XVisualInfo *fl_find_overlay_visual();
extern XVisualInfo *fl_overlay_visual;
extern Colormap fl_overlay_colormap;
extern unsigned long fl_transparent_pixel;
static GC gc;	// the GC used by all X windows
extern uchar fl_overlay; // changes how fltk3::color(x) works

class _Fl_Overlay : public fltk3::Window {
  friend class Fl_Overlay_Window;
  void flush();
  void show();
public:
  _Fl_Overlay(int x, int y, int w, int h) :
    fltk3::Window(x,y,w,h) {set_flag(INACTIVE);}
};

int Fl_Overlay_Window::can_do_overlay() {
  return fl_find_overlay_visual() != 0;
}

void _Fl_Overlay::show() {
  if (shown()) {fltk3::Window::show(); return;}
  fl_background_pixel = int(fl_transparent_pixel);
  Fl_X::make_xid(this, fl_overlay_visual, fl_overlay_colormap);
  fl_background_pixel = -1;
  // find the outermost window to tell wm about the colormap:
  fltk3::Window *w = window();
  for (;;) {fltk3::Window *w1 = w->window(); if (!w1) break; w = w1;}
  XSetWMColormapWindows(fl_display, fl_xid(w), &(Fl_X::i(this)->xid), 1);
}

void _Fl_Overlay::flush() {
  fl_window = fl_xid(this);
  if (!gc) {
	  gc = XCreateGC(fl_display, fl_xid(this), 0, 0);
  }
  fl_gc = gc;
#if defined(FLTK_USE_CAIRO)
      if (fltk3::cairo_autolink_context()) fltk3::cairo_make_current(this); // capture gc changes automatically to update the cairo context adequately
#endif
  fl_overlay = 1;
  Fl_Overlay_Window *w = (Fl_Overlay_Window *)parent();
  Fl_X *myi = Fl_X::i(this);
  if (damage() != fltk3::DAMAGE_EXPOSE) XClearWindow(fl_display, fl_xid(this));
  fltk3::clip_region(myi->region); myi->region = 0;
  w->draw_overlay();
  fl_overlay = 0;
}

void Fl_Overlay_Window::redraw_overlay() {
  if (!fl_display) return; // this prevents fluid -c from opening display
  if (!overlay_) {
    if (can_do_overlay()) {
      fltk3::Group::current(this);
      overlay_ = new _Fl_Overlay(0,0,w(),h());
      fltk3::Group::current(0);
    } else {
      overlay_ = this;	// fake the overlay
    }
  }
  if (shown()) {
    if (overlay_ == this) {
      clear_damage(damage()|fltk3::DAMAGE_OVERLAY);
      fltk3::damage(fltk3::DAMAGE_CHILD);
    } else if (!overlay_->shown())
      overlay_->show();
    else
      overlay_->redraw();
  }
}

#endif

//
// End of "$Id$".
//