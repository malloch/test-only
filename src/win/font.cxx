/
// "$Id $
/
// WIN32 font selection routines for the Fast Light Tool Kit (FLTK)
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
#include "../flstring.h
#include <FL/Fl.H
#include <FL/fl_draw.H
#include <FL/x.H
#include "../Fl_Font.H

#include <stdio.h
#include <stdlib.h

#include "Fl_Win_Display.H

Fl_FontSize::Fl_FontSize(const char* name, int size) 
  int weight = FW_NORMAL
  int italic = 0
  switch (*name++) 
  case 'I': italic = 1; break
  case 'P': italic = 1
  case 'B': weight = FW_BOLD; break
  case ' ': break
  default: name--
  
  fid = CreateFont
    -size, // negative makes it use "char size
    0,	            // logical average character width
    0,	            // angle of escapement
    0,	            // base-line orientation angle
    weight
    italic
    FALSE,	        // underline attribute flag
    FALSE,	        // strikeout attribute flag
    DEFAULT_CHARSET,    // character set identifier
    OUT_DEFAULT_PRECIS,	// output precision
    CLIP_DEFAULT_PRECIS,// clipping precision
    DEFAULT_QUALITY,	// output quality
    DEFAULT_PITCH,	// pitch and family
    name	        // pointer to typeface name string
    )
  if (!fl_gc) fl_GetDC(0)
  SelectObject(fl_gc, fid)
  GetTextMetrics(fl_gc, &metr)
//  BOOL ret = GetCharWidthFloat(fl_gc, metr.tmFirstChar, metr.tmLastChar, font->width+metr.tmFirstChar)
// ...would be the right call, but is not implemented into Window95! (WinNT?
  GetCharWidth(fl_gc, 0, 255, width)
#if HAVE_G
  listbase = 0
#endi
  minsize = maxsize = size


Fl_FontSize* fl_fontsize

Fl_FontSize::~Fl_FontSize() 
#if HAVE_G
// Delete list created by gl_draw().  This is not done by this cod
// as it will link in GL unnecessarily.  There should be some kin
// of "free" routine pointer, or a subclass
// if (listbase) 
//  int base = font->min_char_or_byte2
//  int size = font->max_char_or_byte2-base+1
//  int base = 0; int size = 256
//  glDeleteLists(listbase+base,size)
// 
#endi
  if (this == fl_fontsize) fl_fontsize = 0
  DeleteObject(fid)


///////////////////////////////////////////////////////////////

// WARNING: if you add to this table, you must redefine FL_FREE_FON
// in Enumerations.H & recompile!
static Fl_Fontdesc built_in_table[] = 
{" Arial"}
{"BArial"}
{"IArial"}
{"PArial"}
{" Courier New"}
{"BCourier New"}
{"ICourier New"}
{"PCourier New"}
{" Times New Roman"}
{"BTimes New Roman"}
{"ITimes New Roman"}
{"PTimes New Roman"}
{" Symbol"}
{" Terminal"}
{"BTerminal"}
{" Wingdings"}
}

Fl_Fontdesc* fl_fonts = built_in_table

static Fl_FontSize* find(int fnum, int size) 
  Fl_Fontdesc* s = fl_fonts+fnum
  if (!s->name) s = fl_fonts; // use 0 if fnum undefine
  Fl_FontSize* f
  for (f = s->first; f; f = f->next
    if (f->minsize <= size && f->maxsize >= size) return f
  f = new Fl_FontSize(s->name, size)
  f->next = s->first
  s->first = f
  return f


///////////////////////////////////////////////////////////////
// Public interface

int fl_font_ = 0
int fl_size_ = 0
//static HDC font_gc

//int Fl_Win_Display::size(){return fl_size_;
//int Fl_Win_Display::font(){return fl_font_;

void Fl_Win_Display::font(int fnum, int size) 
  if (fnum == fl_font_ && size == fl_size_) return
  fl_font_ = fnum; fl_size_ = size
  fl_fontsize = find(fnum, size)


int Fl_Win_Display::height() 
if (fl_fontsize
  return (fl_fontsize->metr.tmAscent + fl_fontsize->metr.tmDescent)
else return -1


int Fl_Win_Display::descent() 
if (fl_fontsize
  return fl_fontsize->metr.tmDescent
else return -1


double Fl_Win_Display::width(const char* c, int n) 
 if (!fl_fontsize) return -1.0
  double w = 0.0
  while (n--) w += fl_fontsize->width[uchar(*c++)]
  return w


double Fl_Win_Display::width(unsigned c) 
  if (!fl_fontsize) return -1.0
  return fl_fontsize->width[c]


void Fl_Win_Display::draw(const char* str, int n, int x, int y) 
  COLORREF oldColor = SetTextColor(fl_gc, fl_RGB())
  SelectObject(fl_gc, fl_fontsize->fid)
  TextOut(fl_gc, x, y, str, n)
  SetTextColor(fl_gc, oldColor)



/
// End of "$Id$"
/
