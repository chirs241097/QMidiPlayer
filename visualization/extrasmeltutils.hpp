#ifndef EXTRASMELTUTILS_H
#define EXTRASMELTUTILS_H
#include <vector>
#include <smelt.hpp>
#include <smmath.hpp>
class smEntity3D
{
	private:
		std::vector<smQuad> surfaces;
		static SMELT *sm;
	public:
		smEntity3D();
		~smEntity3D(){surfaces.clear();sm->smRelease();}
		void pushSurface(smQuad q);
		void pushCube(smvec3d a,smvec3d b,DWORD color,DWORD mask);
		void drawAt(smvec3d p);
		void drawWithTransformation(smMatrix t,smvec3d p);
};
#endif // EXTRASMELTUTILS_H
