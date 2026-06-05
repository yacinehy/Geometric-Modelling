#pragma once
#include "myFace.h"
#include "myHalfedge.h"
#include "myVertex.h"
#include "myPoint3D.h"
#include <vector>
#include <string>

class myMesh
{
public:
	std::vector<myVertex *> vertices;
	std::vector<myHalfedge *> halfedges;
	std::vector<myFace *> faces;
	std::string name;

	void checkMesh();
	bool readFile(std::string filename);
	void computeNormals();
	void normalize();

	void subdivisionCatmullClark();

	void splitFaceTRIS(myFace *, myPoint3D *);

	void splitEdge(myHalfedge *, myPoint3D *);
	void splitFaceQUADS(myFace *, myPoint3D *);

	void triangulate();
	bool triangulate(myFace *);
	void simplify();

	void buildRevolutionSurface(std::vector<myPoint3D> profile, int steps);

	void clear();

	myMesh(void);
	~myMesh(void);
};

