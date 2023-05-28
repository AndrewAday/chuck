#include "Material.h"

// set static vars
Material* Material::defaultMat = nullptr;

// for settings properties inherent the overall scene, e.g. cameraPos, time, etc.

void Material::SetGlobalUniforms(const GlobalUniforms& globals) {
	BindShader();

	m_Shader.setMat4f("u_Model", globals.u_Model);
	m_Shader.setMat4f("u_View", globals.u_View);
	m_Shader.setMat4f("u_Projection", globals.u_Projection);
	m_Shader.setMat4f("u_Normal", globals.u_Normal);
	m_Shader.setFloat3("u_ViewPos", globals.u_ViewPos);
	m_Shader.setFloat("u_Time", globals.u_Time);

}

// statics

Material* Material::GetDefaultMaterial() {
	if (defaultMat == nullptr)
		defaultMat = new NormalMaterial();

	return defaultMat;
}

void PhongMaterial::SetLocalUniforms() {
	// TODO: can eventually optimize this in batching so we can group shared materials
	auto& shader = GetShader();
	BindShader();

	// set textures
	m_Uniforms.diffuseMap->Bind(0);
	m_Uniforms.specularMap->Bind(1);
	shader.setInt("u_Material.diffuseMap", 0);
	shader.setInt("u_Material.specularMap", 1);

	// set uniforms
	shader.setFloat3("u_Material.diffuseColor", m_Uniforms.diffuseColor);
	shader.setFloat3("u_Material.specularColor", m_Uniforms.specularColor);
	shader.setFloat("u_Material.shininess", std::pow(2.0, m_Uniforms.logShininess));
}
