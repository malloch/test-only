//
// "$Id: fl_labeltype.cxx,v 1.13 2000/01/09 08:17:30 bill Exp $"
//
// Label drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

// Drawing code for the (one) common label types.
// Other label types (symbols) are in their own source files
// to avoid linking if not used.

#include <FL/Fl_Labeltype.H>
#include <FL/fl_draw.H>
#include <string.h>
#include <config.h>

void Fl_No_Label::draw(const char*, int, int, int, int, Fl_Color, Fl_Flags)
const {}
Fl_No_Label fl_no_label("none");

void Fl_Labeltype_::draw(const char* label,
			 int X, int Y, int W, int H,
			 Fl_Color c, Fl_Flags f) const
{
  fl_color(f&FL_INACTIVE ? fl_inactive(c) : c);
  fl_draw(label, X, Y, W, H, f);
}
Fl_Labeltype_ fl_normal_label("normal");

////////////////////////////////////////////////////////////////

#include <FL/Fl_Widget.H>
#include <FL/Fl_Image.H>
extern char fl_draw_shortcut;

// The normal call for a draw() method, this draws inside labels but
// skips outside labels:
void Fl_Widget::draw_label(Fl_Color c) const {
  if (!(flags()&15) || (flags() & FL_ALIGN_INSIDE)) {
    int X=x_; int Y=y_; int W=w_; int H=h_; box()->inset(X,Y,W,H);
    if (W > 11 && flags()&(FL_ALIGN_LEFT|FL_ALIGN_RIGHT)) {X += 3; W -= 6;}
    draw_label(X, Y, W, H, c, flags());
  }
}

void Fl_Widget::draw_label() const {draw_label(labelcolor());}

// draws a label on a button, you can reposition it to position it
// around other stuff drawn on the button.  The color should be the
// return value of draw_button().
void Fl_Widget::draw_button_label(int X,int Y,int W,int H, Fl_Color c) const {
  if (!(flags()&15) || (flags() & FL_ALIGN_INSIDE))
    draw_label(X,Y,W,H,c,flags());
}

// Anybody can call this to force the label to draw anywhere, this is
// used by Fl_Group and Fl_Tabs to draw outside labels:
void Fl_Widget::draw_label(int X, int Y, int W, int H, Fl_Color c, Fl_Flags f) const
{
  if (!active_r()) f |= FL_INACTIVE;
  if (image_) {
    int w, h;
    image_->measure(w, h);
    fl_color((f&FL_INACTIVE) ? fl_inactive(c) : c);
    image_->draw(X, Y, W, H, f);
    if (f & FL_ALIGN_TOP) {Y += h; H -= h;}
    else if (f & FL_ALIGN_BOTTOM) H -= h;
    else if (f & FL_ALIGN_LEFT) {X += w; W -= w;}
    else if (f & FL_ALIGN_RIGHT) W -= w;
    else {int d = (H+h)/2; Y += d; H -= d;}
  }
  if (label_ && *label_) {
    fl_font(label_font(), label_size());
    if (flags() & FL_SHORTCUT_LABEL) fl_draw_shortcut = 1;
    label_type()->draw(label_, X, Y, W, H, c, f);
    fl_draw_shortcut = 0;
  }
}

void Fl_Widget::draw_label(int X, int Y, int W, int H, Fl_Flags f) const {
  draw_label(X,Y,W,H, labelcolor(), f);
}

void Fl_Widget::measure_label(int& w, int& h) const {
  if (!label_ || !*label_) {w = h = 0; return;}
  fl_font(label_font(), label_size());
  w = 0;
  fl_measure(label(), w, h);
}

const Fl_Labeltype_* Fl_Labeltype_::find(const char* name) {
  for (const Fl_Labeltype_* p = Fl_Labeltype_::first; p; p = p->next)
    if (p->name && !strcasecmp(name, p->name)) return p;
  return 0;
}

const Fl_Labeltype_* Fl_Labeltype_::first = 0;

//
// End of "$Id: fl_labeltype.cxx,v 1.13 2000/01/09 08:17:30 bill Exp $".
//
