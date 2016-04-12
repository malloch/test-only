//
// "$Id$"
//
// Main event handling code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

#include <config.h>

#ifdef WIN32
#elif defined(__APPLE__)
#elif defined(USE_SDL)
#  pragma message "FL_SDL: implement the FLTK core in its own file"
#elif defined(ANDROID)
#  pragma message "ANDROID: implement the FLTK core in its own file"
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement the FLTK core in its own file"
#  include "Fl_porting.cxx"
#endif

#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_Screen_Driver.H>
#include <FL/Fl_Window_Driver.H>
#include <FL/Fl_System_Driver.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tooltip.H>
#include <FL/fl_draw.H>

#include <ctype.h>
#include <stdlib.h>
#include "flstring.h"

#if defined(DEBUG) || defined(DEBUG_WATCH)
#  include <stdio.h>
#endif // DEBUG || DEBUG_WATCH

//
// Runtime configuration flags
//
#ifdef FL_CFG_GFX_XLIB
bool Fl::cfg_gfx_xlib = 1;
#else
bool Fl::cfg_gfx_xlib = 0;
#endif
#ifdef FL_CFG_GFX_QUARTZ
bool Fl::cfg_gfx_quartz = 1;
#else
bool Fl::cfg_gfx_quartz = 0;
#endif
#ifdef FL_CFG_GFX_GDI
bool Fl::cfg_gfx_gdi = 1;
#else
bool Fl::cfg_gfx_gdi = 0;
#endif
#ifdef FL_CFG_GFX_OPENGL
bool Fl::cfg_gfx_opengl = 1;
#else
bool Fl::cfg_gfx_opengl = 0;
#endif
#ifdef FL_CFG_GFX_CAIRO
bool Fl::cfg_gfx_cairo = 1;
#else
bool Fl::cfg_gfx_cairo = 0;
#endif
#ifdef FL_CFG_GFX_DIRECTX
bool Fl::cfg_gfx_directx = 1;
#else
bool Fl::cfg_gfx_directx = 0;
#endif

#ifdef FL_CFG_PRN_PS
bool Fl::cfg_prn_ps = 1;
#else
bool Fl::cfg_prn_ps = 0;
#endif
#ifdef FL_CFG_PRN_QUARTZ
bool Fl::cfg_prn_quartz = 1;
#else
bool Fl::cfg_prn_quartz = 0;
#endif
#ifdef FL_CFG_PRN_GDI
bool Fl::cfg_prn_gdi = 1;
#else
bool Fl::cfg_prn_gdi = 0;
#endif

#ifdef FL_CFG_WIN_X11
bool Fl::cfg_win_x11 = 1;
#else
bool Fl::cfg_win_x11 = 0;
#endif
#ifdef FL_CFG_WIN_COCOA
bool Fl::cfg_win_cocoa = 1;
#else
bool Fl::cfg_win_cocoa = 0;
#endif
#ifdef FL_CFG_WIN_WIN32
bool Fl::cfg_win_win32 = 1;
#else
bool Fl::cfg_win_win32 = 0;
#endif

#ifdef FL_SYS_POSIX
bool Fl::cfg_sys_posix = 1;
#else
bool Fl::cfg_sys_posix = 0;
#endif
#ifdef FL_SYS_WIN32
bool Fl::cfg_sys_win32 = 1;
#else
bool Fl::cfg_sys_win32 = 0;
#endif

//
// Globals...
//

Fl_Widget *fl_selection_requestor;

#ifndef FL_DOXYGEN
Fl_Widget	*Fl::belowmouse_,
		*Fl::pushed_,
		*Fl::focus_,
		*Fl::selection_owner_;
int		Fl::damage_,
		Fl::e_number,
		Fl::e_x,
		Fl::e_y,
		Fl::e_x_root,
		Fl::e_y_root,
		Fl::e_dx,
		Fl::e_dy,
		Fl::e_state,
		Fl::e_clicks,
		Fl::e_is_click,
		Fl::e_keysym,
                Fl::e_original_keysym,
		Fl::scrollbar_size_ = 16;

char		*Fl::e_text = (char *)"";
int		Fl::e_length;
const char*	Fl::e_clipboard_type = "";
void *		Fl::e_clipboard_data = NULL;

Fl_Event_Dispatch Fl::e_dispatch = 0;

unsigned char   Fl::options_[] = { 0, 0 };
unsigned char   Fl::options_read_ = 0;


Fl_Window *fl_xfocus = NULL;	// which window X thinks has focus
Fl_Window *fl_xmousewin;// which window X thinks has FL_ENTER
Fl_Window *Fl::grab_;	// most recent Fl::grab()
Fl_Window *Fl::modal_;	// topmost modal() window

#endif // FL_DOXYGEN

char const * const Fl::clipboard_plain_text = "text/plain";
char const * const Fl::clipboard_image = "image";


//
// Drivers
//

/** Returns a pointer to the unique Fl_Screen_Driver object of the platform */
Fl_Screen_Driver *Fl::screen_driver()
{
  static  Fl_Screen_Driver* screen_driver_ = Fl_Screen_Driver::newScreenDriver();
  return screen_driver_;
}

/** Returns a pointer to the unique Fl_System_Driver object of the platform */
Fl_System_Driver *Fl::system_driver()
{
  static  Fl_System_Driver* system_driver_ = Fl_System_Driver::newSystemDriver();
  return system_driver_;
}

//
// 'Fl::version()' - Return the API version number...
//

/**
  Returns the compiled-in value of the FL_VERSION constant. This
  is useful for checking the version of a shared library.

  \deprecated	Use int Fl::api_version() instead.
*/
double Fl::version() {
  return FL_VERSION;
}

/**
  Returns the compiled-in value of the FL_API_VERSION constant. This
  is useful for checking the version of a shared library.
*/
int Fl::api_version() {
  return FL_API_VERSION;
}

/**
  Returns the compiled-in value of the FL_ABI_VERSION constant. This
  is useful for checking the version of a shared library.
*/
int  Fl::abi_version() {
  return FL_ABI_VERSION;
}

/**
  Gets the default scrollbar size used by
  Fl_Browser_,
  Fl_Help_View,
  Fl_Scroll, and
  Fl_Text_Display widgets.
  \returns The default size for widget scrollbars, in pixels.
*/
int Fl::scrollbar_size() {
  return scrollbar_size_;
}

/**
  Sets the default scrollbar size that is used by the
  Fl_Browser_,
  Fl_Help_View,
  Fl_Scroll, and
  Fl_Text_Display widgets.
  \param[in] W The new default size for widget scrollbars, in pixels.
*/
void Fl::scrollbar_size(int W) {
  scrollbar_size_ = W;
}


/** Returns whether or not the mouse event is inside the given rectangle.

    Returns non-zero if the current Fl::event_x() and Fl::event_y()
    put it inside the given arbitrary bounding box.

    You should always call this rather than doing your own comparison
    so you are consistent about edge effects.

    To find out, whether the event is inside a child widget of the
    current window, you can use Fl::event_inside(const Fl_Widget *).

    \param[in] xx,yy,ww,hh	bounding box
    \return			non-zero, if mouse event is inside
*/
int Fl::event_inside(int xx,int yy,int ww,int hh) /*const*/ {
  int mx = e_x - xx;
  int my = e_y - yy;
  return (mx >= 0 && mx < ww && my >= 0 && my < hh);
}

/** Returns whether or not the mouse event is inside a given child widget.

    Returns non-zero if the current Fl::event_x() and Fl::event_y()
    put it inside the given child widget's bounding box.

    This method can only be used to check whether the mouse event is
    inside a \b child widget of the window that handles the event, and
    there must not be an intermediate subwindow (i.e. the widget must
    not be inside a subwindow of the current window). However, it is
    valid if the widget is inside a nested Fl_Group.

    You must not use it with the window itself as the \p o argument
    in a window's handle() method.

    \note The mentioned restrictions are necessary, because this method
    does not transform coordinates of child widgets, and thus the given
    widget \p o must be within the \e same window that is handling the
    current event. Otherwise the results are undefined.

    You should always call this rather than doing your own comparison
    so you are consistent about edge effects.

    \see Fl::event_inside(int, int, int, int)

    \param[in] o	child widget to be tested
    \return		non-zero, if mouse event is inside the widget
*/
int Fl::event_inside(const Fl_Widget *o) /*const*/ {
  int mx = e_x - o->x();
  int my = e_y - o->y();
  return (mx >= 0 && mx < o->w() && my >= 0 && my < o->h());
}

//
//
// timer support
//
//

void Fl::add_timeout(double time, Fl_Timeout_Handler cb, void *argp) {
  Fl::screen_driver()->add_timeout(time, cb, argp);
}

void Fl::repeat_timeout(double time, Fl_Timeout_Handler cb, void *argp) {
  Fl::screen_driver()->repeat_timeout(time, cb, argp);
}

/**
 Returns true if the timeout exists and has not been called yet.
 */
int Fl::has_timeout(Fl_Timeout_Handler cb, void *argp) {
  return Fl::screen_driver()->has_timeout(cb, argp);
}

/**
 Removes a timeout callback. It is harmless to remove a timeout
 callback that no longer exists.

 \note	This version removes all matching timeouts, not just the first one.
	This may change in the future.
 */
void Fl::remove_timeout(Fl_Timeout_Handler cb, void *argp) {
  Fl::screen_driver()->remove_timeout(cb, argp);
}



////////////////////////////////////////////////////////////////
// Checks are just stored in a list. They are called in the reverse
// order that they were added (this may change in the future).
// This is a bit messy because I want to allow checks to be added,
// removed, and have wait() called from inside them. To do this
// next_check points at the next unprocessed one for the outermost
// call to Fl::wait().

struct Check {
  void (*cb)(void*);
  void* arg;
  Check* next;
};
static Check *first_check, *next_check, *free_check;

/**
  FLTK will call this callback just before it flushes the display and
  waits for events.  This is different than an idle callback because it
  is only called once, then FLTK calls the system and tells it not to
  return until an event happens.

  This can be used by code that wants to monitor the
  application's state, such as to keep a display up to date. The
  advantage of using a check callback is that it is called only when no
  events are pending. If events are coming in quickly, whole blocks of
  them will be processed before this is called once. This can save
  significant time and avoid the application falling behind the events.

  Sample code:

  \code
  bool state_changed; // anything that changes the display turns this on

  void callback(void*) {
   if (!state_changed) return;
   state_changed = false;
   do_expensive_calculation();
   widget-&gt;redraw();
  }

  main() {
   Fl::add_check(callback);
   return Fl::run();
  }
  \endcode
*/
void Fl::add_check(Fl_Timeout_Handler cb, void *argp) {
  Check* t = free_check;
  if (t) free_check = t->next;
  else t = new Check;
  t->cb = cb;
  t->arg = argp;
  t->next = first_check;
  if (next_check == first_check) next_check = t;
  first_check = t;
}

/**
  Removes a check callback. It is harmless to remove a check
  callback that no longer exists.
*/
void Fl::remove_check(Fl_Timeout_Handler cb, void *argp) {
  for (Check** p = &first_check; *p;) {
    Check* t = *p;
    if (t->cb == cb && t->arg == argp) {
      if (next_check == t) next_check = t->next;
      *p = t->next;
      t->next = free_check;
      free_check = t;
    } else {
      p = &(t->next);
    }
  }
}

/**
  Returns 1 if the check exists and has not been called yet, 0 otherwise.
*/
int Fl::has_check(Fl_Timeout_Handler cb, void *argp) {
  for (Check** p = &first_check; *p;) {
    Check* t = *p;
    if (t->cb == cb && t->arg == argp) {
      return 1;
    } else {
      p = &(t->next);
    }
  }
  return 0;
}

void Fl::run_checks()
{
  // checks are a bit messy so that add/remove and wait may be called
  // from inside them without causing an infinite loop:
  if (next_check == first_check) {
    while (next_check) {
      Check* checkp = next_check;
      next_check = checkp->next;
      (checkp->cb)(checkp->arg);
    }
    next_check = first_check;
  }
}


////////////////////////////////////////////////////////////////
// Clipboard notifications

struct Clipboard_Notify {
  Fl_Clipboard_Notify_Handler handler;
  void *data;
  struct Clipboard_Notify *next;
};

static struct Clipboard_Notify *clip_notify_list = NULL;

extern void fl_clipboard_notify_change(); // in Fl_<platform>.cxx

void Fl::add_clipboard_notify(Fl_Clipboard_Notify_Handler h, void *data) {
  struct Clipboard_Notify *node;

  remove_clipboard_notify(h);

  node = new Clipboard_Notify;

  node->handler = h;
  node->data = data;
  node->next = clip_notify_list;

  clip_notify_list = node;

  fl_clipboard_notify_change();
}

void Fl::remove_clipboard_notify(Fl_Clipboard_Notify_Handler h) {
  struct Clipboard_Notify *node, **prev;

  node = clip_notify_list;
  prev = &clip_notify_list;
  while (node != NULL) {
    if (node->handler == h) {
      *prev = node->next;
      delete node;

      fl_clipboard_notify_change();

      return;
    }

    prev = &node->next;
    node = node->next;
  }
}

bool fl_clipboard_notify_empty(void) {
  return clip_notify_list == NULL;
}

void fl_trigger_clipboard_notify(int source) {
  struct Clipboard_Notify *node, *next;

  node = clip_notify_list;
  while (node != NULL) {
    next = node->next;
    node->handler(source, node->data);
    node = next;
  }
}

////////////////////////////////////////////////////////////////
// wait/run/check/ready:

void (*Fl::idle)(); // see Fl::add_idle.cxx for the add/remove functions

extern int fl_ready(); // in Fl_<platform>.cxx
extern int fl_wait(double time); // in Fl_<platform>.cxx

/**
  See int Fl::wait()
*/
double Fl::wait(double time_to_wait) {
  // delete all widgets that were listed during callbacks
  do_widget_deletion();
  return screen_driver()->wait(time_to_wait);
}

#define FOREVER 1e20

/**
  As long as any windows are displayed this calls Fl::wait()
  repeatedly.  When all the windows are closed it returns zero
  (supposedly it would return non-zero on any errors, but FLTK calls
  exit directly for these).  A normal program will end main()
  with return Fl::run();.
*/
int Fl::run() {
  while (Fl_X::first) wait(FOREVER);
  return 0;
}

/**
  Waits until "something happens" and then returns.  Call this
  repeatedly to "run" your program.  You can also check what happened
  each time after this returns, which is quite useful for managing
  program state.

  What this really does is call all idle callbacks, all elapsed
  timeouts, call Fl::flush() to get the screen to update, and
  then wait some time (zero if there are idle callbacks, the shortest of
  all pending timeouts, or infinity), for any events from the user or
  any Fl::add_fd() callbacks.  It then handles the events and
  calls the callbacks and then returns.

  The return value of Fl::wait() is non-zero if there are any
  visible windows - this may change in future versions of FLTK.

  Fl::wait(time) waits a maximum of \e time seconds.
  <i>It can return much sooner if something happens.</i>

  The return value is positive if an event or fd happens before the
  time elapsed.  It is zero if nothing happens (on Win32 this will only
  return zero if \e time is zero).  It is negative if an error
  occurs (this will happen on UNIX if a signal happens).
*/
int Fl::wait() {
  if (!Fl_X::first) return 0;
  wait(FOREVER);
  return Fl_X::first != 0; // return true if there is a window
}

/**
  Same as Fl::wait(0).  Calling this during a big calculation
  will keep the screen up to date and the interface responsive:

  \code
  while (!calculation_done()) {
  calculate();
  Fl::check();
  if (user_hit_abort_button()) break;
  }
  \endcode

  This returns non-zero if any windows are displayed, and 0 if no
  windows are displayed (this is likely to change in future versions of
  FLTK).
*/
int Fl::check() {
  wait(0.0);
  return Fl_X::first != 0; // return true if there is a window
}

/**
  This is similar to Fl::check() except this does \e not
  call Fl::flush() or any callbacks, which is useful if your
  program is in a state where such callbacks are illegal.  This returns
  true if Fl::check() would do anything (it will continue to
  return true until you call Fl::check() or Fl::wait()).

  \code
  while (!calculation_done()) {
    calculate();
    if (Fl::ready()) {
      do_expensive_cleanup();
      Fl::check();
      if (user_hit_abort_button()) break;
    }
  }
  \endcode
*/
int Fl::ready()
{
  return screen_driver()->ready();
}

////////////////////////////////////////////////////////////////
// Window list management:

#ifndef FL_DOXYGEN
Fl_X* Fl_X::first;
#endif

Fl_Window* fl_find(Window xid) {
  Fl_X *window;
  for (Fl_X **pp = &Fl_X::first; (window = *pp); pp = &window->next)
   if (window->xid == xid) {
      if (window != Fl_X::first && !Fl::modal()) {
        // make this window be first to speed up searches
        // this is not done if modal is true to avoid messing up modal stack
        *pp = window->next;
        window->next = Fl_X::first;
        Fl_X::first = window;
      }
      return window->w;
    }
  return 0;
}

/**
  Returns the first top-level window in the list of shown() windows.  If
  a modal() window is shown this is the top-most modal window, otherwise
  it is the most recent window to get an event.
*/
Fl_Window* Fl::first_window() {
  Fl_X* i = Fl_X::first;
  return i ? i->w : 0;
}

/**
  Returns the next top-level window in the list of shown() windows.
  You can use this call to iterate through all the windows that are shown().
  \param[in] window	must be shown and not NULL
*/
Fl_Window* Fl::next_window(const Fl_Window* window) {
  Fl_X* i = Fl_X::i(window)->next;
  return i ? i->w : 0;
}

/**
 Sets the window that is returned by first_window().
 The window is removed from wherever it is in the
 list and inserted at the top.  This is not done if Fl::modal()
 is on or if the window is not shown(). Because the first window
 is used to set the "parent" of modal windows, this is often
 useful.
 */
void Fl::first_window(Fl_Window* window) {
  if (!window || !window->shown()) return;
  fl_find( Fl_X::i(window)->xid );
}

/**
  Redraws all widgets.
*/
void Fl::redraw() {
  for (Fl_X* i = Fl_X::first; i; i = i->next) i->w->redraw();
}

/**
  Causes all the windows that need it to be redrawn and graphics forced
  out through the pipes.

  This is what wait() does before looking for events.

  Note: in multi-threaded applications you should only call Fl::flush()
  from the main thread. If a child thread needs to trigger a redraw event,
  it should instead call Fl::awake() to get the main thread to process the
  event queue.
*/
void Fl::flush() {
  if (damage()) {
    damage_ = 0;
    for (Fl_X* i = Fl_X::first; i; i = i->next) {
      if (i->wait_for_expose) {damage_ = 1; continue;}
      Fl_Window* wi = i->w;
      if (!wi->visible_r()) continue;
      if (wi->damage()) {i->flush(); wi->clear_damage();}
      // destroy damage regions for windows that don't use them:
      if (i->region) {Fl_Graphics_Driver::XDestroyRegion(i->region); i->region = 0;}
    }
  }
  screen_driver()->flush();
}


////////////////////////////////////////////////////////////////
// Event handlers:


struct handler_link {
  int (*handle)(int);
  handler_link *next;
};


static handler_link *handlers = 0;


/**
  Install a function to parse unrecognized events.  If FLTK cannot
  figure out what to do with an event, it calls each of these functions
  (most recent first) until one of them returns non-zero.  If none of
  them returns non-zero then the event is ignored.  Events that cause
  this to be called are:

  - \ref FL_SHORTCUT events that are not recognized by any widget.
    This lets you provide global shortcut keys.
  - \ref FL_SCREEN_CONFIGURATION_CHANGED events.
    Under X11, this event requires the libXrandr.so shared library to be
    loadable at run-time and the X server to implement the RandR extension.
  - \ref FL_FULLSCREEN events sent to a window that enters or leaves
    fullscreen mode.
  - System events that FLTK does not recognize.  See fl_xevent.
  - \e Some other events when the widget FLTK selected returns
    zero from its handle() method.  Exactly which ones may change
    in future versions, however.

 \see Fl::remove_handler(Fl_Event_Handler)
 \see Fl::event_dispatch(Fl_Event_Dispatch d)
 \see Fl::handle(int, Fl_Window*)
*/
void Fl::add_handler(Fl_Event_Handler ha) {
  handler_link *l = new handler_link;
  l->handle = ha;
  l->next = handlers;
  handlers = l;
}


/**
 Removes a previously added event handler.
 \see Fl::handle(int, Fl_Window*)
*/
void Fl::remove_handler(Fl_Event_Handler ha) {
  handler_link *l, *p;

  // Search for the handler in the list...
  for (l = handlers, p = 0; l && l->handle != ha; p = l, l = l->next) {/*empty*/}

  if (l) {
    // Found it, so remove it from the list...
    if (p) p->next = l->next;
    else handlers = l->next;

    // And free the record...
    delete l;
  }
}

int (*fl_local_grab)(int); // used by fl_dnd.cxx

static int send_handlers(int e) {
  for (const handler_link *hl = handlers; hl; hl = hl->next)
    if (hl->handle(e)) return 1;
  return 0;
}


////////////////////////////////////////////////////////////////
// System event handlers:


struct system_handler_link {
  Fl_System_Handler handle;
  void *data;
  system_handler_link *next;
};


static system_handler_link *sys_handlers = 0;


/**
 \brief Install a function to intercept system events.

 FLTK calls each of these functions as soon as a new system event is
 received. The processing will stop at the first function to return
 non-zero. If all functions return zero then the event is passed on
 for normal handling by FLTK.

 Each function will be called with a pointer to the system event as
 the first argument and \p data as the second argument. The system
 event pointer will always be void *, but will point to different
 objects depending on the platform:
   - X11: XEvent
   - Windows: MSG
   - OS X: NSEvent

 \param ha The event handler function to register
 \param data User data to include on each call

 \see Fl::remove_system_handler(Fl_System_Handler)
*/
void Fl::add_system_handler(Fl_System_Handler ha, void *data) {
  system_handler_link *l = new system_handler_link;
  l->handle = ha;
  l->data = data;
  l->next = sys_handlers;
  sys_handlers = l;
}


/**
 Removes a previously added system event handler.

 \param ha The event handler function to remove

 \see Fl::add_system_handler(Fl_System_Handler)
*/
void Fl::remove_system_handler(Fl_System_Handler ha) {
  system_handler_link *l, *p;

  // Search for the handler in the list...
  for (l = sys_handlers, p = 0; l && l->handle != ha; p = l, l = l->next);

  if (l) {
    // Found it, so remove it from the list...
    if (p) p->next = l->next;
    else sys_handlers = l->next;

    // And free the record...
    delete l;
  }
}

int fl_send_system_handlers(void *e) {
  for (const system_handler_link *hl = sys_handlers; hl; hl = hl->next) {
    if (hl->handle(e, hl->data))
      return 1;
  }
  return 0;
}


////////////////////////////////////////////////////////////////

Fl_Widget* fl_oldfocus; // kludge for Fl_Group...

/**
    Sets the widget that will receive FL_KEYBOARD events.

    If you change Fl::focus(), the previous widget and all
    parents (that don't contain the new widget) are sent FL_UNFOCUS
    events.  Changing the focus does \e not send FL_FOCUS to
    this or any widget, because sending FL_FOCUS is supposed to
    \e test if the widget wants the focus (by it returning non-zero from
    handle()).

    \see Fl_Widget::take_focus()
*/
void Fl::focus(Fl_Widget *o) {
  if (o && !o->visible_focus()) return;
  if (grab()) return; // don't do anything while grab is on
  Fl_Widget *p = focus_;
  if (o != p) {
    Fl::compose_reset();
    focus_ = o;
    // make sure that fl_xfocus is set to the top level window
    // of this widget, or fl_fix_focus will clear our focus again
    if (o) {
      Fl_Window *win = 0, *w1 = o->as_window();
      if (!w1) w1 = o->window();
      while (w1) { win=w1; w1=win->window(); }
      if (win) {
        if (fl_xfocus != win) {
          win->driver()->take_focus();
          fl_xfocus = win;
        }
      }
    }
    // take focus from the old focused window
    fl_oldfocus = 0;
    int old_event = e_number;
    e_number = FL_UNFOCUS;
    for (; p; p = p->parent()) {
      p->handle(FL_UNFOCUS);
      fl_oldfocus = p;
    }
    e_number = old_event;
  }
}

static char dnd_flag = 0; // make 'belowmouse' send DND_LEAVE instead of LEAVE

/**
    Sets the widget that is below the mouse.  This is for
    highlighting buttons.  It is not used to send FL_PUSH or
    FL_MOVE directly, for several obscure reasons, but those events
    typically go to this widget.  This is also the first widget tried for
    FL_SHORTCUT events.

    If you change the belowmouse widget, the previous one and all
    parents (that don't contain the new widget) are sent FL_LEAVE
    events.  Changing this does \e not send FL_ENTER to this
    or any widget, because sending FL_ENTER is supposed to \e test
    if the widget wants the mouse (by it returning non-zero from
    handle()).
*/
void Fl::belowmouse(Fl_Widget *o) {
  if (grab()) return; // don't do anything while grab is on
  Fl_Widget *p = belowmouse_;
  if (o != p) {
    belowmouse_ = o;
    int old_event = e_number;
    e_number = dnd_flag ? FL_DND_LEAVE : FL_LEAVE;
    for (; p && !p->contains(o); p = p->parent()) {
      p->handle(e_number);
    }
    e_number = old_event;
  }
}

/**
    Sets the widget that is being pushed. FL_DRAG or
    FL_RELEASE (and any more FL_PUSH) events will be sent to
    this widget.

    If you change the pushed widget, the previous one and all parents
    (that don't contain the new widget) are sent FL_RELEASE
    events.  Changing this does \e not send FL_PUSH to this
    or any widget, because sending FL_PUSH is supposed to \e test
    if the widget wants the mouse (by it returning non-zero from
    handle()).
*/
 void Fl::pushed(Fl_Widget *o) {
  pushed_ = o;
}

static void nothing(Fl_Widget *) {}
void (*Fl_Tooltip::enter)(Fl_Widget *) = nothing;
void (*Fl_Tooltip::exit)(Fl_Widget *) = nothing;

// Update modal(), focus() and other state according to system state,
// and send FL_ENTER, FL_LEAVE, FL_FOCUS, and/or FL_UNFOCUS events.
// This is the only function that produces these events in response
// to system activity.
// This is called whenever a window is added or hidden, and whenever
// X says the focus or mouse window have changed.

void fl_fix_focus() {
#ifdef DEBUG
  puts("fl_fix_focus();");
#endif // DEBUG

  if (Fl::grab()) return; // don't do anything while grab is on.

  // set focus based on Fl::modal() and fl_xfocus
  Fl_Widget* w = fl_xfocus;
  if (w) {
    int saved = Fl::e_keysym;
    if (Fl::e_keysym < (FL_Button + FL_LEFT_MOUSE) ||
        Fl::e_keysym > (FL_Button + FL_RIGHT_MOUSE))
      Fl::e_keysym = 0; // make sure widgets don't think a keystroke moved focus
    while (w->parent()) w = w->parent();
    if (Fl::modal()) w = Fl::modal();
    if (!w->contains(Fl::focus()))
      if (!w->take_focus()) Fl::focus(w);
    Fl::e_keysym = saved;
  } else
    Fl::focus(0);

// MRS: Originally we checked the button state, but a user reported that it
//      broke click-to-focus in FLWM?!?
//  if (!(Fl::event_state() & 0x7f00000 /*FL_BUTTONS*/)) {
  if (!Fl::pushed()) {
    // set belowmouse based on Fl::modal() and fl_xmousewin:
    w = fl_xmousewin;
    if (w) {
      if (Fl::modal()) w = Fl::modal();
      if (!w->contains(Fl::belowmouse())) {
        int old_event = Fl::e_number;
	w->handle(Fl::e_number = FL_ENTER);
	Fl::e_number = old_event;
	if (!w->contains(Fl::belowmouse())) Fl::belowmouse(w);
      } else {
	// send a FL_MOVE event so the enter/leave state is up to date
	Fl::e_x = Fl::e_x_root-fl_xmousewin->x();
	Fl::e_y = Fl::e_y_root-fl_xmousewin->y();
        int old_event = Fl::e_number;
	w->handle(Fl::e_number = FL_MOVE);
	Fl::e_number = old_event;
      }
    } else {
      Fl::belowmouse(0);
      Fl_Tooltip::enter(0);
    }
  }
}


// This function is called by ~Fl_Widget() and by Fl_Widget::deactivate()
// and by Fl_Widget::hide().  It indicates that the widget does not want
// to receive any more events, and also removes all global variables that
// point at the widget.
// I changed this from the 1.0.1 behavior, the older version could send
// FL_LEAVE or FL_UNFOCUS events to the widget.  This appears to not be
// desirable behavior and caused flwm to crash.

void fl_throw_focus(Fl_Widget *o) {
#ifdef DEBUG
  printf("fl_throw_focus(o=%p)\n", o);
#endif // DEBUG

  if (o->contains(Fl::pushed())) Fl::pushed_ = 0;
  if (o->contains(fl_selection_requestor)) fl_selection_requestor = 0;
  if (o->contains(Fl::belowmouse())) Fl::belowmouse_ = 0;
  if (o->contains(Fl::focus())) Fl::focus_ = 0;
  if (o == fl_xfocus) fl_xfocus = 0;
  if (o == Fl_Tooltip::current()) Fl_Tooltip::current(0);
  if (o == fl_xmousewin) fl_xmousewin = 0;
  Fl_Tooltip::exit(o);
  fl_fix_focus();
}

////////////////////////////////////////////////////////////////

// Find the first active_r() widget, starting at the widget wi and
// walking up the widget hierarchy to the top level window.
//
// In other words: find_active() returns an active group that contains
// the inactive widget and all inactive parent groups.
//
// This is used to send FL_SHORTCUT events to the Fl::belowmouse() widget
// in case the target widget itself is inactive_r(). In this case the event
// is sent to the first active_r() parent.
//
// This prevents sending events to inactive widgets that might get the
// input focus otherwise. The search is fast and light and avoids calling
// inactive_r() multiple times.
// See STR #3216.
//
// Returns: first active_r() widget "above" the widget wi or NULL if
// no widget is active. May return the top level window.

static Fl_Widget *find_active(Fl_Widget *wi) {
  Fl_Widget *found = 0;
  for (; wi; wi = wi->parent()) {
    if (wi->active()) {
      if (!found) found = wi;
    }
    else found = 0;
  }
  return found;
}

////////////////////////////////////////////////////////////////

// Call to->handle(), but first replace the mouse x/y with the correct
// values to account for nested windows. 'window' is the outermost
// window the event was posted to by the system:
static int send_event(int event, Fl_Widget* to, Fl_Window* window) {
  int dx, dy;
  int old_event = Fl::e_number;
  if (window) {
    dx = window->x();
    dy = window->y();
  } else {
    dx = dy = 0;
  }
  for (const Fl_Widget* w = to; w; w = w->parent())
    if (w->type()>=FL_WINDOW) {dx -= w->x(); dy -= w->y();}
  int save_x = Fl::e_x; Fl::e_x += dx;
  int save_y = Fl::e_y; Fl::e_y += dy;
  int ret = to->handle(Fl::e_number = event);
  Fl::e_number = old_event;
  Fl::e_y = save_y;
  Fl::e_x = save_x;
  return ret;
}


/**
 \brief Set a new event dispatch function.

 The event dispatch function is called after native events are converted to
 FLTK events, but before they are handled by FLTK. If the dispatch function
 Fl_Event_Dispatch \p d is set, it is up to the dispatch function to call
 Fl::handle_(int, Fl_Window*) or to ignore the event.

 The dispatch function itself must return 0 if it ignored the event,
 or non-zero if it used the event. If you call Fl::handle_(), then
 this will return the correct value.

 The event dispatch can be used to handle exceptions in FLTK events and
 callbacks before they reach the native event handler:

 \code
 int myHandler(int e, Fl_Window *w) {
   try {
     return Fl::handle_(e, w);
   } catch () {
     ...
   }
 }

 main() {
   Fl::event_dispatch(myHandler);
   ...
   Fl::run();
 }
 \endcode

 \param d new dispatch function, or NULL
 \see Fl::add_handler(Fl_Event_Handler)
 \see Fl::handle(int, Fl_Window*)
 \see Fl::handle_(int, Fl_Window*)
 */
void Fl::event_dispatch(Fl_Event_Dispatch d)
{
  e_dispatch = d;
}


/**
 \brief Return the current event dispatch function.
 */
Fl_Event_Dispatch Fl::event_dispatch()
{
  return e_dispatch;
}


/**
 \brief Handle events from the window system.

 This is called from the native event dispatch after native events have been
 converted to FLTK notation. This function calls Fl::handle_(int, Fl_Window*)
 unless the user sets a dispatch function. If a user dispatch function is set,
 the user must make sure that Fl::handle_() is called, or the event will be
 ignored.

 \param e the event type (Fl::event_number() is not yet set)
 \param window the window that caused this event
 \return 0 if the event was not handled

 \see Fl::add_handler(Fl_Event_Handler)
 \see Fl::event_dispatch(Fl_Event_Dispatch)
 */
int Fl::handle(int e, Fl_Window* window)
{
  if (e_dispatch) {
    return e_dispatch(e, window);
  } else {
    return handle_(e, window);
  }
}


/**
 \brief Handle events from the window system.

 This function is called from the native event dispatch, unless the user sets
 another dispatch function. In that case, the user dispatch function must
 decide when to call Fl::handle_(int, Fl_Window*)

 \param e the event type (Fl::event_number() is not yet set)
 \param window the window that caused this event
 \return 0 if the event was not handled

 \see Fl::event_dispatch(Fl_Event_Dispatch)
 */
int Fl::handle_(int e, Fl_Window* window)
{
  e_number = e;
  if (fl_local_grab) return fl_local_grab(e);

  Fl_Widget* wi = window;

  switch (e) {

  case FL_CLOSE:
    if ( grab() || (modal() && window != modal()) ) return 0;
    wi->do_callback();
    return 1;

  case FL_SHOW:
    wi->Fl_Widget::show(); // this calls Fl_Widget::show(), not Fl_Window::show()
    return 1;

  case FL_HIDE:
    wi->Fl_Widget::hide(); // this calls Fl_Widget::hide(), not Fl_Window::hide()
    return 1;

  case FL_PUSH:
#ifdef DEBUG
    printf("Fl::handle(e=%d, window=%p);\n", e, window);
#endif // DEBUG

    if (grab()) wi = grab();
    else if (modal() && wi != modal()) return 0;
    pushed_ = wi;
    Fl_Tooltip::current(wi);
    if (send_event(e, wi, window)) return 1;
    // raise windows that are clicked on:
    window->show();
    return 1;

  case FL_DND_ENTER:
  case FL_DND_DRAG:
    dnd_flag = 1;
    break;

  case FL_DND_LEAVE:
    dnd_flag = 1;
    belowmouse(0);
    dnd_flag = 0;
    return 1;

  case FL_DND_RELEASE:
    wi = belowmouse();
    break;

  case FL_MOVE:
  case FL_DRAG:
    fl_xmousewin = window; // this should already be set, but just in case.
    if (pushed()) {
      wi = pushed();
      if (grab()) wi = grab();
      e_number = e = FL_DRAG;
      break;
    }
    if (modal() && wi != modal()) wi = 0;
    if (grab()) wi = grab();
    { int ret;
      Fl_Widget* pbm = belowmouse();
      ret = (wi && send_event(e, wi, window));
      if (pbm != belowmouse()) {
#ifdef DEBUG
        printf("Fl::handle(e=%d, window=%p);\n", e, window);
#endif // DEBUG
        Fl_Tooltip::enter(belowmouse());
      }
      return ret;
    }

  case FL_RELEASE: {
//    printf("FL_RELEASE: window=%p, pushed() = %p, grab() = %p, modal() = %p\n",
//           window, pushed(), grab(), modal());

    if (grab()) {
      wi = grab();
      pushed_ = 0; // must be zero before callback is done!
    } else if (pushed()) {
      wi = pushed();
      pushed_ = 0; // must be zero before callback is done!
    } else if (modal() && wi != modal()) return 0;
    int r = send_event(e, wi, window);
    fl_fix_focus();
    return r;}

  case FL_UNFOCUS:
    window = 0;
  case FL_FOCUS:
    fl_xfocus = window;
    fl_fix_focus();
    return 1;

  case FL_KEYUP:
    // Send the key-up to the current focus widget. This is not
    // always the same widget that received the corresponding
    // FL_KEYBOARD event because focus may have changed.
    // Sending the KEYUP to the right KEYDOWN is possible, but
    // would require that we track the KEYDOWN for every possible
    // key stroke (users may hold down multiple keys!) and then
    // make sure that the widget still exists before sending
    // a KEYUP there. I believe that the current solution is
    // "close enough".
    for (wi = grab() ? grab() : focus(); wi; wi = wi->parent())
      if (send_event(FL_KEYUP, wi, window)) return 1;
    return 0;

  case FL_KEYBOARD:
#ifdef DEBUG
    printf("Fl::handle(e=%d, window=%p);\n", e, window);
#endif // DEBUG

    Fl_Tooltip::enter((Fl_Widget*)0);

    fl_xfocus = window; // this should not happen!  But maybe it does:

    // Try it as keystroke, sending it to focus and all parents:
    for (wi = grab() ? grab() : focus(); wi; wi = wi->parent())
      if (send_event(FL_KEYBOARD, wi, window)) return 1;

    // recursive call to try shortcut:
    if (handle(FL_SHORTCUT, window)) return 1;

    // and then try a shortcut with the case of the text swapped, by
    // changing the text and falling through to FL_SHORTCUT case:
    {unsigned char* c = (unsigned char*)event_text(); // cast away const
    if (!isalpha(*c)) return 0;
    *c = isupper(*c) ? tolower(*c) : toupper(*c);}
    e_number = e = FL_SHORTCUT;

  case FL_SHORTCUT:
    if (grab()) {wi = grab(); break;} // send it to grab window

    // Try it as shortcut, sending to mouse widget and all parents:
    wi = find_active(belowmouse()); // STR #3216
    if (!wi) {
      wi = modal();
      if (!wi) wi = window;
    } else if (wi->window() != first_window()) {
      if (send_event(FL_SHORTCUT, first_window(), first_window())) return 1;
    }

    for (; wi; wi = wi->parent()) {
      if (send_event(FL_SHORTCUT, wi, wi->window())) return 1;
    }

    // try using add_handle() functions:
    if (send_handlers(FL_SHORTCUT)) return 1;

    // make Escape key close windows:
    if (event_key()==FL_Escape) {
      wi = modal(); if (!wi) wi = window;
      wi->do_callback();
      return 1;
    }

    return 0;

  case FL_ENTER:
#ifdef DEBUG
    printf("Fl::handle(e=%d, window=%p);\n", e, window);
#endif // DEBUG

    fl_xmousewin = window;
    fl_fix_focus();
    Fl_Tooltip::enter(belowmouse());
    return 1;

  case FL_LEAVE:
#ifdef DEBUG
    printf("Fl::handle(e=%d, window=%p);\n", e, window);
#endif // DEBUG

    if (!pushed_) {
      belowmouse(0);
      Fl_Tooltip::enter(0);
    }
    if (window == fl_xmousewin) {fl_xmousewin = 0; fl_fix_focus();}
    return 1;

  case FL_MOUSEWHEEL:
    fl_xfocus = window; // this should not happen!  But maybe it does:

    // Try sending it to the "grab" first
    if (grab() && grab()!=modal() && grab()!=window) {
      if (send_event(FL_MOUSEWHEEL, grab(), window)) return 1;
    }
    // Now try sending it to the "modal" window
    if (modal()) {
      send_event(FL_MOUSEWHEEL, modal(), window);
      return 1;
    }
    // Finally try sending it to the window, the event occured in
    if (send_event(FL_MOUSEWHEEL, window, window)) return 1;
  default:
    break;
  }
  if (wi && send_event(e, wi, window)) {
    dnd_flag = 0;
    return 1;
  }
  dnd_flag = 0;
  return send_handlers(e);
}


////////////////////////////////////////////////////////////////
// Back compatibility cut & paste functions for fltk 1.1 only:

/** Back-compatibility only: The single-argument call can be used to
    move the selection to another widget or to set the owner to
    NULL, without changing the actual text of the
    selection. FL_SELECTIONCLEAR is sent to the previous
    selection owner, if any.

    <i>Copying the buffer every time the selection is changed is
    obviously wasteful, especially for large selections.  An interface will
    probably be added in a future version to allow the selection to be made
    by a callback function.  The current interface will be emulated on top
    of this.</i>
*/
void Fl::selection_owner(Fl_Widget *owner) {selection_owner_ = owner;}

/**
  Changes the current selection.  The block of text is
  copied to an internal buffer by FLTK (be careful if doing this in
  response to an FL_PASTE as this \e may be the same buffer
  returned by event_text()).  The selection_owner()
  widget is set to the passed owner.
*/
void Fl::selection(Fl_Widget &owner, const char* text, int len) {
  selection_owner_ = &owner;
  Fl::copy(text, len, 0);
}

/** Backward compatibility only.
  This calls Fl::paste(receiver, 0);
  \see Fl::paste(Fl_Widget &receiver, int clipboard, const char* type)
*/
void Fl::paste(Fl_Widget &receiver) {
  Fl::paste(receiver, 0);
}
////////////////////////////////////////////////////////////////

void Fl_Widget::redraw() {
  damage(FL_DAMAGE_ALL);
}

void Fl_Widget::redraw_label() {
  if (window()) {
    if (box() == FL_NO_BOX) {
      // Widgets with the FL_NO_BOX boxtype need a parent to
      // redraw, since it is responsible for redrawing the
      // background...
      int X = x() > 0 ? x() - 1 : 0;
      int Y = y() > 0 ? y() - 1 : 0;
      window()->damage(FL_DAMAGE_ALL, X, Y, w() + 2, h() + 2);
    }

    if (align() && !(align() & FL_ALIGN_INSIDE) && window()->shown()) {
      // If the label is not inside the widget, compute the location of
      // the label and redraw the window within that bounding box...
      int W = 0, H = 0;
      label_.measure(W, H);
      W += 5; // Add a little to the size of the label to cover overflow
      H += 5;

      // FIXME:
      // This assumes that measure() returns the correct outline, which it does
      // not in all possible cases of alignment combinedwith image and symbols.
      switch (align() & 0x0f) {
        case FL_ALIGN_TOP_LEFT:
          window()->damage(FL_DAMAGE_EXPOSE, x(), y()-H, W, H); break;
        case FL_ALIGN_TOP:
          window()->damage(FL_DAMAGE_EXPOSE, x()+(w()-W)/2, y()-H, W, H); break;
        case FL_ALIGN_TOP_RIGHT:
          window()->damage(FL_DAMAGE_EXPOSE, x()+w()-W, y()-H, W, H); break;
        case FL_ALIGN_LEFT_TOP:
          window()->damage(FL_DAMAGE_EXPOSE, x()-W, y(), W, H); break;
        case FL_ALIGN_RIGHT_TOP:
          window()->damage(FL_DAMAGE_EXPOSE, x()+w(), y(), W, H); break;
        case FL_ALIGN_LEFT:
          window()->damage(FL_DAMAGE_EXPOSE, x()-W, y()+(h()-H)/2, W, H); break;
        case FL_ALIGN_RIGHT:
          window()->damage(FL_DAMAGE_EXPOSE, x()+w(), y()+(h()-H)/2, W, H); break;
        case FL_ALIGN_LEFT_BOTTOM:
          window()->damage(FL_DAMAGE_EXPOSE, x()-W, y()+h()-H, W, H); break;
        case FL_ALIGN_RIGHT_BOTTOM:
          window()->damage(FL_DAMAGE_EXPOSE, x()+w(), y()+h()-H, W, H); break;
        case FL_ALIGN_BOTTOM_LEFT:
          window()->damage(FL_DAMAGE_EXPOSE, x(), y()+h(), W, H); break;
        case FL_ALIGN_BOTTOM:
          window()->damage(FL_DAMAGE_EXPOSE, x()+(w()-W)/2, y()+h(), W, H); break;
        case FL_ALIGN_BOTTOM_RIGHT:
          window()->damage(FL_DAMAGE_EXPOSE, x()+w()-W, y()+h(), W, H); break;
        default:
          window()->damage(FL_DAMAGE_ALL); break;
      }
    } else {
      // The label is inside the widget, so just redraw the widget itself...
      damage(FL_DAMAGE_ALL);
    }
  }
}

void Fl_Widget::damage(uchar fl) {
  if (type() < FL_WINDOW) {
    // damage only the rectangle covered by a child widget:
    damage(fl, x(), y(), w(), h());
  } else {
    // damage entire window by deleting the region:
    Fl_X* i = Fl_X::i((Fl_Window*)this);
    if (!i) return; // window not mapped, so ignore it
    if (i->region) {Fl_Graphics_Driver::XDestroyRegion(i->region); i->region = 0;}
    damage_ |= fl;
    Fl::damage(FL_DAMAGE_CHILD);
  }
}

void Fl_Widget::damage(uchar fl, int X, int Y, int W, int H) {
  Fl_Widget* wi = this;
  // mark all parent widgets between this and window with FL_DAMAGE_CHILD:
  while (wi->type() < FL_WINDOW) {
    wi->damage_ |= fl;
    wi = wi->parent();
    if (!wi) return;
    fl = FL_DAMAGE_CHILD;
  }
  Fl_X* i = Fl_X::i((Fl_Window*)wi);
  if (!i) return; // window not mapped, so ignore it

  // clip the damage to the window and quit if none:
  if (X < 0) {W += X; X = 0;}
  if (Y < 0) {H += Y; Y = 0;}
  if (W > wi->w()-X) W = wi->w()-X;
  if (H > wi->h()-Y) H = wi->h()-Y;
  if (W <= 0 || H <= 0) return;

  if (!X && !Y && W==wi->w() && H==wi->h()) {
    // if damage covers entire window delete region:
    wi->damage(fl);
    return;
  }

  if (wi->damage()) {
    // if we already have damage we must merge with existing region:
    if (i->region) {
      Fl_Graphics_Driver::add_rectangle_to_region(i->region, X, Y, W, H);
    }
    wi->damage_ |= fl;
  } else {
    // create a new region:
    if (i->region) Fl_Graphics_Driver::XDestroyRegion(i->region);
    i->region = Fl_Graphics_Driver::XRectangleRegion(X,Y,W,H);
    wi->damage_ = fl;
  }
  Fl::damage(FL_DAMAGE_CHILD);
}


//
// The following methods allow callbacks to schedule the deletion of
// widgets at "safe" times.
//


static int		num_dwidgets = 0, alloc_dwidgets = 0;
static Fl_Widget	**dwidgets = 0;


/**
  Schedules a widget for deletion at the next call to the event loop.
  Use this method to delete a widget inside a callback function.

  To avoid early deletion of widgets, this function should be called
  toward the end of a callback and only after any call to the event
  loop (Fl::wait(), Fl::flush(), Fl::check(), fl_ask(), etc.).

  When deleting groups or windows, you must only delete the group or
  window widget and not the individual child widgets.

  \since FLTK 1.3.4 the widget will be hidden immediately, but the actual
  destruction will be delayed until the event loop is finished. Up to
  FLTK 1.3.3 windows wouldn't be hidden before the event loop was done,
  hence you had to hide() a window in your window close callback if
  you called Fl::delete_widget() to destroy (and hide) the window.

  \since FLTK 1.3.0 it is not necessary to remove widgets from their parent
  groups or windows before calling this, because it will be done in the
  widget's destructor, but it is not a failure to do this nevertheless.

  \note In FLTK 1.1 you \b must remove widgets from their parent group
  (or window) before deleting them.

  \see Fl_Widget::~Fl_Widget()
*/
void Fl::delete_widget(Fl_Widget *wi) {
  if (!wi) return;

  // if the widget is shown(), hide() it (FLTK 1.3.4)
  if (wi->visible_r()) wi->hide();
  Fl_Window *win = wi->as_window();
  if (win && win->shown()) win->hide(); // case of iconified window

  // don't add the same widget twice to the widget delete list
  for (int i = 0; i < num_dwidgets; i++) {
    if (dwidgets[i]==wi) return;
  }

  if (num_dwidgets >= alloc_dwidgets) {
    Fl_Widget	**temp;

    temp = new Fl_Widget *[alloc_dwidgets + 10];
    if (alloc_dwidgets) {
      memcpy(temp, dwidgets, alloc_dwidgets * sizeof(Fl_Widget *));
      delete[] dwidgets;
    }

    dwidgets = temp;
    alloc_dwidgets += 10;
  }

  dwidgets[num_dwidgets] = wi;
  num_dwidgets ++;
}


/**
    Deletes widgets previously scheduled for deletion.

    This is for internal use only. You should never call this directly.

    Fl::do_widget_deletion() is called from the FLTK event loop or whenever
    you call Fl::wait(). The previously scheduled widgets are deleted in the
    same order they were scheduled by calling Fl::delete_widget().

    \see Fl::delete_widget(Fl_Widget *wi)
*/
void Fl::do_widget_deletion() {
  if (!num_dwidgets) return;

  for (int i = 0; i < num_dwidgets; i ++)
    delete dwidgets[i];

  num_dwidgets = 0;
}


static Fl_Widget ***widget_watch = 0;
static int num_widget_watch = 0;
static int max_widget_watch = 0;


/**
  Adds a widget pointer to the widget watch list.

  \note Internal use only, please use class Fl_Widget_Tracker instead.

  This can be used, if it is possible that a widget might be deleted during
  a callback or similar function. The widget pointer must be added to the
  watch list before calling the callback. After the callback the widget
  pointer can be queried, if it is NULL. \e If it is NULL, then the widget has been
  deleted during the callback and must not be accessed anymore. If the widget
  pointer is \e not NULL, then the widget has not been deleted and can be accessed
  safely.

  After accessing the widget, the widget pointer must be released from the
  watch list by calling Fl::release_widget_pointer().

  Example for a button that is clicked (from its handle() method):
  \code
    Fl_Widget *wp = this;		// save 'this' in a pointer variable
    Fl::watch_widget_pointer(wp);	// add the pointer to the watch list
    set_changed();			// set the changed flag
    do_callback();			// call the callback
    if (!wp) {				// the widget has been deleted

      // DO NOT ACCESS THE DELETED WIDGET !

    } else {				// the widget still exists
      clear_changed();			// reset the changed flag
    }

    Fl::release_widget_pointer(wp);	// remove the pointer from the watch list
   \endcode

   This works, because all widgets call Fl::clear_widget_pointer() in their
   destructors.

   \see Fl::release_widget_pointer()
   \see Fl::clear_widget_pointer()

   An easier and more convenient method to control widget deletion during
   callbacks is to use the class Fl_Widget_Tracker with a local (automatic)
   variable.

   \see class Fl_Widget_Tracker
*/
void Fl::watch_widget_pointer(Fl_Widget *&w)
{
  Fl_Widget **wp = &w;
  int i;
  for (i=0; i<num_widget_watch; ++i) {
    if (widget_watch[i]==wp) return;
  }
  if (num_widget_watch==max_widget_watch) {
    max_widget_watch += 8;
    widget_watch = (Fl_Widget***)realloc(widget_watch, sizeof(Fl_Widget**)*max_widget_watch);
  }
  widget_watch[num_widget_watch++] = wp;
#ifdef DEBUG_WATCH
  printf ("\nwatch_widget_pointer:   (%d/%d) %8p => %8p\n",
    num_widget_watch,num_widget_watch,wp,*wp);
  fflush(stdout);
#endif // DEBUG_WATCH
}


/**
  Releases a widget pointer from the watch list.

  This is used to remove a widget pointer that has been added to the watch list
  with Fl::watch_widget_pointer(), when it is not needed anymore.

  \note Internal use only, please use class Fl_Widget_Tracker instead.

  \see Fl::watch_widget_pointer()
*/
void Fl::release_widget_pointer(Fl_Widget *&w)
{
  Fl_Widget **wp = &w;
  int i,j=0;
  for (i=0; i<num_widget_watch; ++i) {
    if (widget_watch[i]!=wp) {
      if (j<i) widget_watch[j] = widget_watch[i]; // fill gap
      j++;
    }
#ifdef DEBUG_WATCH
    else { // found widget pointer
      printf ("release_widget_pointer: (%d/%d) %8p => %8p\n",
	i+1,num_widget_watch,wp,*wp);
    }
#endif //DEBUG_WATCH
  }
  num_widget_watch = j;
#ifdef DEBUG_WATCH
  printf ("                        num_widget_watch = %d\n\n",num_widget_watch);
  fflush(stdout);
#endif // DEBUG_WATCH
  return;
}


/**
  Clears a widget pointer \e in the watch list.

  This is called when a widget is destroyed (by its destructor). You should never
  call this directly.

  \note Internal use only !

  This method searches the widget watch list for pointers to the widget and
  clears each pointer that points to it. Widget pointers can be added to the
  widget watch list by calling Fl::watch_widget_pointer() or by using the
  helper class Fl_Widget_Tracker (recommended).

  \see Fl::watch_widget_pointer()
  \see class Fl_Widget_Tracker
*/
void Fl::clear_widget_pointer(Fl_Widget const *w)
{
  if (w==0L) return;
  int i;
  for (i=0; i<num_widget_watch; ++i) {
    if (widget_watch[i] && *widget_watch[i]==w) {
      *widget_watch[i] = 0L;
    }
  }
}


/**
 \brief FLTK library options management.

 This function needs to be documented in more detail. It can be used for more
 optional settings, such as using a native file chooser instead of the FLTK one
 wherever possible, disabling tooltips, disabling visible focus, disabling
 FLTK file chooser preview, etc. .

 There should be a command line option interface.

 There should be an application that manages options system wide, per user, and
 per application.

 Example:
 \code
     if ( Fl::option(Fl::OPTION_ARROW_FOCUS) )
         { ..on..  }
     else
         { ..off..  }
 \endcode

 \note As of FLTK 1.3.0, options can be managed within fluid, using the menu
 <i>Edit/Global FLTK Settings</i>.

 \param opt which option
 \return true or false
 \see enum Fl::Fl_Option
 \see Fl::option(Fl_Option, bool)

 \since FLTK 1.3.0
 */
bool Fl::option(Fl_Option opt)
{
  if (!options_read_) {
    int tmp;
    { // first, read the system wide preferences
      Fl_Preferences prefs(Fl_Preferences::SYSTEM, "fltk.org", "fltk");
      Fl_Preferences opt_prefs(prefs, "options");
      opt_prefs.get("ArrowFocus", tmp, 0);                      // default: off
      options_[OPTION_ARROW_FOCUS] = tmp;
      //opt_prefs.get("NativeFilechooser", tmp, 1);             // default: on
      //options_[OPTION_NATIVE_FILECHOOSER] = tmp;
      //opt_prefs.get("FilechooserPreview", tmp, 1);            // default: on
      //options_[OPTION_FILECHOOSER_PREVIEW] = tmp;
      opt_prefs.get("VisibleFocus", tmp, 1);                    // default: on
      options_[OPTION_VISIBLE_FOCUS] = tmp;
      opt_prefs.get("DNDText", tmp, 1);                         // default: on
      options_[OPTION_DND_TEXT] = tmp;
      opt_prefs.get("ShowTooltips", tmp, 1);                    // default: on
      options_[OPTION_SHOW_TOOLTIPS] = tmp;
      opt_prefs.get("FNFCUsesGTK", tmp, 1);                    // default: on
      options_[OPTION_FNFC_USES_GTK] = tmp;
    }
    { // next, check the user preferences
      // override system options only, if the option is set ( >= 0 )
      Fl_Preferences prefs(Fl_Preferences::USER, "fltk.org", "fltk");
      Fl_Preferences opt_prefs(prefs, "options");
      opt_prefs.get("ArrowFocus", tmp, -1);
      if (tmp >= 0) options_[OPTION_ARROW_FOCUS] = tmp;
      //opt_prefs.get("NativeFilechooser", tmp, -1);
      //if (tmp >= 0) options_[OPTION_NATIVE_FILECHOOSER] = tmp;
      //opt_prefs.get("FilechooserPreview", tmp, -1);
      //if (tmp >= 0) options_[OPTION_FILECHOOSER_PREVIEW] = tmp;
      opt_prefs.get("VisibleFocus", tmp, -1);
      if (tmp >= 0) options_[OPTION_VISIBLE_FOCUS] = tmp;
      opt_prefs.get("DNDText", tmp, -1);
      if (tmp >= 0) options_[OPTION_DND_TEXT] = tmp;
      opt_prefs.get("ShowTooltips", tmp, -1);
      if (tmp >= 0) options_[OPTION_SHOW_TOOLTIPS] = tmp;
      opt_prefs.get("FNFCUsesGTK", tmp, -1);
      if (tmp >= 0) options_[OPTION_FNFC_USES_GTK] = tmp;
    }
    { // now, if the developer has registered this app, we could as for per-application preferences
    }
    options_read_ = 1;
  }
  if (opt<0 || opt>=OPTION_LAST)
    return false;
  return (bool)(options_[opt]!=0);
}

/**
 \brief Override an option while the application is running.

 This function does not change any system or user settings.

 Example:
 \code
     Fl::option(Fl::OPTION_ARROW_FOCUS, true);     // on
     Fl::option(Fl::OPTION_ARROW_FOCUS, false);    // off
 \endcode

 \param opt which option
 \param val set to true or false
 \see enum Fl::Fl_Option
 \see bool Fl::option(Fl_Option)
 */
void Fl::option(Fl_Option opt, bool val)
{
  if (opt<0 || opt>=OPTION_LAST)
    return;
  if (!options_read_) {
    // first read this option, so we don't override our setting later
    option(opt);
  }
  options_[opt] = val;
}


// Helper class Fl_Widget_Tracker

/**
  The constructor adds a widget to the watch list.
*/
Fl_Widget_Tracker::Fl_Widget_Tracker(Fl_Widget *wi)
{
  wp_ = wi;
  Fl::watch_widget_pointer(wp_); // add pointer to watch list
}

/**
  The destructor removes a widget from the watch list.
*/
Fl_Widget_Tracker::~Fl_Widget_Tracker()
{
  Fl::release_widget_pointer(wp_); // remove pointer from watch list
}

int Fl::use_high_res_GL_ = 0;

int Fl::dnd()
{
  return Fl::screen_driver()->dnd();
}

#if !defined(FL_DOXYGEN) // FIXME - silence Doxygen warnings

/**
  Resets marked text.

  \todo Please explain what exactly this does and how to use it.
*/
void Fl::reset_marked_text() {
  Fl::screen_driver()->reset_marked_text();
}

/**
  Sets window coordinates and height of insertion point.

  \todo Please explain what exactly this does and how to use it.
*/
void Fl::insertion_point_location(int x, int y, int height) {
  Fl::screen_driver()->insertion_point_location(x, y, height);
}

#endif // !defined(FL_DOXYGEN) // FIXME - silence Doxygen warnings

int Fl::event_key(int k) {
  return system_driver()->event_key(k);
}

int Fl::get_key(int k) {
  return system_driver()->get_key(k);
}

//
// End of "$Id$".
//
