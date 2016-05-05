#ifndef EXTRASMELTUTILS_H
#define EXTRASMELTUTILS_H
#include <vector>
#include <smelt.hpp>
#include <smmath.hpp>
class smEntity3D
{
	friend class smEntity3DBuffer;
	private:
		std::vector<smVertex> vertices;
		std::vector<WORD> indices;
	public:
		smEntity3D(){vertices.clear();indices.clear();}
		~smEntity3D(){vertices.clear();indices.clear();}
		void addVerices(int n,...);
		void addIndices(int n,...);
		static smEntity3D cube(smvec3d tl,smvec3d br,DWORD color);
};
class smEntity3DBuffer
{
	private:
		std::vector<smVertex> vertices;
		std::vector<WORD> indices;
		static SMELT* sm;
	public:
		smEntity3DBuffer();
		~smEntity3DBuffer(){sm->smRelease();}
		void addTransformedEntity(smEntity3D *entity,smMatrix t,smvec3d p);
		void drawBatch();

};
extern smVertex makeVertex(float x,float y,float z,DWORD color,float tx,float ty);
#endif // EXTRASMELTUTILS_H
