/
// "$Id$
/
// Resize box test program for the Fast Light Tool Kit (FLTK)
/
// Copyright 1998-2003 by Bill Spitzak and others
/
// This library is free software; you can redistribute it and/o
// modify it under the terms of the GNU Library General Publi
// License as published by the Free Software Foundation; eithe
// version 2 of the License, or (at your option) any later version
/
// This library is distributed in the hope that it will be useful
// but WITHOUT ANY WARRANTY; without even the implied warranty o
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GN
// Library General Public License for more details
/
// You should have received a copy of the GNU Library General Publi
// License along with this library; if not, write to the Free Softwar
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-130
// USA
/
// Please report all bugs and problems to "fltk-bugs@fltk.org"
/

#define W1 (big ? 60 : 40
#define B 
#define W3 (5*W1+6*B

#include <FL/Fl.H
#include <FL/Fl_Single_Window.H
#include <FL/Fl_Box.H
#include <FL/Fl_Radio_Button.H
#include <FL/fl_draw.H
#include <FL/fl_message.H

Fl_Single_Window *window
Fl_Box *box

int big = 0

void b_cb(Fl_Widget *,long w) 
  if (window->w() != W3 || window->h() != W3) 
    fl_message("Put window back to minimum size before changing")
    return
  
  window->init_sizes()
  switch (w) 
  case 0: box->hide(); window->box(FL_FLAT_BOX); window->resizable(0); return
  case 8: box->resize(W1+B,W1,2*W1,B); break
  case 2: box->resize(W1+B,W1+B+2*W1,2*W1,B); break
  case 4: box->resize(W1+B,W1,B,2*W1); break
  case 6: box->resize(W1+B+2*W1,W1+B,B,2*W1); break
  
  window->box(FL_NO_BOX)
  if (w == 6 || w == 4
    box->label("re\nsiz\nab\nle")
  else box->label("resizable")
  box->show()
  window->resizable(box)
  window->redraw()


int main(int argc, char **argv) 
  window = new Fl_Single_Window(W3,W3)
  window->box(FL_NO_BOX)
  Fl_Box *n
  for (int x = 0; x<4; x++) for (int y = 0; y<4; y++) 
    if ((x==1 || x==2) && (y==1 || y==2)) continue
    n = new Fl_Box(FL_FRAME_BOX,x*(B+W1)+B,y*(B+W1)+B,W1,W1,0)
    n->color(x+y+8)
  
  n = new Fl_Box(FL_FRAME_BOX,B,4*W1+5*B,4*W1+3*B,W1,0)
  n->color(12)
  n = new Fl_Box(FL_FRAME_BOX,4*W1+5*B,B,W1,5*W1+4*B,0)
  n->color(13)
  n = new Fl_Box(FL_FRAME_BOX,W1+B+B,W1+B+B,2*W1+B,2*W1+B,0)
  n->color(8)

  Fl_Button *b = new Fl_Radio_Button(W1+B+50,W1+B+30,20,20,"@6>")
  b->callback(b_cb,6)
  (new Fl_Radio_Button(W1+B+30,W1+B+10,20,20,"@8>"))->callback(b_cb,8)
  (new Fl_Radio_Button(W1+B+10,W1+B+30,20,20,"@4>"))->callback(b_cb,4)
  (new Fl_Radio_Button(W1+B+30,W1+B+50,20,20,"@2>"))->callback(b_cb,2)
  (new Fl_Radio_Button(W1+B+30,W1+B+30,20,20,"off"))->callback(b_cb,0)

  box = new Fl_Box(FL_FLAT_BOX,0,0,0,0,"resizable")
  box->color(FL_DARK2)
  b->set()
  b->do_callback()
  window->end()

  window->size_range(W3,W3)
  window->show(argc,argv)
  return Fl::run()


/
// End of "$Id$"
/
