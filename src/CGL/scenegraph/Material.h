#pragma once

#include "SceneGraphNode.h"
#include "SceneGraphObject.h"
#include "Texture.h"
#include "Shader.h"
#include <string>
// #include <cmath>

// builtin material type struct
enum class MaterialType {
	Base = 0,
	Normal,
	Phong,
};

// helper struct for global uniforms
struct GlobalUniforms {
	glm::mat4 u_Model, u_View, u_Projection;

	// normals
	glm::mat4 u_Normal;

	// camera
	glm::vec3 u_ViewPos;

	// time
	float u_Time;
};

// Material abstract base class
class Material : public SceneGraphNode
{
public:
	Material(const std::string& vertPath, const std::string& fragPath) 
		: m_Shader(vertPath, fragPath), m_WireFrame(false), m_WireFrameLineWidth(1.0f) {};

	virtual MaterialType GetMaterialType() { return MaterialType::Base; }

	virtual void BindShader() { m_Shader.Bind(); }
	virtual void SetLocalUniforms() = 0;  // for setting properties specific to the material, e.g. color
	virtual Material* Clone(bool copyID = true) = 0;

	// for settings properties inherent the overall scene, e.g. cameraPos, time, etc.
	virtual void SetGlobalUniforms(const GlobalUniforms& globals);;
	Shader& GetShader() { return m_Shader;  }
	std::string GetVertPath() { return m_Shader.GetVertPath(); }
	std::string GetFragPath() { return m_Shader.GetFragPath(); }

public: // statics
	static Material* GetDefaultMaterial();

private:
	static Material* defaultMat;

	Shader m_Shader;
	// TODO: wireframing
	bool m_WireFrame;
	float m_WireFrameLineWidth;
};

// material that colors using worldspace normals as rgb
class NormalMaterial : public Material
{
public:
	NormalMaterial() : 
		Material("../CGL/res/shaders/BasicLightingVert.glsl", "../CGL/res/shaders/NormalFrag.glsl"),
		m_UseLocalNormals(false) {}
	virtual MaterialType GetMaterialType() override { return MaterialType::Normal; }
	virtual void SetLocalUniforms() override {
		auto& shader = GetShader();
		BindShader();

		// set uniforms
		shader.setInt("u_UseLocalNormal", m_UseLocalNormals ? 1 : 0);
	}
	virtual Material* Clone(bool copyID = true) override {
		
		NormalMaterial* normMat = new NormalMaterial();
		// id
		if (copyID) { normMat->SetID(this->GetID()); }

		// material uniforms
		if (m_UseLocalNormals)
			normMat->UseLocalNormals();
		else
			normMat->UseWorldNormals();

		return normMat;
	}
	void UseLocalNormals() { m_UseLocalNormals = true; }
	void UseWorldNormals() { m_UseLocalNormals = false; }
private:
	bool m_UseLocalNormals;
	// none, no textures!
};

// match the materials definition in the frag shader!
struct PhongMatUniforms {
    // textures (TODO change to shared ptr)
	// can't use references here because textures can be unitialized, and they can be reassigned
	Texture * diffuseMap, * specularMap; 
    // colors
    glm::vec3 diffuseColor, specularColor;
    // specular highlights
    float logShininess;  // LOG of shininess. e.g. logShininess = 5 ==> shininess = 2^5 = 32.
};
// phone lighting (ambient + diffuse + specular)
class PhongMaterial : public Material
{
public:
	// TODO: use a locator class or something better to get the shader paths 
	PhongMaterial(
		Texture* diffuseMap = &Texture::DefaultWhiteTexture, 
		Texture* specularMap = &Texture::DefaultWhiteTexture,
		glm::vec3 diffuseColor = glm::vec3(1.0f),
		glm::vec3 specularColor = glm::vec3(1.0f),
		float logShininess = 5  // ==> 32 shininess
	) : Material("./res/shaders/BasicLightingVert.glsl", "./res/shaders/BasicLightingFrag.glsl"),
		m_Uniforms({ diffuseMap, specularMap, diffuseColor, specularColor, logShininess })
	{

	}
	virtual MaterialType GetMaterialType() override { return MaterialType::Phong; }
	virtual void SetLocalUniforms() override;
private:
	PhongMatUniforms m_Uniforms;
};
