//
// "$Id$"
//
// System color support for the Fast Light Tool Kit (FLTK).
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

#include <fltk3/run.h>
#include <fltk3/draw.h>
#include <fltk3/x.h>
#include <fltk3/math.h>
#include <fltk3/utf8.h>
#include "flstring.h"
#include <stdio.h>
#include <stdlib.h>
#include <fltk3/Pixmap.h>
#include <fltk3/TiledImage.h>
#include "tile.xpm"

#if defined(WIN32) && !defined(__CYGWIN__) && !defined(__WATCOMC__)
// Visual C++ 2005 incorrectly displays a warning about the use of POSIX APIs
// on Windows, which is supposed to be POSIX compliant...
#  define putenv _putenv
#endif // WIN32 && !__CYGWIN__

static char	fl_bg_set = 0;
static char	fl_bg2_set = 0;
static char	fl_fg_set = 0;

/**
    Changes fltk3::color(fltk3::BACKGROUND_COLOR) to the given color, 
    and changes the gray ramp from 32 to 56 to black to white.  These are 
    the colors used as backgrounds by almost all widgets and used to draw 
    the edges of all the boxtypes.
*/
void fltk3::background(uchar r, uchar g, uchar b) {
  fl_bg_set = 1;

  // replace the gray ramp so that fltk3::GRAY is this color
  if (!r) r = 1; else if (r==255) r = 254;
  double powr = log(r/255.0)/log((fltk3::GRAY-fltk3::GRAY_RAMP)/(fltk3::NUM_GRAY-1.0));
  if (!g) g = 1; else if (g==255) g = 254;
  double powg = log(g/255.0)/log((fltk3::GRAY-fltk3::GRAY_RAMP)/(fltk3::NUM_GRAY-1.0));
  if (!b) b = 1; else if (b==255) b = 254;
  double powb = log(b/255.0)/log((fltk3::GRAY-fltk3::GRAY_RAMP)/(fltk3::NUM_GRAY-1.0));
  for (unsigned int i = 0; i < fltk3::NUM_GRAY; i++) {
    double gray = i/(fltk3::NUM_GRAY-1.0);
    fltk3::set_color(fltk3::gray_ramp(i),
		  uchar(pow(gray,powr)*255+.5),
		  uchar(pow(gray,powg)*255+.5),
		  uchar(pow(gray,powb)*255+.5));
  }
}
/** Changes fltk3::color(fltk3::FOREGROUND_COLOR). */
void fltk3::foreground(uchar r, uchar g, uchar b) {
  fl_fg_set = 1;

  fltk3::set_color(fltk3::FOREGROUND_COLOR,r,g,b);
}

/**
    Changes the alternative background color. This color is used as a 
    background by fltk3::Input and other text widgets.
    <P>This call may change fltk3::color(fltk3::FOREGROUND_COLOR) if it 
    does not provide sufficient contrast to fltk3::BACKGROUND2_COLOR.
*/
void fltk3::background2(uchar r, uchar g, uchar b) {
  fl_bg2_set = 1;

  fltk3::set_color(fltk3::BACKGROUND2_COLOR,r,g,b);
  fltk3::set_color(fltk3::FOREGROUND_COLOR,
                get_color(fltk3::contrast(fltk3::FOREGROUND_COLOR,fltk3::BACKGROUND2_COLOR)));
}

// these are set by fltk3::args() and override any system colors:
const char *fl_fg = NULL;
const char *fl_bg = NULL;
const char *fl_bg2 = NULL;

static void set_selection_color(uchar r, uchar g, uchar b) {
  fltk3::set_color(fltk3::SELECTION_COLOR,r,g,b);
}

#if defined(WIN32) || defined(__APPLE__)

#  include <stdio.h>
// simulation of XParseColor:
int fl_parse_color(const char* p, uchar& r, uchar& g, uchar& b) {
  if (*p == '#') p++;
  int n = strlen(p);
  int m = n/3;
  const char *pattern = 0;
  switch(m) {
  case 1: pattern = "%1x%1x%1x"; break;
  case 2: pattern = "%2x%2x%2x"; break;
  case 3: pattern = "%3x%3x%3x"; break;
  case 4: pattern = "%4x%4x%4x"; break;
  default: return 0;
  }
  int R,G,B; if (sscanf(p,pattern,&R,&G,&B) != 3) return 0;
  switch(m) {
  case 1: R *= 0x11; G *= 0x11; B *= 0x11; break;
  case 3: R >>= 4; G >>= 4; B >>= 4; break;
  case 4: R >>= 8; G >>= 8; B >>= 8; break;
  }
  r = (uchar)R; g = (uchar)G; b = (uchar)B;
  return 1;
}
#else
// Wrapper around XParseColor...
int fl_parse_color(const char* p, uchar& r, uchar& g, uchar& b) {
  XColor x;
  if (!fl_display) fl_open_display();
  if (XParseColor(fl_display, fl_colormap, p, &x)) {
    r = (uchar)(x.red>>8);
    g = (uchar)(x.green>>8);
    b = (uchar)(x.blue>>8);
    return 1;
  } else return 0;
}
#endif // WIN32 || __APPLE__
/** \fn fltk3::get_system_colors()
    Read the user preference colors from the system and use them to call
    fltk3::foreground(), fltk3::background(), and 
    fltk3::background2().  This is done by
    fltk3::Window::show(argc,argv) before applying the -fg and -bg
    switches.
    
    <P>On X this reads some common values from the Xdefaults database.
    KDE users can set these values by running the "krdb" program, and
    newer versions of KDE set this automatically if you check the "apply
    style to other X programs" switch in their control panel.
*/
#if defined(WIN32)
static void
getsyscolor(int what, const char* arg, void (*func)(uchar,uchar,uchar))
{
  if (arg) {
    uchar r,g,b;
    if (!fl_parse_color(arg, r,g,b))
      fltk3::error("Unknown color: %s", arg);
    else
      func(r,g,b);
  } else {
    DWORD x = GetSysColor(what);
    func(uchar(x&255), uchar(x>>8), uchar(x>>16));
  }
}

void fltk3::get_system_colors() {
  if (!fl_bg2_set) getsyscolor(COLOR_WINDOW,	fl_bg2,fltk3::background2);
  if (!fl_fg_set) getsyscolor(COLOR_WINDOWTEXT,	fl_fg, fltk3::foreground);
  if (!fl_bg_set) getsyscolor(COLOR_BTNFACE,	fl_bg, fltk3::background);
  getsyscolor(COLOR_HIGHLIGHT,	0,     set_selection_color);
}

#elif defined(__APPLE__)
// MacOS X currently supports two color schemes - Blue and Graphite.
// Since we aren't emulating the Aqua interface (even if Apple would
// let us), we use some defaults that are similar to both.  The
// fltk3::scheme("plastic") color/box scheme provides a usable Aqua-like
// look-n-feel...
void fltk3::get_system_colors()
{
  fl_open_display();

  if (!fl_bg2_set) background2(0xff, 0xff, 0xff);
  if (!fl_fg_set) foreground(0, 0, 0);
  if (!fl_bg_set) background(0xd8, 0xd8, 0xd8);
  
#if 0 
  // this would be the correct code, but it does not run on all versions
  // of OS X. Also, setting a bright selection color would require 
  // some updates in fltk3::Adjuster and Fl_Help_Browser
  OSStatus err;
  RGBColor c;
  err = GetThemeBrushAsColor(kThemeBrushPrimaryHighlightColor, 24, true, &c);
  if (err)
    set_selection_color(0x00, 0x00, 0x80);
  else
    set_selection_color(c.red, c.green, c.blue);
#else
  set_selection_color(0x00, 0x00, 0x80);
#endif
}
#else

// Read colors that KDE writes to the xrdb database.

// XGetDefault does not do the expected thing: it does not like
// periods in either word. Therefore it cannot match class.Text.background.
// However *.Text.background is matched by pretending the program is "Text".
// But this will also match *.background if there is no *.Text.background
// entry, requiring users to put in both (unless they want the text fields
// the same color as the windows).

static void
getsyscolor(const char *key1, const char* key2, const char *arg, const char *defarg, void (*func)(uchar,uchar,uchar))
{
  if (!arg) {
    arg = XGetDefault(fl_display, key1, key2);
    if (!arg) arg = defarg;
  }
  XColor x;
  if (!XParseColor(fl_display, fl_colormap, arg, &x))
    fltk3::error("Unknown color: %s", arg);
  else
    func(x.red>>8, x.green>>8, x.blue>>8);
}

void fltk3::get_system_colors()
{
  fl_open_display();
  const char* key1 = 0;
  if (fltk3::first_window()) key1 = fltk3::first_window()->xclass();
  if (!key1) key1 = "fltk";
  if (!fl_bg2_set) getsyscolor("Text","background",	fl_bg2,	"#ffffff", fltk3::background2);
  if (!fl_fg_set) getsyscolor(key1,  "foreground",	fl_fg,	"#000000", fltk3::foreground);
  if (!fl_bg_set) getsyscolor(key1,  "background",	fl_bg,	"#c0c0c0", fltk3::background);
  getsyscolor("Text", "selectBackground", 0, "#000080", set_selection_color);
}

#endif


//// Simple implementation of 2.0 fltk3::scheme() interface...
#define D1 BORDER_WIDTH
#define D2 (BORDER_WIDTH+BORDER_WIDTH)

extern void	fl_gtk_up_box(int, int, int, int, fltk3::Color, fltk3::Boxtype);
extern void	fl_gtk_down_box(int, int, int, int, fltk3::Color, fltk3::Boxtype);
extern void	fl_gtk_thin_up_box(int, int, int, int, fltk3::Color, fltk3::Boxtype);
extern void	fl_gtk_thin_down_box(int, int, int, int, fltk3::Color, fltk3::Boxtype);
extern void	fl_gtk_round_up_box(int, int, int, int, fltk3::Color, fltk3::Boxtype);
extern void	fl_gtk_round_down_box(int, int, int, int, fltk3::Color, fltk3::Boxtype);

extern void	fl_gtk_up_frame(int, int, int, int, fltk3::Color, fltk3::Boxtype);
extern void	fl_gtk_down_frame(int, int, int, int, fltk3::Color, fltk3::Boxtype);
extern void	fl_gtk_thin_up_frame(int, int, int, int, fltk3::Color, fltk3::Boxtype);
extern void	fl_gtk_thin_down_frame(int, int, int, int, fltk3::Color, fltk3::Boxtype);

#ifndef FLTK3_DOXYGEN
const char	*fltk3::scheme_ = (const char *)0;	    // current scheme 
fltk3::Image	*fltk3::scheme_bg_ = (fltk3::Image *)0;    // current background image for the scheme
#endif

static fltk3::Pixmap	tile(tile_xpm);

/**
    Gets or sets the current widget scheme. NULL will use
    the scheme defined in the FLTK_SCHEME environment
    variable or the scheme resource under X11. Otherwise,
    any of the following schemes can be used:</P>
    
    <ul>
    
    	<li>"none" or "gtk+" - This is the default look-n-feel which
    	is inspired by the Red Hat Bluecurve theme</li>
    
    	<li>"plastic" - This scheme is inspired by the Aqua user interface
    	on Mac OS X</li>
    
        <li>"classic" - This is the FLTK1 look which resembles old
        Windows (95/98/Me/NT/2000) and old GTK/KDE</li>

    </ul>
*/
int fltk3::scheme(const char *s) {
  if (!s) {
    if ((s = getenv("FLTK_SCHEME")) == NULL) {
#if !defined(WIN32) && !defined(__APPLE__)
      const char* key = 0;
      if (fltk3::first_window()) key = fltk3::first_window()->xclass();
      if (!key) key = "fltk";
      fl_open_display();
      s = XGetDefault(fl_display, key, "scheme");
#endif // !WIN32 && !__APPLE__
    }
  }

  if (s) {
    if (!strcasecmp(s, "none") || !strcasecmp(s, "base") || !strcasecmp(s, "gtk+") || !*s) s = 0;
    else if (!strcasecmp(s, "classic")) s = strdup("classic");
    else if (!strcasecmp(s, "plastic")) s = strdup("plastic");
    else s = 0;
  }
  if (scheme_) free((void*)scheme_);
  scheme_ = s;

  // Save the new scheme in the FLTK_SCHEME env var so that child processes
  // inherit it...
  static char e[1024];
  strcpy(e,"FLTK_SCHEME=");
  if (s) strlcat(e,s,sizeof(e));
  putenv(e);

  // Load the scheme...
  return reload_scheme();
}

int fltk3::reload_scheme() {
  fltk3::Window *win;

  if (scheme_ && !strcasecmp(scheme_, "plastic")) {
    // Update the tile image to match the background color...
    uchar r, g, b;
    int nr, ng, nb;
    int i;
//    static uchar levels[3] = { 0xff, 0xef, 0xe8 };
    // OSX 10.3 and higher use a background with less contrast...
    static uchar levels[3] = { 0xff, 0xf8, 0xf4 };

    get_color(fltk3::GRAY, r, g, b);

//    printf("fltk3::GRAY = 0x%02x 0x%02x 0x%02x\n", r, g, b);

    for (i = 0; i < 3; i ++) {
      nr = levels[i] * r / 0xe8;
      if (nr > 255) nr = 255;

      ng = levels[i] * g / 0xe8;
      if (ng > 255) ng = 255;

      nb = levels[i] * b / 0xe8;
      if (nb > 255) nb = 255;

      sprintf(tile_cmap[i], "%c c #%02x%02x%02x", "Oo."[i], nr, ng, nb);
//      puts(tile_cmap[i]);
    }

    tile.uncache();

    // FIXME: there is a misunderstanding: please fix the ROUNDED vs. ROUND box types!
    if (!scheme_bg_) scheme_bg_ = new fltk3::TiledImage(&tile, w(), h());

    // Load plastic buttons, etc...
    set_boxtype(fltk3::UP_FRAME,        fltk3::PLASTIC_UP_FRAME);
    set_boxtype(fltk3::DOWN_FRAME,      fltk3::PLASTIC_DOWN_FRAME);
    set_boxtype(fltk3::THIN_UP_FRAME,   fltk3::PLASTIC_UP_FRAME);
    set_boxtype(fltk3::THIN_DOWN_FRAME, fltk3::PLASTIC_DOWN_FRAME);

    set_boxtype(fltk3::UP_BOX,          fltk3::PLASTIC_UP_BOX);
    set_boxtype(fltk3::DOWN_BOX,        fltk3::PLASTIC_DOWN_BOX);
    set_boxtype(fltk3::THIN_UP_BOX,     fltk3::PLASTIC_THIN_UP_BOX);
    set_boxtype(fltk3::THIN_DOWN_BOX,   fltk3::PLASTIC_THIN_DOWN_BOX);
    set_boxtype(fltk3::ROUND_UP_BOX,    fltk3::PLASTIC_ROUND_UP_BOX);
    set_boxtype(fltk3::ROUND_DOWN_BOX,  fltk3::PLASTIC_ROUND_DOWN_BOX);

    // Use standard size scrollbars...
    fltk3::scrollbar_size(16);
  } else if (scheme_ && !strcasecmp(scheme_, "classic")) {
    // Use the standard FLTK look-n-feel...
    if (scheme_bg_) {
      delete scheme_bg_;
      scheme_bg_ = (fltk3::Image *)0;
    }
    
    set_boxtype(fltk3::UP_FRAME,        fltk3::CLASSIC_UP_FRAME);
    set_boxtype(fltk3::DOWN_FRAME,      fltk3::CLASSIC_DOWN_FRAME);
    set_boxtype(fltk3::THIN_UP_FRAME,   fltk3::CLASSIC_THIN_UP_FRAME);
    set_boxtype(fltk3::THIN_DOWN_FRAME, fltk3::CLASSIC_THIN_DOWN_FRAME);
    
    set_boxtype(fltk3::UP_BOX,          fltk3::CLASSIC_UP_BOX);
    set_boxtype(fltk3::DOWN_BOX,        fltk3::CLASSIC_DOWN_BOX);
    set_boxtype(fltk3::THIN_UP_BOX,     fltk3::CLASSIC_THIN_UP_BOX);
    set_boxtype(fltk3::THIN_DOWN_BOX,   fltk3::CLASSIC_THIN_DOWN_BOX);
    set_boxtype(fltk3::ROUND_UP_BOX,    fltk3::CLASSIC_ROUND_UP_BOX);
    set_boxtype(fltk3::ROUND_DOWN_BOX,  fltk3::CLASSIC_ROUND_DOWN_BOX);
    
    // Use standard size scrollbars...
    fltk3::scrollbar_size(16);
  } else {
    // Use a GTK+ inspired look-n-feel...
    if (scheme_bg_) {
      delete scheme_bg_;
      scheme_bg_ = (fltk3::Image *)0;
    }
    
    set_boxtype(fltk3::UP_FRAME,        fl_gtk_up_frame,	2,2,4,4);
    set_boxtype(fltk3::DOWN_FRAME,      fl_gtk_down_frame,      2,2,4,4);
    set_boxtype(fltk3::THIN_UP_FRAME,   fl_gtk_thin_up_frame,	1,1,2,2);
    set_boxtype(fltk3::THIN_DOWN_FRAME, fl_gtk_thin_down_frame,	1,1,2,2);
    
    set_boxtype(fltk3::UP_BOX,          fl_gtk_up_box,		2,2,4,4);
    set_boxtype(fltk3::DOWN_BOX,        fl_gtk_down_box,	2,2,4,4);
    set_boxtype(fltk3::THIN_UP_BOX,     fl_gtk_thin_up_box,	1,1,2,2);
    set_boxtype(fltk3::THIN_DOWN_BOX,   fl_gtk_thin_down_box,	1,1,2,2);
    set_boxtype(fltk3::ROUND_UP_BOX,    fl_gtk_round_up_box,	2,2,4,4);
    set_boxtype(fltk3::ROUND_DOWN_BOX,  fl_gtk_round_down_box,	2,2,4,4);
        
    // Use slightly thinner scrollbars...
    fltk3::scrollbar_size(15);
  }

  // Set (or clear) the background tile for all windows...
  for (win = first_window(); win; win = next_window(win)) {
    win->labeltype(scheme_bg_ ? fltk3::NORMAL_LABEL : fltk3::NO_LABEL);
    win->align(fltk3::ALIGN_CENTER | fltk3::ALIGN_INSIDE | fltk3::ALIGN_CLIP);
    win->image(scheme_bg_);
    win->redraw();
  }

  return 1;
}


//
// End of "$Id$".
//