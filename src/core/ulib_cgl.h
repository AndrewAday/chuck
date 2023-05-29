#pragma once

#include "chuck_dl.h"

#include "CGL/scenegraph/SceneGraphObject.h"
#include "CGL/scenegraph/Camera.h"
#include "CGL/scenegraph/Scene.h"
#include "CGL/scenegraph/Command.h"
#include "CGL/scenegraph/Geometry.h"

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

// CGL Geometry
CK_DLL_CTOR(cgl_geo_ctor);
CK_DLL_DTOR(cgl_geo_dtor);
CK_DLL_CTOR(cgl_geo_box_ctor);
CK_DLL_CTOR(cgl_geo_sphere_ctor);

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

	static Scene mainScene;
	static SceneGraphObject mainCamera;
	// static PerspectiveCamera mainCamera;

	static Chuck_Event s_UpdateChuckEvent;  // event used for waiting on update()

public: // command queue methods
	// swap the command queue double buffer
	static void SwapCommandQueues() { 

		// grab lock
		std::lock_guard<std::mutex> lock(m_CQLock);

		// swap
		m_CQReadTarget = !m_CQReadTarget;

		// lock released
	}

	// performan all queued commands to sync the renderer scenegraph with the CGL scenegraph
	static void FlushCommandQueue(Scene& scene, bool swap) {  // TODO: shouldn't command be associated with scenes?
		// swap the command queues (so we can read from what was just being written to)
		if (swap)
			SwapCommandQueues();  // Note: this already locks the command queue

		// we no longer need to hold a lock here because all writes are done to the other queue

		// get the new read queue
		std::vector<SceneGraphCommand*>& readQueue = GetReadCommandQueue();\
		
		// std::cout << "flushing " + std::to_string(readQueue.size()) + " commands\n";

		// execute all commands in the read queue
		for (auto& cmd : readQueue) {
			cmd->execute(&scene);
			delete cmd;  // release memory TODO make this a unique_ptr or something instead
		}

		// clear the read queue
		readQueue.clear();
	}

	// adds command to the read queue
	static void PushCommand(SceneGraphCommand * cmd) {
		// lock the command queue
		std::lock_guard<std::mutex> lock(m_CQLock);

		// get the write queue
		std::vector<SceneGraphCommand*>& writeQueue = GetWriteCommandQueue();

		// add the command to the write queue
		writeQueue.push_back(cmd);
	}

private: // attributes
	// command queues 
	// the commands need to be executed before renderering...putting here for now
	static std::vector<SceneGraphCommand*> m_ThisCommandQueue;
	static std::vector<SceneGraphCommand*> m_ThatCommandQueue;
	static bool m_CQReadTarget;  // false = this, true = that
	// command queue lock
	static std::mutex m_CQLock; // only held when 1: adding new command and 2: swapping the read/write queues
private:
	static inline std::vector<SceneGraphCommand*>& GetReadCommandQueue() { 
		return m_CQReadTarget ? m_ThatCommandQueue : m_ThisCommandQueue; 
	}
	// get the write target command queue
	static inline std::vector<SceneGraphCommand*>& GetWriteCommandQueue() {
		return m_CQReadTarget ? m_ThisCommandQueue : m_ThatCommandQueue;
	}

	


};