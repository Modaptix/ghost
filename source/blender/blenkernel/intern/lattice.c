/**
 * lattice.c
 *
 *
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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "MEM_guardedalloc.h"

#include "BLI_blenlib.h"
#include "BLI_arithb.h"

#include "DNA_armature_types.h"
#include "DNA_mesh_types.h"
#include "DNA_meshdata_types.h"
#include "DNA_object_types.h"
#include "DNA_scene_types.h"
#include "DNA_lattice_types.h"
#include "DNA_curve_types.h"
#include "DNA_key_types.h"

#include "BKE_anim.h"
#include "BKE_armature.h"
#include "BKE_curve.h"
#include "BKE_deform.h"
#include "BKE_displist.h"
#include "BKE_global.h"
#include "BKE_key.h"
#include "BKE_lattice.h"
#include "BKE_library.h"
#include "BKE_main.h"
#include "BKE_object.h"
#include "BKE_screen.h"
#include "BKE_utildefines.h"

#include "BIF_editdeform.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "blendef.h"

Lattice *editLatt=0, *deformLatt=0;

float *latticedata=0, latmat[4][4];

void resizelattice(Lattice *lt)
{
	BPoint *bp;
	int u, v, w;
	float vec[3], fu, fv, fw, du=0.0, dv=0.0, dw=0.0;
	

	MEM_freeN(lt->def);
	lt->def= MEM_callocN(lt->pntsu*lt->pntsv*lt->pntsw*sizeof(BPoint), "lattice bp");
	
	bp= lt->def;
	
	while(lt->pntsu*lt->pntsv*lt->pntsw > 32000) {
		if( lt->pntsu>=lt->pntsv && lt->pntsu>=lt->pntsw) lt->pntsu--;
		else if( lt->pntsv>=lt->pntsu && lt->pntsv>=lt->pntsw) lt->pntsv--;
		else lt->pntsw--;
	}
	
	calc_lat_fudu(lt->flag, lt->pntsu, &fu, &du);
	calc_lat_fudu(lt->flag, lt->pntsv, &fv, &dv);
	calc_lat_fudu(lt->flag, lt->pntsw, &fw, &dw);
	
	vec[2]= fw;
	for(w=0; w<lt->pntsw; w++) {
		vec[1]= fv;
		for(v=0; v<lt->pntsv; v++) {
			vec[0]= fu;
			for(u=0; u<lt->pntsu; u++, bp++) {
				VECCOPY(bp->vec, vec);
				vec[0]+= du;
			}
			vec[1]+= dv;
		}
		vec[2]+= dw;
	}
}

Lattice *add_lattice()
{
	Lattice *lt;
	
	lt= alloc_libblock(&G.main->latt, ID_LT, "Lattice");
	
	lt->pntsu=lt->pntsv=lt->pntsw= 2;
	lt->flag= LT_GRID;
	
	lt->typeu= lt->typev= lt->typew= KEY_BSPLINE;
	
	/* temporally */
	lt->def= MEM_callocN(sizeof(BPoint), "lattvert");
	
	resizelattice(lt);	/* creates a uniform lattice */
		
	return lt;
}

Lattice *copy_lattice(Lattice *lt)
{
	Lattice *ltn;

	ltn= copy_libblock(lt);
	ltn->def= MEM_dupallocN(lt->def);
		
	id_us_plus((ID *)ltn->ipo);

	ltn->key= copy_key(ltn->key);
	if(ltn->key) ltn->key->from= (ID *)ltn;
	
	return ltn;
}

void free_lattice(Lattice *lt)
{
	if(lt->def) MEM_freeN(lt->def);
}


void make_local_lattice(Lattice *lt)
{
	Object *ob;
	Lattice *ltn;
	int local=0, lib=0;

	/* - only lib users: do nothing
	 * - only local users: set flag
	 * - mixed: make copy
	 */
	
	if(lt->id.lib==0) return;
	if(lt->id.us==1) {
		lt->id.lib= 0;
		lt->id.flag= LIB_LOCAL;
		new_id(0, (ID *)lt, 0);
		return;
	}
	
	ob= G.main->object.first;
	while(ob) {
		if(ob->data==lt) {
			if(ob->id.lib) lib= 1;
			else local= 1;
		}
		ob= ob->id.next;
	}
	
	if(local && lib==0) {
		lt->id.lib= 0;
		lt->id.flag= LIB_LOCAL;
		new_id(0, (ID *)lt, 0);
	}
	else if(local && lib) {
		ltn= copy_lattice(lt);
		ltn->id.us= 0;
		
		ob= G.main->object.first;
		while(ob) {
			if(ob->data==lt) {
				
				if(ob->id.lib==0) {
					ob->data= ltn;
					ltn->id.us++;
					lt->id.us--;
				}
			}
			ob= ob->id.next;
		}
	}
}



void calc_lat_fudu(int flag, int res, float *fu, float *du)
{
	
	if(res==1) {
		*fu= 0.0;
		*du= 0.0;
	}
	else if(flag & LT_GRID) {
		*fu= -0.5f*(res-1);
		*du= 1.0f;
	}
	else {
		*fu= -1.0f;
		*du= 2.0f/(res-1);
	}
	
}

void init_latt_deform(Object *oblatt, Object *ob)
{
	/* we make an array with all differences */
	BPoint *bp;
	float *fp, imat[4][4];
	float vec[3], fu, fv, fw, du=0.0, dv=0.0, dw=0.0;
	int u, v, w;
	
	if(oblatt==G.obedit) deformLatt= editLatt;
	else deformLatt= oblatt->data;
	
	fp= latticedata= MEM_mallocN(sizeof(float)*3*deformLatt->pntsu*deformLatt->pntsv*deformLatt->pntsw, "latticedata");
	
	lattice_modifier(oblatt, 's');
	bp= deformLatt->def;

	//if(ob) where_is_object(ob); causes lag here, but why! (ton)

	/* for example with a particle system: ob==0 */
	if(ob==0) {
		/* in deformspace, calc matrix  */
		Mat4Invert(latmat, oblatt->obmat);
	
		/* back: put in deform array */
		Mat4Invert(imat, latmat);
	}
	else {
		/* in deformspace, calc matrix */
		Mat4Invert(imat, oblatt->obmat);
		Mat4MulMat4(latmat, ob->obmat, imat);
	
		/* back: put in deform array */
		Mat4Invert(imat, latmat);
	}
	calc_lat_fudu(deformLatt->flag, deformLatt->pntsu, &fu, &du);
	calc_lat_fudu(deformLatt->flag, deformLatt->pntsv, &fv, &dv);
	calc_lat_fudu(deformLatt->flag, deformLatt->pntsw, &fw, &dw);
	
	/* we keep calculating the u v w lattice coordinates, not enough reason to store that */
	
	vec[2]= fw;
	for(w=0; w<deformLatt->pntsw; w++) {
		vec[1]= fv;
		for(v=0; v<deformLatt->pntsv; v++) {
			vec[0]= fu;
			for(u=0; u<deformLatt->pntsu; u++, bp++) {
				
				VecSubf(fp, bp->vec, vec);
				Mat4Mul3Vecfl(imat, fp);
		
				vec[0]+= du;
				fp+= 3;
			}
			vec[1]+= dv;
		}
		vec[2]+= dw;
	}

	lattice_modifier(oblatt, 'e');

}

void calc_latt_deform(float *co)
{
	Lattice *lt;
	float fu, du, u, v, w, tu[4], tv[4], tw[4];
	float *fpw, *fpv, *fpu, vec[3];
	int ui, vi, wi, uu, vv, ww;
	
	if(latticedata==0) return;
	
	lt= deformLatt;	/* just for shorter notation! */
	
	/* co is in local coords, treat with latmat */
	
	VECCOPY(vec, co);
	Mat4MulVecfl(latmat, vec);
	
	/* u v w coords */
	
	if(lt->pntsu>1) {
		calc_lat_fudu(lt->flag, lt->pntsu, &fu, &du);
		u= (vec[0]-fu)/du;
		ui= (int)floor(u);
		u -= ui;
		set_four_ipo(u, tu, lt->typeu);
	}
	else {
		tu[0]= tu[2]= tu[3]= 0.0; tu[1]= 1.0;
		ui= 0;
	}
	
	if(lt->pntsv>1) {
		calc_lat_fudu(lt->flag, lt->pntsv, &fu, &du);
		v= (vec[1]-fu)/du;
		vi= (int)floor(v);
		v -= vi;
		set_four_ipo(v, tv, lt->typev);
	}
	else {
		tv[0]= tv[2]= tv[3]= 0.0; tv[1]= 1.0;
		vi= 0;
	}
	
	if(lt->pntsw>1) {
		calc_lat_fudu(lt->flag, lt->pntsw, &fu, &du);
		w= (vec[2]-fu)/du;
		wi= (int)floor(w);
		w -= wi;
		set_four_ipo(w, tw, lt->typew);
	}
	else {
		tw[0]= tw[2]= tw[3]= 0.0; tw[1]= 1.0;
		wi= 0;
	}
	
	for(ww= wi-1; ww<=wi+2; ww++) {
		w= tw[ww-wi+1];
		
		if(w!=0.0) {
			if(ww>0) {
				if(ww<lt->pntsw) fpw= latticedata + 3*ww*lt->pntsu*lt->pntsv;
				else fpw= latticedata + 3*(lt->pntsw-1)*lt->pntsu*lt->pntsv;
			}
			else fpw= latticedata;
			
			for(vv= vi-1; vv<=vi+2; vv++) {
				v= w*tv[vv-vi+1];
				
				if(v!=0.0) {
					if(vv>0) {
						if(vv<lt->pntsv) fpv= fpw + 3*vv*lt->pntsu;
						else fpv= fpw + 3*(lt->pntsv-1)*lt->pntsu;
					}
					else fpv= fpw;
					
					for(uu= ui-1; uu<=ui+2; uu++) {
						u= v*tu[uu-ui+1];
						
						if(u!=0.0) {
							if(uu>0) {
								if(uu<lt->pntsu) fpu= fpv + 3*uu;
								else fpu= fpv + 3*(lt->pntsu-1);
							}
							else fpu= fpv;
							
							co[0]+= u*fpu[0];
							co[1]+= u*fpu[1];
							co[2]+= u*fpu[2];
						}
					}
				}
			}
		}
	}
}

void end_latt_deform()
{

	MEM_freeN(latticedata);
	latticedata= 0;
}

	/* calculations is in local space of deformed object
	   so we store in latmat transform from path coord inside object 
	 */
typedef struct {
	float dmin[3], dmax[3], dsize, dloc[3];
	float curvespace[4][4], objectspace[4][4];
} CurveDeform;

static void init_curve_deform(Object *par, Object *ob, CurveDeform *cd)
{
	Mat4Invert(ob->imat, ob->obmat);
	Mat4MulMat4(cd->objectspace, par->obmat, ob->imat);
	Mat4Invert(cd->curvespace, cd->objectspace);

	// offset vector for 'no smear'
	Mat4Invert(par->imat, par->obmat);
	VecMat4MulVecfl(cd->dloc, par->imat, ob->obmat[3]);

}

/* this makes sure we can extend for non-cyclic. *vec needs 4 items! */
static int where_on_path_deform(Object *ob, float ctime, float *vec, float *dir)	/* returns OK */
{
	Curve *cu= ob->data;
	BevList *bl;
	float ctime1;
	int cycl=0;
	
	/* test for cyclic */
	bl= cu->bev.first;
	if(bl && bl->poly> -1) cycl= 1;

	if(cycl==0) {
		ctime1= CLAMPIS(ctime, 0.0, 1.0);
	}
	else ctime1= ctime;
	
	/* vec needs 4 items */
	if(where_on_path(ob, ctime1, vec, dir)) {
		
		if(cycl==0) {
			Path *path= cu->path;
			float dvec[3];
			
			if(ctime < 0.0) {
				VecSubf(dvec, path->data+4, path->data);
				VecMulf(dvec, ctime*(float)path->len);
				VECADD(vec, vec, dvec);
			}
			else if(ctime > 1.0) {
				VecSubf(dvec, path->data+4*path->len-4, path->data+4*path->len-8);
				VecMulf(dvec, (ctime-1.0)*(float)path->len);
				VECADD(vec, vec, dvec);
			}
		}
		return 1;
	}
	return 0;
}

	/* for each point, rotate & translate to curve */
	/* use path, since it has constant distances */
	/* co: local coord, result local too */
static void calc_curve_deform(Object *par, float *co, short axis, CurveDeform *cd)
{
	Curve *cu= par->data;
	float fac, loc[4], dir[3], *quat, q[4], mat[3][3], cent[3];
	short upflag, index;
	
	if(axis==OB_POSX || axis==OB_NEGX) {
		upflag= OB_POSZ;
		cent[0]= 0.0;
		cent[1]= co[1];
		cent[2]= co[2];
		index= 0;
	}
	else if(axis==OB_POSY || axis==OB_NEGY) {
		upflag= OB_POSZ;
		cent[0]= co[0];
		cent[1]= 0.0;
		cent[2]= co[2];
		index= 1;
	}
	else {
		upflag= OB_POSY;
		cent[0]= co[0];
		cent[1]= co[1];
		cent[2]= 0.0;
		index= 2;
	}
	/* to be sure */
	if(cu->path==NULL) {
		calc_curvepath(par);
		if(cu->path==NULL) return;	// happens on append...
	}
	/* options */
	if(cu->flag & CU_STRETCH)
		fac= (co[index]-cd->dmin[index])/(cd->dmax[index] - cd->dmin[index]);
	else
		fac= (cd->dloc[index])/(cu->path->totdist) + (co[index]-cd->dmin[index])/(cu->path->totdist);
	
	if( where_on_path_deform(par, fac, loc, dir)) {	/* returns OK */

		quat= vectoquat(dir, axis, upflag);
		
		/* the tilt */
		if(loc[3]!=0.0) {
			Normalise(dir);
			q[0]= (float)cos(0.5*loc[3]);
			fac= (float)sin(0.5*loc[3]);
			q[1]= -fac*dir[0];
			q[2]= -fac*dir[1];
			q[3]= -fac*dir[2];
			QuatMul(quat, q, quat);
		}		
		QuatToMat3(quat, mat);
	
		/* local rotation */
		Mat3MulVecfl(mat, cent);
		
		/* translation */
		VECADD(co, cent, loc);
		
	}

}

void curve_deform_verts(Object *cuOb, Object *target, float (*vertexCos)[3], int numVerts)
{
	Curve *cu = cuOb->data;
	int a, flag = cu->flag;
	CurveDeform cd;
	
	cu->flag |= (CU_PATH|CU_FOLLOW); // needed for path & bevlist

	init_curve_deform(cuOb, target, &cd);
		
	INIT_MINMAX(cd.dmin, cd.dmax);
		
	for(a=0; a<numVerts; a++) {
		Mat4MulVecfl(cd.curvespace, vertexCos[a]);
		DO_MINMAX(vertexCos[a], cd.dmin, cd.dmax);
	}

	for(a=0; a<numVerts; a++) {
		calc_curve_deform(cuOb, vertexCos[a], target->trackflag, &cd);
		Mat4MulVecfl(cd.objectspace, vertexCos[a]);
	}

	cu->flag = flag;
}

void lattice_deform_verts(Object *laOb, Object *target, float (*vertexCos)[3], int numVerts)
{
	int a;

	init_latt_deform(laOb, target);

	for(a=0; a<numVerts; a++) {
		calc_latt_deform(vertexCos[a]);
	}

	end_latt_deform();
}

void armature_deform_verts(Object *armOb, Object *target, float (*vertexCos)[3], int numVerts) 
{
	int a;

	init_armature_deform(armOb, target);

	for(a=0; a<numVerts; a++) {
		calc_armature_deform(armOb, vertexCos[a], a);
	}
}

int object_deform(Object *ob)
{
	Curve *cu;
	DispList *dl;
	float *fp;
	int a, tot;

	if(ob->parent==NULL) return 0;
	
	if(ob->parent->type==OB_LATTICE) {
		init_latt_deform(ob->parent, ob);
		
		if(ob->type==OB_MBALL) {
			dl=ob->disp.first;
			while(dl) {
				fp = dl->verts;
				for(a=0;a<dl->nr;a++,fp+=3)
					calc_latt_deform(fp);
				dl=dl->next;
			}
		}
		else if ELEM(ob->type, OB_CURVE, OB_SURF) {
			cu= ob->data;
			/* apply deform on displist */
			dl= cu->disp.first;
			while(dl) {
				
				fp= dl->verts;
				
				if(dl->type==DL_INDEX3) tot=dl->parts;
				else tot= dl->nr*dl->parts;
				
				for(a=0; a<tot; a++, fp+=3) {
					calc_latt_deform(fp);
				}
				
				dl= dl->next;
			}
		}
		end_latt_deform();
		return 1;
	}
	
	return 0;

}

BPoint *latt_bp(Lattice *lt, int u, int v, int w)
{
	return lt->def+ u + v*lt->pntsu + w*lt->pntsu*lt->pntsv;
}

void outside_lattice(Lattice *lt)
{
	BPoint *bp, *bp1, *bp2;
	int u, v, w;
	float fac1, du=0.0, dv=0.0, dw=0.0;

	bp= lt->def;

	if(lt->pntsu>1) du= 1.0f/((float)lt->pntsu-1);
	if(lt->pntsv>1) dv= 1.0f/((float)lt->pntsv-1);
	if(lt->pntsw>1) dw= 1.0f/((float)lt->pntsw-1);
		
	for(w=0; w<lt->pntsw; w++) {
		
		for(v=0; v<lt->pntsv; v++) {
		
			for(u=0; u<lt->pntsu; u++, bp++) {
				if(u==0 || v==0 || w==0 || u==lt->pntsu-1 || v==lt->pntsv-1 || w==lt->pntsw-1);
				else {
				
					bp->hide= 1;
					bp->f1 &= ~SELECT;
					
					/* u extrema */
					bp1= latt_bp(lt, 0, v, w);
					bp2= latt_bp(lt, lt->pntsu-1, v, w);
					
					fac1= du*u;
					bp->vec[0]= (1.0f-fac1)*bp1->vec[0] + fac1*bp2->vec[0];
					bp->vec[1]= (1.0f-fac1)*bp1->vec[1] + fac1*bp2->vec[1];
					bp->vec[2]= (1.0f-fac1)*bp1->vec[2] + fac1*bp2->vec[2];
					
					/* v extrema */
					bp1= latt_bp(lt, u, 0, w);
					bp2= latt_bp(lt, u, lt->pntsv-1, w);
					
					fac1= dv*v;
					bp->vec[0]+= (1.0f-fac1)*bp1->vec[0] + fac1*bp2->vec[0];
					bp->vec[1]+= (1.0f-fac1)*bp1->vec[1] + fac1*bp2->vec[1];
					bp->vec[2]+= (1.0f-fac1)*bp1->vec[2] + fac1*bp2->vec[2];
					
					/* w extrema */
					bp1= latt_bp(lt, u, v, 0);
					bp2= latt_bp(lt, u, v, lt->pntsw-1);
					
					fac1= dw*w;
					bp->vec[0]+= (1.0f-fac1)*bp1->vec[0] + fac1*bp2->vec[0];
					bp->vec[1]+= (1.0f-fac1)*bp1->vec[1] + fac1*bp2->vec[1];
					bp->vec[2]+= (1.0f-fac1)*bp1->vec[2] + fac1*bp2->vec[2];
					
					VecMulf(bp->vec, 0.3333333f);
					
				}
			}
			
		}
		
	}
	
}
