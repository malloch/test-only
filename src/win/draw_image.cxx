/
// "$Id$
/
// WIN32 image drawing code for the Fast Light Tool Kit (FLTK)
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

// I hope a simple and portable method of drawing color and monochrom
// images.  To keep this simple, only a single storage type i
// supported: 8 bit unsigned data, byte order RGB, and pixels ar
// stored packed into rows with the origin at the top-left.  It i
// possible to alter the size of pixels with the "delta" argument, t
// add alpha or other information per pixel.  It is also possible t
// change the origin and direction of the image data by messing wit
// the "delta" and "linedelta", making them negative, though this ma
// defeat some of the shortcuts in translating the image for X

// Unbelievably (since it conflicts with how most PC software works
// Micro$oft picked a bottom-up and BGR storage format for thei
// DIB images.  I'm pretty certain there is a way around this, bu
// I can't find any other than the brute-force method of drawin
// each line as a seperate image.  This may also need to be don
// if the delta is any amount other than 1, 3, or 4

///////////////////////////////////////////////////////////////

#include <config.h
#include <FL/Fl.H
#include <FL/fl_draw.H
#include <FL/x.H
#include "Fl_Win_Display.H

#define MAXBUFFER 0x40000 // 256

#if HAVE_ALPHABLEN
FL_EXPORT int fl_draw_alpha = 0
#endi

#if USE_COLORMA

// error-diffusion dither into the FLTK colorma
static void dither(uchar* to, const uchar* from, int w, int delta) 
  static int ri, gi, bi, dir
  int r=ri, g=gi, b=bi
  int d, td
  if (dir) 
    dir = 0
    from = from+(w-1)*delta
    to = to+(w-1)
    d = -delta
    td = -1
  } else 
    dir = 1
    d = delta
    td = 1
  
  for (; w--; from += d, to += td) 
    r += from[0]; if (r < 0) r = 0; else if (r>255) r = 255
    int rr = r*FL_NUM_RED/256
    r -= rr*255/(FL_NUM_RED-1)
    g += from[1]; if (g < 0) g = 0; else if (g>255) g = 255
    int gg = g*FL_NUM_GREEN/256
    g -= gg*255/(FL_NUM_GREEN-1)
    b += from[2]; if (b < 0) b = 0; else if (b>255) b = 255
    int bb = b*FL_NUM_BLUE/256
    b -= bb*255/(FL_NUM_BLUE-1)
    *to = uchar(FL_COLOR_CUBE+(bb*FL_NUM_RED+rr)*FL_NUM_GREEN+gg)
  
  ri = r; gi = g; bi = b


// error-diffusion dither into the FLTK colorma
static void monodither(uchar* to, const uchar* from, int w, int delta) 
  static int ri,dir
  int r=ri
  int d, td
  if (dir) 
    dir = 0
    from = from+(w-1)*delta
    to = to+(w-1)
    d = -delta
    td = -1
  } else 
    dir = 1
    d = delta
    td = 1
  
  for (; w--; from += d, to += td) 
    r += *from; if (r < 0) r = 0; else if (r>255) r = 255
    int rr = r*FL_NUM_GRAY/256
    r -= rr*255/(FL_NUM_GRAY-1)
    *to = uchar(FL_GRAY_RAMP+rr)
  
  ri = r


#endif // USE_COLORMA

extern uchar **fl_mask_bitmap

static void innards(const uchar *buf, int X, int Y, int W, int H
		    int delta, int linedelta, int mono
		    Fl_Draw_Image_Cb cb, void* userdata

#if USE_COLORMA
  char indexed = (fl_palette != 0)
#endi

  if (!linedelta) linedelta = W*delta

  int x, y, w, h
  fl_clip_box(X,Y,W,H,x,y,w,h)
  if (w<=0 || h<=0) return
  if (buf) buf += (x-X)*delta + (y-Y)*linedelta

  static U32 bmibuffer[256+12]
  BITMAPINFO &bmi = *((BITMAPINFO*)bmibuffer)
  if (!bmi.bmiHeader.biSize) 
    //bmi.bmiHeader.biSize = sizeof(bmi)-4; // does it use this to determine type
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER)
    bmi.bmiHeader.biPlanes = 1
    bmi.bmiHeader.biCompression = BI_RGB
    bmi.bmiHeader.biXPelsPerMeter = 0
    bmi.bmiHeader.biYPelsPerMeter = 0
    bmi.bmiHeader.biClrUsed = 0
    bmi.bmiHeader.biClrImportant = 0
  

  // -1 == non
  // 0  == indexe
  // 1  == mon
  static int current_cmap = -1

#if USE_COLORMA
  if (indexed) 
	if(current_cmap != 0) 
      current_cmap = 0
      for (short i=0; i<256; i++) 
        *((short*)(bmi.bmiColors)+i) = i
	  
	
  } els
#endi
  if (mono) 
	if(current_cmap != 1) 
      current_cmap = 1
      for (int i=0; i<256; i++) 
        bmi.bmiColors[i].rgbBlue = (uchar)i
        bmi.bmiColors[i].rgbGreen = (uchar)i
        bmi.bmiColors[i].rgbRed = (uchar)i
        bmi.bmiColors[i].rgbReserved = (uchar)i
	  
	
  

#if HAVE_ALPHABLEN
# if USE_COLORMA
  int pixelsize = mono|indexed ? 1 : 4
# els
  int pixelsize = mono ? 1 : 4
# endi
  int linesize = pixelsize * w
  if ((linesize % 4) != 0) 
    linesize += 4 - (linesize % 4)
  

#els

# if USE_COLORMA
  int pixelsize = mono|indexed ? 1 : 3
# els
  int pixelsize = mono ? 1 : 3
# endi
  int linesize = (pixelsize*w+3)&~3

#endi

  bmi.bmiHeader.biWidth = w
  bmi.bmiHeader.biBitCount = pixelsize*8

  static U32* buffer
  int blocking = h
  {int size = linesize*h
  if (size > MAXBUFFER) 
    size = MAXBUFFER
    blocking = MAXBUFFER/linesize
  
  static long buffer_size
  if (size > buffer_size) 
    delete[] buffer
    buffer_size = size
    buffer = new U32[(size+3)/4]
  }
  bmi.bmiHeader.biHeight = blocking
  static U32* line_buffer
  if (!buf) 
    int size = W*delta
    static int line_buf_size
    if (size > line_buf_size) 
      delete[] line_buffer
      line_buf_size = size
      line_buffer = new U32[(size+3)/4]
    
  
  for (int j=0; j<h; ) 
    int k
    for (k = 0; j<h && k<blocking; k++, j++) 
      const uchar* from
      if (!buf) { // run the converter
	    cb(userdata, x-X, y-Y+j, w, (uchar*)line_buffer)
	    from = (uchar*)line_buffer
      } else 
	    from = buf
	    buf += linedelta
      
      uchar *to = (uchar*)buffer+(blocking-k-1)*linesize
#if USE_COLORMA
      if (indexed) 
	    if (mono
	      monodither(to, from, w, delta)
	    else
	      dither(to, from, w, delta)
	    to += w
      } els
#endi
	 
        if (mono) 
	        for (int i=w; i--; from += delta) *to++ = *from
        } else 
	        for (int i=w; i--; from += delta, to += pixelsize) 
	          to[0] = from[2]
	          to[1] = from[1]
	          to[2] = from[0]
#if HAVE_ALPHABLEN
		        if(fl_draw_alpha)
			        uchar a = to[3] = from[3]
			        to[0] = (uchar)(((int)to[0]) * a / 0xff)
			        to[1] = (uchar)(((int)to[1]) * a / 0xff)
			        to[2] = (uchar)(((int)to[2]) * a / 0xff)
            
#endi
          
        
    
    SetDIBitsToDevice(fl_gc, x, y+j-k, w, k, 0, 0, 0, k
		      (LPSTR)((uchar*)buffer+(blocking-k)*linesize)
		      &bmi
#if USE_COLORMA
		      indexed ? DIB_PAL_COLORS : DIB_RGB_COLOR
#els
		      DIB_RGB_COLOR
#endi
		      )
  


void Fl_Win_Display::draw_image(const uchar* buf, int x, int y, int w, int h, int d, int l)
  innards(buf,x,y,w,h,d,l,(d<3&&d>-3),0,0)

void Fl_Win_Display::draw_image(Fl_Draw_Image_Cb cb, void* data
		   int x, int y, int w, int h,int d) 
  innards(0,x,y,w,h,d,0,(d<3&&d>-3),cb,data)

void Fl_Win_Display::draw_image_mono(const uchar* buf, int x, int y, int w, int h, int d, int l)
  innards(buf,x,y,w,h,d,l,1,0,0)

void Fl_Win_Display::draw_image_mono(Fl_Draw_Image_Cb cb, void* data
		   int x, int y, int w, int h,int d) 
  innards(0,x,y,w,h,d,0,1,cb,data)


void Fl_Win_Display::rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b) 
#if USE_COLORMA
  // use the error diffusion dithering code to produce a much nicer block
  if (fl_palette) 
    uchar c[3]
    c[0] = r; c[1] = g; c[2] = b
    innards(c,x,y,w,h,0,0,0,0,0)
    return
  
#endi
  fl_color(r,g,b)
  fl_rectf(x,y,w,h)


/
// End of "$Id$"
/
