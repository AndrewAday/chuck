#pragma once

#include "Material.h"
#include "Geometry.h"
#include "Group.h"
#include "Scene.h"
#include "Mesh.h"
#include "Scene.h"



class SceneGraphCommand
{
public:
	virtual ~SceneGraphCommand() {}
	virtual void execute(Scene* scene) = 0;
};



//==================== Creation Commands =====a==================//
// TODO: all this creation command logic can be moved into the classes themselves
// add a virtual Clone() = 0 to base class SceneGraphNode

// TODO: should use observer pattern here, create hooks for all
// creation commands that the renderer can listen to in order to
// setup GPU-side data 

class CreateMaterialCommand : public SceneGraphCommand
{
public:
    CreateMaterialCommand(Material* mat) : mat(mat) {
        assert(mat->GetMaterialType() != MaterialType::Base);  // must be a concrete material
    };
    virtual void execute(Scene* scene) override {
        Material* newMat = mat->Clone(true);
        std::cout << "copied material with id: " + std::to_string(newMat->GetID()) 
                  << std::endl;

        scene->RegisterNode(newMat);
    }
private:
    Material* mat;
};

// create geometry
class CreateGeometryCommand : public SceneGraphCommand
{
public:
    CreateGeometryCommand(Geometry* geo) : geo(geo) {};
    virtual void execute(Scene* scene) override {
        Geometry* newGeo = geo->Clone();

        // assign the gpu buffer data
        // JK we do this in renderer instead
        // newGeo->BuildGeometry();

        newGeo->SetID(geo->GetID());  // copy ID
        std::cout << "copied geometry with id: " + std::to_string(newGeo->GetID())
            << std::endl;

        scene->RegisterNode(newGeo);
    }

private:
    Geometry* geo;
};

// create Group
class CreateGroupCommand : public SceneGraphCommand
{
public:
    CreateGroupCommand(Group* group) : group(group) {};
    virtual void execute(Scene* scene) override {  // TODO: just add virtual clone() interface to scenegraph node
        Group* newGroup = new Group();
        newGroup->SetID(group->GetID());  // copy ID
        std::cout << "copied group with id: " + std::to_string(newGroup->GetID())
            << std::endl;

        scene->RegisterNode(newGroup);
    }
private:
    Group* group;
};

// create Mesh
class CreateMeshCommand : public SceneGraphCommand
{
public:
    CreateMeshCommand(Mesh* mesh) : mesh(mesh) {};
    virtual void execute(Scene* scene) override {
        // Get the cloned material and geometry
        Material* clonedMat = nullptr;
        Geometry* clonedGeo = nullptr;

        if (mesh->GetMaterial())
            clonedMat = dynamic_cast<Material*>(scene->GetNode(mesh->GetMaterial()->GetID()));
        if (mesh->GetGeometry())
            clonedGeo = dynamic_cast<Geometry*>(scene->GetNode(mesh->GetGeometry()->GetID()));

        Mesh* newMesh = new Mesh(clonedGeo, clonedMat);
        newMesh->SetID(mesh->GetID());  // copy ID
        std::cout << "copied mesh with id: " + std::to_string(newMesh->GetID())
            << std::endl;

        scene->RegisterNode(newMesh);
    }

private:
    Mesh* mesh;
};

// create Camera
class CreateCameraCommand : public SceneGraphCommand
{
public:
    CreateCameraCommand(Camera* camera) : m_Camera(camera) {};
    virtual void execute(Scene* scene) override {
        Camera* newCamera = m_Camera->Clone();
        newCamera->SetID(m_Camera->GetID());  // copy ID
        std::cout << "copied camera with id: " + std::to_string(newCamera->GetID())
            << std::endl;

        scene->RegisterNode(newCamera);
    }
private:
    Camera* m_Camera;
};

// Create Scene
class CreateSceneCommand : public SceneGraphCommand
{
public:
    CreateSceneCommand(Scene* scene) : m_Scene(scene) {};

    // TODO: this is weird, we have to pass in a scene pointer to create a scene...
    // basically the only point is to copy the ID and register itself in its node map
    virtual void execute(Scene* scene) override {
        scene->SetID(m_Scene->GetID());  // copy ID
        std::cout << "copied scene with id: " + std::to_string(m_Scene->GetID())
            << std::endl;

        scene->RegisterNode(scene);  // register itself so it shows up in node lookups
    }
private:
    Scene* m_Scene;
};

//==================== SceneGraph Relationship Commands =======================//

// add child
class AddChildCommand : public SceneGraphCommand
{
public:
    AddChildCommand(SceneGraphObject* parent, SceneGraphObject* child) :
        m_ParentID(parent->GetID()), m_ChildID(child->GetID()) {};
    virtual void execute(Scene* scene) override {
        SceneGraphObject* parent = dynamic_cast<SceneGraphObject*>(scene->GetNode(m_ParentID));
        SceneGraphObject* child = dynamic_cast<SceneGraphObject*>(scene->GetNode(m_ChildID));

        assert(parent && child);
        parent->AddChild(child);
    }
private:
    size_t m_ParentID, m_ChildID;
};


//==================== Parameter Modification Commands =======================//

// set transform
class TransformCommand : public SceneGraphCommand
{
public:
    TransformCommand(SceneGraphObject* obj) :
        m_ID(obj->GetID()), m_Position(obj->GetPosition()), m_Rotation(obj->GetRotation()), m_Scale(obj->GetScale())
    {}
    virtual void execute(Scene* scene) override {
        SceneGraphObject* obj = dynamic_cast<SceneGraphObject*>(scene->GetNode(m_ID));
        assert(obj);
        obj->SetPosition(m_Position);
        obj->SetRotation(m_Rotation);
        obj->SetScale(m_Scale);
    }

private:
    size_t m_ID; // which scenegraph object to modify

    // transform  (making public for now for easier debug)
    glm::vec3 m_Position;
    glm::quat m_Rotation;
    glm::vec3 m_Scale;
};

// Set Mesh params (geometry and material)
class SetMeshCommand : public SceneGraphCommand
{

};