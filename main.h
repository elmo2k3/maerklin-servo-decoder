/*
 * Copyright (C) 2010 Bjoern Biesenbach <bjoern@bjoern-b.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __MAIN_H__
#define __MAIN_H__

// this code is from peter dannegger (mikrocontroller.net)
#define debounce( port, pin )						\
({									\
  static uint8_t flag = 0;	/* new variable on every macro usage */	\
  uint8_t i = 0;							\
									\
  if( flag ){			/* check for key release: */		\
    for(;;){			/* loop ... */				\
      if( !(port & 1<<pin) ){	/* ... until key pressed or ... */	\
	i = 0;			/* 0 = bounce */			\
	break;								\
      }									\
      _delay_us( 98 );		/* * 256 = 25ms */			\
      if( --i == 0 ){		/* ... until key >25ms released */	\
	flag = 0;		/* clear press flag */			\
	i = 0;			/* 0 = key release debounced */		\
	break;								\
      }									\
    }									\
  }else{			/* else check for key press: */		\
    for(;;){			/* loop ... */				\
      if( (port & 1<<pin) ){	/* ... until key released or ... */	\
	i = 0;			/* 0 = bounce */			\
	break;								\
      }									\
      _delay_us( 98 );		/* * 256 = 25ms */			\
      if( --i == 0 ){		/* ... until key >25ms pressed */	\
	flag = 1;		/* set press flag */			\
	i = 1;			/* 1 = key press debounced */		\
	break;								\
      }									\
    }									\
  }									\
  i;				/* return value of Macro */		\
})

#endif

