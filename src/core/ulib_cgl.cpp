#include "ulib_cgl.h"
#include "chuck_vm.h"

#include <stdexcept>
#include <iostream>


// exports =========================================
static t_CKUINT cglframe_data_offset = 0;
static t_CKUINT cglupdate_data_offset = 0;
static t_CKUINT cglobject_data_offset = 0;
static t_CKUINT cglcamera_data_offset = 0;
DLL_QUERY cgl_query(Chuck_DL_Query* QUERY)
{
	// Frame event =================================
	QUERY->begin_class(QUERY, "CglFrame", "Event");
	QUERY->add_ctor(QUERY, cgl_frame_ctor);
	QUERY->add_dtor(QUERY, cgl_frame_dtor);
	// TODO: maybe add a frame count member var
	// TODO: add a glfw time variable (will be different from chuck time)
	// reserve varaible in chuck internal class to store reference
	cglframe_data_offset = QUERY->add_mvar(QUERY, "int", "@cglframe_data", false);
	QUERY->end_class(QUERY);

	// Update event ================================
	// triggered by main render thread after deepcopy is complete, and safe for chuck to begin updating the scene graph
	QUERY->begin_class(QUERY, "CglUpdate", "Event");
	QUERY->add_ctor(QUERY, cgl_update_ctor);
	QUERY->add_dtor(QUERY, cgl_update_dtor);
	cglupdate_data_offset = QUERY->add_mvar(QUERY, "int", "@cglupdate_data", false);
	QUERY->end_class(QUERY);



	// CglObject =========================================
	QUERY->begin_class(QUERY, "CglObject", "Object");
	QUERY->add_ctor(QUERY, cgl_obj_ctor);
	QUERY->add_dtor(QUERY, cgl_obj_dtor);

	// transform getters ===========
	// get obj direction vectors in world space
	QUERY->add_mfun(QUERY, cgl_obj_get_right, "vec3", "GetRight");
	QUERY->add_mfun(QUERY, cgl_obj_get_forward, "vec3", "GetForward");
	QUERY->add_mfun(QUERY, cgl_obj_get_up, "vec3", "GetUp");

	QUERY->add_mfun(QUERY, cgl_obj_get_pos, "vec3", "GetPosition");
	QUERY->add_mfun(QUERY, cgl_obj_get_rot, "vec3", "GetRotation");
	QUERY->add_mfun(QUERY, cgl_obj_get_scale, "vec3", "GetScale");

	// transform setters ===========
	QUERY->add_mfun(QUERY, cgl_obj_translate_by, "CglObject", "TranslateBy");
	QUERY->add_arg(QUERY, "vec3", "trans_vec");

	QUERY->add_mfun(QUERY, cgl_obj_scale_by, "CglObject", "ScaleBy");
	QUERY->add_arg(QUERY, "vec3", "scale_vec");

	QUERY->add_mfun(QUERY, cgl_obj_rot_on_local_axis, "CglObject", "RotateOnLocalAxis");
	QUERY->add_arg(QUERY, "vec3", "axis");
	QUERY->add_arg(QUERY, "float", "deg");

	QUERY->add_mfun(QUERY, cgl_obj_rot_on_world_axis, "CglObject", "RotateOnWorldAxis");
	QUERY->add_arg(QUERY, "vec3", "axis");
	QUERY->add_arg(QUERY, "float", "deg");

	QUERY->add_mfun(QUERY, cgl_obj_rot_x, "CglObject", "RotateX");
	QUERY->add_arg(QUERY, "float", "deg");

	QUERY->add_mfun(QUERY, cgl_obj_rot_y, "CglObject", "RotateY");
	QUERY->add_arg(QUERY, "float", "deg");

	QUERY->add_mfun(QUERY, cgl_obj_rot_z, "CglObject", "RotateZ");
	QUERY->add_arg(QUERY, "float", "deg");

	QUERY->add_mfun(QUERY, cgl_obj_lookat_vec3, "CglObject", "LookAt");
	QUERY->add_arg(QUERY, "vec3", "pos");

	QUERY->add_mfun(QUERY, cgl_obj_lookat_float, "CglObject", "LookAt");
	QUERY->add_arg(QUERY, "float", "x");
	QUERY->add_arg(QUERY, "float", "y");
	QUERY->add_arg(QUERY, "float", "z");

	QUERY->add_mfun(QUERY, cgl_obj_set_pos, "CglObject", "SetPosition");
	QUERY->add_arg(QUERY, "vec3", "pos_vec");

	QUERY->add_mfun(QUERY, cgl_obj_set_rot, "CglObject", "SetRotation");  // sets from eulers
	QUERY->add_arg(QUERY, "vec3", "eulers");

	QUERY->add_mfun(QUERY, cgl_obj_set_scale, "CglObject", "SetScale");  
	QUERY->add_arg(QUERY, "vec3", "scale");

	cglobject_data_offset = QUERY->add_mvar(QUERY, "int", "@cglobject_data", false);
	QUERY->end_class(QUERY);

	// CGL camera
	QUERY->begin_class(QUERY, "CglCamera", "CglObject");
	QUERY->add_ctor(QUERY, cgl_cam_ctor);
	QUERY->add_dtor(QUERY, cgl_cam_dtor);
	//cglcamera_data_offset = QUERY->add_mvar(QUERY, "int", "@cglcamera_data", false);
	QUERY->end_class(QUERY);

	// CGL for global stuff ===============================
	QUERY->begin_class(QUERY, "CGL", "Object");
	QUERY->add_sfun(QUERY, cgl_render, "void", "Render");
	QUERY->end_class(QUERY);


	return TRUE;
}

/*============CglFrame Event============*/
CK_DLL_CTOR(cgl_frame_ctor)
{
	// store reference to our new class
	OBJ_MEMBER_INT(SELF, cglframe_data_offset) = (t_CKINT) new CglEvent(
		(Chuck_Event*)SELF, SHRED->vm_ref, CglEventType::CGL_FRAME
	);
}
CK_DLL_DTOR(cgl_frame_dtor)
{
	CglEvent* cglEvent = (CglEvent*)OBJ_MEMBER_INT(SELF, cglframe_data_offset);
	SAFE_DELETE(cglEvent);
	OBJ_MEMBER_INT(SELF, cglframe_data_offset) = 0;
}
/*============CglUpdate Event============*/
CK_DLL_CTOR(cgl_update_ctor)
{
	// store reference to our new class
	OBJ_MEMBER_INT(SELF, cglupdate_data_offset) = (t_CKINT) new CglEvent(
		(Chuck_Event*)SELF, SHRED->vm_ref, CglEventType::CGL_UPDATE
	);
}
CK_DLL_DTOR(cgl_update_dtor)
{
	CglEvent* cglEvent = (CglEvent*)OBJ_MEMBER_INT(SELF, cglupdate_data_offset);
	SAFE_DELETE(cglEvent);
	OBJ_MEMBER_INT(SELF, cglupdate_data_offset) = 0;
}

/*============CGL static fns============*/
/*
CK_DLL_CTOR(cgl_ctor)
{
	// store reference to our new class
	OBJ_MEMBER_INT(SELF, cgl_data_offset) = (t_CKINT) new CGL();
}

CK_DLL_DTOR(cgl_dtor)
{
	CGL* cgl = (CGL*)OBJ_MEMBER_INT(SELF, cgl_data_offset);
	SAFE_DELETE(cgl);
	OBJ_MEMBER_INT(SELF, cgl_data_offset) = 0;
}
*/

CK_DLL_SFUN(cgl_render)
{
	// before notifying render thread, place calling shred on 
	// the event queue for the CglUpdate event
	// this is to prevent a deadlock condition where
	// the renderer broadcasts UpdateEvent and begins waiting on the condition var
	// BEFORE the chuck shred places itself on the update event queue
	//VM->

	// create new CGL update event (TODO could optimize by making this a static event
	// rather than allocating every time, if we enforce only one shred can wait on it per program)
	//CglEvent* updateEvent = new CglEvent(
	//	&CGL::s_UpdateChuckEvent, VM, CglEventType::CGL_UPDATE
	//);
	//std::cout << " adding shred to update event\n";
	//CGL::s_UpdateChuckEvent.wait(SHRED, VM);  // add shred to event queue

	//std::cout << " calling CGL Render()\n";

	// wakeup renderer thread
	CGL::Render();
}

// TODO: what's the right way to do this?
//CK_DLL_SFUN(cgl_main_camera)
//{
//	RETURN->v_object = CGL::mainCamera();
//	CGL::Render();
//}

// CGLObject DLL ==============================================
CK_DLL_CTOR(cgl_obj_ctor)
{
	OBJ_MEMBER_INT(SELF, cglobject_data_offset) = (t_CKINT) new SceneGraphObject();
}
CK_DLL_DTOR(cgl_obj_dtor)
{
	SceneGraphObject* cglObj = (SceneGraphObject*)OBJ_MEMBER_INT(SELF, cglobject_data_offset);
	SAFE_DELETE(cglObj);
	OBJ_MEMBER_INT(SELF, cglobject_data_offset) = 0;
}

CK_DLL_MFUN(cgl_obj_get_right)
{
	SceneGraphObject* cglObj = (SceneGraphObject*)OBJ_MEMBER_INT(SELF, cglobject_data_offset);
	const auto& right = cglObj->GetRight();
	RETURN->v_vec3 = { right.x, right.y, right.z };
}
CK_DLL_MFUN(cgl_obj_get_forward)
{
	SceneGraphObject* cglObj = (SceneGraphObject*)OBJ_MEMBER_INT(SELF, cglobject_data_offset);
	const auto& vec = cglObj->GetForward();
	RETURN->v_vec3 = { vec.x, vec.y, vec.z };
}

CK_DLL_MFUN(cgl_obj_get_up)
{
	SceneGraphObject* cglObj = (SceneGraphObject*)OBJ_MEMBER_INT(SELF, cglobject_data_offset);
	const auto& vec = cglObj->GetUp();
	RETURN->v_vec3 = { vec.x, vec.y, vec.z };
}


CK_DLL_MFUN(cgl_obj_translate_by)
{
	SceneGraphObject* cglObj = (SceneGraphObject*) OBJ_MEMBER_INT (SELF, cglobject_data_offset);
	t_CKVEC3 trans = GET_NEXT_VEC3(ARGS);
	cglObj->Translate(glm::vec3(trans.x, trans.y, trans.z));
	RETURN->v_object = SELF;
}

CK_DLL_MFUN(cgl_obj_scale_by)
{
	SceneGraphObject* cglObj = (SceneGraphObject*) OBJ_MEMBER_INT (SELF, cglobject_data_offset);
	t_CKVEC3 vec = GET_NEXT_VEC3(ARGS);
	cglObj->Scale(glm::vec3(vec.x, vec.y, vec.z));
	RETURN->v_object = SELF;
}

CK_DLL_MFUN(cgl_obj_rot_on_local_axis)
{
	SceneGraphObject* cglObj = (SceneGraphObject*) OBJ_MEMBER_INT (SELF, cglobject_data_offset);
	t_CKVEC3 vec = GET_NEXT_VEC3(ARGS);
	t_CKFLOAT deg = GET_NEXT_FLOAT(ARGS);
	cglObj->RotateOnLocalAxis(glm::vec3(vec.x, vec.y, vec.z), deg);
	RETURN->v_object = SELF;
}

CK_DLL_MFUN(cgl_obj_rot_on_world_axis)
{
	SceneGraphObject* cglObj = (SceneGraphObject*) OBJ_MEMBER_INT (SELF, cglobject_data_offset);
	t_CKVEC3 vec = GET_NEXT_VEC3(ARGS);
	t_CKFLOAT deg = GET_NEXT_FLOAT(ARGS);
	cglObj->RotateOnWorldAxis(glm::vec3(vec.x, vec.y, vec.z), deg);
	RETURN->v_object = SELF;
}

CK_DLL_MFUN(cgl_obj_rot_x)
{
	SceneGraphObject* cglObj = (SceneGraphObject*) OBJ_MEMBER_INT (SELF, cglobject_data_offset);
	t_CKFLOAT deg = GET_NEXT_FLOAT(ARGS);
	cglObj->RotateX(deg);
	RETURN->v_object = SELF;
}

CK_DLL_MFUN(cgl_obj_rot_y)
{
	SceneGraphObject* cglObj = (SceneGraphObject*) OBJ_MEMBER_INT (SELF, cglobject_data_offset);
	t_CKFLOAT deg = GET_NEXT_FLOAT(ARGS);
	cglObj->RotateY(deg);
	RETURN->v_object = SELF;
}

CK_DLL_MFUN(cgl_obj_rot_z)
{
	SceneGraphObject* cglObj = (SceneGraphObject*) OBJ_MEMBER_INT (SELF, cglobject_data_offset);
	t_CKFLOAT deg = GET_NEXT_FLOAT(ARGS);
	cglObj->RotateZ(deg);
	RETURN->v_object = SELF;
}

CK_DLL_MFUN(cgl_obj_lookat_vec3)
{
	SceneGraphObject* cglObj = (SceneGraphObject*) OBJ_MEMBER_INT (SELF, cglobject_data_offset);
	t_CKVEC3 vec = GET_NEXT_VEC3(ARGS);
	cglObj->LookAt(glm::vec3(vec.x, vec.y, vec.z));
	RETURN->v_object = SELF;
}

CK_DLL_MFUN(cgl_obj_lookat_float)
{
	SceneGraphObject* cglObj = (SceneGraphObject*) OBJ_MEMBER_INT (SELF, cglobject_data_offset);
	t_CKFLOAT x = GET_NEXT_FLOAT(ARGS);
	t_CKFLOAT y = GET_NEXT_FLOAT(ARGS);
	t_CKFLOAT z = GET_NEXT_FLOAT(ARGS);
	cglObj->LookAt(glm::vec3(x, y, z));
	RETURN->v_object = SELF;
}

CK_DLL_MFUN(cgl_obj_set_pos)
{
	SceneGraphObject* cglObj = (SceneGraphObject*) OBJ_MEMBER_INT (SELF, cglobject_data_offset);
	t_CKVEC3 vec = GET_NEXT_VEC3(ARGS);
	cglObj->SetPosition(glm::vec3(vec.x, vec.y, vec.z));
	RETURN->v_object = SELF;
}

CK_DLL_MFUN(cgl_obj_set_rot)
{
	SceneGraphObject* cglObj = (SceneGraphObject*) OBJ_MEMBER_INT (SELF, cglobject_data_offset);
	t_CKVEC3 vec = GET_NEXT_VEC3(ARGS);
	cglObj->SetRotation(glm::vec3(vec.x, vec.y, vec.z));
	RETURN->v_object = SELF;
}

CK_DLL_MFUN(cgl_obj_set_scale)
{
	SceneGraphObject* cglObj = (SceneGraphObject*) OBJ_MEMBER_INT (SELF, cglobject_data_offset);
	t_CKVEC3 vec = GET_NEXT_VEC3(ARGS);
	cglObj->SetScale(glm::vec3(vec.x, vec.y, vec.z));
	RETURN->v_object = SELF;
}

CK_DLL_MFUN(cgl_obj_get_pos)
{
	SceneGraphObject* cglObj = (SceneGraphObject*) OBJ_MEMBER_INT (SELF, cglobject_data_offset);
	const auto& vec = cglObj->GetPosition();
	RETURN->v_vec3 = { vec.x, vec.y, vec.z };
}

CK_DLL_MFUN(cgl_obj_get_rot)
{
	SceneGraphObject* cglObj = (SceneGraphObject*) OBJ_MEMBER_INT (SELF, cglobject_data_offset);
	const auto& vec = glm::degrees(glm::eulerAngles(cglObj->GetRotation()));
	RETURN->v_vec3 = { vec.x, vec.y, vec.z };
}

CK_DLL_MFUN(cgl_obj_get_scale)
{
	SceneGraphObject* cglObj = (SceneGraphObject*) OBJ_MEMBER_INT (SELF, cglobject_data_offset);
	const auto& vec = cglObj->GetScale();
	RETURN->v_vec3 = { vec.x, vec.y, vec.z };
}

// CGL Camera =======================

CK_DLL_CTOR(cgl_cam_ctor)
{
	// store reference to main camera 
	// NOT A BUG: camera inherits methods from cglobject, so it needs 
	// to use the same offset. wtf!!
	// TODO: ask Ge is this is the right way to do inheritence in this DLL interface
	//OBJ_MEMBER_INT(SELF, cglcamera_data_offset) = (t_CKINT) &CGL::mainCamera;
	OBJ_MEMBER_INT(SELF, cglobject_data_offset) = (t_CKINT) &CGL::mainCamera;
}
CK_DLL_DTOR(cgl_cam_dtor)
{
	//SceneGraphObject* mainCam = (SceneGraphObject*)OBJ_MEMBER_INT(SELF, cglcamera_data_offset);
	//// don't call delete! because this is a static var
	//OBJ_MEMBER_INT(SELF, cglcamera_data_offset) = 0;  // zero out the memory

	SceneGraphObject* mainCam = (SceneGraphObject*)OBJ_MEMBER_INT(SELF, cglobject_data_offset);
	// don't call delete! because this is a static var
	OBJ_MEMBER_INT(SELF, cglobject_data_offset) = 0;  // zero out the memory
}

// CglEvent ========================================

// CglEventstatic initialization (again, should be refactored to be accessible through chuck.h)
std::vector<CglEvent*> CglEvent::m_FrameEvents;
std::vector<CglEvent*> CglEvent::m_UpdateEvents;
std::vector<CglEvent*> CglEvent::m_WindowResizeEvents;

// CGL static initialization
bool CGL::shouldRender = false;
std::mutex CGL::GameLoopLock;
std::condition_variable CGL::renderCondition;
SceneGraphObject CGL::mainCamera;
Chuck_Event CGL::s_UpdateChuckEvent;


std::vector<CglEvent*>& CglEvent::GetEventQueue(CglEventType type)
{
	switch (type) {
	case CglEventType::CGL_UPDATE:			return m_UpdateEvents;
	case CglEventType::CGL_FRAME:			return m_FrameEvents;
	case CglEventType::CGL_WINDOW_RESIZE:	return m_WindowResizeEvents;
	default:								throw std::runtime_error("invalid CGL event type");
	}
	
}

void CglEvent::wait(Chuck_VM_Shred* shred)
{
	m_Event->wait(shred, m_VM);
}

CglEvent::CglEvent(Chuck_Event* event, Chuck_VM* vm, CglEventType event_type)
	: m_Event(event), m_VM(vm), m_EventType(event_type)
{
	GetEventQueue(event_type).push_back(this);
}

CglEvent::~CglEvent()
{
	auto& eventQueue = GetEventQueue(m_EventType);

	// remove from listeners list
	auto it = std::find(eventQueue.begin(), eventQueue.end(), this);
	assert(it != eventQueue.end());  // sanity check
	if (it != eventQueue.end()) {
		eventQueue.erase(it);
	}
}

void CglEvent::Broadcast()
{
	// using non-thread-safe event buffer for now
	m_VM->queue_event(m_Event, 1, NULL);
}

// broadcasts all events of type event_type
void CglEvent::Broadcast(CglEventType event_type)
{
	// hack for now
	//if (event_type == CglEventType::CGL_UPDATE) {
	//	m_VM->queue_event(CGL::s_UpdateChuckEvent
	//}

	// normal case
	auto& eventQueue = GetEventQueue(event_type);

	for (auto& event : eventQueue)
	{
		event->Broadcast();
	}
}

// CGL impl ==============================================

void CGL::WaitOnUpdateDone() {
	// std::cout << "waiting for chuck to finish updating and call Render()\n";
	std::unique_lock<std::mutex> lock(GameLoopLock);
	renderCondition.wait(lock, []() { return shouldRender; });
	CGL::shouldRender = false;  // set back to false
	// lock auto releases in destructor
}

// can pick a better name maybe...calling this notifes the renderer its safe to proceed
void CGL::Render()
{
	//std::cout << "Render()\n";
	std::unique_lock<std::mutex> lock(CGL::GameLoopLock);
	shouldRender = true;
	lock.unlock();

	renderCondition.notify_one();  // wakeup the blocking render thread
}


