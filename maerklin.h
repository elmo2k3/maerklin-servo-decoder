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
#ifndef __MAERKLIN_H__
#define __MAERKLIN_H__

#define	KEY_1_OPEN 1
#define KEY_1_CLOSE 2
#define KEY_2_OPEN 3
#define KEY_2_CLOSE 4
#define KEY_3_OPEN 5
#define KEY_3_CLOSE 6

void maerklin_init(void);
uint8_t maerklin(void);

#endif

