#pragma once

#include "SceneGraphNode.h"
// TODO: move these rendererAPI abstractions into their own directory
#include "CGL/VertexBufferLayout.h"
#include "CGL/VertexArray.h"
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/epsilon.hpp"
#include <vector>

enum class GeometryType {
	Base = 0,
	Box,
	Sphere,
	Cylinder,
	Cone,
	Plane,
	Quad,
	Custom
};

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
	
	static float& VecIndex(glm::vec3& vec, char c) {
		if (c == 'x' || c == 'r')
			return vec.x;
		if (c == 'y' || c == 'g')
			return vec.y;
		if (c == 'z' || c == 'b')
			return vec.z;
	}

	float& Pos(char c) { return VecIndex(Position, c); }
	float& Norm(char c) { return VecIndex(Normal, c); }
};

struct Index {
	unsigned int a, b, c;
};

// returns reference to float at index 'c'

class Geometry : public SceneGraphNode // abstract base class for buffered geometry data
{
public:
	virtual ~Geometry() {}
	virtual GeometryType GetGeoType() = 0;
	virtual Geometry* Clone() = 0;  // deepcopy the geometry data

	std::vector<Index> m_Indices;
	std::vector<Vertex> m_Vertices;
};

class BoxGeometry : public Geometry
{
public:
	BoxGeometry(
		float width = 1, float height = 1, float depth = 1,
		int widthSeg = 1, int heightSeg = 1, int depthSeg = 1
	);
	virtual GeometryType GetGeoType() { return GeometryType::Box; }
	virtual Geometry* Clone() override { 
		return new BoxGeometry(m_Width, m_Height, m_Depth, m_WidthSeg, m_HeightSeg, m_DepthSeg);
	}

	float m_Width, m_Height, m_Depth;
	int m_WidthSeg, m_HeightSeg, m_DepthSeg;
private:
	void buildPlane(
		char u, char v, char w, 
		int udir, int vdir,  // uv dirs
		float width, float height, float depth, 
		int gridX, int gridY, // how much we subdivide
		int materialIndex  // for allowing groups within a single geometry. ignore for now
	);
};

class SphereGeometry : public Geometry
{
public:
	SphereGeometry(
		float radius = 1.0, int widthSegments = 32, int heightSegments = 16, 
		float phiStart = 0.0, float phiLength = glm::pi<float>() * 2.0, // how much along circumference
		float thetaStart = 0.0, float thetaLength = glm::pi<float>()  // how much along central diameter
	);
	virtual GeometryType GetGeoType() { return GeometryType::Box; }
	virtual Geometry* Clone() override { 
		return new SphereGeometry(
			m_Radius, m_WidthSeg, m_HeightSeg, m_PhiStart, m_PhiLength, m_ThetaStart, m_ThetaLength
		); 
	}

public:
	float m_Radius;
	int m_WidthSeg, m_HeightSeg;
	float m_PhiStart, m_PhiLength;
	float m_ThetaStart, m_ThetaLength;
};

