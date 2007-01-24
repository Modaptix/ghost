/**
 * $Id$
 *
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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <unistd.h>
#else
#include <io.h>
#include <direct.h>
#endif   
#include "MEM_guardedalloc.h"

#include "BLI_arithb.h"
#include "BLI_blenlib.h"
#include "BLI_storage_types.h"

#include "DNA_material_types.h"
#include "DNA_texture_types.h"
#include "DNA_world_types.h"
#include "DNA_object_types.h"
#include "DNA_lamp_types.h"
#include "DNA_image_types.h"
#include "DNA_texture_types.h"
#include "DNA_world_types.h"
#include "DNA_camera_types.h"
#include "DNA_image_types.h"
#include "DNA_scene_types.h"
#include "DNA_screen_types.h"
#include "DNA_space_types.h"
#include "DNA_userdef_types.h"

#include "BKE_global.h"
#include "BKE_material.h"
#include "BKE_texture.h"
#include "BKE_world.h"
#include "BKE_image.h"
#include "BKE_object.h"
#include "BKE_utildefines.h"
#include "BKE_icons.h"
#include "BKE_packedFile.h"

#include "IMB_imbuf.h"
#include "IMB_imbuf_types.h"

#include "BIF_gl.h"
#include "BIF_glutil.h"
#include "BIF_interface.h"
#include "BIF_interface_icons.h"
#include "BIF_previewrender.h"
#include "BIF_screen.h"
#include "BIF_resources.h" /* elubie: should be removed once the enum for the ICONS is in BIF_preview_icons.h */

#include "interface.h"

#include "PIL_time.h"

#include "blendef.h"	// CLAMP
#include "datatoc.h"
#include "mydevice.h"

/* OpenGL textures have to be size 2n+2 x 2m+2 for some n,m */
/* choose ICON_RENDERSIZE accordingly */
#define ICON_RENDERSIZE 32	
#define ICON_MIPMAPS 8

#define ICON_IMAGE_W		512
#define ICON_IMAGE_H		256

#define ICON_GRID_COLS		25
#define ICON_GRID_ROWS		12

#define ICON_GRID_MARGIN	5
#define ICON_GRID_W		15
#define ICON_GRID_H		16

typedef struct DrawInfo {
	int w;
	int h;
	int rw;
	int rh;
	VectorDrawFunc drawFunc; /* If drawFunc is defined then it is a vector icon, otherwise use rect */
	float aspect;
	unsigned int *rect; 
} DrawInfo;


/* ******************* STATIC LOCAL VARS ******************* */
/* static here to cache results of icon directory scan, so it's not 
 * scanning the filesystem each time the menu is drawn */
static struct ListBase iconfilelist = {0, 0};


/* **************************************************** */

static void def_internal_icon(ImBuf *bbuf, int icon_id, int xofs, int yofs)
{
	Icon *new_icon = NULL;
	DrawInfo *di;
	int y = 0;

	new_icon = MEM_callocN(sizeof(Icon), "texicon");

	new_icon->obj = 0; /* icon is not for library object */
	new_icon->type = 0;
	new_icon->changed = 0; 
	

	di = MEM_callocN(sizeof(DrawInfo), "drawinfo");
	di->drawFunc = 0;
	di->w = ICON_DEFAULT_HEIGHT;
	di->h = ICON_DEFAULT_HEIGHT;
	di->rw = ICON_DEFAULT_HEIGHT;
	di->rh = ICON_DEFAULT_HEIGHT;
	di->aspect = 1.0f;
	di->rect = MEM_mallocN(ICON_DEFAULT_HEIGHT*ICON_DEFAULT_HEIGHT*sizeof(unsigned int), "icon_rect");
	
	/* Here we store the rect in the icon - same as before */
	for (y=0; y<ICON_DEFAULT_HEIGHT; y++) {
		memcpy(&di->rect[y*ICON_DEFAULT_HEIGHT], &bbuf->rect[(y+yofs)*512+xofs], ICON_DEFAULT_HEIGHT*sizeof(int));
	}

	new_icon->drawinfo_free = BIF_icons_free_drawinfo;
	new_icon->drawinfo = di;

	BKE_icon_set(icon_id, new_icon);
}

static void def_internal_vicon( int icon_id, VectorDrawFunc drawFunc)
{
	Icon *new_icon = NULL;
	DrawInfo* di;

	new_icon = MEM_callocN(sizeof(Icon), "texicon");

	new_icon->obj = 0; /* icon is not for library object */
	new_icon->type = 0;
	new_icon->changed = 0; 
	
	di = MEM_callocN(sizeof(DrawInfo), "drawinfo");
	di->drawFunc =drawFunc;
	di->w = ICON_DEFAULT_HEIGHT;
	di->h = ICON_DEFAULT_HEIGHT;
	di->rw = ICON_DEFAULT_HEIGHT;
	di->rh = ICON_DEFAULT_HEIGHT;
	di->aspect = 1.0f;
	di->rect = NULL;
	
	new_icon->drawinfo_free = 0;
	new_icon->drawinfo = di;

	BKE_icon_set(icon_id, new_icon);
}

/* Vector Icon Drawing Routines */

	/* Utilities */

static void viconutil_set_point(GLint pt[2], int x, int y)
{
	pt[0] = x;
	pt[1] = y;
}

static void viconutil_draw_tri(GLint (*pts)[2])
{
	glBegin(GL_TRIANGLES);
	glVertex2iv(pts[0]);
	glVertex2iv(pts[1]);
	glVertex2iv(pts[2]);
	glEnd();
}

#if 0
static void viconutil_draw_quad(GLint (*pts)[2])
{
	glBegin(GL_QUADS);
	glVertex2iv(pts[0]);
	glVertex2iv(pts[1]);
	glVertex2iv(pts[2]);
	glVertex2iv(pts[3]);
	glEnd();
}
#endif

static void viconutil_draw_lineloop(GLint (*pts)[2], int numPoints)
{
	int i;

	glBegin(GL_LINE_LOOP);
	for (i=0; i<numPoints; i++) {
		glVertex2iv(pts[i]);
	}
	glEnd();
}

static void viconutil_draw_lineloop_smooth(GLint (*pts)[2], int numPoints)
{
	glEnable(GL_LINE_SMOOTH);
	viconutil_draw_lineloop(pts, numPoints);
	glDisable(GL_LINE_SMOOTH);
}

static void viconutil_draw_points(GLint (*pts)[2], int numPoints, int pointSize)
{
	int i;

	glBegin(GL_QUADS);
	for (i=0; i<numPoints; i++) {
		int x = pts[i][0], y = pts[i][1];

		glVertex2i(x-pointSize,y-pointSize);
		glVertex2i(x+pointSize,y-pointSize);
		glVertex2i(x+pointSize,y+pointSize);
		glVertex2i(x-pointSize,y+pointSize);
	}
	glEnd();
}

	/* Drawing functions */

static void vicon_x_draw(int x, int y, int w, int h, float alpha)
{
	x += 3;
	y += 3;
	w -= 6;
	h -= 6;

	glEnable( GL_LINE_SMOOTH );

	glLineWidth(2.5);
	
	glColor4f(0.0, 0.0, 0.0, alpha);
	glBegin(GL_LINES);
	glVertex2i(x  ,y  );
	glVertex2i(x+w,y+h);
	glVertex2i(x+w,y  );
	glVertex2i(x  ,y+h);
	glEnd();

	glLineWidth(1.0);
	
	glDisable( GL_LINE_SMOOTH );
}

static void vicon_view3d_draw(int x, int y, int w, int h, float alpha)
{
	int cx = x + w/2;
	int cy = y + h/2;
	int d = MAX2(2, h/3);

	glColor4f(0.5, 0.5, 0.5, alpha);
	glBegin(GL_LINES);
	glVertex2i(x  , cy-d);
	glVertex2i(x+w, cy-d);
	glVertex2i(x  , cy+d);
	glVertex2i(x+w, cy+d);

	glVertex2i(cx-d, y  );
	glVertex2i(cx-d, y+h);
	glVertex2i(cx+d, y  );
	glVertex2i(cx+d, y+h);
	glEnd();
	
	glColor4f(0.0, 0.0, 0.0, alpha);
	glBegin(GL_LINES);
	glVertex2i(x  , cy);
	glVertex2i(x+w, cy);
	glVertex2i(cx, y  );
	glVertex2i(cx, y+h);
	glEnd();
}

static void vicon_edit_draw(int x, int y, int w, int h, float alpha)
{
	GLint pts[4][2];

	viconutil_set_point(pts[0], x+3  , y+3  );
	viconutil_set_point(pts[1], x+w-3, y+3  );
	viconutil_set_point(pts[2], x+w-3, y+h-3);
	viconutil_set_point(pts[3], x+3  , y+h-3);

	glColor4f(0.0, 0.0, 0.0, alpha);
	viconutil_draw_lineloop(pts, 4);

	glColor3f(1, 1, 0.0);
	viconutil_draw_points(pts, 4, 1);
}

static void vicon_editmode_hlt_draw(int x, int y, int w, int h, float alpha)
{
	GLint pts[3][2];

	viconutil_set_point(pts[0], x+w/2, y+h-2);
	viconutil_set_point(pts[1], x+3, y+4);
	viconutil_set_point(pts[2], x+w-3, y+4);

	glColor4f(0.5, 0.5, 0.5, alpha);
	viconutil_draw_tri(pts);

	glColor4f(0.0, 0.0, 0.0, 1);
	viconutil_draw_lineloop_smooth(pts, 3);

	glColor3f(1, 1, 0.0);
	viconutil_draw_points(pts, 3, 1);
}

static void vicon_editmode_dehlt_draw(int x, int y, int w, int h, float alpha)
{
	GLint pts[3][2];

	viconutil_set_point(pts[0], x+w/2, y+h-2);
	viconutil_set_point(pts[1], x+3, y+4);
	viconutil_set_point(pts[2], x+w-3, y+4);

	glColor4f(0.0, 0.0, 0.0, 1);
	viconutil_draw_lineloop_smooth(pts, 3);

	glColor3f(.9, .9, .9);
	viconutil_draw_points(pts, 3, 1);
}

static void vicon_disclosure_tri_right_draw(int x, int y, int w, int h, float alpha)
{
	GLint pts[3][2];
	int cx = x+w/2;
	int cy = y+w/2;
	int d = w/3, d2 = w/5;

	viconutil_set_point(pts[0], cx-d2, cy+d);
	viconutil_set_point(pts[1], cx-d2, cy-d);
	viconutil_set_point(pts[2], cx+d2, cy);

	glShadeModel(GL_SMOOTH);
	glBegin(GL_TRIANGLES);
	glColor4f(0.8, 0.8, 0.8, alpha);
	glVertex2iv(pts[0]);
	glVertex2iv(pts[1]);
	glColor4f(0.3, 0.3, 0.3, alpha);
	glVertex2iv(pts[2]);
	glEnd();
	glShadeModel(GL_FLAT);

	glColor4f(0.0, 0.0, 0.0, 1);
	viconutil_draw_lineloop_smooth(pts, 3);
}

static void vicon_disclosure_tri_down_draw(int x, int y, int w, int h, float alpha)
{
	GLint pts[3][2];
	int cx = x+w/2;
	int cy = y+w/2;
	int d = w/3, d2 = w/5;

	viconutil_set_point(pts[0], cx+d, cy+d2);
	viconutil_set_point(pts[1], cx-d, cy+d2);
	viconutil_set_point(pts[2], cx, cy-d2);

	glShadeModel(GL_SMOOTH);
	glBegin(GL_TRIANGLES);
	glColor4f(0.8, 0.8, 0.8, alpha);
	glVertex2iv(pts[0]);
	glVertex2iv(pts[1]);
	glColor4f(0.3, 0.3, 0.3, alpha);
	glVertex2iv(pts[2]);
	glEnd();
	glShadeModel(GL_FLAT);

	glColor4f(0.0, 0.0, 0.0, 1);
	viconutil_draw_lineloop_smooth(pts, 3);
}

static void vicon_move_up_draw(int x, int y, int w, int h, float alpha)
{
	int d=-2;

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1);
	glColor3f(0.0, 0.0, 0.0);

	glBegin(GL_LINE_STRIP);
	glVertex2i(x+w/2-d*2, y+h/2+d);
	glVertex2i(x+w/2, y+h/2-d + 1);
	glVertex2i(x+w/2+d*2, y+h/2+d);
	glEnd();

	glLineWidth(1.0);
	glDisable(GL_LINE_SMOOTH);
}

static void vicon_move_down_draw(int x, int y, int w, int h, float alpha)
{
	int d=2;

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1);
	glColor3f(0.0, 0.0, 0.0);

	glBegin(GL_LINE_STRIP);
	glVertex2i(x+w/2-d*2, y+h/2+d);
	glVertex2i(x+w/2, y+h/2-d - 1);
	glVertex2i(x+w/2+d*2, y+h/2+d);
	glEnd();

	glLineWidth(1.0);
	glDisable(GL_LINE_SMOOTH);
}

/***/


/* this only works for the hardcoded buttons image, turning the grey AA pixels to alpha, and slight off-grey to half alpha */
#if 0
static void clear_transp_rect_soft(unsigned char *transp, unsigned char *rect, int w, int h, int rowstride)
{
	int x, y, val;
	
	for (y=1; y<h-1; y++) {
		unsigned char *row0= &rect[(y-1)*rowstride];
		unsigned char *row= &rect[y*rowstride];
		unsigned char *row1= &rect[(y+1)*rowstride];
		for (x=1; x<w-1; x++) {
			unsigned char *pxl0= &row0[x*4];
			unsigned char *pxl= &row[x*4];
			unsigned char *pxl1= &row1[x*4];
			
			if(pxl[3]!=0) {
				val= (abs(pxl[0]-transp[0]) + abs(pxl[1]-transp[1]) + abs(pxl[2]-transp[2]))/3;
				if(val<20) {
					pxl[3]= 128;
				}
				else if(val<50) {
					// one of pixels surrounding has alpha null?
					if(pxl[3-4]==0 || pxl[3+4]==0 || pxl0[3]==0 || pxl1[3]==0) {
				
						if(pxl[0]>val) pxl[0]-= val; else pxl[0]= 0;
						if(pxl[1]>val) pxl[1]-= val; else pxl[1]= 0;
						if(pxl[2]>val) pxl[2]-= val; else pxl[2]= 0;
						
						pxl[3]= 128;
					}
				}
			}
		}
	}
}
#endif

static void clear_icon_grid_margins(unsigned char *rect, int w, int h)
{
	int x, y;
	int xoffs=ICON_GRID_W+ICON_GRID_MARGIN;
	int yoffs=ICON_GRID_H+ICON_GRID_MARGIN;

	for (y=0; y<h; y++) {
		unsigned char *row= &rect[y*w*4];

		for (x=0; x<w; x++) {
			unsigned char *pxl= &row[x*4];

			if ((x % xoffs < ICON_GRID_MARGIN-2) || (x % xoffs > ICON_GRID_W+2))
				pxl[3] = 0;	//alpha channel == x+3
			else if ((y % yoffs < ICON_GRID_MARGIN-2) || (y % yoffs > ICON_GRID_H+2))
				pxl[3] = 0;
		}
	}
}

static void prepare_internal_icons(ImBuf *bbuf)
{

	char *back= (char *)bbuf->rect;
	
	/* this sets the icon grid margin area outside of icon to zero alpha */
	clear_icon_grid_margins(back, bbuf->x, bbuf->y);
	
	/* hack! */
#if 0	
	for (y=0; y<12; y++) {
		for (x=0; x<21; x++) {
			unsigned char *start= ((unsigned char*) bbuf->rect) + (y*21 + 3)*rowstride + (x*20 + 3)*4;
			/* this sets backdrop of icon to zero alpha */
			transp[0]= start[0];
			transp[1]= start[1];
			transp[2]= start[2];
			transp[3]= start[3];
			clear_transp_rect(transp, start, 20, 21, rowstride);
			clear_transp_rect_soft(transp, start, 20, 21, rowstride);
				
		}
	} 
#endif
}


static void init_internal_icons()
{
	bTheme *btheme= U.themes.first;
	ImBuf *bbuf;
	int x, y;
	char iconfilestr[FILE_MAXDIR+FILE_MAXFILE];
	char filenamestr[FILE_MAXFILE+16];	// 16 == strlen(".blender/icons/")+1
	
	if ((btheme!=NULL) && (strlen(btheme->tui.iconfile) > 0)) {
	
#ifdef WIN32
		sprintf(filenamestr, "icons/%s", btheme->tui.iconfile);
#else
		sprintf(filenamestr, ".blender/icons/%s", btheme->tui.iconfile);
#endif
		
		BLI_make_file_string("/", iconfilestr, BLI_gethome(), filenamestr);
		
		if (BLI_exists(iconfilestr)) {
			bbuf = IMB_loadiffname(iconfilestr, IB_rect);
		} else {
			bbuf = IMB_ibImageFromMemory((int *)datatoc_blenderbuttons, datatoc_blenderbuttons_size, IB_rect);
		}
	} else {
		bbuf = IMB_ibImageFromMemory((int *)datatoc_blenderbuttons, datatoc_blenderbuttons_size, IB_rect);
	}
	
	prepare_internal_icons(bbuf);

	for (y=0; y<ICON_GRID_ROWS; y++) {
		for (x=0; x<ICON_GRID_COLS; x++) {
			def_internal_icon(bbuf, BIFICONID_FIRST + y*ICON_GRID_COLS + x,
				x*(ICON_GRID_W+ICON_GRID_MARGIN)+3,
				y*(ICON_GRID_H+ICON_GRID_MARGIN)+3);
		}
	}

	def_internal_vicon(VICON_VIEW3D, vicon_view3d_draw);
	def_internal_vicon(VICON_EDIT, vicon_edit_draw);
	def_internal_vicon(VICON_EDITMODE_DEHLT, vicon_editmode_dehlt_draw);
	def_internal_vicon(VICON_EDITMODE_HLT, vicon_editmode_hlt_draw);
	def_internal_vicon(VICON_DISCLOSURE_TRI_RIGHT, vicon_disclosure_tri_right_draw);
	def_internal_vicon(VICON_DISCLOSURE_TRI_DOWN, vicon_disclosure_tri_down_draw);
	def_internal_vicon(VICON_MOVE_UP, vicon_move_up_draw);
	def_internal_vicon(VICON_MOVE_DOWN, vicon_move_down_draw);
	def_internal_vicon(VICON_X, vicon_x_draw);

	IMB_freeImBuf(bbuf);
}


static void init_iconfile_list(struct ListBase *list)
{
	IconFile *ifile;
	ImBuf *bbuf= NULL;
	struct direntry *dir;
	int restoredir = 1; /* restore to current directory */
	int totfile, i, index=1;
	int ifilex, ifiley;
	char icondirstr[FILE_MAXDIR];
	char iconfilestr[FILE_MAXDIR+FILE_MAXFILE];
	char olddir[FILE_MAXDIR+FILE_MAXFILE];
	
	list->first = list->last = NULL;

#ifdef WIN32
	BLI_make_file_string("/", icondirstr, BLI_gethome(), "icons");
#else
	BLI_make_file_string("/", icondirstr, BLI_gethome(), ".blender/icons");
#endif
	
	if(BLI_exists(icondirstr)==0)
		return;
	
	/* since BLI_getdir changes the current working directory, restore it 
	   back to old value afterwards */
	if(!BLI_getwdN(olddir)) 
		restoredir = 0;
	totfile = BLI_getdir(icondirstr, &dir);
	if (restoredir)
		chdir(olddir);

	for(i=0; i<totfile; i++) {
		if( (dir[i].type & S_IFREG) ) {
			char *filename = dir[i].relname;
			
			if(BLI_testextensie(filename, ".png")) {
			
				/* check to see if the image is the right size, continue if not */
				sprintf(iconfilestr, "%s/%s", icondirstr, filename);
				if(BLI_exists(iconfilestr)) bbuf = IMB_loadiffname(iconfilestr, IB_rect);
				
				ifilex = bbuf->x;
				ifiley = bbuf->y;
				IMB_freeImBuf(bbuf);
				
				if ((ifilex != ICON_IMAGE_W) || (ifiley != ICON_IMAGE_H))
					continue;
			
				/* found a potential icon file, so make an entry for it in the cache list */
				ifile = MEM_callocN(sizeof(IconFile), "IconFile");
				
				BLI_strncpy(ifile->filename, filename, sizeof(ifile->filename));
				ifile->index = index;

				BLI_addtail(list, ifile);
				
				index++;
			}
		}
	}
	
	/* free temporary direntry structure that's been created by BLI_getdir() */
	i= totfile-1;
	
	for(; i>=0; i--){
		MEM_freeN(dir[i].relname);
		if (dir[i].string) MEM_freeN(dir[i].string);
	}
	free(dir);
	dir= 0;
}

static void free_iconfile_list(struct ListBase *list)
{
	IconFile *ifile=NULL, *next_ifile=NULL;
	
	for(ifile=list->first; ifile; ifile=next_ifile) {
		next_ifile = ifile->next;
		BLI_freelinkN(list, ifile);
	}
}

int BIF_iconfile_get_index(char *filename)
{
	IconFile *ifile;
	ListBase *list=&(iconfilelist);
	
	for(ifile=list->first; ifile; ifile=ifile->next) {
		if ( BLI_streq(filename, ifile->filename)) {
			return ifile->index;
		}
	}
	
	return 0;
}

ListBase *BIF_iconfile_list(void)
{
	ListBase *list=&(iconfilelist);
	
	return list;
}


void BIF_icons_free()
{
	free_iconfile_list(&iconfilelist);
	BKE_icons_free();
}

void BIF_icons_free_drawinfo(void *drawinfo)
{
	DrawInfo *di = drawinfo;

	if (di)
	{
		MEM_freeN(di->rect);
		MEM_freeN(di);
	}
}

static DrawInfo *icon_create_drawinfo()
{
	DrawInfo *di = NULL;

	di = MEM_callocN(sizeof(DrawInfo), "di_icon");
	
	di->drawFunc = 0;
	di->w = 16;
	di->h = 16;
	di->rw = ICON_RENDERSIZE;
	di->rh = ICON_RENDERSIZE;
	di->rect = 0;
	di->aspect = 1.0f;

	return di;
}

int BIF_icon_get_width(int icon_id)
{
	Icon *icon = NULL;
	DrawInfo *di = NULL;

	icon = BKE_icon_get(icon_id);
	
	if (!icon) {
		printf("BIF_icon_get_width: Internal error, no icon for icon ID: %d\n", icon_id);
		return 0;
	}
	
	di = (DrawInfo *)icon->drawinfo;
	if (!di) {
		di = icon_create_drawinfo();
		icon->drawinfo = di;
	}

	if (di)
		return di->w;

	return 0;
}

int BIF_icon_get_height(int icon_id)
{
	Icon *icon = NULL;
	DrawInfo *di = NULL;

	icon = BKE_icon_get(icon_id);
	
	if (!icon) {
		printf("BIF_icon_get_width: Internal error, no icon for icon ID: %d\n", icon_id);
		return 0;
	}
	
	di = (DrawInfo*)icon->drawinfo;

	if (!di) {
		di = icon_create_drawinfo();
		icon->drawinfo = di;
	}
	
	if (di)
		return di->h;

	return 0;
}

void BIF_icons_init(int first_dyn_id)
{
	init_iconfile_list(&iconfilelist);
	BKE_icons_init(first_dyn_id);
	init_internal_icons();
}

static void icon_copy_rect(ImBuf *ibuf, RenderInfo *ri)
{
	struct ImBuf *ima;
	unsigned int *drect, *srect;
	float scaledx, scaledy;
	short ex, ey, dx, dy;

	/* paranoia test */
	if(ibuf==NULL || (ibuf->rect==NULL && ibuf->rect_float==NULL))
		return;
	
	/* waste of cpu cyles... but the imbuf API has no other way to scale fast (ton) */
	ima = IMB_dupImBuf(ibuf);
	
	if (!ima) 
		return;
	
	if (ima->x > ima->y) {
		scaledx = (float)ri->pr_rectx;
		scaledy =  ( (float)ima->y/(float)ima->x )*(float)ri->pr_rectx;
	}
	else {			
		scaledx =  ( (float)ima->x/(float)ima->y )*(float)ri->pr_recty;
		scaledy = (float)ri->pr_recty;
	}
	
	ex = (short)scaledx;
	ey = (short)scaledy;
	
	dx = (ri->pr_rectx - ex) / 2;
	dy = (ri->pr_recty - ey) / 2;
	
	IMB_scalefastImBuf(ima, ex, ey);
	
	/* if needed, convert to 32 bits */
	if(ima->rect==NULL)
		IMB_rect_from_float(ima);

	srect = ima->rect;
	drect = ri->rect;

	drect+= dy*ri->pr_rectx+dx;
	for (;ey > 0; ey--){		
		memcpy(drect,srect, ex * sizeof(int));
		drect += ri->pr_rectx;
		srect += ima->x;
	}
	IMB_freeImBuf(ima);
}

/* create single icon from jpg, png etc. */
static void icon_from_image(Image *img, RenderInfo *ri)
{
	unsigned int pr_size = ri->pr_rectx*ri->pr_recty*sizeof(unsigned int);

	/* img->ok is zero when Image cannot load */
	if (img==NULL || img->ok==0)
		return;
	
	if (!ri->rect) {
		ri->rect= MEM_callocN(pr_size, "butsrect");
		memset(ri->rect, 0x00, pr_size);
	}
	
	/* we only load image if there's no preview saved already ... 
		always loading and reducing images is too expensive */
	/* new rule: never read images, so icons get created while user works, 
	    not always on first use of a menu */
	if(!img->preview) {
		ImBuf *ibuf;
		if(img->ok!=IMA_OK_LOADED) {				
			return;
		}
		ibuf= BKE_image_get_ibuf(img, NULL);
		icon_copy_rect(ibuf, ri);
		
		/* now copy the created preview to the DNA struct to be saved in file */
		img->preview = MEM_callocN(sizeof(PreviewImage), "img_prv");
		if (img->preview) {
			printf("created image prv\n");
			img->preview->w = ri->pr_rectx;
			img->preview->h = ri->pr_recty;
			img->preview->rect = MEM_callocN(pr_size, "prv_rect");
			memcpy(img->preview->rect, ri->rect, pr_size);
		}
	}
	else {
		unsigned int img_prv_size = img->preview->w*img->preview->h*sizeof(unsigned int);
		if (!img->preview->rect || img_prv_size != pr_size) {
			printf("Missing preview or wrong preview size!\n");
			return;
		}
		memcpy(ri->rect, img->preview->rect, pr_size);
	}
}

static void set_alpha(char* cp, int sizex, int sizey, char alpha) 
{
	int x,y;
	for(y=0; y<sizey; y++) {
		for(x=0; x<sizex; x++, cp+=4) {
			cp[3]= alpha;
		}
	}
}

/* only called when icon has changed */
/* only call with valid pointer from BIF_icon_draw */
static void icon_set_image(ID *id, DrawInfo *di)
{
	RenderInfo ri;	

	if (!di) return;			

	if (!di->rect)
		di->rect = MEM_callocN(di->rw*di->rh*sizeof(unsigned int), "laprevrect");		
	
	ri.curtile= 0;
	ri.tottile= 0;
	ri.rect = NULL;
	ri.pr_rectx = di->rw;
	ri.pr_recty = di->rh;

	/* no drawing (see last parameter doDraw, just calculate preview image 
		- hopefully small enough to be fast */
	if (GS(id->name) == ID_IM)
		icon_from_image((struct Image*)id, &ri);
	else {
		BIF_previewrender(id, &ri, NULL, PR_ICON_RENDER);
		/* world is rendered with alpha=0, so it wasn't displayed 
		   this could be render option for sky to, for later */
		if (GS(id->name) == ID_WO) { 
			set_alpha( (char*) ri.rect, ri.pr_rectx, ri.pr_recty, 255);
		} 
		else if (GS(id->name) == ID_MA) {
			Material* mat = (Material*)id;
			if (mat->mode & MA_HALO) {
				set_alpha( (char*) ri.rect, ri.pr_rectx, ri.pr_recty, 255);
			} 
		}
	}

	/* and copy the image into the icon */
	if (ri.rect) {
		memcpy(di->rect, ri.rect,di->rw*di->rh*sizeof(unsigned int));		

		/* and clean up */
		MEM_freeN(ri.rect);
		ri.rect = 0;
	}
}

void BIF_icon_draw_aspect(float x, float y, int icon_id, float aspect)
{
	Icon *icon = NULL;
	DrawInfo *di = NULL;
	
	icon = BKE_icon_get(icon_id);
	
	if (!icon) {
		printf("BIF_icon_set_aspect: Internal error, no icon for icon ID: %d\n", icon_id);
		return;
	}
	
	di = (DrawInfo*)icon->drawinfo;
	
	if (!di) {
		di = icon_create_drawinfo();
		
		icon->changed = 1; 
		icon->drawinfo = di;		
		icon->drawinfo_free = BIF_icons_free_drawinfo;		
	}
	
	di->aspect = aspect;
	/* scale width and height according to aspect */
	di->w = (int)(ICON_DEFAULT_HEIGHT/di->aspect + 0.5f);
	di->h = (int)(ICON_DEFAULT_HEIGHT/di->aspect + 0.5f);
	
	if (di->drawFunc) {
		/* vector icons use the uiBlock transformation, they are not drawn
		with untransformed coordinates like the other icons */
		di->drawFunc(x, y, ICON_DEFAULT_HEIGHT, ICON_DEFAULT_HEIGHT, 1.0f); 
	}
	else {
		if (icon->changed) /* changed only ever set by dynamic icons */
		{
			waitcursor(1);
			icon_set_image((ID*)icon->obj, icon->drawinfo);
			icon->changed = 0;		
			waitcursor(0);
		}
		
		if (!di->rect) return; /* something has gone wrong! */
		
		ui_rasterpos_safe(x, y, di->aspect);
		
		if(di->w<1 || di->h<1) {
			printf("what the heck!\n");
		}
		/* di->rect contains image in 'rendersize', we only scale if needed */
		else if(di->rw!=di->w && di->rh!=di->h) {
			ImBuf *ima;
			if(di->w>2000 || di->h>2000) { /* something has gone wrong! */
				printf("insane icon size di->w %d di->h %d\n",di->w,di->h);
				return;
			}
			/* first allocate imbuf for scaling and copy preview into it */
			ima = IMB_allocImBuf(di->rw, di->rh, 32, IB_rect, 0);
			memcpy(ima->rect, di->rect, di->rw*di->rh*sizeof(unsigned int));	
			
			/* scale it */
			IMB_scaleImBuf(ima, di->w, di->h);
			glDrawPixels(di->w, di->h, GL_RGBA, GL_UNSIGNED_BYTE, ima->rect);
			
			IMB_freeImBuf(ima);
		}
		else
			glDrawPixels(di->w, di->h, GL_RGBA, GL_UNSIGNED_BYTE, di->rect);
	}
}

void BIF_icon_draw(float x, float y, int icon_id)
{
	BIF_icon_draw_aspect(x, y, icon_id, 1.0f);
}


void BIF_icon_draw_aspect_blended(float x, float y, int icon_id, float aspect, int shade)
{
	
	if(shade < 0) {
		float r= (128+shade)/128.0f;
		glPixelTransferf(GL_ALPHA_SCALE, r);
	}

	BIF_icon_draw_aspect(x, y, icon_id, aspect);

	if(shade < 0)
		glPixelTransferf(GL_ALPHA_SCALE, 1.0f);
}
