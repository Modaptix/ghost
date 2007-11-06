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

#ifndef BL_ACTIONACTUATOR
#define BL_ACTIONACTUATOR

#include "GEN_HashedPtr.h"
#include "SCA_IActuator.h"
#include "MT_Point3.h"

class BL_ActionActuator : public SCA_IActuator  
{
public:
	Py_Header;
	BL_ActionActuator(SCA_IObject* gameobj,
						const STR_String& propname,
						float starttime,
						float endtime,
						struct bAction *action,
						short	playtype,
						short	blendin,
						short	priority,
						float	stride,
						PyTypeObject* T=&Type) 
		: SCA_IActuator(gameobj,T),
		
		m_lastpos(0, 0, 0),
		m_blendframe(0),
		m_flag(0),
		m_startframe (starttime),
		m_endframe(endtime) ,
		m_starttime(0),
		m_localtime(starttime),
		m_lastUpdate(-1),
		m_blendin(blendin),
		m_blendstart(0),
		m_stridelength(stride),
		m_playtype(playtype),
		m_priority(priority),
		m_pose(NULL),
		m_blendpose(NULL),
		m_userpose(NULL),
		m_action(action),
		m_propname(propname)
	{
	};
	virtual ~BL_ActionActuator();
	virtual	bool Update(double curtime, bool frame);
	virtual CValue* GetReplica();
	virtual void ProcessReplica();
	
	void SetBlendTime (float newtime);

	KX_PYMETHOD_DOC(BL_ActionActuator,SetAction);
	KX_PYMETHOD_DOC(BL_ActionActuator,SetBlendin);
	KX_PYMETHOD_DOC(BL_ActionActuator,SetPriority);
	KX_PYMETHOD_DOC(BL_ActionActuator,SetStart);
	KX_PYMETHOD_DOC(BL_ActionActuator,SetEnd);
	KX_PYMETHOD_DOC(BL_ActionActuator,SetFrame);
	KX_PYMETHOD_DOC(BL_ActionActuator,SetProperty);
	KX_PYMETHOD_DOC(BL_ActionActuator,SetBlendtime);
	KX_PYMETHOD_DOC(BL_ActionActuator,SetChannel);

	KX_PYMETHOD_DOC(BL_ActionActuator,GetAction);
	KX_PYMETHOD_DOC(BL_ActionActuator,GetBlendin);
	KX_PYMETHOD_DOC(BL_ActionActuator,GetPriority);
	KX_PYMETHOD_DOC(BL_ActionActuator,GetStart);
	KX_PYMETHOD_DOC(BL_ActionActuator,GetEnd);
	KX_PYMETHOD_DOC(BL_ActionActuator,GetFrame);
	KX_PYMETHOD_DOC(BL_ActionActuator,GetProperty);
//	KX_PYMETHOD(BL_ActionActuator,GetChannel);
	KX_PYMETHOD_DOC(BL_ActionActuator,GetType);
	KX_PYMETHOD_DOC(BL_ActionActuator,SetType);

	virtual PyObject* _getattr(const STR_String& attr);

	enum ActionActType
	{
		KX_ACT_ACTION_PLAY = 0,
		KX_ACT_ACTION_FLIPPER = 2,
		KX_ACT_ACTION_LOOPSTOP,
		KX_ACT_ACTION_LOOPEND,
		KX_ACT_ACTION_PROPERTY = 6
	};

protected:

	void SetStartTime(float curtime);
	void SetLocalTime(float curtime);
	bool ClampLocalTime();

	MT_Point3	m_lastpos;
	float	m_blendframe;
	int		m_flag;
	/** The frame this action starts */
	float	m_startframe;
	/** The frame this action ends */
	float	m_endframe;
	/** The time this action started */
	float	m_starttime;
	/** The current time of the action */
	float	m_localtime;
	
	float	m_lastUpdate;
	float	m_blendin;
	float	m_blendstart;
	float	m_stridelength;
	short	m_playtype;
	short	m_priority;
	struct bPose* m_pose;
	struct bPose* m_blendpose;
	struct bPose* m_userpose;
	struct bAction *m_action;
	STR_String	m_propname;
};

enum {
	ACT_FLAG_REVERSE	= 0x00000001,
	ACT_FLAG_LOCKINPUT	= 0x00000002,
	ACT_FLAG_KEYUP		= 0x00000004,
	ACT_FLAG_ACTIVE		= 0x00000008
};

#endif

