/**
 * $Id$
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. The Blender
 * Foundation also sells licenses for use in proprietary software under
 * the Blender License.  See http://www.blender.org/BL/ for information
 * about this.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */

#include "BMF_FontData.h"

#include "BMF_Settings.h"

#if BMF_INCLUDE_HELVB10

static unsigned char bitmap_data[]= {
	0x00,0xc0,0x00,0x80,0x80,0xc0,0xc0,0xc0,
	0xc0,0xa0,0xa0,0xa0,0x50,0x50,0xfc,0x28,
	0x7e,0x28,0x28,0x20,0x70,0xa8,0x28,0x38,
	0x70,0xe0,0xa8,0x70,0x20,0x8c,0x56,0x2c,
	0x10,0x10,0x68,0xb4,0x62,0x76,0xdc,0xcc,
	0xde,0x70,0xd8,0xd8,0x70,0x80,0x40,0xc0,
	0xc0,0x20,0x60,0x40,0xc0,0xc0,0xc0,0xc0,
	0x40,0x60,0x20,0x80,0xc0,0x40,0x60,0x60,
	0x60,0x60,0x40,0xc0,0x80,0xa0,0x40,0xa0,
	0x30,0x30,0xfc,0x30,0x30,0x80,0x40,0xc0,
	0xc0,0xf8,0xc0,0xc0,0x80,0x80,0x40,0x40,
	0x20,0x20,0x10,0x10,0x70,0xd8,0xd8,0xd8,
	0xd8,0xd8,0xd8,0x70,0x60,0x60,0x60,0x60,
	0x60,0x60,0xe0,0x60,0xf8,0xc0,0x60,0x30,
	0x18,0x18,0xd8,0x70,0x70,0xd8,0x18,0x18,
	0x30,0x18,0xd8,0x70,0x18,0x18,0xfc,0x98,
	0x58,0x38,0x18,0x08,0x70,0xd8,0x98,0x18,
	0xf0,0xc0,0xc0,0xf8,0x70,0xd8,0xd8,0xd8,
	0xf0,0xc0,0xd8,0x70,0x60,0x60,0x60,0x30,
	0x30,0x18,0x18,0xf8,0x70,0xd8,0xd8,0xd8,
	0x70,0xd8,0xd8,0x70,0x70,0xd8,0x18,0x78,
	0xd8,0xd8,0xd8,0x70,0xc0,0xc0,0x00,0x00,
	0xc0,0xc0,0x80,0x40,0xc0,0xc0,0x00,0x00,
	0xc0,0xc0,0x30,0x60,0xc0,0x60,0x30,0xf8,
	0x00,0xf8,0xc0,0x60,0x30,0x60,0xc0,0x60,
	0x00,0x60,0x60,0x30,0x18,0xd8,0x70,0x3e,
	0x00,0x40,0x00,0x9b,0x00,0xa4,0x80,0xa2,
	0x40,0x92,0x40,0x4d,0x40,0x60,0x80,0x1f,
	0x00,0xc6,0xc6,0xfe,0x6c,0x6c,0x6c,0x38,
	0x38,0xf8,0xcc,0xcc,0xcc,0xf8,0xcc,0xcc,
	0xf8,0x3c,0x66,0xc2,0xc0,0xc0,0xc2,0x66,
	0x3c,0xf0,0xd8,0xcc,0xcc,0xcc,0xcc,0xd8,
	0xf0,0xf8,0xc0,0xc0,0xc0,0xf8,0xc0,0xc0,
	0xf8,0xc0,0xc0,0xc0,0xc0,0xf0,0xc0,0xc0,
	0xf8,0x3a,0x66,0xc6,0xce,0xc0,0xc2,0x66,
	0x3c,0xcc,0xcc,0xcc,0xcc,0xfc,0xcc,0xcc,
	0xcc,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,
	0xc0,0x70,0xd8,0x18,0x18,0x18,0x18,0x18,
	0x18,0xc6,0xcc,0xd8,0xf0,0xe0,0xf0,0xd8,
	0xcc,0xf8,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,
	0xc0,0xc9,0x80,0xc9,0x80,0xdd,0x80,0xd5,
	0x80,0xf7,0x80,0xe3,0x80,0xe3,0x80,0xc1,
	0x80,0xc6,0xce,0xce,0xd6,0xd6,0xe6,0xe6,
	0xc6,0x38,0x6c,0xc6,0xc6,0xc6,0xc6,0x6c,
	0x38,0xc0,0xc0,0xc0,0xf8,0xcc,0xcc,0xcc,
	0xf8,0x02,0x3c,0x6c,0xd6,0xc6,0xc6,0xc6,
	0x6c,0x38,0xcc,0xcc,0xcc,0xf8,0xcc,0xcc,
	0xcc,0xf8,0x78,0xcc,0x8c,0x1c,0x78,0xe0,
	0xcc,0x78,0x30,0x30,0x30,0x30,0x30,0x30,
	0x30,0xfc,0x78,0xcc,0xcc,0xcc,0xcc,0xcc,
	0xcc,0xcc,0x10,0x38,0x38,0x6c,0x6c,0x6c,
	0xc6,0xc6,0x33,0x00,0x33,0x00,0x7f,0x80,
	0x6d,0x80,0x6d,0x80,0xcc,0xc0,0xcc,0xc0,
	0xcc,0xc0,0xc6,0xc6,0x6c,0x38,0x38,0x6c,
	0xc6,0xc6,0x18,0x18,0x18,0x3c,0x66,0x66,
	0xc3,0xc3,0xfc,0xc0,0x60,0x70,0x30,0x18,
	0x0c,0xfc,0xe0,0xc0,0xc0,0xc0,0xc0,0xc0,
	0xc0,0xc0,0xc0,0xe0,0x10,0x10,0x20,0x20,
	0x40,0x40,0x80,0x80,0xe0,0x60,0x60,0x60,
	0x60,0x60,0x60,0x60,0x60,0xe0,0x90,0x90,
	0xf0,0x60,0xfc,0xc0,0xc0,0x80,0x40,0x6c,
	0xd8,0xd8,0x78,0x98,0x70,0xf0,0xd8,0xd8,
	0xd8,0xd8,0xf0,0xc0,0xc0,0x70,0xd0,0xc0,
	0xc0,0xd0,0x70,0x78,0xd8,0xd8,0xd8,0xd8,
	0x78,0x18,0x18,0x70,0xd8,0xc0,0xf8,0xd8,
	0x70,0x60,0x60,0x60,0x60,0x60,0xf0,0x60,
	0x38,0x70,0x18,0x78,0xd8,0xd8,0xd8,0xd8,
	0x68,0xd8,0xd8,0xd8,0xd8,0xd8,0xf0,0xc0,
	0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0x00,
	0xc0,0xc0,0x60,0x60,0x60,0x60,0x60,0x60,
	0x60,0x00,0x60,0xcc,0xd8,0xf0,0xe0,0xf0,
	0xd8,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,
	0xc0,0xc0,0xc0,0xdb,0xdb,0xdb,0xdb,0xdb,
	0xb6,0xd8,0xd8,0xd8,0xd8,0xd8,0xb0,0x70,
	0xd8,0xd8,0xd8,0xd8,0x70,0xc0,0xc0,0xf0,
	0xd8,0xd8,0xd8,0xd8,0xb0,0x18,0x18,0x78,
	0xd8,0xd8,0xd8,0xd8,0x68,0xc0,0xc0,0xc0,
	0xc0,0xe0,0xb0,0x70,0xd8,0x18,0x70,0xd8,
	0x70,0x30,0x60,0x60,0x60,0x60,0xf0,0x60,
	0x60,0x68,0xd8,0xd8,0xd8,0xd8,0xd8,0x20,
	0x70,0x50,0xd8,0xd8,0xd8,0x6c,0x6c,0x6c,
	0xd6,0xd6,0xd6,0xcc,0xcc,0x78,0x30,0x78,
	0xcc,0x60,0x30,0x30,0x78,0xd8,0xd8,0xd8,
	0xd8,0xf8,0xc0,0x60,0x30,0x18,0xf8,0x30,
	0x60,0x60,0x60,0x60,0xc0,0x60,0x60,0x60,
	0x30,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0xc0,0x60,0x60,0x60,0x60,
	0x30,0x60,0x60,0x60,0xc0,0xb0,0x68,0x00,
	0xc0,0xc0,0xc0,0xc0,0x40,0x40,0x00,0xc0,
	0x40,0x70,0xd8,0xa0,0xa0,0xd8,0x70,0x10,
	0xd8,0x68,0x60,0x60,0xf0,0x60,0x68,0x38,
	0x84,0x78,0x48,0x48,0x78,0x84,0x30,0xfc,
	0x30,0xfc,0x48,0xcc,0x84,0x84,0x80,0x80,
	0x80,0x80,0x00,0x00,0x80,0x80,0x80,0x80,
	0x70,0x98,0x38,0x70,0xc8,0x98,0x70,0xe0,
	0xc8,0x70,0xa0,0x3c,0x42,0x9d,0xa1,0xa5,
	0x99,0x42,0x3c,0xe0,0x00,0xa0,0x20,0xe0,
	0x6c,0xd8,0x6c,0x08,0x08,0xf8,0xf0,0x3c,
	0x42,0xa5,0xb9,0xa5,0xbd,0x42,0x3c,0xe0,
	0xc0,0xa0,0x60,0xfc,0x00,0x30,0x30,0xfc,
	0x30,0x30,0xe0,0x40,0xa0,0x60,0xc0,0x20,
	0x40,0xe0,0x80,0x40,0xc0,0xc0,0xe8,0xd8,
	0xd8,0xd8,0xd8,0xd8,0x28,0x28,0x28,0x28,
	0x28,0x68,0xe8,0xe8,0xe8,0x7c,0xc0,0xc0,
	0x40,0x40,0x40,0xc0,0x40,0xe0,0x00,0xe0,
	0xa0,0xe0,0xd8,0x6c,0xd8,0x42,0x2f,0x26,
	0x12,0x48,0x48,0xc4,0x44,0x4e,0x24,0x2a,
	0x16,0x48,0x48,0xc4,0x44,0x42,0x2f,0x26,
	0x12,0xc8,0x28,0x44,0xe4,0x70,0xd8,0xc0,
	0x60,0x30,0x30,0x00,0x30,0xc6,0xc6,0xfe,
	0x6c,0x6c,0x6c,0x38,0x38,0x00,0x10,0x20,
	0xc6,0xc6,0xfe,0x6c,0x6c,0x6c,0x38,0x38,
	0x00,0x10,0x08,0xc6,0xc6,0xfe,0x6c,0x6c,
	0x6c,0x38,0x38,0x00,0x28,0x10,0xc6,0xc6,
	0xfe,0x6c,0x6c,0x6c,0x38,0x38,0x00,0x28,
	0x14,0xc6,0xc6,0xfe,0x6c,0x6c,0x6c,0x38,
	0x38,0x00,0x28,0xc6,0xc6,0xfe,0x6c,0x6c,
	0x6c,0x38,0x38,0x10,0x28,0x10,0xcf,0x80,
	0xcc,0x00,0xfc,0x00,0x6c,0x00,0x6f,0x80,
	0x6c,0x00,0x3c,0x00,0x3f,0x80,0x30,0x10,
	0x3c,0x66,0xc2,0xc0,0xc0,0xc2,0x66,0x3c,
	0xf8,0xc0,0xc0,0xc0,0xf8,0xc0,0xc0,0xf8,
	0x00,0x20,0x40,0xf8,0xc0,0xc0,0xc0,0xf8,
	0xc0,0xc0,0xf8,0x00,0x20,0x10,0xf8,0xc0,
	0xc0,0xc0,0xf8,0xc0,0xc0,0xf8,0x00,0x50,
	0x20,0xf8,0xc0,0xc0,0xc0,0xf8,0xc0,0xc0,
	0xf8,0x00,0x50,0xc0,0xc0,0xc0,0xc0,0xc0,
	0xc0,0xc0,0xc0,0x00,0x40,0x80,0xc0,0xc0,
	0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0x00,0x80,
	0x40,0x60,0x60,0x60,0x60,0x60,0x60,0x60,
	0x60,0x00,0xa0,0x40,0x60,0x60,0x60,0x60,
	0x60,0x60,0x60,0x60,0x00,0x90,0x78,0x6c,
	0x66,0x66,0xf6,0x66,0x6c,0x78,0xc6,0xce,
	0xce,0xd6,0xd6,0xe6,0xe6,0xc6,0x00,0x28,
	0x14,0x38,0x6c,0xc6,0xc6,0xc6,0xc6,0x6c,
	0x38,0x00,0x08,0x10,0x38,0x6c,0xc6,0xc6,
	0xc6,0xc6,0x6c,0x38,0x00,0x10,0x08,0x38,
	0x6c,0xc6,0xc6,0xc6,0xc6,0x6c,0x38,0x00,
	0x28,0x10,0x38,0x6c,0xc6,0xc6,0xc6,0xc6,
	0x6c,0x38,0x00,0x28,0x14,0x38,0x6c,0xc6,
	0xc6,0xc6,0xc6,0x6c,0x38,0x00,0x28,0xcc,
	0x78,0x30,0x78,0xcc,0xb8,0x6c,0xe6,0xd6,
	0xd6,0xce,0x6c,0x3a,0x78,0xcc,0xcc,0xcc,
	0xcc,0xcc,0xcc,0xcc,0x00,0x10,0x20,0x78,
	0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0x00,
	0x10,0x08,0x78,0xcc,0xcc,0xcc,0xcc,0xcc,
	0xcc,0xcc,0x00,0x50,0x20,0x78,0xcc,0xcc,
	0xcc,0xcc,0xcc,0xcc,0xcc,0x00,0x48,0x18,
	0x18,0x18,0x3c,0x66,0x66,0xc3,0xc3,0x00,
	0x08,0x04,0xc0,0xc0,0xf8,0xcc,0xcc,0xcc,
	0xf8,0xc0,0xd0,0xc8,0xc8,0xc8,0xd0,0xc8,
	0xc8,0x70,0x6c,0xd8,0xd8,0x78,0x98,0x70,
	0x00,0x10,0x20,0x6c,0xd8,0xd8,0x78,0x98,
	0x70,0x00,0x20,0x10,0x6c,0xd8,0xd8,0x78,
	0x98,0x70,0x00,0x50,0x20,0x6c,0xd8,0xd8,
	0x78,0x98,0x70,0x00,0x50,0x28,0x6c,0xd8,
	0xd8,0x78,0x98,0x70,0x00,0x50,0x6c,0xd8,
	0xd8,0x78,0x98,0x70,0x20,0x50,0x20,0x6e,
	0xdb,0xd8,0x7f,0x9b,0x7e,0x60,0x20,0x70,
	0xd0,0xc0,0xc0,0xd0,0x70,0x70,0xd8,0xc0,
	0xf8,0xd8,0x70,0x00,0x20,0x40,0x70,0xd8,
	0xc0,0xf8,0xd8,0x70,0x00,0x20,0x10,0x70,
	0xd8,0xc0,0xf8,0xd8,0x70,0x00,0x50,0x20,
	0x70,0xd8,0xc0,0xf8,0xd8,0x70,0x00,0x50,
	0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0x00,0x40,
	0x80,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0x00,
	0x80,0x40,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,
	0x00,0xa0,0x40,0xc0,0xc0,0xc0,0xc0,0xc0,
	0xc0,0x00,0xa0,0x70,0xd8,0xd8,0xd8,0xd8,
	0x70,0xa0,0x60,0x50,0xd8,0xd8,0xd8,0xd8,
	0xd8,0xb0,0x00,0xa0,0x50,0x70,0xd8,0xd8,
	0xd8,0xd8,0x70,0x00,0x20,0x40,0x70,0xd8,
	0xd8,0xd8,0xd8,0x70,0x00,0x20,0x10,0x70,
	0xd8,0xd8,0xd8,0xd8,0x70,0x00,0x50,0x20,
	0x70,0xd8,0xd8,0xd8,0xd8,0x70,0x00,0xa0,
	0x50,0x70,0xd8,0xd8,0xd8,0xd8,0x70,0x00,
	0x50,0x30,0x00,0xfc,0x00,0x30,0xb8,0x6c,
	0x6c,0x7c,0x6c,0x3a,0x68,0xd8,0xd8,0xd8,
	0xd8,0xd8,0x00,0x20,0x40,0x68,0xd8,0xd8,
	0xd8,0xd8,0xd8,0x00,0x20,0x10,0x68,0xd8,
	0xd8,0xd8,0xd8,0xd8,0x00,0x50,0x20,0x68,
	0xd8,0xd8,0xd8,0xd8,0xd8,0x00,0x50,0x60,
	0x30,0x30,0x78,0xd8,0xd8,0xd8,0xd8,0x00,
	0x20,0x10,0xc0,0xc0,0xf0,0xd8,0xc8,0xc8,
	0xd8,0xf0,0xc0,0xc0,0x60,0x30,0x30,0x78,
	0xd8,0xd8,0xd8,0xd8,0x00,0x50,
};

BMF_FontData BMF_font_helvb10 = {
	-1, -2,
	10, 11,
	{
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0, 0, 0, 0, 12, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{1, 1, 0, 0, 3, 0},
		{2, 8, -1, 0, 4, 1},
		{3, 3, -1, -5, 5, 9},
		{7, 7, 1, 0, 6, 12},
		{5, 10, 0, 1, 6, 19},
		{7, 8, 0, 0, 8, 29},
		{7, 8, 0, 0, 8, 37},
		{2, 4, 0, -4, 3, 45},
		{3, 10, 0, 2, 4, 49},
		{3, 10, 0, 2, 4, 59},
		{3, 3, 0, -5, 4, 69},
		{6, 5, 0, -1, 6, 72},
		{2, 4, 0, 2, 3, 77},
		{5, 1, -1, -3, 7, 81},
		{2, 2, 0, 0, 3, 82},
		{4, 8, 0, 0, 4, 84},
		{5, 8, 0, 0, 6, 92},
		{3, 8, -1, 0, 6, 100},
		{5, 8, 0, 0, 6, 108},
		{5, 8, 0, 0, 6, 116},
		{6, 8, 0, 0, 6, 124},
		{5, 8, 0, 0, 6, 132},
		{5, 8, 0, 0, 6, 140},
		{5, 8, 0, 0, 6, 148},
		{5, 8, 0, 0, 6, 156},
		{5, 8, 0, 0, 6, 164},
		{2, 6, 0, 0, 3, 172},
		{2, 8, 0, 2, 3, 178},
		{4, 5, 0, -1, 5, 186},
		{5, 3, 0, -2, 6, 191},
		{4, 5, 0, -1, 5, 194},
		{5, 8, 0, 0, 6, 199},
		{10, 9, 0, 1, 11, 207},
		{7, 8, 0, 0, 8, 225},
		{6, 8, 0, 0, 7, 233},
		{7, 8, 0, 0, 8, 241},
		{6, 8, 0, 0, 7, 249},
		{5, 8, 0, 0, 6, 257},
		{5, 8, 0, 0, 6, 265},
		{7, 8, 0, 0, 8, 273},
		{6, 8, 0, 0, 7, 281},
		{2, 8, 0, 0, 3, 289},
		{5, 8, 0, 0, 6, 297},
		{7, 8, 0, 0, 7, 305},
		{5, 8, 0, 0, 6, 313},
		{9, 8, 0, 0, 10, 321},
		{7, 8, 0, 0, 8, 337},
		{7, 8, 0, 0, 8, 345},
		{6, 8, 0, 0, 7, 353},
		{7, 9, 0, 1, 8, 361},
		{6, 8, 0, 0, 7, 370},
		{6, 8, 0, 0, 7, 378},
		{6, 8, 0, 0, 7, 386},
		{6, 8, 0, 0, 7, 394},
		{7, 8, 0, 0, 8, 402},
		{10, 8, 0, 0, 11, 410},
		{7, 8, 0, 0, 8, 426},
		{8, 8, 0, 0, 9, 434},
		{6, 8, 0, 0, 7, 442},
		{3, 10, 0, 2, 4, 450},
		{4, 8, 0, 0, 4, 460},
		{3, 10, 0, 2, 4, 468},
		{4, 4, 0, -4, 5, 478},
		{6, 1, 0, 2, 6, 482},
		{2, 4, 0, -4, 3, 483},
		{6, 6, 0, 0, 6, 487},
		{5, 8, 0, 0, 6, 493},
		{4, 6, 0, 0, 5, 501},
		{5, 8, 0, 0, 6, 507},
		{5, 6, 0, 0, 6, 515},
		{5, 8, 1, 0, 4, 521},
		{5, 8, 0, 2, 6, 529},
		{5, 8, 0, 0, 6, 537},
		{2, 8, 0, 0, 3, 545},
		{3, 10, 1, 2, 3, 553},
		{6, 8, 0, 0, 6, 563},
		{2, 8, 0, 0, 3, 571},
		{8, 6, 0, 0, 9, 579},
		{5, 6, 0, 0, 6, 585},
		{5, 6, 0, 0, 6, 591},
		{5, 8, 0, 2, 6, 597},
		{5, 8, 0, 2, 6, 605},
		{4, 6, 0, 0, 4, 613},
		{5, 6, 0, 0, 6, 619},
		{4, 8, 1, 0, 4, 625},
		{5, 6, 0, 0, 6, 633},
		{5, 6, 0, 0, 6, 639},
		{7, 6, 0, 0, 8, 645},
		{6, 6, 0, 0, 7, 651},
		{5, 8, 0, 2, 6, 657},
		{5, 6, 0, 0, 6, 665},
		{4, 10, 0, 2, 5, 671},
		{1, 10, -1, 2, 3, 681},
		{4, 10, 0, 2, 5, 691},
		{5, 2, 0, -3, 6, 701},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{1, 1, 0, 0, 3, 703},
		{2, 8, -1, 2, 4, 704},
		{5, 8, 0, 1, 6, 712},
		{5, 8, 0, 0, 6, 720},
		{6, 6, 0, -1, 6, 728},
		{6, 8, 0, 0, 7, 734},
		{1, 10, -1, 2, 3, 742},
		{5, 10, 0, 2, 6, 752},
		{3, 1, 0, -7, 3, 762},
		{8, 8, -1, 0, 10, 763},
		{3, 5, -1, -3, 5, 771},
		{6, 3, 0, -1, 7, 776},
		{5, 3, -1, -2, 7, 779},
		{4, 1, 0, -3, 5, 782},
		{8, 8, -1, 0, 10, 783},
		{3, 1, 0, -7, 3, 791},
		{3, 3, 0, -5, 4, 792},
		{6, 7, 0, 0, 6, 795},
		{3, 4, 0, -4, 3, 802},
		{3, 4, 0, -4, 3, 806},
		{2, 2, 0, -7, 3, 810},
		{5, 8, 0, 2, 6, 812},
		{6, 10, 0, 2, 6, 820},
		{2, 1, 0, -3, 3, 830},
		{2, 2, 0, 2, 3, 831},
		{2, 4, 0, -4, 3, 833},
		{3, 5, -1, -3, 5, 837},
		{6, 3, 0, -1, 7, 842},
		{8, 8, 0, 0, 9, 845},
		{7, 8, 0, 0, 9, 853},
		{8, 8, 0, 0, 9, 861},
		{5, 8, 0, 2, 6, 869},
		{7, 11, 0, 0, 8, 877},
		{7, 11, 0, 0, 8, 888},
		{7, 11, 0, 0, 8, 899},
		{7, 11, 0, 0, 8, 910},
		{7, 10, 0, 0, 8, 921},
		{7, 11, 0, 0, 8, 931},
		{9, 8, 0, 0, 10, 942},
		{7, 10, 0, 2, 8, 958},
		{5, 11, 0, 0, 6, 968},
		{5, 11, 0, 0, 6, 979},
		{5, 11, 0, 0, 6, 990},
		{5, 10, 0, 0, 6, 1001},
		{2, 11, 0, 0, 3, 1011},
		{2, 11, 0, 0, 3, 1022},
		{3, 11, 1, 0, 3, 1033},
		{4, 10, 1, 0, 3, 1044},
		{7, 8, 1, 0, 7, 1054},
		{7, 11, 0, 0, 8, 1062},
		{7, 11, 0, 0, 8, 1073},
		{7, 11, 0, 0, 8, 1084},
		{7, 11, 0, 0, 8, 1095},
		{7, 11, 0, 0, 8, 1106},
		{7, 10, 0, 0, 8, 1117},
		{6, 5, 0, -1, 6, 1127},
		{7, 8, 0, 0, 8, 1132},
		{6, 11, 0, 0, 7, 1140},
		{6, 11, 0, 0, 7, 1151},
		{6, 11, 0, 0, 7, 1162},
		{6, 10, 0, 0, 7, 1173},
		{8, 11, 0, 0, 9, 1183},
		{6, 8, 0, 0, 7, 1194},
		{5, 8, 0, 0, 6, 1202},
		{6, 9, 0, 0, 6, 1210},
		{6, 9, 0, 0, 6, 1219},
		{6, 9, 0, 0, 6, 1228},
		{6, 9, 0, 0, 6, 1237},
		{6, 8, 0, 0, 6, 1246},
		{6, 9, 0, 0, 6, 1254},
		{8, 6, 0, 0, 9, 1263},
		{4, 8, 0, 2, 5, 1269},
		{5, 9, 0, 0, 6, 1277},
		{5, 9, 0, 0, 6, 1286},
		{5, 9, 0, 0, 6, 1295},
		{5, 8, 0, 0, 6, 1304},
		{2, 9, 0, 0, 3, 1312},
		{2, 9, 0, 0, 3, 1321},
		{3, 9, 0, 0, 3, 1330},
		{3, 8, 0, 0, 3, 1339},
		{5, 9, 0, 0, 6, 1347},
		{5, 9, 0, 0, 6, 1356},
		{5, 9, 0, 0, 6, 1365},
		{5, 9, 0, 0, 6, 1374},
		{5, 9, 0, 0, 6, 1383},
		{5, 9, 0, 0, 6, 1392},
		{5, 8, 0, 0, 6, 1401},
		{6, 5, 0, -1, 6, 1409},
		{7, 6, 1, 0, 6, 1414},
		{5, 9, 0, 0, 6, 1420},
		{5, 9, 0, 0, 6, 1429},
		{5, 9, 0, 0, 6, 1438},
		{5, 8, 0, 0, 6, 1447},
		{5, 11, 0, 2, 6, 1455},
		{5, 10, 0, 2, 6, 1466},
		{5, 10, 0, 2, 6, 1476},
	},
	bitmap_data
};

#endif

