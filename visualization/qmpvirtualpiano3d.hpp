#ifndef QMPVIRTUALPIANO3D_H
#define QMPVIRTUALPIANO3D_H
#include <smelt.hpp>
#include "extrasmeltutils.hpp"
#define WK_PREWIDTH 0.8
#define WK_TALWIDTH 1.2
#define WK_PRELEN 3.5
#define WK_TALLEN 2.5
#define WK_WING 0.1
#define WK_HEIGHT 1.5
#define BK_WIDTH 7*(WK_TALWIDTH-WK_PREWIDTH)/5
#define BK_FWIDTH 7*(WK_TALWIDTH-WK_PREWIDTH)/5
#define BK_PRELEN 3.
#define BK_HEIGHT 1.5
#define BK_BOTTOM 0.5
#define BK_DBOTTOM 1.
class qmpVirtualPiano3D
{
	private:
		smEntity3D *wkcf,*wkeb,*wkd,*wkg,*wka,*bk;
		smEntity3DBuffer *ebuf;
		void buildKeys();
		double traveld[128];
	public:
		qmpVirtualPiano3D();
		~qmpVirtualPiano3D();
		void render(smvec3d p);
		void setKeyTravelDist(int k,double td);
};
#endif // QMPVIRTUALPIANO3D_H
