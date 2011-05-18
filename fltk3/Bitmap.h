//
// "$Id: Bitmap.h 7903 2010-11-28 21:06:39Z matt $"
//
// Bitmap header file for the Fast Light Tool Kit (FLTK).
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

/* \file
 fltk3::Bitmap widget . */

#ifndef Fltk3_Bitmap_H
#define Fltk3_Bitmap_H

#include "Image.h"

namespace fltk3 { class Widget; }
struct Fl_Menu_Item;
class Fl_Image;
class Fl_Quartz_Graphics_Driver;
class Fl_GDI_Graphics_Driver;
class Fl_Xlib_Graphics_Driver;

namespace fltk3 {
  
  /**
   The fltk3::Bitmap class supports caching and drawing of mono-color
   (bitmap) images. Images are drawn using the current color.
   */
  class FLTK3_EXPORT Bitmap : public Fl_Image {
    friend class ::Fl_Quartz_Graphics_Driver;
    friend class ::Fl_GDI_Graphics_Driver;
    friend class ::Fl_Xlib_Graphics_Driver;
  public:
    
    /** pointer to raw bitmap data */
    const uchar *array;
    /** Non-zero if array points to bitmap data allocated internally */
    int alloc_array;
    
  private:
    
#if defined(__APPLE__) || defined(WIN32)
    /** for internal use */
    void *id_;
#else
    /** for internal use */
    unsigned id_;
#endif // __APPLE__ || WIN32
    
  public:
    
    /** The constructors create a new bitmap from the specified bitmap data */
    Bitmap(const uchar *bits, int W, int H) :
    Fl_Image(W,H,0), array(bits), alloc_array(0), id_(0) {data((const char **)&array, 1);}
    /** The constructors create a new bitmap from the specified bitmap data */
    Bitmap(const char *bits, int W, int H) :
    Fl_Image(W,H,0), array((const uchar *)bits), alloc_array(0), id_(0) {data((const char **)&array, 1);}
    virtual ~Bitmap();
    virtual Fl_Image *copy(int W, int H);
    Fl_Image *copy() { return copy(w(), h()); }
    virtual void draw(int X, int Y, int W, int H, int cx=0, int cy=0);
    void draw(int X, int Y) {draw(X, Y, w(), h(), 0, 0);}
    virtual void label(fltk3::Widget*w);
    virtual void label(Fl_Menu_Item*m);
    virtual void uncache();
  };
  
}

#endif

//
// End of "$Id: Bitmap.h 7903 2010-11-28 21:06:39Z matt $".
//