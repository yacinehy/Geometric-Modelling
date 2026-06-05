#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <utility>
#include <cmath>
#include <algorithm>
#include <limits>
#include <GL/glew.h>
#include "myVector3D.h"

using namespace std;

myMesh::myMesh(void)
{
}


myMesh::~myMesh(void)
{
}

void myMesh::clear()
{
	for (unsigned int i = 0; i < vertices.size(); i++) if (vertices[i]) delete vertices[i];
	for (unsigned int i = 0; i < halfedges.size(); i++) if (halfedges[i]) delete halfedges[i];
	for (unsigned int i = 0; i < faces.size(); i++) if (faces[i]) delete faces[i];

	vector<myVertex *> empty_vertices;    vertices.swap(empty_vertices);
	vector<myHalfedge *> empty_halfedges; halfedges.swap(empty_halfedges);
	vector<myFace *> empty_faces;         faces.swap(empty_faces);
}

void myMesh::checkMesh()
{
	vector<myHalfedge *>::iterator it;
	for (it = halfedges.begin(); it != halfedges.end(); it++)
	{
		if ((*it)->twin == NULL)
			break;
	}
	if (it != halfedges.end())
		cout << "Error! Not all edges have their twins!\n";
	else cout << "Each edge has a twin!\n";
}


bool myMesh::readFile(std::string filename)
{
	string s, t, u;
	vector<int> faceids;
	myHalfedge **hedges;

	ifstream fin(filename);
	if (!fin.is_open()) {
		cout << "Unable to open file!\n";
		return false;
	}
	name = filename;

	map<pair<int, int>, myHalfedge *> twin_map;
	map<pair<int, int>, myHalfedge *>::iterator it;

	while (getline(fin, s))
	{
		stringstream myline(s);
		myline >> t;
		if (t == "g") {}
		else if (t == "v")
		{
			float x, y, z;
			myline >> x >> y >> z;
			myVertex *v = new myVertex();
			v->point = new myPoint3D(x, y, z);
			v->index = vertices.size();
			vertices.push_back(v);
		}
		else if (t == "mtllib") {}
		else if (t == "usemtl") {}
		else if (t == "s") {}
		else if (t == "f")
		{
			faceids.clear();
			while (myline >> u) {
				int vi = atoi((u.substr(0, u.find("/"))).c_str()) - 1;
				faceids.push_back(vi);
			}

			if (faceids.size() < 3) continue;

			myFace *f = new myFace();
			f->index = faces.size();
			faces.push_back(f);

			hedges = new myHalfedge*[faceids.size()];
			for (size_t i = 0; i < faceids.size(); i++) {
				hedges[i] = new myHalfedge();
				hedges[i]->source = vertices[faceids[i]];
				hedges[i]->adjacent_face = f;
				hedges[i]->index = halfedges.size();
				halfedges.push_back(hedges[i]);

				if (vertices[faceids[i]]->originof == NULL) {
					vertices[faceids[i]]->originof = hedges[i];
				}
			}

			f->adjacent_halfedge = hedges[0];

			for (size_t i = 0; i < faceids.size(); i++) {
				hedges[i]->next = hedges[(i + 1) % faceids.size()];
				hedges[i]->prev = hedges[(i + faceids.size() - 1) % faceids.size()];

				int v1 = faceids[i];
				int v2 = faceids[(i + 1) % faceids.size()];

				twin_map[make_pair(v1, v2)] = hedges[i];

				it = twin_map.find(make_pair(v2, v1));
				if (it != twin_map.end()) {
					hedges[i]->twin = it->second;
					it->second->twin = hedges[i];
				}
			}
			delete[] hedges;
		}
	}

	checkMesh();
	normalize();

	return true;
}


void myMesh::computeNormals()
{
	for (unsigned int i = 0; i < faces.size(); i++) {
		if (faces[i] != NULL) faces[i]->computeNormal();
	}
	for (unsigned int i = 0; i < vertices.size(); i++) {
		if (vertices[i] != NULL) vertices[i]->computeNormal();
	}
}

void myMesh::normalize()
{
	if (vertices.size() < 1) return;

	int tmpxmin = 0, tmpymin = 0, tmpzmin = 0, tmpxmax = 0, tmpymax = 0, tmpzmax = 0;

	for (unsigned int i = 0; i < vertices.size(); i++) {
		if (vertices[i]->point->X < vertices[tmpxmin]->point->X) tmpxmin = i;
		if (vertices[i]->point->X > vertices[tmpxmax]->point->X) tmpxmax = i;

		if (vertices[i]->point->Y < vertices[tmpymin]->point->Y) tmpymin = i;
		if (vertices[i]->point->Y > vertices[tmpymax]->point->Y) tmpymax = i;

		if (vertices[i]->point->Z < vertices[tmpzmin]->point->Z) tmpzmin = i;
		if (vertices[i]->point->Z > vertices[tmpzmax]->point->Z) tmpzmax = i;
	}

	double xmin = vertices[tmpxmin]->point->X, xmax = vertices[tmpxmax]->point->X,
		ymin = vertices[tmpymin]->point->Y, ymax = vertices[tmpymax]->point->Y,
		zmin = vertices[tmpzmin]->point->Z, zmax = vertices[tmpzmax]->point->Z;

	double scale = (xmax - xmin) > (ymax - ymin) ? (xmax - xmin) : (ymax - ymin);
	scale = scale > (zmax - zmin) ? scale : (zmax - zmin);

	for (unsigned int i = 0; i < vertices.size(); i++) {
		vertices[i]->point->X -= (xmax + xmin) / 2;
		vertices[i]->point->Y -= (ymax + ymin) / 2;
		vertices[i]->point->Z -= (zmax + zmin) / 2;

		vertices[i]->point->X /= scale;
		vertices[i]->point->Y /= scale;
		vertices[i]->point->Z /= scale;
	}
}


void myMesh::buildRevolutionSurface(std::vector<myPoint3D> profile, int steps)
{
	clear();
	int n = profile.size();
	double pi = 3.14159265358979;

	for (int i = 0; i < steps; i++) {
		double a = 2.0 * pi * i / steps;
		for (int j = 0; j < n; j++) {
			myVertex* v = new myVertex();
			v->point = new myPoint3D(profile[j].X * cos(a), profile[j].Y, profile[j].X * sin(a));
			v->index = vertices.size();
			vertices.push_back(v);
		}
	}

	std::vector<myHalfedge*> he(steps * (n - 1) * 4, NULL);

	for (int i = 0; i < steps; i++) {
		for (int j = 0; j < n - 1; j++) {
			int fi = i * (n - 1) + j;
			myFace* f = new myFace();
			f->index = faces.size();
			faces.push_back(f);

			int idx[4] = {
				i * n + j,
				i * n + j + 1,
				((i + 1) % steps) * n + j + 1,
				((i + 1) % steps) * n + j
			};

			for (int k = 0; k < 4; k++) {
				myHalfedge* h = new myHalfedge();
				h->source = vertices[idx[k]];
				h->adjacent_face = f;
				h->index = halfedges.size();
				halfedges.push_back(h);
				he[fi * 4 + k] = h;
				if (vertices[idx[k]]->originof == NULL)
					vertices[idx[k]]->originof = h;
			}

			f->adjacent_halfedge = he[fi * 4];
			for (int k = 0; k < 4; k++) {
				he[fi * 4 + k]->next = he[fi * 4 + (k + 1) % 4];
				he[fi * 4 + k]->prev = he[fi * 4 + (k + 3) % 4];
			}
		}
	}

	for (int i = 0; i < steps; i++) {
		for (int j = 0; j < n - 1; j++) {
			int fi = i * (n - 1) + j;
			int fp = ((i - 1 + steps) % steps) * (n - 1) + j;
			he[fi * 4]->twin     = he[fp * 4 + 2];
			he[fp * 4 + 2]->twin = he[fi * 4];
			if (j < n - 2) {
				int fa = i * (n - 1) + j + 1;
				he[fi * 4 + 1]->twin = he[fa * 4 + 3];
				he[fa * 4 + 3]->twin = he[fi * 4 + 1];
			}
		}
	}

	normalize();
}

void myMesh::splitFaceTRIS(myFace *f, myPoint3D *p)
{
}

void myMesh::splitEdge(myHalfedge *e1, myPoint3D *p)
{
}

void myMesh::splitFaceQUADS(myFace *f, myPoint3D *p)
{
}


void myMesh::subdivisionCatmullClark()
{
	using std::map; using std::pair; using std::vector; using std::set; using std::make_pair;

	if (faces.empty()) return;

	auto ekey = [](myVertex* a, myVertex* b) {
		return (a < b) ? make_pair(a, b) : make_pair(b, a);
	};

	map<myFace*, myPoint3D> facePt;
	for (myFace* f : faces) {
		if (!f || !f->adjacent_halfedge) continue;
		myPoint3D sum(0, 0, 0);
		int cnt = 0;
		myHalfedge* h = f->adjacent_halfedge;
		do {
			sum.X += h->source->point->X;
			sum.Y += h->source->point->Y;
			sum.Z += h->source->point->Z;
			cnt++;
			h = h->next;
		} while (h != f->adjacent_halfedge && h != nullptr);
		if (cnt > 0) { sum.X /= cnt; sum.Y /= cnt; sum.Z /= cnt; }
		facePt[f] = sum;
	}

	map<myVertex*, set<myVertex*>> neighbors;
	map<myVertex*, vector<myFace*>> incFaces;
	map<pair<myVertex*, myVertex*>, int> edgeFaceCount;

	for (myHalfedge* h : halfedges) {
		if (!h || !h->source || !h->next || !h->next->source) continue;
		myVertex* v = h->source;
		myVertex* w = h->next->source;
		neighbors[v].insert(w);
		neighbors[w].insert(v);
		if (h->adjacent_face) {
			incFaces[v].push_back(h->adjacent_face);
			edgeFaceCount[ekey(v, w)]++;
		}
	}

	auto isBoundaryEdge = [&](myVertex* a, myVertex* b) {
		return edgeFaceCount[ekey(a, b)] < 2;
	};

	map<pair<myVertex*, myVertex*>, myPoint3D> edgePt;
	for (myHalfedge* h : halfedges) {
		if (!h || !h->source || !h->next) continue;
		myVertex* v = h->source;
		myVertex* w = h->next->source;
		pair<myVertex*, myVertex*> key = ekey(v, w);
		if (edgePt.count(key)) continue;

		myPoint3D pt(0, 0, 0);
		if (!isBoundaryEdge(v, w) && h->twin &&
			h->adjacent_face && h->twin->adjacent_face) {
			myPoint3D& f1 = facePt[h->adjacent_face];
			myPoint3D& f2 = facePt[h->twin->adjacent_face];
			pt.X = (v->point->X + w->point->X + f1.X + f2.X) / 4.0;
			pt.Y = (v->point->Y + w->point->Y + f1.Y + f2.Y) / 4.0;
			pt.Z = (v->point->Z + w->point->Z + f1.Z + f2.Z) / 4.0;
		} else {
			pt.X = (v->point->X + w->point->X) / 2.0;
			pt.Y = (v->point->Y + w->point->Y) / 2.0;
			pt.Z = (v->point->Z + w->point->Z) / 2.0;
		}
		edgePt[key] = pt;
	}

	map<myVertex*, myPoint3D> vertPt;
	for (auto& kv : neighbors) {
		myVertex* v = kv.first;
		set<myVertex*>& nb = kv.second;
		int n = (int)nb.size();
		if (n == 0) { vertPt[v] = *v->point; continue; }

		vector<myVertex*> bneigh;
		for (myVertex* w : nb)
			if (isBoundaryEdge(v, w)) bneigh.push_back(w);

		if (!bneigh.empty()) {
			myPoint3D np(6.0 * v->point->X, 6.0 * v->point->Y, 6.0 * v->point->Z);
			for (myVertex* w : bneigh) {
				np.X += w->point->X; np.Y += w->point->Y; np.Z += w->point->Z;
			}
			double denom = 6.0 + (double)bneigh.size();
			np.X /= denom; np.Y /= denom; np.Z /= denom;
			vertPt[v] = np;
		} else {
			myPoint3D F(0, 0, 0);
			vector<myFace*>& fs = incFaces[v];
			for (myFace* f : fs) { F.X += facePt[f].X; F.Y += facePt[f].Y; F.Z += facePt[f].Z; }
			int nf = (int)fs.size();
			if (nf > 0) { F.X /= nf; F.Y /= nf; F.Z /= nf; }

			myPoint3D R(0, 0, 0);
			for (myVertex* w : nb) {
				R.X += (v->point->X + w->point->X) / 2.0;
				R.Y += (v->point->Y + w->point->Y) / 2.0;
				R.Z += (v->point->Z + w->point->Z) / 2.0;
			}
			R.X /= n; R.Y /= n; R.Z /= n;

			myPoint3D P = *v->point;
			myPoint3D np(0, 0, 0);
			np.X = (F.X + 2.0 * R.X + (n - 3.0) * P.X) / n;
			np.Y = (F.Y + 2.0 * R.Y + (n - 3.0) * P.Y) / n;
			np.Z = (F.Z + 2.0 * R.Z + (n - 3.0) * P.Z) / n;
			vertPt[v] = np;
		}
	}

	vector<myVertex*>   nVerts;
	vector<myHalfedge*> nHedges;
	vector<myFace*>     nFaces;

	auto newVertex = [&](const myPoint3D& p) -> myVertex* {
		myVertex* v = new myVertex();
		v->point = new myPoint3D(p.X, p.Y, p.Z);
		v->index = (int)nVerts.size();
		nVerts.push_back(v);
		return v;
	};

	map<myVertex*, myVertex*> VP;
	for (auto& kv : vertPt) VP[kv.first] = newVertex(kv.second);
	for (myVertex* v : vertices) if (v && !VP.count(v)) VP[v] = newVertex(*v->point);

	map<myFace*, myVertex*> FP;
	for (auto& kv : facePt) FP[kv.first] = newVertex(kv.second);

	map<pair<myVertex*, myVertex*>, myVertex*> EP;
	for (auto& kv : edgePt) EP[kv.first] = newVertex(kv.second);

	map<pair<myVertex*, myVertex*>, myHalfedge*> twinMap;

	auto addFace = [&](vector<myVertex*>& vs) {
		int k = (int)vs.size();
		if (k < 3) return;
		myFace* f = new myFace();
		f->index = (int)nFaces.size();
		nFaces.push_back(f);

		vector<myHalfedge*> hs(k);
		for (int i = 0; i < k; i++) {
			myHalfedge* h = new myHalfedge();
			h->source = vs[i];
			h->adjacent_face = f;
			h->index = (int)nHedges.size();
			nHedges.push_back(h);
			hs[i] = h;
			if (vs[i]->originof == nullptr) vs[i]->originof = h;
		}
		f->adjacent_halfedge = hs[0];
		for (int i = 0; i < k; i++) {
			hs[i]->next = hs[(i + 1) % k];
			hs[i]->prev = hs[(i + k - 1) % k];
			myVertex* a = vs[i];
			myVertex* b = vs[(i + 1) % k];
			twinMap[make_pair(a, b)] = hs[i];
			auto it = twinMap.find(make_pair(b, a));
			if (it != twinMap.end()) {
				hs[i]->twin = it->second;
				it->second->twin = hs[i];
			}
		}
	};

	for (myFace* f : faces) {
		if (!f || !f->adjacent_halfedge) continue;
		myHalfedge* h = f->adjacent_halfedge;
		do {
			myVertex* v = h->source;
			myVertex* w = h->next->source;
			myVertex* u = h->prev->source;
			vector<myVertex*> quad(4);
			quad[0] = VP[v];
			quad[1] = EP[ekey(v, w)];
			quad[2] = FP[f];
			quad[3] = EP[ekey(u, v)];
			addFace(quad);
			h = h->next;
		} while (h != f->adjacent_halfedge && h != nullptr);
	}

	clear();
	vertices = nVerts;
	halfedges = nHedges;
	faces = nFaces;

	computeNormals();
}

static void collapseEdge(myMesh* mesh, myHalfedge* h)
{
    if (!h || !h->twin) return;

    myHalfedge* h_T = h->twin;

    if (!h->adjacent_face || !h_T->adjacent_face) return;

    myVertex* v_keep   = h->source;
    myVertex* v_remove = h_T->source;

    myHalfedge* h_A = h->next;
    myHalfedge* h_B = h->prev;
    myHalfedge* h_C = h_T->next;
    myHalfedge* h_D = h_T->prev;

    if (!h_A || !h_B || !h_C || !h_D) return;

    myVertex* v_a = h_B->source;
    myVertex* v_b = h_D->source;

    if (h_A->twin == h_B || h_C->twin == h_D) return;
    if (v_a == v_b) return;

    myFace* f1 = h->adjacent_face;
    myFace* f2 = h_T->adjacent_face;

    v_keep->point->X = (v_keep->point->X + v_remove->point->X) * 0.5;
    v_keep->point->Y = (v_keep->point->Y + v_remove->point->Y) * 0.5;
    v_keep->point->Z = (v_keep->point->Z + v_remove->point->Z) * 0.5;

    myHalfedge* h_A_twin = h_A->twin;
    myHalfedge* h_B_twin = h_B->twin;
    myHalfedge* h_C_twin = h_C->twin;
    myHalfedge* h_D_twin = h_D->twin;

    if (h_A_twin) h_A_twin->twin = h_B_twin;
    if (h_B_twin) h_B_twin->twin = h_A_twin;
    if (h_C_twin) h_C_twin->twin = h_D_twin;
    if (h_D_twin) h_D_twin->twin = h_C_twin;

    for (myHalfedge* he : mesh->halfedges)
        if (he && he->source == v_remove)
            he->source = v_keep;

    auto fixOriginof = [&](myVertex* v) {
        if (!v) return;
        if (v->originof != h   && v->originof != h_T && v->originof != h_A &&
            v->originof != h_B && v->originof != h_C && v->originof != h_D) return;
        v->originof = nullptr;
        for (myHalfedge* he : mesh->halfedges) {
            if (he && he != h && he != h_T && he != h_A &&
                he != h_B && he != h_C && he != h_D && he->source == v) {
                v->originof = he;
                return;
            }
        }
    };
    fixOriginof(v_keep);
    fixOriginof(v_a);
    fixOriginof(v_b);

    for (myHalfedge*& he : mesh->halfedges) {
        if (he == h || he == h_T || he == h_A ||
            he == h_B || he == h_C || he == h_D) {
            delete he; he = nullptr;
        }
    }
    for (myFace*& f : mesh->faces) {
        if (f == f1 || f == f2) { delete f; f = nullptr; }
    }
    for (myVertex*& v : mesh->vertices) {
        if (v == v_remove) { delete v; v = nullptr; }
    }
}

static void compactMesh(myMesh* mesh)
{
    mesh->halfedges.erase(
        std::remove(mesh->halfedges.begin(), mesh->halfedges.end(), (myHalfedge*)nullptr),
        mesh->halfedges.end());
    mesh->faces.erase(
        std::remove(mesh->faces.begin(), mesh->faces.end(), (myFace*)nullptr),
        mesh->faces.end());
    mesh->vertices.erase(
        std::remove(mesh->vertices.begin(), mesh->vertices.end(), (myVertex*)nullptr),
        mesh->vertices.end());

    for (size_t i = 0; i < mesh->vertices.size();  i++) mesh->vertices[i]->index  = (int)i;
    for (size_t i = 0; i < mesh->halfedges.size(); i++) mesh->halfedges[i]->index = (int)i;
    for (size_t i = 0; i < mesh->faces.size();     i++) mesh->faces[i]->index     = (int)i;
}

void myMesh::simplify()
{
    triangulate();

    int target = std::max(1, (int)(vertices.size() / 2));

    for (int iter = 0; iter < target; iter++) {
        myHalfedge* best   = nullptr;
        double       minLen = std::numeric_limits<double>::max();

        for (myHalfedge* he : halfedges) {
            if (!he || !he->twin) continue;
            if (!he->adjacent_face || !he->twin->adjacent_face) continue;
            if (he > he->twin) continue;

            myPoint3D* p1 = he->source->point;
            myPoint3D* p2 = he->twin->source->point;
            double dx = p1->X - p2->X;
            double dy = p1->Y - p2->Y;
            double dz = p1->Z - p2->Z;
            double len2 = dx*dx + dy*dy + dz*dz;
            if (len2 < minLen) { minLen = len2; best = he; }
        }

        if (!best) break;
        collapseEdge(this, best);
    }

    compactMesh(this);
    computeNormals();
}

void myMesh::triangulate()
{
	int original_size = faces.size();
	for (int i = 0; i < original_size; ++i) {
		if (faces[i] != NULL) {
			triangulate(faces[i]);
		}
	}
}

bool myMesh::triangulate(myFace *f)
{
	if (f == NULL || f->adjacent_halfedge == NULL) return false;

	int N = 0;
	myHalfedge* curr = f->adjacent_halfedge;
	do {
		N++;
		curr = curr->next;
	} while (curr != f->adjacent_halfedge && curr != NULL);

	if (N <= 3) return false;

	myVector3D normal(0.0, 0.0, 0.0);
	curr = f->adjacent_halfedge;
	do {
		myPoint3D* p1 = curr->source->point;
		myPoint3D* p2 = curr->next->source->point;
		normal.dX += (p1->Y - p2->Y) * (p1->Z + p2->Z);
		normal.dY += (p1->Z - p2->Z) * (p1->X + p2->X);
		normal.dZ += (p1->X - p2->X) * (p1->Y + p2->Y);
		curr = curr->next;
	} while (curr != f->adjacent_halfedge);
	normal.normalize();

	while (N > 3) {
		myHalfedge* ear_edge = NULL;

		curr = f->adjacent_halfedge;
		for (int i = 0; i < N; ++i) {
			myHalfedge* prev = curr->prev;
			myHalfedge* next = curr->next;
			myPoint3D* p_prev = prev->source->point;
			myPoint3D* p_curr = curr->source->point;
			myPoint3D* p_next = next->source->point;

			myVector3D v1(p_curr->X - p_prev->X, p_curr->Y - p_prev->Y, p_curr->Z - p_prev->Z);
			myVector3D v2(p_next->X - p_curr->X, p_next->Y - p_curr->Y, p_next->Z - p_curr->Z);
			myVector3D cross(
				v1.dY * v2.dZ - v1.dZ * v2.dY,
				v1.dZ * v2.dX - v1.dX * v2.dZ,
				v1.dX * v2.dY - v1.dY * v2.dX
			);
			
			if (cross * normal > 1e-5) {
				bool is_ear = true;
				myHalfedge* test_edge = next->next;
				while (test_edge != prev) {
					myPoint3D* p_test = test_edge->source->point;
					
					myVector3D v0(p_next->X - p_prev->X, p_next->Y - p_prev->Y, p_next->Z - p_prev->Z);
					myVector3D v1_(p_curr->X - p_prev->X, p_curr->Y - p_prev->Y, p_curr->Z - p_prev->Z);
					myVector3D v2_(p_test->X - p_prev->X, p_test->Y - p_prev->Y, p_test->Z - p_prev->Z);
					
					double dot00 = v0 * v0;
					double dot01 = v0 * v1_;
					double dot02 = v0 * v2_;
					double dot11 = v1_ * v1_;
					double dot12 = v1_ * v2_;
					
					double denom = (dot00 * dot11 - dot01 * dot01);
					if (denom > 1e-8 || denom < -1e-8) {
					    double invDenom = 1.0 / denom;
					    double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
					    double v_bary = (dot00 * dot12 - dot01 * dot02) * invDenom;
					    
					    if ((u >= -1e-5) && (v_bary >= -1e-5) && (u + v_bary <= 1.0 + 1e-5)) {
						    is_ear = false;
						    break;
					    }
					}
					test_edge = test_edge->next;
				}
				
				if (is_ear) {
					ear_edge = prev; 
					break;
				}
			}
			curr = curr->next;
		}

		if (ear_edge == NULL) {
			curr = f->adjacent_halfedge;
			for (int i = 0; i < N; ++i) {
				myPoint3D* p_prev = curr->prev->source->point;
				myPoint3D* p_curr = curr->source->point;
				myPoint3D* p_next = curr->next->source->point;
				myVector3D v1(p_curr->X - p_prev->X, p_curr->Y - p_prev->Y, p_curr->Z - p_prev->Z);
				myVector3D v2(p_next->X - p_curr->X, p_next->Y - p_curr->Y, p_next->Z - p_curr->Z);
				myVector3D cross(
					v1.dY * v2.dZ - v1.dZ * v2.dY,
					v1.dZ * v2.dX - v1.dX * v2.dZ,
					v1.dX * v2.dY - v1.dY * v2.dX
				);
				if (cross * normal > -1e-5) {
					ear_edge = curr->prev;
					break;
				}
				curr = curr->next;
			}
			if (ear_edge == NULL) ear_edge = f->adjacent_halfedge;
		}

		myHalfedge* h0 = ear_edge;
		myHalfedge* h1 = h0->next;
		myHalfedge* h2 = h1->next;
		myHalfedge* h_prev = h0->prev;

		myVertex* v0 = h0->source;
		myVertex* v2 = h2->source;

		myHalfedge* h_new = new myHalfedge();
		myHalfedge* h_new_twin = new myHalfedge();
		
		h_new->source = v2;
		h_new_twin->source = v0;
		
		h_new->twin = h_new_twin;
		h_new_twin->twin = h_new;
		
		h_new->index = halfedges.size(); halfedges.push_back(h_new);
		h_new_twin->index = halfedges.size(); halfedges.push_back(h_new_twin);

		myFace* f_tri = new myFace();
		f_tri->index = faces.size();
		faces.push_back(f_tri);
		f_tri->adjacent_halfedge = h0;

		h1->next = h_new; h_new->prev = h1;
		h_new->next = h0; h0->prev = h_new;
		
		h0->adjacent_face = f_tri;
		h1->adjacent_face = f_tri;
		h_new->adjacent_face = f_tri;

		f->adjacent_halfedge = h_new_twin; 
		
		h_prev->next = h_new_twin; h_new_twin->prev = h_prev;
		h_new_twin->next = h2; h2->prev = h_new_twin;
		
		h_new_twin->adjacent_face = f;
		
		N--;
	}

	return true;
}

