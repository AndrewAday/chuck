#pragma once

#include "RendererState.h"
#include "SceneGraphObject.h"
#include "Scene.h"
#include "Mesh.h"
#include "Camera.h"
#include "Material.h"
#include "Command.h"

#include <mutex>
#include <vector> 

class VertexArray;
class Shader;
class Geometry;

class Renderer
{
public:
	void Clear(bool color = true, bool depth = true);
	void Draw(VertexArray& va, Shader& shader); // TODO: refactor to use materials
	void Draw(Geometry& geo, Shader& shader); // TODO: refactor to use materials



	// Rendering =======================================================================
	// TODO add cacheing for world matrices
	void RenderScene(Scene* scene, Camera* camera = nullptr) {
		assert(scene->IsScene());

		// optionally change the camera to render from
		if (camera) {
			m_MainCamera = camera;
		}

		// clear the render state
		m_RenderState.Reset();

		// cache camera values
		m_RenderState.ComputeCameraUniforms(m_MainCamera);
		
		// TODO set globals (lighting)
		RenderNodeAndChildren(scene);
	}

	// commmand queue scenegraph state sync ==================================================================

	// performan all queued commands to sync the renderer scenegraph with the CGL scenegraph
	void FlushCommandQueue(Scene& scene) {  // TODO: shouldn't command be associated with scenes?
		// swap the command queues (so we can read from what was just being written to)
		SwapCommandQueues();  // Note: this already locks the command queue

		// we no longer need to hold a lock here because all writes are done to the other queue

		// get the new read queue
		std::vector<SceneGraphCommand*>& readQueue = GetReadCommandQueue();

		// execute all commands in the read queue
		for (auto& cmd : readQueue) {
			cmd->execute(&scene);
			delete cmd;  // release memory TODO make this a unique_ptr or something instead
		}

		// clear the read queue
		readQueue.clear();
	}

	// adds command to the read queue
	void PushCommand(SceneGraphCommand * cmd) {
		// lock the command queue
		std::lock_guard<std::mutex> lock(m_CQLock);

		// get the write queue
		std::vector<SceneGraphCommand*>& writeQueue = GetWriteCommandQueue();

		// add the command to the write queue
		writeQueue.push_back(cmd);
	}

private:  // private methods
	void RenderNodeAndChildren(SceneGraphObject* sgo) {
		// TODO add matrix caching
		glm::mat4 worldTransform = m_RenderState.GetTopTransform() * sgo->GetModelMatrix();
		// glm::mat4 worldTransform = sgo->GetModelMatrix();

		// if its drawable, draw it
		// Note: this allows us to keep the actual render logic and graphics API calls OUT of the SceneGraphObjects
		// the scenegraph structure and data is decoupled from the rendering.
		// scenegraph just provides data; renderer parses and figures out what to draw
		// this decoupling allows supporting multiple renderers in the future
		if (sgo->IsMesh()) {
			RenderMesh(dynamic_cast<Mesh*>(sgo), worldTransform);
		}
		
		// add transform state
		m_RenderState.PushTransform(worldTransform);

		// recursively render children (TODO: should do this inplace in case of deeeeep scenegraphs blowing the stack)
		for (const auto& child : sgo->GetChildren()) {
			RenderNodeAndChildren(child);
		}

		// done rendering children, pop last transform from stack
		m_RenderState.PopTransform();
	}
	
	// TODO change to render command for batch rendering
	void RenderMesh(Mesh* mesh, glm::mat4 worldTransform) {
		Geometry* geo = mesh->GetGeometry();
		Material* mat = mesh->GetMaterial();

		// if no geometry nothing to draw
		if (!geo) { return; }

		// if no material, use default
		if (!mat) { 
			mat = Material::GetDefaultMaterial(); 
		}

		// set uniforms
		mat->SetLocalUniforms();
		mat->SetGlobalUniforms({
			worldTransform,
			m_RenderState.GetViewMat(),
			m_RenderState.GetProjMat(),
			glm::transpose(glm::inverse(worldTransform)),  // TODO cache this normal matrix or move to math util library
			m_RenderState.GetViewPos(),
			0.0f // time
		});

		// draw
		Draw(*geo, mat->GetShader());
	}



private:  // private member vars
	RendererState m_RenderState;
	Camera* m_MainCamera;


	// command queues (TODO is renderer the right place for these to live??)
	// the commands need to be executed before renderering...putting here for now
	std::vector<SceneGraphCommand*> m_ThisCommandQueue;
	std::vector<SceneGraphCommand*> m_ThatCommandQueue;
	bool m_CQReadTarget = false;  // false = this, true = that
	// command queue lock
	std::mutex m_CQLock; // only held when 1: adding new command and 2: swapping the read/write queues

private: // methods
	inline std::vector<SceneGraphCommand*>& GetReadCommandQueue() { 
		return m_CQReadTarget ? m_ThatCommandQueue : m_ThisCommandQueue; 
	}
	// get the write target command queue
	inline std::vector<SceneGraphCommand*>& GetWriteCommandQueue() {
		return m_CQReadTarget ? m_ThisCommandQueue : m_ThatCommandQueue;
	}

	// swap the command queue double buffer
	void SwapCommandQueues() { 
		// grab lock
		std::lock_guard<std::mutex> lock(m_CQLock);

		// swap
		m_CQReadTarget = !m_CQReadTarget;

		// lock released
	}
};

