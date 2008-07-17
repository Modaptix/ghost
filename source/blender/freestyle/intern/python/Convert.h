#ifndef FREESTYLE_PYTHON_CONVERT_H
#define FREESTYLE_PYTHON_CONVERT_H

#include "../geometry/Geom.h"
using namespace Geometry;

#include "Id.h"
#include "Interface0D/SVertex.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////

#include <Python.h>
#include "api2_2x/vector.h"
#include "api2_2x/gen_utils.h"

PyObject *PyBool_from_bool( bool b );

PyObject *Vector_from_Vec2f( Vec2f v );
PyObject *Vector_from_Vec3f( Vec3f v );
PyObject *Vector_from_Vec3r( Vec3r v );

PyObject *BPy_Id_from_Id( Id id );
PyObject *BPy_SVertex_from_SVertex( SVertex sv );

///////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif


#endif /* FREESTYLE_PYTHON_CONVERT_H */
