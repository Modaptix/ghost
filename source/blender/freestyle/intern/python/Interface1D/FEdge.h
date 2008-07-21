#ifndef FREESTYLE_PYTHON_FEDGE_H
#define FREESTYLE_PYTHON_FEDGE_H

#include "../Interface1D.h"
#include "../../view_map/Silhouette.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////

#include <Python.h>

extern PyTypeObject FEdge_Type;

#define BPy_FEdge_Check(v)	(( (PyObject *) v)->ob_type == &FEdge_Type)

/*---------------------------Python BPy_FEdge structure definition----------*/
typedef struct {
	BPy_Interface1D py_if1D;
	FEdge *fe;
} BPy_FEdge;

///////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif /* FREESTYLE_PYTHON_FEDGE_H */
