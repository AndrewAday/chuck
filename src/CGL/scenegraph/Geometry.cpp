#include "Geometry.h"

/* =============================================================================
									Sphere Geo
===============================================================================*/

SphereGeometry::SphereGeometry(float radius, int widthSegments, int heightSegments, float phiStart, float phiLength, float thetaStart, float thetaLength) : m_Radius(radius), m_WidthSeg(widthSegments), m_HeightSeg(heightSegments),
m_PhiStart(phiStart), m_PhiLength(phiLength),
m_ThetaStart(thetaStart), m_ThetaLength(thetaLength)
{
	constexpr float pi = glm::pi<float>();
	const float epsilon = .00001f; // tolerance
	widthSegments = std::max(3, widthSegments);
	heightSegments = std::max(2, heightSegments);

	const float thetaEnd = std::min(thetaStart + thetaLength, pi);

	unsigned int index = 0;
	std::vector<unsigned int> grid;


	// generate vertices, normals and uvs
	for (int iy = 0; iy <= heightSegments; iy++) {

		const float v = (float)iy / (float)heightSegments;

		// special case for the poles
		float uOffset = 0;
		if (iy == 0 && glm::epsilonEqual(thetaStart, 0.0f, epsilon)) {
			uOffset = 0.5f / widthSegments;
		}
		else if (iy == heightSegments && glm::epsilonEqual(thetaEnd, pi, epsilon)) {
			uOffset = -0.5 / widthSegments;
		}

		for (int ix = 0; ix <= widthSegments; ix++) {

			const float u = (float)ix / (float)widthSegments;

			Vertex vert;

			// vertex
			vert.Position.x = -radius * glm::cos(phiStart + u * phiLength) * glm::sin(thetaStart + v * thetaLength);
			vert.Position.y = radius * glm::cos(thetaStart + v * thetaLength);
			vert.Position.z = radius * glm::sin(phiStart + u * phiLength) * glm::sin(thetaStart + v * thetaLength);

			// normal
			vert.Normal = glm::normalize(vert.Position);

			// uv
			vert.TexCoords.x = u + uOffset;
			vert.TexCoords.y = 1 - v;

			m_Vertices.push_back(vert);

			grid.push_back(index++);
		}
	}

	// indices

	const size_t rowSize = (size_t)widthSegments + 1;
	for (size_t iy = 0; iy < heightSegments; iy++) {
		for (size_t ix = 0; ix < widthSegments; ix++) {

			const unsigned int a = grid[(iy * rowSize) + ix + 1];
			const unsigned int b = grid[(iy * rowSize) + ix];
			const unsigned int c = grid[(rowSize * (iy + 1)) + ix];
			const unsigned int d = grid[rowSize * (iy + 1) + (ix + 1)];

			if (iy != 0 || thetaStart > epsilon) {
				m_Indices.push_back({ a, b, d });
			};
			if (iy != (size_t)heightSegments - 1 || thetaEnd < pi - epsilon) {
				m_Indices.push_back({ b, c, d });
			};
		}
	}

	// setup vao (no don't do this by default!!)
	// BuildGeometry();
}

/* =============================================================================
									Box Geo
===============================================================================*/

BoxGeometry::BoxGeometry(float width, float height, float depth, int widthSeg, int heightSeg, int depthSeg) : m_Width(width), m_Height(height), m_Depth(depth),
m_WidthSeg(widthSeg), m_HeightSeg(heightSeg), m_DepthSeg(depthSeg)
{
	std::cout << "calling boxgeo constructor\n";
	// buffers

	// helper variables
	// int groupStart = 0;

	// build each side of the box geometry
	buildPlane('z', 'y', 'x', -1, -1, depth, height, width, m_DepthSeg, m_HeightSeg, 0); // px
	buildPlane('z', 'y', 'x', 1, -1, depth, height, -width, m_DepthSeg, m_HeightSeg, 1); // nx
	buildPlane('x', 'z', 'y', 1, 1, width, depth, height, m_WidthSeg, m_DepthSeg, 2); // py
	buildPlane('x', 'z', 'y', 1, -1, width, depth, -height, m_WidthSeg, m_DepthSeg, 3); // ny
	buildPlane('x', 'y', 'z', 1, -1, width, height, depth, m_WidthSeg, m_HeightSeg, 4); // pz
	buildPlane('x', 'y', 'z', -1, -1, width, height, -depth, m_WidthSeg, m_HeightSeg, 5); // nz

	// BuildGeometry();
}

void BoxGeometry::buildPlane(char u, char v, char w, int udir, int vdir, float width, float height, float depth, int gridX, int gridY, int materialIndex) {

	const float segmentWidth = width / (float)gridX;
	const float segmentHeight = height / (float)gridY;

	const float widthHalf = width / 2;
	const float heightHalf = height / 2;
	const float depthHalf = depth / 2;

	const int gridX1 = gridX + 1;
	const int gridY1 = gridY + 1;

	unsigned int vertexCounter = 0;
	unsigned int groupCount = 0;

	const glm::vec3 vector = glm::vec3(0.0);

	// save number of vertices BEFORE adding any this round
	// used to figure out indices
	const int numberOfVertices = m_Vertices.size();

	// generate vertices, normals and uvs
	for (int iy = 0; iy < gridY1; iy++) {
		const float y = iy * segmentHeight - heightHalf;
		for (int ix = 0; ix < gridX1; ix++) {
			const float x = ix * segmentWidth - widthHalf;

			// prepare new vertex
			Vertex vert;

			// set position
			vert.Pos(u) = x * udir;
			vert.Pos(v) = y * vdir;
			vert.Pos(w) = depthHalf;

			// set normals
			vert.Norm(u) = 0;
			vert.Norm(v) = 0;
			vert.Norm(w) = depth > 0 ? 1 : -1;

			// set uvs
			vert.TexCoords.x = (ix / gridX);
			vert.TexCoords.y = (1 - (iy / gridY));

			// copy to list
			m_Vertices.push_back(vert);

			// counters
			vertexCounter += 1;
		}
	}

	// indices

	// 1. you need three indices to draw a single face
	// 2. a single segment consists of two faces
	// 3. so we need to generate six (2*3) indices per segment

	for (int iy = 0; iy < gridY; iy++) {
		for (int ix = 0; ix < gridX; ix++) {

			unsigned int a = numberOfVertices + ix + gridX1 * iy;
			unsigned int b = numberOfVertices + ix + gridX1 * (iy + 1);
			unsigned int c = numberOfVertices + (ix + 1) + gridX1 * (iy + 1);
			unsigned int d = numberOfVertices + (ix + 1) + gridX1 * iy;

			// faces
			m_Indices.push_back({ a, b, d });
			m_Indices.push_back({ b, c, d });

			// increase group counter
			groupCount += 6;
		}

	}

	// add a group to the geometry. this will ensure multi material support
	// TODO: add this later. too complex for now. assume a single geometry is rendererd with a single mat
	// scope.addGroup(groupStart, groupCount, materialIndex);

	// calculate new start value for groups
	// groupStart += groupCount;
}

