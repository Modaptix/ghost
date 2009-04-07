/**
 * $Id: PyObjectPlus.h 19511 2009-04-03 02:16:56Z campbellbarton $
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
 * Contributor(s): Campbell Barton
 *
 * ***** END GPL LICENSE BLOCK *****
 */



#ifndef _adr_py_init_types_h_				// only process once,
#define _adr_py_init_types_h_				// even if multiply included

/* Only for Class::Parents */
#include "BL_BlenderShader.h"
#include "BL_ShapeActionActuator.h"
#include "KX_BlenderMaterial.h"
#include "KX_CDActuator.h"
#include "KX_CameraActuator.h"
#include "KX_ConstraintActuator.h"
#include "KX_ConstraintWrapper.h"
#include "KX_GameActuator.h"
#include "KX_Light.h"
#include "KX_MeshProxy.h"
#include "KX_MouseFocusSensor.h"
#include "KX_NetworkMessageActuator.h"
#include "KX_NetworkMessageSensor.h"
#include "KX_ObjectActuator.h"
#include "KX_ParentActuator.h"
#include "KX_PhysicsObjectWrapper.h"
#include "KX_PolyProxy.h"
#include "KX_PolygonMaterial.h"
#include "KX_SCA_AddObjectActuator.h"
#include "KX_SCA_EndObjectActuator.h"
#include "KX_SCA_ReplaceMeshActuator.h"
#include "KX_SceneActuator.h"
#include "KX_StateActuator.h"
#include "KX_TrackToActuator.h"
#include "KX_VehicleWrapper.h"
#include "KX_VertexProxy.h"
#include "SCA_2DFilterActuator.h"
#include "SCA_ANDController.h"
#include "SCA_ActuatorSensor.h"
#include "SCA_AlwaysSensor.h"
#include "SCA_DelaySensor.h"
#include "SCA_JoystickSensor.h"
#include "SCA_KeyboardSensor.h"
#include "SCA_MouseSensor.h"
#include "SCA_NANDController.h"
#include "SCA_NORController.h"
#include "SCA_ORController.h"
#include "SCA_RandomSensor.h"
#include "SCA_XNORController.h"
#include "SCA_XORController.h"
#include "KX_IpoActuator.h"
#include "KX_NearSensor.h"
#include "KX_RadarSensor.h"
#include "KX_RaySensor.h"
#include "KX_SCA_DynamicActuator.h"
#include "KX_SoundActuator.h"
#include "KX_TouchSensor.h"
#include "KX_VisibilityActuator.h"
#include "SCA_PropertySensor.h"
#include "SCA_PythonController.h"
#include "SCA_RandomActuator.h"


void initPyObjectPlusType(PyTypeObject **parents)
{
	int i;

	for (i=0; parents[i]; i++) {
		if(PyType_Ready(parents[i]) < 0) {
			/* This is very very unlikely */
			printf("Error, pytype could not initialize, Blender may crash \"%s\"\n", parents[i]->tp_name);
			return;
		}

#if 0
		PyObject_Print((PyObject *)parents[i], stderr, 0);
		fprintf(stderr, "\n");
		PyObject_Print(parents[i]->tp_dict, stderr, 0);
		fprintf(stderr, "\n\n");
#endif

	}

	 PyObject *dict= NULL;

	 while(i) {
		 i--;

		 if (dict) {
			PyDict_Update(parents[i]->tp_dict, dict);
		 }
		 dict= parents[i]->tp_dict;

#if 1
		PyObject_Print((PyObject *)parents[i], stderr, 0);
		fprintf(stderr, "\n");
		PyObject_Print(parents[i]->tp_dict, stderr, 0);
		fprintf(stderr, "\n\n");
#endif

	}
}




static void PyType_Ready_ADD(PyObject *dict, PyTypeObject * tp)
{
	PyType_Ready(tp);
	PyDict_SetItemString(dict, tp->tp_name, (PyObject *)tp);
}


void initPyTypes(void)
{
	
/*
	initPyObjectPlusType(BL_ActionActuator::Parents);
	.....
*/

	/* For now just do PyType_Ready */
	PyObject *mod= PyModule_New("GameTypes");
	PyObject *dict= PyModule_GetDict(mod);
	PyDict_SetItemString(PySys_GetObject("modules"), "GameTypes", mod);
	Py_DECREF(mod);
	
	PyType_Ready_ADD(dict, &BL_ActionActuator::Type);
	PyType_Ready_ADD(dict, &BL_Shader::Type);
	PyType_Ready_ADD(dict, &BL_ShapeActionActuator::Type);
	PyType_Ready_ADD(dict, &CListValue::Type);
	PyType_Ready_ADD(dict, &CValue::Type);
	PyType_Ready_ADD(dict, &KX_BlenderMaterial::Type);
	PyType_Ready_ADD(dict, &KX_CDActuator::Type);
	PyType_Ready_ADD(dict, &KX_Camera::Type);
	PyType_Ready_ADD(dict, &KX_CameraActuator::Type);
	PyType_Ready_ADD(dict, &KX_ConstraintActuator::Type);
	PyType_Ready_ADD(dict, &KX_ConstraintWrapper::Type);
	PyType_Ready_ADD(dict, &KX_GameActuator::Type);
	PyType_Ready_ADD(dict, &KX_GameObject::Type);
	PyType_Ready_ADD(dict, &KX_IpoActuator::Type);
	PyType_Ready_ADD(dict, &KX_LightObject::Type);
	PyType_Ready_ADD(dict, &KX_MeshProxy::Type);
	PyType_Ready_ADD(dict, &KX_MouseFocusSensor::Type);
	PyType_Ready_ADD(dict, &KX_NearSensor::Type);
	PyType_Ready_ADD(dict, &KX_NetworkMessageActuator::Type);
	PyType_Ready_ADD(dict, &KX_NetworkMessageSensor::Type);
	PyType_Ready_ADD(dict, &KX_ObjectActuator::Type);
	PyType_Ready_ADD(dict, &KX_ParentActuator::Type);
	PyType_Ready_ADD(dict, &KX_PhysicsObjectWrapper::Type);
	PyType_Ready_ADD(dict, &KX_PolyProxy::Type);
	PyType_Ready_ADD(dict, &KX_PolygonMaterial::Type);
	PyType_Ready_ADD(dict, &KX_RadarSensor::Type);
	PyType_Ready_ADD(dict, &KX_RaySensor::Type);
	PyType_Ready_ADD(dict, &KX_SCA_AddObjectActuator::Type);
	PyType_Ready_ADD(dict, &KX_SCA_DynamicActuator::Type);
	PyType_Ready_ADD(dict, &KX_SCA_EndObjectActuator::Type);
	PyType_Ready_ADD(dict, &KX_SCA_ReplaceMeshActuator::Type);
	PyType_Ready_ADD(dict, &KX_Scene::Type);
	PyType_Ready_ADD(dict, &KX_SceneActuator::Type);
	PyType_Ready_ADD(dict, &KX_SoundActuator::Type);
	PyType_Ready_ADD(dict, &KX_StateActuator::Type);
	PyType_Ready_ADD(dict, &KX_TouchSensor::Type);
	PyType_Ready_ADD(dict, &KX_TrackToActuator::Type);
	PyType_Ready_ADD(dict, &KX_VehicleWrapper::Type);
	PyType_Ready_ADD(dict, &KX_VertexProxy::Type);
	PyType_Ready_ADD(dict, &KX_VisibilityActuator::Type);
	PyType_Ready_ADD(dict, &PyObjectPlus::Type);
	PyType_Ready_ADD(dict, &SCA_2DFilterActuator::Type);
	PyType_Ready_ADD(dict, &SCA_ANDController::Type);
	PyType_Ready_ADD(dict, &SCA_ActuatorSensor::Type);
	PyType_Ready_ADD(dict, &SCA_AlwaysSensor::Type);
	PyType_Ready_ADD(dict, &SCA_DelaySensor::Type);
	PyType_Ready_ADD(dict, &SCA_ILogicBrick::Type);
	PyType_Ready_ADD(dict, &SCA_IObject::Type);
	PyType_Ready_ADD(dict, &SCA_ISensor::Type);
	PyType_Ready_ADD(dict, &SCA_JoystickSensor::Type);
	PyType_Ready_ADD(dict, &SCA_KeyboardSensor::Type);
	PyType_Ready_ADD(dict, &SCA_MouseSensor::Type);
	PyType_Ready_ADD(dict, &SCA_NANDController::Type);
	PyType_Ready_ADD(dict, &SCA_NORController::Type);
	PyType_Ready_ADD(dict, &SCA_ORController::Type);
	PyType_Ready_ADD(dict, &SCA_PropertyActuator::Type);
	PyType_Ready_ADD(dict, &SCA_PropertySensor::Type);
	PyType_Ready_ADD(dict, &SCA_PythonController::Type);
	PyType_Ready_ADD(dict, &SCA_RandomActuator::Type);
	PyType_Ready_ADD(dict, &SCA_RandomSensor::Type);
	PyType_Ready_ADD(dict, &SCA_XNORController::Type);
	PyType_Ready_ADD(dict, &SCA_XORController::Type);
	
	
	
}

#endif