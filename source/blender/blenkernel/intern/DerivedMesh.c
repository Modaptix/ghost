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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "MEM_guardedalloc.h"

#include "DNA_effect_types.h"
#include "DNA_mesh_types.h"
#include "DNA_meshdata_types.h"
#include "DNA_object_types.h"

#include "BLI_blenlib.h"
#include "BLI_editVert.h"

#include "BKE_DerivedMesh.h"
#include "BKE_displist.h"
#include "BKE_effect.h"
#include "BKE_global.h"
#include "BKE_mesh.h"
#include "BKE_object.h"

#include "BIF_gl.h"
#include "BIF_glutil.h"

///////////////////////////////////
///////////////////////////////////

typedef struct {
	DerivedMesh dm;

	Object *ob;
	float *extverts, *nors;
} MeshDerivedMesh;

static void meshDM_drawVerts(DerivedMesh *dm)
{
	MeshDerivedMesh *mdm = (MeshDerivedMesh*) dm;
	Mesh *me = mdm->ob->data;
	int a, start=0, end=me->totvert;
	MVert *mvert = me->mvert;
	float *extverts = mdm->extverts;

	set_buildvars(mdm->ob, &start, &end);

	glBegin(GL_POINTS);
	if(extverts) {
		extverts+= 3*start;
		for(a= start; a<end; a++, extverts+=3) {
			glVertex3fv(extverts);
		}
	}
	else {
		mvert+= start;
		for(a= start; a<end; a++, mvert++) {
			glVertex3fv(mvert->co);
		}
	}
	glEnd();
}
static void meshDM_drawEdges(DerivedMesh *dm)
{
	MeshDerivedMesh *mdm = (MeshDerivedMesh*) dm;
	Mesh *me= mdm->ob->data;
	int a, start= 0, end= me->totface;
	MVert *mvert = me->mvert;
	MFace *mface = me->mface;
	float *extverts= mdm->extverts;
	float *f1, *f2, *f3, *f4;

	set_buildvars(mdm->ob, &start, &end);
	mface+= start;
	
		// edges can't cope with buildvars, draw with
		// faces if build is in use.
	if(me->medge && start==0 && end==me->totface) {
		MEdge *medge= me->medge;
		
		glBegin(GL_LINES);
		for(a=me->totedge; a>0; a--, medge++) {
			if(medge->flag & ME_EDGEDRAW) {
				if(extverts) {
					f1= extverts+3*medge->v1;
					f2= extverts+3*medge->v2;
				}
				else {
					f1= (mvert+medge->v1)->co;
					f2= (mvert+medge->v2)->co;
				}
				glVertex3fv(f1); glVertex3fv(f2); 
			}
		}
		glEnd();
	}
	else {
		glBegin(GL_LINES);
		for(a=start; a<end; a++, mface++) {
			int test= mface->edcode;
			
			if(test) {
				if(extverts) {
					f1= extverts+3*mface->v1;
					f2= extverts+3*mface->v2;
				}
				else {
					f1= (mvert+mface->v1)->co;
					f2= (mvert+mface->v2)->co;
				}
				
				if(test&ME_V1V2){
					glVertex3fv(f1); glVertex3fv(f2);
				}

				if(mface->v3) {
					if(extverts) f3= extverts+3*mface->v3;
					else f3= (mvert+mface->v3)->co;

					if(test&ME_V2V3){
						glVertex3fv(f2); glVertex3fv(f3);
					}

					if (mface->v4) {
						if(extverts) f4= extverts+3*mface->v4;
						else f4= (mvert+mface->v4)->co;
						
						if(test&ME_V3V4){
							glVertex3fv(f3); glVertex3fv(f4);
						}
						if(test&ME_V4V1){
							glVertex3fv(f4); glVertex3fv(f1);
						}
					} else {
						if(test&ME_V3V1){
							glVertex3fv(f3); glVertex3fv(f1);
						}
					}
				}
			}
		}
		glEnd();
	}
}
static void meshDM_drawLooseEdges(DerivedMesh *dm)
{
	MeshDerivedMesh *mdm = (MeshDerivedMesh*) dm;
	Mesh *me = mdm->ob->data;
	MVert *mvert= me->mvert;
	MFace *mface= me->mface;
	float *extverts = mdm->extverts;
	int a, start=0, end=me->totface;

	set_buildvars(mdm->ob, &start, &end);
	mface+= start;
		
	glBegin(GL_LINES);
	for(a=start; a<end; a++, mface++) {
		if(!mface->v3) {
			float *v1, *v2;

			if(extverts) {
				v1= extverts+3*mface->v1;
				v2= extverts+3*mface->v2;
			}
			else {
				v1= (mvert+mface->v1)->co;
				v2= (mvert+mface->v2)->co;
			}
				
			glVertex3fv(v1);
			glVertex3fv(v2);
		} 
	}
	glEnd();
}
static void meshDM_drawFacesSolid(DerivedMesh *dm, void (*setMaterial)(int))
{
	MeshDerivedMesh *mdm = (MeshDerivedMesh*) dm;
	Mesh *me = mdm->ob->data;
	MVert *mvert= me->mvert;
	MFace *mface= me->mface;
	float *extverts = mdm->extverts;
	float *nors = mdm->nors;
	int a, start=0, end=me->totface;
	int glmode=-1, shademodel=-1, matnr=-1;

	set_buildvars(mdm->ob, &start, &end);
	mface+= start;
	
#define PASSVERT(co, index, punoBit) {			\
	if (shademodel==GL_SMOOTH) {				\
		short *no = (mvert+index)->no;			\
		if (mface->puno&punoBit) {				\
			glNormal3s(-no[0], -no[1], -no[2]); \
		} else {								\
			glNormal3sv(no);					\
		}										\
	}											\
	glVertex3fv(co);							\
}

	glBegin(glmode=GL_QUADS);
	for(a=start; a<end; a++, mface++, nors+=3) {
		if(mface->v3) {
			int new_glmode, new_matnr, new_shademodel;
			float *v1, *v2, *v3, *v4;

			if(extverts) {
				v1= extverts+3*mface->v1;
				v2= extverts+3*mface->v2;
				v3= mface->v3?extverts+3*mface->v3:NULL;
				v4= mface->v4?extverts+3*mface->v4:NULL;
			}
			else {
				v1= (mvert+mface->v1)->co;
				v2= (mvert+mface->v2)->co;
				v3= mface->v3?(mvert+mface->v3)->co:NULL;
				v4= mface->v4?(mvert+mface->v4)->co:NULL;
			}
				
			new_glmode = v4?GL_QUADS:GL_TRIANGLES;
			new_matnr = mface->mat_nr+1;
			new_shademodel = (!(me->flag&ME_AUTOSMOOTH) && (mface->flag & ME_SMOOTH))?GL_SMOOTH:GL_FLAT;
			
			if (new_glmode!=glmode || new_matnr!=matnr || new_shademodel!=shademodel) {
				glEnd();

				setMaterial(matnr=new_matnr);

				glShadeModel(shademodel=new_shademodel);
				glBegin(glmode=new_glmode);
			}
				
			if(shademodel==GL_FLAT) 
				glNormal3fv(nors);

			PASSVERT(v1, mface->v1, ME_FLIPV1);
			PASSVERT(v2, mface->v2, ME_FLIPV2);
			PASSVERT(v3, mface->v3, ME_FLIPV3);
			if (v4) {
				PASSVERT(v4, mface->v4, ME_FLIPV4);
			}
		}
	}
	glEnd();

	glShadeModel(GL_FLAT);
#undef PASSVERT
}

static void meshDM_drawFacesColored(DerivedMesh *dm, int useTwoSide, unsigned char *col1, unsigned char *col2)
{
	MeshDerivedMesh *mdm = (MeshDerivedMesh*) dm;
	Object *ob= mdm->ob;
	Mesh *me= ob->data;
	MVert *mvert= me->mvert;
	MFace *mface= me->mface;
	float *extverts= mdm->extverts;
	int a, glmode, start=0, end=me->totface;
	unsigned char *cp1, *cp2;

	set_buildvars(ob, &start, &end);
	mface+= start;
	col1+= 4*start;
	if(col2) col2+= 4*start;
	
	cp1= col1;
	if(col2) {
		cp2= col2;
	} else {
		cp2= NULL;
		useTwoSide= 0;
	}

	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glBegin(glmode=GL_QUADS);
	for(a=start; a<end; a++, mface++, cp1+= 16) {
		if(mface->v3) {
			int new_glmode= mface->v4?GL_QUADS:GL_TRIANGLES;
			float *v1, *v2, *v3, *v4;

			if(extverts) {
				v1= extverts+3*mface->v1;
				v2= extverts+3*mface->v2;
				v3= extverts+3*mface->v3;
				v4= mface->v4?extverts+3*mface->v4:NULL;
			}
			else {
				v1= (mvert+mface->v1)->co;
				v2= (mvert+mface->v2)->co;
				v3= (mvert+mface->v3)->co;
				v4= mface->v4?(mvert+mface->v4)->co:NULL;
			}

			if (new_glmode!=glmode) {
				glEnd();
				glBegin(glmode= new_glmode);
			}
				
			glColor3ub(cp1[3], cp1[2], cp1[1]);
			glVertex3fv( v1 );
			glColor3ub(cp1[7], cp1[6], cp1[5]);
			glVertex3fv( v2 );
			glColor3ub(cp1[11], cp1[10], cp1[9]);
			glVertex3fv( v3 );
			if(v4) {
				glColor3ub(cp1[15], cp1[14], cp1[13]);
				glVertex3fv( v4 );
			}
				
			if(useTwoSide) {
				glColor3ub(cp2[11], cp2[10], cp2[9]);
				glVertex3fv( v3 );
				glColor3ub(cp2[7], cp2[6], cp2[5]);
				glVertex3fv( v2 );
				glColor3ub(cp2[3], cp2[2], cp2[1]);
				glVertex3fv( v1 );
				if(mface->v4) {
					glColor3ub(cp2[15], cp2[14], cp2[13]);
					glVertex3fv( v4 );
				}
			}
		}
		if(col2) cp2+= 16;
	}
	glEnd();

	glShadeModel(GL_FLAT);
	glDisable(GL_CULL_FACE);
}

static int meshDM_getNumVerts(DerivedMesh *dm)
{
	MeshDerivedMesh *mdm = (MeshDerivedMesh*) dm;
	Mesh *me = mdm->ob->data;

	return me->totvert;
}
static int meshDM_getNumFaces(DerivedMesh *dm)
{
	MeshDerivedMesh *mdm = (MeshDerivedMesh*) dm;
	Mesh *me = mdm->ob->data;

	return me->totface;
}

static DispListMesh *meshDM_convertToDispListMesh(DerivedMesh *dm)
{
	MeshDerivedMesh *mdm= (MeshDerivedMesh*) dm;

	return displistmesh_from_mesh(mdm->ob->data, mdm->extverts);
}


static DerivedMesh *getMeshDerivedMesh(Object *ob, float *extverts, float *nors)
{
	MeshDerivedMesh *mdm = MEM_mallocN(sizeof(*mdm), "dm");

	mdm->dm.getNumVerts = meshDM_getNumVerts;
	mdm->dm.getNumFaces = meshDM_getNumFaces;
	mdm->dm.convertToDispListMesh = meshDM_convertToDispListMesh;

	mdm->dm.drawVerts = meshDM_drawVerts;
	mdm->dm.drawMappedVertEM = NULL;
	mdm->dm.drawMappedVertsEM = NULL;
	mdm->dm.drawMappedVertsEMSelect = NULL;

	mdm->dm.drawEdges = meshDM_drawEdges;
	mdm->dm.drawMappedEdges = meshDM_drawEdges;
	mdm->dm.drawLooseEdges = meshDM_drawLooseEdges;
	mdm->dm.drawMappedEdgeEM = NULL;
	mdm->dm.drawMappedEdgesEM = NULL;
	mdm->dm.drawMappedEdgesEMSelect = NULL;
	mdm->dm.drawMappedFacesEMSelect = NULL;

	mdm->dm.drawFacesSolid = meshDM_drawFacesSolid;
	mdm->dm.drawFacesColored = meshDM_drawFacesColored;
	mdm->dm.drawFacesEM = NULL;

	mdm->dm.release = MEM_freeN;
	
	mdm->ob = ob;
	mdm->extverts = extverts;
	mdm->nors = nors;

	return (DerivedMesh*) mdm;
}

///

typedef struct {
	DerivedMesh dm;

	EditMesh *em;
} EditMeshDerivedMesh;

static void emDM_getMappedVertCoEM(DerivedMesh *dm, void *vert, float co_r[3])
{
	EditVert *eve = vert;

	co_r[0] = eve->co[0];
	co_r[1] = eve->co[1];
	co_r[2] = eve->co[2];
}
static void emDM_drawMappedVertEM(DerivedMesh *dm, void *vert)
{
	EditVert *eve = vert;

	bglBegin(GL_POINTS);
	bglVertex3fv(eve->co);
	bglEnd();		
}
static void emDM_drawMappedVertsEM(DerivedMesh *dm, int sel)
{
	EditMeshDerivedMesh *emdm= (EditMeshDerivedMesh*) dm;
	EditVert *eve;

	bglBegin(GL_POINTS);
	for(eve= emdm->em->verts.first; eve; eve= eve->next) {
		if(eve->h==0 && (eve->f & SELECT)==sel ) 
			bglVertex3fv(eve->co);
	}
	bglEnd();		
}
static int emDM_drawMappedVertsEMSelect(DerivedMesh *dm, void (*setColor)(int index), int offset)
{
	EditMeshDerivedMesh *emdm= (EditMeshDerivedMesh*) dm;
	EditVert *eve;
	int i = offset;

	bglBegin(GL_POINTS);
	for(eve= emdm->em->verts.first; eve; eve= eve->next, i++) {
		if(eve->h==0) {
			setColor(i);
			bglVertex3fv(eve->co);
		}
	}
	bglEnd();
	
	return i;
}
static void emDM_drawMappedEdgeEM(DerivedMesh *dm, void *edge)
{
	EditMeshDerivedMesh *emdm= (EditMeshDerivedMesh*) dm;
	EditEdge *eed = edge;

	glBegin(GL_LINES);
	glVertex3fv(eed->v1->co);
	glVertex3fv(eed->v2->co);
	glEnd();
}
static void emDM_drawMappedEdgesEM(DerivedMesh *dm, int useColor, char *baseCol, char *selCol, int onlySeams) 
{
	EditMeshDerivedMesh *emdm= (EditMeshDerivedMesh*) dm;
	EditEdge *eed;

	glBegin(GL_LINES);
	for(eed= emdm->em->edges.first; eed; eed= eed->next) {
		if(eed->h==0 && (!onlySeams || eed->seam)) {
			if (useColor==1) {
				glColor4ubv((eed->f&SELECT)?selCol:baseCol);
			} else if (useColor==2) {
				glColor4ubv((eed->v1->f&SELECT)?selCol:baseCol);
			}
			glVertex3fv(eed->v1->co);
			if (useColor==2) {
				glColor4ubv((eed->v2->f&SELECT)?selCol:baseCol);
			}
			glVertex3fv(eed->v2->co);
		}
	}
	glEnd();
}
static int emDM_drawMappedEdgesEMSelect(DerivedMesh *dm, void (*setColor)(int index), int offset)
{
	EditMeshDerivedMesh *emdm= (EditMeshDerivedMesh*) dm;
	EditEdge *eed;
	int i;

	i = offset;
	glBegin(GL_LINES);
	for(eed= emdm->em->edges.first; eed; eed= eed->next, i++) {
		if (eed->h==0) {
			setColor(i);
			glVertex3fv(eed->v1->co);
			glVertex3fv(eed->v2->co);
		}
	}
	glEnd();

	return i;
}
static void emDM_drawFacesEM(DerivedMesh *dm, int useColor, unsigned char *baseCol, unsigned char *selCol)
{
	EditMeshDerivedMesh *emdm= (EditMeshDerivedMesh*) dm;
	EditFace *efa;

	for (efa= emdm->em->faces.first; efa; efa= efa->next) {
		if(efa->h==0) {
			if (useColor) {
				glColor4ubv((efa->f & SELECT)?selCol:baseCol);
			}

			glBegin(efa->v4?GL_QUADS:GL_TRIANGLES);
			glVertex3fv(efa->v1->co);
			glVertex3fv(efa->v2->co);
			glVertex3fv(efa->v3->co);
			if(efa->v4) glVertex3fv(efa->v4->co);
			glEnd();
		}
	}
}
static void emDM_drawFacesSolid(DerivedMesh *dm, void (*setMaterial)(int))
{
	EditMeshDerivedMesh *emdm= (EditMeshDerivedMesh*) dm;
	EditFace *efa;

	for (efa= emdm->em->faces.first; efa; efa= efa->next) {
		if(efa->h==0) {
			setMaterial(efa->mat_nr+1);

			glNormal3fv(efa->n);
			glBegin(efa->v4?GL_QUADS:GL_TRIANGLES);
			glVertex3fv(efa->v1->co);
			glVertex3fv(efa->v2->co);
			glVertex3fv(efa->v3->co);
			if(efa->v4) glVertex3fv(efa->v4->co);
			glEnd();
		}
	}
}
static int emDM_drawMappedFacesEMSelect(DerivedMesh *dm, void (*setColor)(int index), int offset)
{
	EditMeshDerivedMesh *emdm= (EditMeshDerivedMesh*) dm;
	EditFace *efa;
	int i = offset;

	for (efa= emdm->em->faces.first; efa; efa= efa->next, i++) {
		if (efa->h==0) {
			setColor(i);

			glBegin(efa->v4?GL_QUADS:GL_TRIANGLES);
			glVertex3fv(efa->v1->co);
			glVertex3fv(efa->v2->co);
			glVertex3fv(efa->v3->co);
			if(efa->v4) glVertex3fv(efa->v4->co);
			glEnd();
		}
	}

	return i;
}

static int emDM_getNumVerts(DerivedMesh *dm)
{
	EditMeshDerivedMesh *emdm= (EditMeshDerivedMesh*) dm;

	return BLI_countlist(&emdm->em->verts);
}
static int emDM_getNumFaces(DerivedMesh *dm)
{
	EditMeshDerivedMesh *emdm= (EditMeshDerivedMesh*) dm;

	return BLI_countlist(&emdm->em->faces);
}

static DispListMesh *emDM_convertToDispListMesh(DerivedMesh *dm)
{
	EditMeshDerivedMesh *emdm= (EditMeshDerivedMesh*) dm;

	return displistmesh_from_editmesh(emdm->em);
}

static DerivedMesh *getEditMeshDerivedMesh(EditMesh *em)
{
	EditMeshDerivedMesh *emdm = MEM_mallocN(sizeof(*emdm), "dm");

	emdm->dm.getNumVerts = emDM_getNumVerts;
	emdm->dm.getNumFaces = emDM_getNumFaces;
	emdm->dm.getMappedVertCoEM = emDM_getMappedVertCoEM;
	emdm->dm.convertToDispListMesh = emDM_convertToDispListMesh;

	emdm->dm.drawVerts = NULL;
	emdm->dm.drawMappedVertEM = emDM_drawMappedVertEM;
	emdm->dm.drawMappedVertsEM = emDM_drawMappedVertsEM;
	emdm->dm.drawMappedVertsEMSelect = emDM_drawMappedVertsEMSelect;

	emdm->dm.drawEdges = NULL;
	emdm->dm.drawMappedEdges = NULL;
	emdm->dm.drawLooseEdges = NULL;
	emdm->dm.drawMappedEdgeEM = emDM_drawMappedEdgeEM;
	emdm->dm.drawMappedEdgesEM = emDM_drawMappedEdgesEM;
	emdm->dm.drawMappedEdgesEMSelect = emDM_drawMappedEdgesEMSelect;
	emdm->dm.drawMappedFacesEMSelect = emDM_drawMappedFacesEMSelect;

	emdm->dm.drawFacesSolid = emDM_drawFacesSolid;
	emdm->dm.drawFacesColored = NULL;
	emdm->dm.drawFacesEM = emDM_drawFacesEM;

	emdm->dm.release = MEM_freeN;
	
	emdm->em = em;

	return (DerivedMesh*) emdm;
}

///

typedef struct {
	DerivedMesh dm;

	DispListMesh *dlm;
	EditMesh *em;
} SSDerivedMesh;

static void ssDM_getMappedVertCoEM(DerivedMesh *dm, void *vert, float co_r[3])
{
	EditVert *eve = vert;

	co_r[0] = eve->ssco[0];
	co_r[1] = eve->ssco[1];
	co_r[2] = eve->ssco[2];
}
static void ssDM_drawMappedVertEM(DerivedMesh *dm, void *vert)
{
	EditVert *eve = vert;

	bglBegin(GL_POINTS);
	bglVertex3fv(eve->ssco);
	bglEnd();		
}
static void ssDM_drawMappedVertsEM(DerivedMesh *dm, int sel)
{
	SSDerivedMesh *ssdm = (SSDerivedMesh*) dm;
	EditVert *eve;

	bglBegin(GL_POINTS);
	for (eve=ssdm->em->verts.first; eve; eve=eve->next) {
		if(eve->h==0 && (eve->f & SELECT)==sel && eve->ssco) 
			bglVertex3fv(eve->ssco);
	}
	bglEnd();
}
static int ssDM_drawMappedVertsEMSelect(DerivedMesh *dm, void (*setColor)(int index), int offset)
{
	SSDerivedMesh *ssdm = (SSDerivedMesh*) dm;
	EditVert *eve;
	int i = offset;

	bglBegin(GL_POINTS);
	for(eve= ssdm->em->verts.first; eve; eve= eve->next, i++) {
		if(eve->h==0 && eve->ssco) {
			setColor(i);
			bglVertex3fv(eve->ssco);
		}
	}
	bglEnd();
	
	return i;
}

static void ssDM_drawMappedEdges(DerivedMesh *dm)
{
	SSDerivedMesh *ssdm = (SSDerivedMesh*) dm;
	DispListMesh *dlm = ssdm->dlm;
	MEdge *medge= dlm->medge;
	MVert *mvert= dlm->mvert;
	int a;
	
	glBegin(GL_LINES);
	for (a=0; a<dlm->totedge; a++, medge++) {
		if (medge->flag&ME_EDGEDRAW) {
			glVertex3fv(mvert[medge->v1].co); 
			glVertex3fv(mvert[medge->v2].co);
		}
	}
	glEnd();
}

static void ssDM_drawLooseEdges(DerivedMesh *dm)
{
	/* Can't implement currently */ 
}

static void ssDM_drawMappedEdgeEM(DerivedMesh *dm, void *edge)
{
	SSDerivedMesh *ssdm = (SSDerivedMesh*) dm;
	DispListMesh *dlm = ssdm->dlm;
	MEdge *medge= dlm->medge;
	MVert *mvert= dlm->mvert;
	int a;

	glBegin(GL_LINES);
	for (a=0; a<dlm->totedge; a++, medge++) {
		if (medge->flag&ME_EDGEDRAW) {
			if (edge==dlm->editedge[a]) {
				glVertex3fv(mvert[medge->v1].co); 
				glVertex3fv(mvert[medge->v2].co);
			}
		}
	}
	glEnd();
}
static void ssDM_drawMappedEdgesEM(DerivedMesh *dm, int useColor, unsigned char *baseCol, unsigned char *selCol, int onlySeams) 
{
	SSDerivedMesh *ssdm = (SSDerivedMesh*) dm;
	DispListMesh *dlm = ssdm->dlm;
	MEdge *medge= dlm->medge;
	MVert *mvert= dlm->mvert;
	int a;
	
	glBegin(GL_LINES);
	for (a=0; a<dlm->totedge; a++, medge++) {
		if (medge->flag&ME_EDGEDRAW) {
			EditEdge *eed = dlm->editedge[a];

			if (eed && eed->h==0 && (!onlySeams || eed->seam)) {
				if (useColor) {
					glColor4ubv((eed->f&SELECT)?selCol:baseCol);
				}
				glVertex3fv(mvert[medge->v1].co); 
				glVertex3fv(mvert[medge->v2].co);
			}
		}
	}
	glEnd();
}
static int ssDM_drawMappedEdgesEMSelect(DerivedMesh *dm, void (*setColor)(int index), int offset)
{
	SSDerivedMesh *ssdm = (SSDerivedMesh*) dm;
	DispListMesh *dlm = ssdm->dlm;
	int i, endOffset;
	MVert *mvert= dlm->mvert;
	MEdge *medge= dlm->medge;
	EditEdge *eed;

	i = offset;
	for(eed= ssdm->em->edges.first; eed; eed= eed->next)
		eed->vn= (EditVert*) i++;
	endOffset = i;

	glBegin(GL_LINES);
	for (i=0; i<dlm->totedge; i++, medge++) {
		if(medge->flag & ME_EDGEDRAW) {
			EditEdge *eed= dlm->editedge[i];
			if(eed && eed->h==0) {
				setColor((int) eed->vn);
				glVertex3fv(mvert[medge->v1].co); 
				glVertex3fv(mvert[medge->v2].co);
			}
		}
	}
	glEnd();

	return endOffset;
}
static void ssDM_drawVerts(DerivedMesh *dm)
{
	SSDerivedMesh *ssdm = (SSDerivedMesh*) dm;
	DispListMesh *dlm = ssdm->dlm;
	MVert *mvert= dlm->mvert;
	int i;

	bglBegin(GL_POINTS);
	for (i=0; i<dlm->totvert; i++) {
		bglVertex3fv(mvert[i].co);
	}
	bglEnd();
}
static void ssDM_drawEdges(DerivedMesh *dm) 
{
	SSDerivedMesh *ssdm = (SSDerivedMesh*) dm;
	DispListMesh *dlm = ssdm->dlm;
	MVert *mvert= dlm->mvert;
	int i;

	if (dlm->medge) {
		MEdge *medge= dlm->medge;
	
		glBegin(GL_LINES);
		for (i=0; i<dlm->totedge; i++, medge++) {
			glVertex3fv(mvert[medge->v1].co); 
			glVertex3fv(mvert[medge->v2].co);
		}
		glEnd();
	} else {
		MFace *mface= dlm->mface;

		glBegin(GL_LINES);
		for (i=0; i<dlm->totface; i++, mface++) {
			glVertex3fv(mvert[mface->v1].co);
			glVertex3fv(mvert[mface->v2].co);

			if (mface->v3) {
				glVertex3fv(mvert[mface->v2].co);
				glVertex3fv(mvert[mface->v3].co);

				glVertex3fv(mvert[mface->v3].co);
				if (mface->v4) {
					glVertex3fv(mvert[mface->v4].co);

					glVertex3fv(mvert[mface->v4].co);
				}
				glVertex3fv(mvert[mface->v1].co);
			}
		}
		glEnd();
	}
}
static void ssDM_drawFacesSolid(DerivedMesh *dm, void (*setMaterial)(int))
{
	SSDerivedMesh *ssdm = (SSDerivedMesh*) dm;
	DispListMesh *dlm = ssdm->dlm;
	float *nors = dlm->nors;
	int glmode=-1, shademodel=-1, matnr=-1;
	int i;

#define PASSVERT(ind) {						\
	if (shademodel==GL_SMOOTH)				\
		glNormal3sv(dlm->mvert[(ind)].no);	\
	glVertex3fv(dlm->mvert[(ind)].co);		\
}

	glBegin(glmode=GL_QUADS);
	for (i=0; i<dlm->totface; i++) {
		MFace *mf= &dlm->mface[i];
		
		if (mf->v3) {
			int new_glmode = mf->v4?GL_QUADS:GL_TRIANGLES;
			int new_shademodel = (mf->flag&ME_SMOOTH)?GL_SMOOTH:GL_FLAT;
			int new_matnr = mf->mat_nr+1;
			
			if(new_glmode!=glmode || new_shademodel!=shademodel || new_matnr!=matnr) {
				glEnd();

				setMaterial(matnr=new_matnr);

				glShadeModel(shademodel=new_shademodel);
				glBegin(glmode=new_glmode);
			}
			
			if (shademodel==GL_FLAT)
				glNormal3fv(&nors[i*3]);
				
			PASSVERT(mf->v1);
			PASSVERT(mf->v2);
			PASSVERT(mf->v3);
			if (mf->v4)
				PASSVERT(mf->v4);
		}
	}
	glEnd();
	
#undef PASSVERT
}
static void ssDM_drawFacesColored(DerivedMesh *dm, int useTwoSided, unsigned char *vcols1, unsigned char *vcols2)
{
	SSDerivedMesh *ssdm = (SSDerivedMesh*) dm;
	DispListMesh *dlm = ssdm->dlm;
	int i, lmode;
	
	glShadeModel(GL_SMOOTH);
	if (vcols2) {
		glEnable(GL_CULL_FACE);
	} else {
		useTwoSided = 0;
	}
		
#define PASSVERT(vidx, fidx) {					\
	unsigned char *col= &colbase[fidx*4];		\
	glColor3ub(col[3], col[2], col[1]);			\
	glVertex3fv(dlm->mvert[(vidx)].co);			\
}

	glBegin(lmode= GL_QUADS);
	for (i=0; i<dlm->totface; i++) {
		MFace *mf= &dlm->mface[i];
		
		if (mf->v3) {
			int nmode= mf->v4?GL_QUADS:GL_TRIANGLES;
			unsigned char *colbase= &vcols1[i*16];
			
			if (nmode!=lmode) {
				glEnd();
				glBegin(lmode= nmode);
			}
			
			PASSVERT(mf->v1, 0);
			PASSVERT(mf->v2, 1);
			PASSVERT(mf->v3, 2);
			if (mf->v4)
				PASSVERT(mf->v4, 3);
			
			if (useTwoSided) {
				unsigned char *colbase= &vcols2[i*16];

				if (mf->v4)
					PASSVERT(mf->v4, 3);
				PASSVERT(mf->v3, 2);
				PASSVERT(mf->v2, 1);
				PASSVERT(mf->v1, 0);
			}
		}
	}
	glEnd();

	if (vcols2)
		glDisable(GL_CULL_FACE);
	
#undef PASSVERT
}
static void ssDM_drawFacesEM(DerivedMesh *dm, int useColor, unsigned char *baseCol, unsigned char *selCol)
{
	SSDerivedMesh *ssdm = (SSDerivedMesh*) dm;
	DispListMesh *dlm = ssdm->dlm;
	MFace *mface= dlm->mface;
	int a;

	for(a=0; a<dlm->totface; a++, mface++) {
		if(mface->v3) {
			if (useColor) {
				glColor4ubv((dlm->editface[a]->f & SELECT)?selCol:baseCol);
			}
			
			glBegin(mface->v4?GL_QUADS:GL_TRIANGLES);
			glVertex3fv(dlm->mvert[mface->v1].co);
			glVertex3fv(dlm->mvert[mface->v2].co);
			glVertex3fv(dlm->mvert[mface->v3].co);
			if (mface->v4) glVertex3fv(dlm->mvert[mface->v4].co);
			glEnd();
		}
	}
}
static int ssDM_drawMappedFacesEMSelect(DerivedMesh *dm, void (*setColor)(int index), int offset)
{
	SSDerivedMesh *ssdm = (SSDerivedMesh*) dm;
	DispListMesh *dlm = ssdm->dlm;
	MFace *mface= dlm->mface;
	int i, endOffset;
	EditFace *prevefa, *efa;

	for (i=offset, efa=ssdm->em->faces.first; efa; efa= efa->next, i++)
		efa->prev = (EditFace*) i;
	endOffset = i;

	for(i=0; i<dlm->totface; i++, mface++) {
		if(mface->v3 && dlm->editface[i]->h==0) {
			setColor((int) dlm->editface[i]->prev);
			
			glBegin(mface->v4?GL_QUADS:GL_TRIANGLES);
			glVertex3fv(dlm->mvert[mface->v1].co);
			glVertex3fv(dlm->mvert[mface->v2].co);
			glVertex3fv(dlm->mvert[mface->v3].co);
			if (mface->v4) glVertex3fv(dlm->mvert[mface->v4].co);
			glEnd();
		}
	}

	for (prevefa= NULL, efa= ssdm->em->faces.first; efa; prevefa= efa, efa= efa->next)
		efa->prev= prevefa;

	return endOffset;
}

static int ssDM_getNumVerts(DerivedMesh *dm)
{
	SSDerivedMesh *ssdm = (SSDerivedMesh*) dm;

	return ssdm->dlm->totvert;
}
static int ssDM_getNumFaces(DerivedMesh *dm)
{
	SSDerivedMesh *ssdm = (SSDerivedMesh*) dm;

	return ssdm->dlm->totface;
}

static DispListMesh *ssDM_convertToDispListMesh(DerivedMesh *dm)
{
	SSDerivedMesh *ssdm = (SSDerivedMesh*) dm;

	return displistmesh_copy(ssdm->dlm);
}

static DerivedMesh *getSSDerivedMesh(EditMesh *em, DispListMesh *dlm)
{
	SSDerivedMesh *ssdm = MEM_mallocN(sizeof(*ssdm), "dm");

	ssdm->dm.getNumVerts = ssDM_getNumVerts;
	ssdm->dm.getNumFaces = ssDM_getNumFaces;
	ssdm->dm.getMappedVertCoEM = ssDM_getMappedVertCoEM;
	ssdm->dm.convertToDispListMesh = ssDM_convertToDispListMesh;

	ssdm->dm.drawVerts = ssDM_drawVerts;
	ssdm->dm.drawMappedVertEM = ssDM_drawMappedVertEM;
	ssdm->dm.drawMappedVertsEM = ssDM_drawMappedVertsEM;
	ssdm->dm.drawMappedVertsEMSelect = ssDM_drawMappedVertsEMSelect;

	ssdm->dm.drawEdges = ssDM_drawEdges;
	ssdm->dm.drawMappedEdges = ssDM_drawMappedEdges;
	ssdm->dm.drawLooseEdges = ssDM_drawLooseEdges;
	ssdm->dm.drawMappedEdgeEM = ssDM_drawMappedEdgeEM;
	ssdm->dm.drawMappedEdgesEM = ssDM_drawMappedEdgesEM;
	ssdm->dm.drawMappedEdgesEMSelect = ssDM_drawMappedEdgesEMSelect;
	ssdm->dm.drawMappedFacesEMSelect = ssDM_drawMappedFacesEMSelect;

	ssdm->dm.drawFacesSolid = ssDM_drawFacesSolid;
	ssdm->dm.drawFacesColored = ssDM_drawFacesColored;
	ssdm->dm.drawFacesEM = ssDM_drawFacesEM;

	ssdm->dm.release = MEM_freeN;
	
	ssdm->dlm = dlm;
	ssdm->em = em;

	return (DerivedMesh*) ssdm;
}

///

static void build_mesh_data(Object *ob)
{
	Mesh *me = ob->data;

	if(ob->disp.first==NULL) {
		if(ob->parent && ob->partype==PARSKEL) makeDispList(ob);
		else if(ob->parent && ob->parent->type==OB_LATTICE) makeDispList(ob);
		else if(ob->hooks.first) makeDispList(ob);
		else if(ob->softflag & 0x01) makeDispList(ob);
		else if(ob->effect.first) {
			Effect *eff= ob->effect.first;
			if(eff->type==EFF_WAVE) makeDispList(ob);
		}
	}
	if(me->disp.first==NULL && mesh_uses_displist(me)) {
		makeDispList(ob);
	}

	if(!me->disp.first || !((DispList*) me->disp.first)->nors) {
		addnormalsDispList(ob, &me->disp);
	}
}

DerivedMesh *mesh_get_derived(Object *ob)
{
	Mesh *me= ob->data;

	if (mesh_uses_displist(me)) {
		DispList *dl;

		build_mesh_data(ob);
		dl= find_displist(&me->disp, DL_MESH);
		
		if(dl) {
			if(G.obedit && me==G.obedit->data) {
				return getSSDerivedMesh(G.editMesh, dl->mesh);
			} else {
				return getSSDerivedMesh(NULL, dl->mesh);
			}
		}
		else return NULL;
	} else {
		return NULL;
	}
}

DerivedMesh *mesh_get_base_derived(Object *ob)
{
	Mesh *me= ob->data;

		/* Build's extverts, nors */
	build_mesh_data(ob);

	if (G.obedit && me==G.obedit->data) {
		return getEditMeshDerivedMesh(G.editMesh);
	} else {
		DispList *meDL = me->disp.first;
		DispList *dl = find_displist(&ob->disp, DL_VERTS);
		return getMeshDerivedMesh(ob, dl?dl->verts:NULL, meDL?meDL->nors:NULL);
	}
}

DerivedMesh *mesh_get_cage_derived(struct Object *ob)
{
	Mesh *me= ob->data;
	DerivedMesh *dm = NULL;

	if (me->flag&ME_OPT_EDGES) {
		dm = mesh_get_derived(ob);
	}
	if (!dm) {
		dm = mesh_get_base_derived(ob);
	}

	return dm;
}
