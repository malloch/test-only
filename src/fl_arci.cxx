/
// "$Id$
/
// Arc (integer) drawing functions for the Fast Light Tool Kit (FLTK)
/
// Copyright 1998-2004 by Bill Spitzak and others
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

// "integer" circle drawing functions.  These draw the limite
// circle types provided by X and NT graphics.  The advantage o
// these is that small ones draw quite nicely (probably due to store
// hand-drawn bitmaps of small circles!) and may be implemented b
// hardware and thus are fast

// Probably should add fl_chord

// 3/10/98: create


#ifdef WIN3
#  include "win/arci.cxx
#elif defined(__APPLE__
#  include "carbon/arci.cxx
#els
#  include "xlib/arci.cxx
#endi

/
// End of "$Id$"
/
