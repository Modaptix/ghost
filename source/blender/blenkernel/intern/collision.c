/*  collision.c      
* 
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
* The Original Code is Copyright (C) Blender Foundation
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
#include "MEM_guardedalloc.h"
/* types */
#include "DNA_curve_types.h"
#include "DNA_object_types.h"
#include "DNA_object_force.h"
#include "DNA_cloth_types.h"	
#include "DNA_key_types.h"
#include "DNA_mesh_types.h"
#include "DNA_meshdata_types.h"
#include "DNA_lattice_types.h"
#include "DNA_scene_types.h"
#include "DNA_modifier_types.h"
#include "BLI_blenlib.h"
#include "BLI_arithb.h"
#include "BLI_edgehash.h"
#include "BLI_linklist.h"
#include "BKE_curve.h"
#include "BKE_deform.h"
#include "BKE_DerivedMesh.h"
#include "BKE_cdderivedmesh.h"
#include "BKE_displist.h"
#include "BKE_effect.h"
#include "BKE_global.h"
#include "BKE_mesh.h"
#include "BKE_object.h"
#include "BKE_cloth.h"
#include "BKE_modifier.h"
#include "BKE_utildefines.h"
#include "BKE_DerivedMesh.h"
#include "DNA_screen_types.h"
#include "BSE_headerbuttons.h"
#include "BIF_screen.h"
#include "BIF_space.h"
#include "mydevice.h"

#include "Bullet-C-Api.h"



/**
 * gsl_poly_solve_cubic -
 *
 * copied from SOLVE_CUBIC.C --> GSL
 */
#define mySWAP(a,b) do { float tmp = b ; b = a ; a = tmp ; } while(0)

int gsl_poly_solve_cubic (float a, float b, float c, float *x0, float *x1, float *x2)
{
	float q = (a * a - 3 * b);
	float r = (2 * a * a * a - 9 * a * b + 27 * c);

	float Q = q / 9;
	float R = r / 54;

	float Q3 = Q * Q * Q;
	float R2 = R * R;

	float CR2 = 729 * r * r;
	float CQ3 = 2916 * q * q * q;

	if (R == 0 && Q == 0)
	{
		*x0 = - a / 3 ;
		*x1 = - a / 3 ;
		*x2 = - a / 3 ;
		return 3 ;
	}
	else if (CR2 == CQ3) 
	{
	  /* this test is actually R2 == Q3, written in a form suitable
		for exact computation with integers */

	  /* Due to finite precision some float roots may be missed, and
		considered to be a pair of complex roots z = x +/- epsilon i
		close to the real axis. */

		float sqrtQ = sqrtf (Q);

		if (R > 0)
		{
			*x0 = -2 * sqrtQ  - a / 3;
			*x1 = sqrtQ - a / 3;
			*x2 = sqrtQ - a / 3;
		}
		else
		{
			*x0 = - sqrtQ  - a / 3;
			*x1 = - sqrtQ - a / 3;
			*x2 = 2 * sqrtQ - a / 3;
		}
		return 3 ;
	}
	else if (CR2 < CQ3) /* equivalent to R2 < Q3 */
	{
		float sqrtQ = sqrtf (Q);
		float sqrtQ3 = sqrtQ * sqrtQ * sqrtQ;
		float theta = acosf (R / sqrtQ3);
		float norm = -2 * sqrtQ;
		*x0 = norm * cosf (theta / 3) - a / 3;
		*x1 = norm * cosf ((theta + 2.0 * M_PI) / 3) - a / 3;
		*x2 = norm * cosf ((theta - 2.0 * M_PI) / 3) - a / 3;
      
		/* Sort *x0, *x1, *x2 into increasing order */

		if (*x0 > *x1)
			mySWAP(*x0, *x1) ;
      
		if (*x1 > *x2)
		{
			mySWAP(*x1, *x2) ;
          
			if (*x0 > *x1)
				mySWAP(*x0, *x1) ;
		}
      
		return 3;
	}
	else
	{
		float sgnR = (R >= 0 ? 1 : -1);
		float A = -sgnR * powf (fabs (R) + sqrtf (R2 - Q3), 1.0/3.0);
		float B = Q / A ;
		*x0 = A + B - a / 3;
		return 1;
	}
}


/**
 * gsl_poly_solve_quadratic
 *
 * copied from GSL
 */
int gsl_poly_solve_quadratic (float a, float b, float c,  float *x0, float *x1)
{
	float disc = b * b - 4 * a * c;

	if (disc > 0)
	{
		if (b == 0)
		{
			float r = fabs (0.5 * sqrtf (disc) / a);
			*x0 = -r;
			*x1 =  r;
		}
		else
		{
			float sgnb = (b > 0 ? 1 : -1);
			float temp = -0.5 * (b + sgnb * sqrtf (disc));
			float r1 = temp / a ;
			float r2 = c / temp ;

			if (r1 < r2) 
			{
				*x0 = r1 ;
				*x1 = r2 ;
			} 
			else 
			{
				*x0 = r2 ;
				*x1 = r1 ;
			}
		}
		return 2;
	}
	else if (disc == 0) 
	{
		*x0 = -0.5 * b / a ;
		*x1 = -0.5 * b / a ;
		return 2 ;
	}
	else
	{
		return 0;
	}
}



/*
 * See Bridson et al. "Robust Treatment of Collision, Contact and Friction for Cloth Animation"
 *     page 4, left column
 */

int cloth_get_collision_time(float a[3], float b[3], float c[3], float d[3], float e[3], float f[3], float solution[3]) 
{
	int num_sols = 0;
	
	float g = -a[2] * c[1] * e[0] + a[1] * c[2] * e[0] +
			a[2] * c[0] * e[1] - a[0] * c[2] * e[1] -
			a[1] * c[0] * e[2] + a[0] * c[1] * e[2];

	float h = -b[2] * c[1] * e[0] + b[1] * c[2] * e[0] - a[2] * d[1] * e[0] +
			a[1] * d[2] * e[0] + b[2] * c[0] * e[1] - b[0] * c[2] * e[1] +
			a[2] * d[0] * e[1] - a[0] * d[2] * e[1] - b[1] * c[0] * e[2] +
			b[0] * c[1] * e[2] - a[1] * d[0] * e[2] + a[0] * d[1] * e[2] -
			a[2] * c[1] * f[0] + a[1] * c[2] * f[0] + a[2] * c[0] * f[1] -
			a[0] * c[2] * f[1] - a[1] * c[0] * f[2] + a[0] * c[1] * f[2];

	float i = -b[2] * d[1] * e[0] + b[1] * d[2] * e[0] +
			b[2] * d[0] * e[1] - b[0] * d[2] * e[1] -
			b[1] * d[0] * e[2] + b[0] * d[1] * e[2] -
			b[2] * c[1] * f[0] + b[1] * c[2] * f[0] -
			a[2] * d[1] * f[0] + a[1] * d[2] * f[0] +
			b[2] * c[0] * f[1] - b[0] * c[2] * f[1] + 
			a[2] * d[0] * f[1] - a[0] * d[2] * f[1] -
			b[1] * c[0] * f[2] + b[0] * c[1] * f[2] -
			a[1] * d[0] * f[2] + a[0] * d[1] * f[2];

	float j = -b[2] * d[1] * f[0] + b[1] * d[2] * f[0] +
			b[2] * d[0] * f[1] - b[0] * d[2] * f[1] -
			b[1] * d[0] * f[2] + b[0] * d[1] * f[2];

	// Solve cubic equation to determine times t1, t2, t3, when the collision will occur.
	if(ABS(j) > ALMOST_ZERO)
	{
		i /= j;
		h /= j;
		g /= j;
		
		num_sols = gsl_poly_solve_cubic(i, h, g, &solution[0], &solution[1], &solution[2]);
	}
	else if(ABS(i) > ALMOST_ZERO)
	{	
		num_sols = gsl_poly_solve_quadratic(i, h, g, &solution[0], &solution[1]);
		solution[2] = -1.0;
	}
	else if(ABS(h) > ALMOST_ZERO)
	{
		solution[0] = -g / h;
		solution[1] = solution[2] = -1.0;
		num_sols = 1;
	}
	else if(ABS(g) > ALMOST_ZERO)
	{
		solution[0] = 0;
		solution[1] = solution[2] = -1.0;
		num_sols = 1;
	}

	// Discard negative solutions
	if ((num_sols >= 1) && (solution[0] < 0)) 
	{
		--num_sols;
		solution[0] = solution[num_sols];
	}
	if ((num_sols >= 2) && (solution[1] < 0)) 
	{
		--num_sols;
		solution[1] = solution[num_sols];
	}
	if ((num_sols == 3) && (solution[2] < 0)) 
	{
		--num_sols;
	}

	// Sort
	if (num_sols == 2) 
	{
		if (solution[0] > solution[1]) 
		{
			double tmp = solution[0];
			solution[0] = solution[1];
			solution[1] = tmp;
		}
	}
	else if (num_sols == 3) 
	{

		// Bubblesort
		if (solution[0] > solution[1]) {
			double tmp = solution[0]; solution[0] = solution[1]; solution[1] = tmp;
		}
		if (solution[1] > solution[2]) {
			double tmp = solution[1]; solution[1] = solution[2]; solution[2] = tmp;
		}
		if (solution[0] > solution[1]) {
			double tmp = solution[0]; solution[0] = solution[1]; solution[1] = tmp;
		}
	}

	return num_sols;
}

// w3 is not perfect
void cloth_compute_barycentric (float pv[3], float p1[3], float p2[3], float p3[3], float *w1, float *w2, float *w3)
{
	double	tempV1[3], tempV2[3], tempV4[3];
	double	a,b,c,d,e,f;

	VECSUB (tempV1, p1, p3);	
	VECSUB (tempV2, p2, p3);	
	VECSUB (tempV4, pv, p3);	
	
	a = INPR (tempV1, tempV1);	
	b = INPR (tempV1, tempV2);	
	c = INPR (tempV2, tempV2);	
	e = INPR (tempV1, tempV4);	
	f = INPR (tempV2, tempV4);	
	
	d = (a * c - b * b);
	
	if (ABS(d) < ALMOST_ZERO) {
		*w1 = *w2 = *w3 = 1.0 / 3.0;
		return;
	}
	
	w1[0] = (float)((e * c - b * f) / d);
	
	if(w1[0] < 0)
		w1[0] = 0;
	
	w2[0] = (float)((f - b * (double)w1[0]) / c);
	
	if(w2[0] < 0)
		w2[0] = 0;
	
	w3[0] = 1.0f - w1[0] - w2[0];
}

DO_INLINE void interpolateOnTriangle(float to[3], float v1[3], float v2[3], float v3[3], double w1, double w2, double w3) 
{
	to[0] = to[1] = to[2] = 0;
	VECADDMUL(to, v1, w1);
	VECADDMUL(to, v2, w2);
	VECADDMUL(to, v3, w3);
}



// unused in the moment, has some bug in
DO_INLINE void calculateFrictionImpulse(float to[3], float vrel[3], float normal[3], double normalVelocity,
					double frictionConstant, double delta_V_n) 
{
	float vrel_t_pre[3];
	float vrel_t[3];
	VECSUBS(vrel_t_pre, vrel, normal, normalVelocity);
	VECCOPY(to, vrel_t_pre);
	VecMulf(to, MAX2(1.0f - frictionConstant * delta_V_n / INPR(vrel_t_pre,vrel_t_pre), 0.0f));
}

int cloth_collision_response_static(ClothModifierData *clmd, ClothModifierData *coll_clmd)
{
	unsigned int i = 0;
	int result = 0;
	LinkNode *search = NULL;
	CollPair *collpair = NULL;
	Cloth *cloth1, *cloth2;
	float w1, w2, w3, u1, u2, u3;
	float v1[3], v2[3], relativeVelocity[3];
	float magrelVel;
	
	cloth1 = clmd->clothObject;
	cloth2 = coll_clmd->clothObject;

	search = clmd->coll_parms.collision_list;
	
	while(search)
	{
		collpair = search->link;
		
		// compute barycentric coordinates for both collision points
		cloth_compute_barycentric(collpair->pa,
					cloth1->verts[collpair->ap1].txold,
					cloth1->verts[collpair->ap2].txold,
					cloth1->verts[collpair->ap3].txold, 
					&w1, &w2, &w3);
	
		cloth_compute_barycentric(collpair->pb,
					cloth2->verts[collpair->bp1].txold,
					cloth2->verts[collpair->bp2].txold,
					cloth2->verts[collpair->bp3].txold,
					&u1, &u2, &u3);
	
		// Calculate relative "velocity".
		interpolateOnTriangle(v1, cloth1->verts[collpair->ap1].tv, cloth1->verts[collpair->ap2].tv, cloth1->verts[collpair->ap3].tv, w1, w2, w3);
		
		interpolateOnTriangle(v2, cloth2->verts[collpair->bp1].tv, cloth2->verts[collpair->bp2].tv, cloth2->verts[collpair->bp3].tv, u1, u2, u3);
		
		VECSUB(relativeVelocity, v1, v2);
			
		// Calculate the normal component of the relative velocity (actually only the magnitude - the direction is stored in 'normal').
		magrelVel = INPR(relativeVelocity, collpair->normal);
		
		// printf("magrelVel: %f\n", magrelVel);
				
		// Calculate masses of points.
		
		// If v_n_mag < 0 the edges are approaching each other.
		if(magrelVel < -ALMOST_ZERO) 
		{
			// Calculate Impulse magnitude to stop all motion in normal direction.
			// const double I_mag = v_n_mag / (1/m1 + 1/m2);
			float magnitude_i = magrelVel / 2.0f; // TODO implement masses
			float tangential[3], magtangent, magnormal, collvel[3];
			float vrel_t_pre[3];
			float vrel_t[3];
			double impulse;
			float epsilon = clmd->coll_parms.epsilon;
			float overlap = (epsilon + ALMOST_ZERO-collpair->distance);
			
			// calculateFrictionImpulse(tangential, relativeVelocity, collpair->normal, magrelVel, clmd->coll_parms.friction*0.01, magrelVel);
			
			// magtangent = INPR(tangential, tangential);
			
			// Apply friction impulse.
			if (magtangent < -ALMOST_ZERO) 
			{
				
				// printf("friction applied: %f\n", magtangent);
				// TODO check original code 
				/*
				VECSUB(cloth1->verts[face1->v1].tv, cloth1->verts[face1->v1].tv,tangential);
				VECSUB(cloth1->verts[face1->v1].tv, cloth1->verts[face1->v2].tv,tangential);
				VECSUB(cloth1->verts[face1->v1].tv, cloth1->verts[face1->v3].tv,tangential);
				VECSUB(cloth1->verts[face1->v1].tv, cloth1->verts[face1->v4].tv,tangential);
				*/
			}
			

			impulse = -2.0f * magrelVel / ( 1.0 + w1*w1 + w2*w2 + w3*w3);
			
			// printf("impulse: %f\n", impulse);
			
			VECADDMUL(cloth1->verts[collpair->ap1].impulse, collpair->normal, w1 * impulse); 
			cloth1->verts[collpair->ap1].impulse_count++;
			
			VECADDMUL(cloth1->verts[collpair->ap2].impulse, collpair->normal, w2 * impulse); 
			cloth1->verts[collpair->ap2].impulse_count++;
			
			VECADDMUL(cloth1->verts[collpair->ap3].impulse, collpair->normal, w3 * impulse); 
			cloth1->verts[collpair->ap3].impulse_count++;
			
			result = 1;
			
			/*
			if (overlap > ALMOST_ZERO) {
			double I_mag  = overlap * 0.1;
				
			impulse = -I_mag / ( 1.0 + w1*w1 + w2*w2 + w3*w3);
				
			VECADDMUL(cloth1->verts[collpair->ap1].impulse, collpair->normal, w1 * impulse); 
			cloth1->verts[collpair->ap1].impulse_count++;
							
			VECADDMUL(cloth1->verts[collpair->ap2].impulse, collpair->normal, w2 * impulse); 
			cloth1->verts[collpair->ap2].impulse_count++;
			
			VECADDMUL(cloth1->verts[collpair->ap3].impulse, collpair->normal, w3 * impulse); 
			cloth1->verts[collpair->ap3].impulse_count++;
		}
			*/
		
			// printf("magnitude_i: %f\n", magnitude_i); // negative before collision in my case
			
			// Apply the impulse and increase impulse counters.

			/*			
			// calculateFrictionImpulse(tangential, collvel, collpair->normal, magtangent, clmd->coll_parms.friction*0.01, magtangent);
			VECSUBS(vrel_t_pre, collvel, collpair->normal, magnormal);
			// VecMulf(vrel_t_pre, clmd->coll_parms.friction*0.01f/INPR(vrel_t_pre,vrel_t_pre));
			magtangent = Normalize(vrel_t_pre);
			VecMulf(vrel_t_pre, MIN2(clmd->coll_parms.friction*0.01f*magnormal,magtangent));
			
			VECSUB(cloth1->verts[face1->v1].tv, cloth1->verts[face1->v1].tv,vrel_t_pre);
			*/
			
			
			
		}
		
		search = search->next;
	}
	
		
	return result;
}

int cloth_collision_response_moving_tris(ClothModifierData *clmd, ClothModifierData *coll_clmd)
{
	
}


int cloth_collision_response_moving_edges(ClothModifierData *clmd, ClothModifierData *coll_clmd)
{
	
}

void cloth_collision_static(ClothModifierData *clmd, ClothModifierData *coll_clmd, Tree *tree1, Tree *tree2)
{
	CollPair *collpair = NULL;
	Cloth *cloth1=NULL, *cloth2=NULL;
	MFace *face1=NULL, *face2=NULL;
	ClothVertex *verts1=NULL, *verts2=NULL;
	double distance = 0;
	float epsilon = clmd->coll_parms.epsilon;
	unsigned int i = 0;

	for(i = 0; i < 4; i++)
	{
		collpair = (CollPair *)MEM_callocN(sizeof(CollPair), "cloth coll pair");		
		
		cloth1 = clmd->clothObject;
		cloth2 = coll_clmd->clothObject;
		
		verts1 = cloth1->verts;
		verts2 = cloth2->verts;
	
		face1 = &(cloth1->mfaces[tree1->tri_index]);
		face2 = &(cloth2->mfaces[tree2->tri_index]);
		
		// check all possible pairs of triangles
		if(i == 0)
		{
			collpair->ap1 = face1->v1;
			collpair->ap2 = face1->v2;
			collpair->ap3 = face1->v3;
			
			collpair->bp1 = face2->v1;
			collpair->bp2 = face2->v2;
			collpair->bp3 = face2->v3;
			
		}
		
		if(i == 1)
		{
			if(face1->v4)
			{
				collpair->ap1 = face1->v3;
				collpair->ap2 = face1->v4;
				collpair->ap3 = face1->v1;
				
				collpair->bp1 = face2->v1;
				collpair->bp2 = face2->v2;
				collpair->bp3 = face2->v3;
			}
			else
				i++;
		}
		
		if(i == 2)
		{
			if(face2->v4)
			{
				collpair->ap1 = face1->v1;
				collpair->ap2 = face1->v2;
				collpair->ap3 = face1->v3;
				
				collpair->bp1 = face2->v3;
				collpair->bp2 = face2->v4;
				collpair->bp3 = face2->v1;
			}
			else
				i+=2;
		}
		
		if(i == 3)
		{
			if((face1->v4)&&(face2->v4))
			{
				collpair->ap1 = face1->v3;
				collpair->ap2 = face1->v4;
				collpair->ap3 = face1->v1;
				
				collpair->bp1 = face2->v3;
				collpair->bp2 = face2->v4;
				collpair->bp3 = face2->v1;
			}
			else
				i++;
		}
		
		// calc SIPcode (?)
		
		if(i < 4)
		{
			// calc distance + normal 	
			distance = plNearestPoints(
					verts1[collpair->ap1].txold, verts1[collpair->ap2].txold, verts1[collpair->ap3].txold, verts2[collpair->bp1].txold, verts2[collpair->bp2].txold, verts2[collpair->bp3].txold, collpair->pa,collpair->pb,collpair->vector);
			
			if (distance <= (epsilon + ALMOST_ZERO))
			{
				// printf("dist: %f\n", (float)distance);
				
				// collpair->face1 = tree1->tri_index;
				// collpair->face2 = tree2->tri_index;
				
				VECCOPY(collpair->normal, collpair->vector);
				Normalize(collpair->normal);
				
				collpair->distance = distance;
				BLI_linklist_append(&clmd->coll_parms.collision_list, collpair);
			}
			else
			{
				MEM_freeN(collpair);
			}
		}
		else
		{
			MEM_freeN(collpair);
		}
	}
}

int cloth_are_edges_adjacent(ClothModifierData *clmd, ClothModifierData *coll_clmd, EdgeCollPair *edgecollpair)
{
	Cloth *cloth1, *cloth2;
	ClothVertex *verts1, *verts2;
	float temp[3];
	 
	cloth1 = clmd->clothObject;
	cloth2 = coll_clmd->clothObject;
	
	verts1 = cloth1->verts;
	verts2 = cloth2->verts;
	
	VECSUB(temp, verts1[edgecollpair->p11].xold, verts2[edgecollpair->p21].xold);
	if(ABS(INPR(temp, temp)) < ALMOST_ZERO)
		return 1;
	
	VECSUB(temp, verts1[edgecollpair->p11].xold, verts2[edgecollpair->p22].xold);
	if(ABS(INPR(temp, temp)) < ALMOST_ZERO)
		return 1;
	
	VECSUB(temp, verts1[edgecollpair->p12].xold, verts2[edgecollpair->p21].xold);
	if(ABS(INPR(temp, temp)) < ALMOST_ZERO)
		return 1;
	
	VECSUB(temp, verts1[edgecollpair->p12].xold, verts2[edgecollpair->p22].xold);
	if(ABS(INPR(temp, temp)) < ALMOST_ZERO)
		return 1;
		
	return 0;
}

void cloth_collision_moving_edges(ClothModifierData *clmd, ClothModifierData *coll_clmd, Tree *tree1, Tree *tree2)
{
	EdgeCollPair edgecollpair;
	Cloth *cloth1=NULL, *cloth2=NULL;
	MFace *face1=NULL, *face2=NULL;
	ClothVertex *verts1=NULL, *verts2=NULL;
	double distance = 0;
	float epsilon = clmd->coll_parms.epsilon;
	unsigned int i = 0, j = 0, k = 0;
	int numsolutions = 0;
	float a[3], b[3], c[3], d[3], e[3], f[3], solution[3];
	
	cloth1 = clmd->clothObject;
	cloth2 = coll_clmd->clothObject;
	
	verts1 = cloth1->verts;
	verts2 = cloth2->verts;

	face1 = &(cloth1->mfaces[tree1->tri_index]);
	face2 = &(cloth2->mfaces[tree2->tri_index]);
	
	for( i = 0; i < 5; i++)
	{
		if(i == 0) 
		{
			edgecollpair.p11 = face1->v1;
			edgecollpair.p12 = face1->v2;
		}
		else if(i == 1) 
		{
			edgecollpair.p11 = face1->v2;
			edgecollpair.p12 = face1->v3;
		}
		else if(i == 2) 
		{
			if(face1->v4) 
			{
				edgecollpair.p11 = face1->v3;
				edgecollpair.p12 = face1->v4;
			}
			else 
			{
				edgecollpair.p11 = face1->v3;
				edgecollpair.p12 = face1->v1;
				i+=5; // get out of here after this edge pair is handled
			}
		}
		else if(i == 3) 
		{
			if(face1->v4) 
			{
				edgecollpair.p11 = face1->v4;
				edgecollpair.p12 = face1->v1;
			}	
			else
				continue;
		}
		else
		{
			edgecollpair.p11 = face1->v3;
			edgecollpair.p12 = face1->v1;
		}

		
		for( j = 0; j < 5; j++)
		{
			if(j == 0)
			{
				edgecollpair.p21 = face2->v1;
				edgecollpair.p22 = face2->v2;
			}
			else if(j == 1)
			{
				edgecollpair.p21 = face2->v2;
				edgecollpair.p22 = face2->v3;
			}
			else if(j == 2)
			{
				if(face2->v4) 
				{
					edgecollpair.p21 = face2->v3;
					edgecollpair.p22 = face2->v4;
				}
				else 
				{
					edgecollpair.p21 = face2->v3;
					edgecollpair.p22 = face2->v1;
				}
			}
			else if(j == 3)
			{
				if(face2->v4) 
				{
					edgecollpair.p21 = face2->v4;
					edgecollpair.p22 = face2->v1;
				}
				else
					continue;
			}
			else
			{
				edgecollpair.p21 = face2->v3;
				edgecollpair.p22 = face2->v1;
			}
			
			
			if(!cloth_are_edges_adjacent(clmd, coll_clmd, &edgecollpair))
			{
				VECSUB(a, verts1[edgecollpair.p12].xold, verts1[edgecollpair.p11].xold);
				VECSUB(b, verts1[edgecollpair.p12].v, verts1[edgecollpair.p11].v);
				VECSUB(c, verts1[edgecollpair.p21].xold, verts1[edgecollpair.p11].xold);
				VECSUB(d, verts1[edgecollpair.p21].v, verts1[edgecollpair.p11].v);
				VECSUB(e, verts2[edgecollpair.p22].xold, verts1[edgecollpair.p11].xold);
				VECSUB(f, verts2[edgecollpair.p22].v, verts1[edgecollpair.p11].v);
				
				numsolutions = cloth_get_collision_time(a, b, c, d, e, f, solution);
				
				for (k = 0; k < numsolutions; k++) 
				{								
					if ((solution[k] >= 0.0) && (solution[k] <= 1.0)) 
					{
						float out_collisionTime = solution[k];
						
						// TODO: check for collisions 
						
						// TODO: put into (edge) collision list
						
						printf("Moving edge found!\n");
					}
				}
			}
		}
	}		
}

void cloth_collision_moving_tris(ClothModifierData *clmd, ClothModifierData *coll_clmd, Tree *tree1, Tree *tree2)
{
	CollPair collpair;
	Cloth *cloth1=NULL, *cloth2=NULL;
	MFace *face1=NULL, *face2=NULL;
	ClothVertex *verts1=NULL, *verts2=NULL;
	double distance = 0;
	float epsilon = clmd->coll_parms.epsilon;
	unsigned int i = 0, j = 0, k = 0;
	int numsolutions = 0;
	float a[3], b[3], c[3], d[3], e[3], f[3], solution[3];

	for(i = 0; i < 2; i++)
	{		
		cloth1 = clmd->clothObject;
		cloth2 = coll_clmd->clothObject;
		
		verts1 = cloth1->verts;
		verts2 = cloth2->verts;
	
		face1 = &(cloth1->mfaces[tree1->tri_index]);
		face2 = &(cloth2->mfaces[tree2->tri_index]);
		
		// check all possible pairs of triangles
		if(i == 0)
		{
			collpair.ap1 = face1->v1;
			collpair.ap2 = face1->v2;
			collpair.ap3 = face1->v3;
			
			collpair.pointsb[0] = face2->v1;
			collpair.pointsb[1] = face2->v2;
			collpair.pointsb[2] = face2->v3;
			collpair.pointsb[3] = face2->v4;
		}
		
		if(i == 1)
		{
			if(face1->v4)
			{
				collpair.ap1 = face1->v3;
				collpair.ap2 = face1->v4;
				collpair.ap3 = face1->v1;
				
				collpair.pointsb[0] = face2->v1;
				collpair.pointsb[1] = face2->v2;
				collpair.pointsb[2] = face2->v3;
				collpair.pointsb[3] = face2->v4;
			}
			else
				i++;
		}
		
		// calc SIPcode (?)
		
		if(i < 2)
		{
			VECSUB(a, verts1[collpair.ap2].xold, verts1[collpair.ap1].xold);
			VECSUB(b, verts1[collpair.ap2].v, verts1[collpair.ap1].v);
			VECSUB(c, verts1[collpair.ap3].xold, verts1[collpair.ap1].xold);
			VECSUB(d, verts1[collpair.ap3].v, verts1[collpair.ap1].v);
				
			for(j = 0; j < 4; j++)
			{					
				if((j==3) && !(face2->v4))
					break;
				
				VECSUB(e, verts2[collpair.pointsb[j]].xold, verts1[collpair.ap1].xold);
				VECSUB(f, verts2[collpair.pointsb[j]].v, verts1[collpair.ap1].v);
				
				numsolutions = cloth_get_collision_time(a, b, c, d, e, f, solution);
				
				for (k = 0; k < numsolutions; k++) 
				{								
					if ((solution[k] >= 0.0) && (solution[k] <= 1.0)) 
					{
						float out_collisionTime = solution[k];
						
						// TODO: check for collisions 
						
						// TODO: put into (point-face) collision list
						
						printf("Moving found!\n");
					}
				}
				
				// TODO: check borders for collisions
			}
			
		}
	}
}

void cloth_collision_moving(ClothModifierData *clmd, ClothModifierData *coll_clmd, Tree *tree1, Tree *tree2)
{
	// TODO: check for adjacent
	cloth_collision_moving_edges(clmd, coll_clmd, tree1, tree2);
	
	cloth_collision_moving_tris(clmd, coll_clmd, tree1, tree2);
	cloth_collision_moving_tris(coll_clmd, clmd, tree2, tree1);
}

// move collision objects forward in time and update static bounding boxes
void cloth_update_collision_objects(float step)
{
	Base *base=NULL;
	ClothModifierData *coll_clmd=NULL;
	Object *coll_ob=NULL;
	unsigned int i=0;
	
	// search all objects for collision object
	for (base = G.scene->base.first; base; base = base->next)
	{

		coll_ob = base->object;
		coll_clmd = (ClothModifierData *) modifiers_findByType (coll_ob, eModifierType_Cloth);
		if (!coll_clmd)
			continue;

		// if collision object go on
		if (coll_clmd->sim_parms.flags & CSIMSETT_FLAG_COLLOBJ)
		{
			if (coll_clmd->clothObject && coll_clmd->clothObject->tree) 
			{
				Cloth *coll_cloth = coll_clmd->clothObject;
				BVH *coll_bvh = coll_clmd->clothObject->tree;
				unsigned int coll_numverts = coll_cloth->numverts;

				// update position of collision object
				for(i = 0; i < coll_numverts; i++)
				{
					VECCOPY(coll_cloth->verts[i].txold, coll_cloth->verts[i].tx);

					VECADDS(coll_cloth->verts[i].tx, coll_cloth->verts[i].xold, coll_cloth->verts[i].v, step);
					
					// no dt here because of float rounding errors
					VECSUB(coll_cloth->verts[i].tv, coll_cloth->verts[i].tx, coll_cloth->verts[i].txold);
				}
				
				// update BVH of collision object
				bvh_update(coll_clmd, coll_bvh, 0); // 0 means STATIC, 1 means MOVING 
			}
			else
				printf ("cloth_bvh_objcollision: found a collision object with clothObject or collData NULL.\n");
		}
	}
}

// CLOTH_MAX_THRESHOLD defines how much collision rounds/loops should be taken
#define CLOTH_MAX_THRESHOLD 10

// cloth - object collisions
int cloth_bvh_objcollision(ClothModifierData * clmd, float step, float dt)
{
	Base *base=NULL;
	ClothModifierData *coll_clmd=NULL;
	Cloth *cloth=NULL;
	Object *coll_ob=NULL;
	BVH *cloth_bvh=NULL;
	unsigned int i=0, j = 0, numfaces = 0, numverts = 0;
	unsigned int result = 0, ic = 0, rounds = 0; // result counts applied collisions; ic is for debug output; 
	ClothVertex *verts = NULL;
	float tnull[3] = {0,0,0};
	int ret = 0;

	if ((clmd->sim_parms.flags & CSIMSETT_FLAG_COLLOBJ) || !(((Cloth *)clmd->clothObject)->tree))
	{
		return 0;
	}
	cloth = clmd->clothObject;
	verts = cloth->verts;
	cloth_bvh = (BVH *) cloth->tree;
	numfaces = clmd->clothObject->numfaces;
	numverts = clmd->clothObject->numverts;
	
	////////////////////////////////////////////////////////////
	// static collisions
	////////////////////////////////////////////////////////////

	// update cloth bvh
	bvh_update(clmd, cloth_bvh, 0); // 0 means STATIC, 1 means MOVING (see later in this function)
	
	// update collision objects
	cloth_update_collision_objects(step);
	
	do
	{
		result = 0;
		ic = 0;
		clmd->coll_parms.collision_list = NULL; 
		
		// check all collision objects
		for (base = G.scene->base.first; base; base = base->next)
		{
			coll_ob = base->object;
			coll_clmd = (ClothModifierData *) modifiers_findByType (coll_ob, eModifierType_Cloth);
			
			if (!coll_clmd)
				continue;
			
			// if collision object go on
			if (coll_clmd->sim_parms.flags & CSIMSETT_FLAG_COLLOBJ)
			{
				if (coll_clmd->clothObject && coll_clmd->clothObject->tree) 
				{
					BVH *coll_bvh = coll_clmd->clothObject->tree;
					
					bvh_traverse(clmd, coll_clmd, cloth_bvh->root, coll_bvh->root, step, cloth_collision_static);
				}
				else
					printf ("cloth_bvh_objcollision: found a collision object with clothObject or collData NULL.\n");
			}
		}
		
		// process all collisions (calculate impulses, TODO: also repulses if distance too short)
		result = 1;
		for(j = 0; j < 10; j++) // 10 is just a value that ensures convergence
		{
			result = 0;
			
			// handle all collision objects
			for (base = G.scene->base.first; base; base = base->next)
			{
		
				coll_ob = base->object;
				coll_clmd = (ClothModifierData *) modifiers_findByType (coll_ob, eModifierType_Cloth);
				if (!coll_clmd)
					continue;
		
				// if collision object go on
				if (coll_clmd->sim_parms.flags & CSIMSETT_FLAG_COLLOBJ)
				{
					if (coll_clmd->clothObject) 
						result += cloth_collision_response_static(clmd, coll_clmd);
					else
						printf ("cloth_bvh_objcollision: found a collision object with clothObject or collData NULL.\n");
				}
			}
			
			// apply impulses in parallel
			ic=0;
			for(i = 0; i < numverts; i++)
			{
				// calculate "velocities" (just xnew = xold + v; no dt in v)
				if(verts[i].impulse_count)
				{
					VECADDMUL(verts[i].tv, verts[i].impulse, 1.0f / verts[i].impulse_count);
					VECCOPY(verts[i].impulse, tnull);
					verts[i].impulse_count = 0;
				
					ic++;
					ret++;
				}
			}
		}
		
		// free collision list
		if(clmd->coll_parms.collision_list)
		{
			LinkNode *search = clmd->coll_parms.collision_list;
			while(search)
			{
				CollPair *coll_pair = search->link;
							
				MEM_freeN(coll_pair);
				search = search->next;
			}
			BLI_linklist_free(clmd->coll_parms.collision_list,NULL);
			
			clmd->coll_parms.collision_list = NULL;
		}
		
		printf("ic: %d\n", ic);
		rounds++;
	}
	while(result && (CLOTH_MAX_THRESHOLD>rounds));
	
	printf("\n");
			
	////////////////////////////////////////////////////////////
	// update positions
	// this is needed for bvh_calc_DOP_hull_moving() [kdop.c]
	////////////////////////////////////////////////////////////
	
	// verts come from clmd
	for(i = 0; i < numverts; i++)
	{
		VECADD(verts[i].tx, verts[i].txold, verts[i].tv);
	}
	////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////
	// moving collisions
	////////////////////////////////////////////////////////////

	
	// update cloth bvh
	bvh_update(clmd, cloth_bvh, 1);  // 0 means STATIC, 1 means MOVING 
	
	// update moving bvh for collision object once
	for (base = G.scene->base.first; base; base = base->next)
	{
		
		coll_ob = base->object;
		coll_clmd = (ClothModifierData *) modifiers_findByType (coll_ob, eModifierType_Cloth);
		if (!coll_clmd)
			continue;
		
		if(!coll_clmd->clothObject)
			continue;
		
				// if collision object go on
		if (coll_clmd->clothObject && coll_clmd->clothObject->tree) 
		{
			BVH *coll_bvh = coll_clmd->clothObject->tree;
			
			bvh_update(coll_clmd, coll_bvh, 1);  // 0 means STATIC, 1 means MOVING 	
		}
	}
	
	
	do
	{
		result = 0;
		ic = 0;
		clmd->coll_parms.collision_list = NULL; 
		
		// check all collision objects
		for (base = G.scene->base.first; base; base = base->next)
		{
			coll_ob = base->object;
			coll_clmd = (ClothModifierData *) modifiers_findByType (coll_ob, eModifierType_Cloth);
			
			if (!coll_clmd)
				continue;
			
			// if collision object go on
			if (coll_clmd->sim_parms.flags & CSIMSETT_FLAG_COLLOBJ)
			{
				if (coll_clmd->clothObject && coll_clmd->clothObject->tree) 
				{
					BVH *coll_bvh = coll_clmd->clothObject->tree;
					
					bvh_traverse(clmd, coll_clmd, cloth_bvh->root, coll_bvh->root, step, cloth_collision_moving);
				}
				else
					printf ("cloth_bvh_objcollision: found a collision object with clothObject or collData NULL.\n");
			}
		}
		
		// process all collisions (calculate impulses, TODO: also repulses if distance too short)
		result = 1;
		for(j = 0; j < 10; j++) // 10 is just a value that ensures convergence
		{
			result = 0;
				
			// handle all collision objects
			for (base = G.scene->base.first; base; base = base->next)
			{
			
				coll_ob = base->object;
				coll_clmd = (ClothModifierData *) modifiers_findByType (coll_ob, eModifierType_Cloth);
						
				if (!coll_clmd)
				continue;
				
				// if collision object go on
				if (coll_clmd->sim_parms.flags & CSIMSETT_FLAG_COLLOBJ)
				{
					if (coll_clmd->clothObject) 
					result += cloth_collision_response_moving_tris(clmd, coll_clmd);
					else
					printf ("cloth_bvh_objcollision: found a collision object with clothObject or collData NULL.\n");
				}
			}
						
			// apply impulses in parallel
			ic=0;
			for(i = 0; i < numverts; i++)
			{
				// calculate "velocities" (just xnew = xold + v; no dt in v)
				if(verts[i].impulse_count)
				{
					VECADDMUL(verts[i].tv, verts[i].impulse, 1.0f / verts[i].impulse_count);
					VECCOPY(verts[i].impulse, tnull);
					verts[i].impulse_count = 0;
							
					ic++;
					ret++;
				}
			}
		}
		
		
		// verts come from clmd
		for(i = 0; i < numverts; i++)
		{
			VECADD(verts[i].tx, verts[i].txold, verts[i].tv);
		}
		
		// update cloth bvh
		bvh_update(clmd, cloth_bvh, 1);  // 0 means STATIC, 1 means MOVING 
		
		
		// free collision list
		if(clmd->coll_parms.collision_list)
		{
			LinkNode *search = clmd->coll_parms.collision_list;
			while(search)
			{
				CollPair *coll_pair = search->link;
							
				MEM_freeN(coll_pair);
				search = search->next;
			}
			BLI_linklist_free(clmd->coll_parms.collision_list,NULL);
			
			clmd->coll_parms.collision_list = NULL;
		}
		
		printf("ic: %d\n", ic);
		rounds++;
	}
	while(result && (CLOTH_MAX_THRESHOLD>rounds));
	
	
	////////////////////////////////////////////////////////////
	// update positions + velocities
	////////////////////////////////////////////////////////////
	
	// verts come from clmd
	for(i = 0; i < numverts; i++)
	{
		VECADD(verts[i].tx, verts[i].txold, verts[i].tv);
	}
	////////////////////////////////////////////////////////////

	return MIN2(ret, 1);
}
