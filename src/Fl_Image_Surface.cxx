//
// "$Id$"
//
// Draw-to-image code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2014 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Printer.H>
#include <FL/Fl.H>

#if defined(WIN32) || defined(__APPLE__)
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement image surface handling here"
#else
#endif

const char *Fl_Image_Surface::class_id = "Fl_Image_Surface";

/** The constructor.
 \param w and \param h give the size in pixels of the resulting image.
 */
Fl_Image_Surface::Fl_Image_Surface(int w, int h) : Fl_Surface_Device(NULL) {
  width = w;
  height = h;
#ifdef __APPLE__
  offscreen = Fl_Quartz_Graphics_Driver::create_offscreen_with_alpha(w, h);
  helper = new Fl_Quartz_Flipped_Surface_(width, height);
  driver(helper->driver());
  CGContextSaveGState(offscreen);
  CGContextTranslateCTM(offscreen, 0, height);
  CGContextScaleCTM(offscreen, 1.0f, -1.0f);
#elif defined(WIN32)
  offscreen = fl_create_offscreen(w, h);
  helper = new Fl_GDI_Surface_();
  driver(helper->driver());
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement Fl_Image_Surface"
#else
  gc = 0;
  if (!fl_gc) { // allows use of this class before any window is shown
    fl_open_display();
    gc = XCreateGC(fl_display, RootWindow(fl_display, fl_screen), 0, 0);
    fl_gc = gc;
  }
  offscreen = fl_create_offscreen(w, h);
  helper = new Fl_Xlib_Surface_();
  driver(helper->driver());
#endif
}

/** The destructor.
 */
Fl_Image_Surface::~Fl_Image_Surface() {
#ifdef __APPLE__
  void *data = CGBitmapContextGetData((CGContextRef)offscreen);
  free(data);
  CGContextRelease((CGContextRef)offscreen);
  delete (Fl_Quartz_Flipped_Surface_*)helper;
#elif defined(WIN32)
  fl_delete_offscreen(offscreen);
  delete (Fl_GDI_Surface_*)helper;
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement Fl_Image_Surface"
#else
  fl_delete_offscreen(offscreen);
  if (gc) { XFreeGC(fl_display, gc); fl_gc = 0; }
  delete (Fl_Xlib_Surface_*)helper;
#endif
}

/** Returns an image made of all drawings sent to the Fl_Image_Surface object.
 The returned object contains its own copy of the RGB data.
 */
Fl_RGB_Image* Fl_Image_Surface::image()
{
  unsigned char *data;
  int depth = 3, ld = 0;
#ifdef __APPLE__
  CGContextFlush(offscreen);
  ld = CGBitmapContextGetBytesPerRow(offscreen);
  data = (uchar*)malloc(ld * height);
  memcpy(data, (uchar *)CGBitmapContextGetData(offscreen), ld * height);
  depth = 4;
  fl_gc = 0;
#elif defined(WIN32)
  fl_pop_clip(); 
  data = fl_read_image(NULL, 0, 0, width, height, 0);
  RestoreDC(fl_gc, _savedc); 
  DeleteDC(fl_gc); 
  _ss->set_current(); 
  fl_window=_sw; 
  fl_gc = _sgc;
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement Fl_Image_Surface"
#else
  fl_pop_clip(); 
  data = fl_read_image(NULL, 0, 0, width, height, 0);
  fl_window = pre_window; 
  previous->set_current();
#endif
  Fl_RGB_Image *image = new Fl_RGB_Image(data, width, height, depth, ld);
  image->alloc_array = 1;
  return image;
}

/** Draws a widget in the image surface
 
 \param widget any FLTK widget (e.g., standard, custom, window, GL view) to draw in the image
 \param delta_x and \param delta_y give 
 the position in the image of the top-left corner of the widget
 */
void Fl_Image_Surface::draw(Fl_Widget *widget, int delta_x, int delta_y)
{
  helper->print_widget(widget, delta_x, delta_y);
}


void Fl_Image_Surface::set_current()
{
#if defined(__APPLE__)
  fl_gc = offscreen; fl_window = 0;
  Fl_Surface_Device::set_current();
#elif defined(WIN32)
  _sgc=fl_gc; 
  _sw=fl_window;
  _ss = Fl_Surface_Device::surface(); 
  Fl_Surface_Device::set_current();
  fl_gc = fl_makeDC(offscreen); 
   _savedc = SaveDC(fl_gc); 
  fl_window=(HWND)offscreen; 
  fl_push_no_clip();
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement Fl_Image_Surface"
#else
  pre_window = fl_window; 
  fl_window = offscreen; 
  previous = Fl_Surface_Device::surface(); 
  Fl_Surface_Device::set_current();
  fl_push_no_clip();
#endif
}

#if defined(__APPLE__)

Fl_Quartz_Flipped_Surface_::Fl_Quartz_Flipped_Surface_(int w, int h) : Fl_Quartz_Surface_(w, h) {
}

void Fl_Quartz_Flipped_Surface_::translate(int x, int y) {
  CGContextRestoreGState(fl_gc);
  CGContextSaveGState(fl_gc);
  CGContextTranslateCTM(fl_gc, x, -y);
  CGContextSaveGState(fl_gc);
  CGContextTranslateCTM(fl_gc, 0, height);
  CGContextScaleCTM(fl_gc, 1.0f, -1.0f);
}

void Fl_Quartz_Flipped_Surface_::untranslate() {
  CGContextRestoreGState(fl_gc);
}

const char *Fl_Quartz_Flipped_Surface_::class_id = "Fl_Quartz_Flipped_Surface_";

#endif

/** Draws a window and its borders and title bar to the image drawing surface. 
 \param win an FLTK window to draw in the image
 \param delta_x and \param delta_y give
 the position in the image of the top-left corner of the window's title bar
*/
void Fl_Image_Surface::draw_decorated_window(Fl_Window* win, int delta_x, int delta_y)
{
  helper->draw_decorated_window(win, delta_x, delta_y);
}


//
// End of "$Id$".
//
