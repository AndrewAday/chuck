#pragma once

#include "chuck_dl.h"

#include "CGL/scenegraph/SceneGraphObject.h"

#include <condition_variable>
#include <vector>


// exports =========================================
DLL_QUERY cgl_query(Chuck_DL_Query* QUERY);

// frame event
CK_DLL_CTOR(cgl_frame_ctor);
CK_DLL_DTOR(cgl_frame_dtor);

// update event
CK_DLL_CTOR(cgl_update_ctor);
CK_DLL_DTOR(cgl_update_dtor);

// CGL static class
//CK_DLL_CTOR(cgl_ctor);
//CK_DLL_DTOR(cgl_dtor);
CK_DLL_SFUN(cgl_render);
//CK_DLL_SFUN(cgl_main_camera);


// CGLObj
// will impl these after finishing scenegraph
CK_DLL_CTOR(cgl_obj_ctor);
CK_DLL_DTOR(cgl_obj_dtor);

CK_DLL_MFUN(cgl_obj_get_right);
CK_DLL_MFUN(cgl_obj_get_forward);
CK_DLL_MFUN(cgl_obj_get_up);

CK_DLL_MFUN(cgl_obj_translate_by);
CK_DLL_MFUN(cgl_obj_scale_by);
CK_DLL_MFUN(cgl_obj_rot_on_local_axis);
CK_DLL_MFUN(cgl_obj_rot_on_world_axis);
CK_DLL_MFUN(cgl_obj_rot_x);
CK_DLL_MFUN(cgl_obj_rot_y);
CK_DLL_MFUN(cgl_obj_rot_z);
CK_DLL_MFUN(cgl_obj_lookat_vec3);
CK_DLL_MFUN(cgl_obj_lookat_float);

CK_DLL_MFUN(cgl_obj_set_pos);
CK_DLL_MFUN(cgl_obj_set_rot);
CK_DLL_MFUN(cgl_obj_set_scale);

CK_DLL_MFUN(cgl_obj_get_pos);
CK_DLL_MFUN(cgl_obj_get_rot);
CK_DLL_MFUN(cgl_obj_get_scale);

// CGL Camera
CK_DLL_CTOR(cgl_cam_ctor);
CK_DLL_DTOR(cgl_cam_dtor);

// class definitions ===============================

enum class CglEventType {
	CGL_FRAME,		// Class to receive Frame events from GLFW window. assume only 1 window exists
	CGL_UPDATE,			// scenegraph is safe to write to, renderer has finished deepcopy. assume only 1 renderer
	CGL_WINDOW_RESIZE	// triggered on window resize
};

// storage class for thread-safe events
// all events are broadcast on the shared m_event_queue
// (as opposed to creating a separate circular buff for each event like how Osc does it)
class CglEvent  
{
public:
	CglEvent(Chuck_Event* event, Chuck_VM* vm, CglEventType event_type);
	~CglEvent();
	void wait(Chuck_VM_Shred* shred);
	void Broadcast();
	static void Broadcast(CglEventType event_type);
	static std::vector<CglEvent*>& GetEventQueue(CglEventType event_type);
private:
	Chuck_VM* m_VM;
	Chuck_Event* m_Event;
	CglEventType m_EventType;

	// event queues, shared by all events
	static std::vector<CglEvent*> m_FrameEvents;
	static std::vector<CglEvent*> m_UpdateEvents;
	static std::vector<CglEvent*> m_WindowResizeEvents;
};


// catch all class for exposing rendering API, will refactor later
class CGL
{
public:
	static void Render();  // called from chuck side to say update is done! begin deepcopy
	static void WaitOnUpdateDone();
	static bool shouldRender;
	static std::mutex GameLoopLock;
	static std::condition_variable renderCondition;

	static SceneGraphObject mainCamera;

	static Chuck_Event s_UpdateChuckEvent;  // event used for waiting on update()
private:
	


};