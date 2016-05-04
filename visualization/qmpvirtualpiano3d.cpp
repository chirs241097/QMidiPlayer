#include <cstring>
#include "qmpvirtualpiano3d.hpp"
#define configureVertex(v,sub,_x,_y,_z) v[sub].x=_x,v[sub].y=_y,v[sub].z=_z;
const double gap[]={WK_TALWIDTH/2*0.92,WK_TALWIDTH/2*1.23,WK_TALWIDTH/2*1.2,
				   WK_TALWIDTH/2*0.95,WK_TALWIDTH*1.1,WK_TALWIDTH/2*0.95,
				   WK_TALWIDTH/2*1.2,WK_TALWIDTH/2*1.15,WK_TALWIDTH/2*1,
				   WK_TALWIDTH/2*1.3,WK_TALWIDTH/2*0.85,WK_TALWIDTH*1.1};
qmpVirtualPiano3D::qmpVirtualPiano3D()
{
	buildKeys();memset(traveld,0,sizeof(traveld));
}
qmpVirtualPiano3D::~qmpVirtualPiano3D()
{
	delete wkcf;delete wkeb;delete wkd;delete wkg;delete wka;delete bk;
}
void qmpVirtualPiano3D::render(smvec3d p)
{
	p.x-=WK_TALWIDTH*37*1.075;
	for(int i=0;i<128;++i)
	{
		smMatrix m;m.loadIdentity();m.rotate(-0.2*traveld[i],1,0,0);
		switch(i%12)
		{
			case 0:case 5:
				wkcf->drawWithTransformation(m,p);
			break;
			case 2:
				wkd->drawWithTransformation(m,p);
			break;
			case 4:case 11:
				wkeb->drawWithTransformation(m,p);
			break;
			case 7:
				wkg->drawWithTransformation(m,p);
			break;
			case 9:
				wka->drawWithTransformation(m,p);
			break;
			case 1:case 3:case 6:case 8:case 10:
				bk->drawWithTransformation(m,p);
			break;
		}
		p.x+=gap[i%12];
	}
}
void qmpVirtualPiano3D::setKeyTravelDist(int k,double td)
{traveld[k]=td;}

void qmpVirtualPiano3D::buildKeys()
{
	wkcf=new smEntity3D();wkeb=new smEntity3D();wkd=new smEntity3D();
	wkg=new smEntity3D();wka=new smEntity3D();
	smQuad q;q.blend=BLEND_ALPHABLEND;q.tex=0;
	for(int i=0;i<4;++i)q.v[i].col=0xFFFFFFFF,q.v[i].tx=q.v[i].ty=0;
	//TAL
	configureVertex(q.v,0,-WK_TALWIDTH/2,WK_PRELEN          ,WK_HEIGHT);
	configureVertex(q.v,1, WK_TALWIDTH/2,WK_PRELEN          ,WK_HEIGHT);
	configureVertex(q.v,2, WK_TALWIDTH/2,WK_PRELEN+WK_TALLEN,WK_HEIGHT);
	configureVertex(q.v,3,-WK_TALWIDTH/2,WK_PRELEN+WK_TALLEN,WK_HEIGHT);
	wkcf->pushSurface(q);wkeb->pushSurface(q);wkd->pushSurface(q);
	wkg->pushSurface(q);wka->pushSurface(q);
	wkcf->pushCube(
				smvec3d(-WK_TALWIDTH/2,WK_PRELEN,WK_HEIGHT),
				smvec3d( WK_TALWIDTH/2,WK_PRELEN+WK_TALLEN-WK_WING,0),
				0xFFCCCCCC,30);
	wkeb->pushCube(
				smvec3d(-WK_TALWIDTH/2,WK_PRELEN,WK_HEIGHT),
				smvec3d( WK_TALWIDTH/2,WK_PRELEN+WK_TALLEN-WK_WING,0),
				0xFFCCCCCC,30);
	wkd->pushCube(
				smvec3d(-WK_TALWIDTH/2,WK_PRELEN,WK_HEIGHT),
				smvec3d( WK_TALWIDTH/2,WK_PRELEN+WK_TALLEN-WK_WING,0),
				0xFFCCCCCC,30);
	wkg->pushCube(
				smvec3d(-WK_TALWIDTH/2,WK_PRELEN,WK_HEIGHT),
				smvec3d( WK_TALWIDTH/2,WK_PRELEN+WK_TALLEN-WK_WING,0),
				0xFFCCCCCC,30);
	wka->pushCube(
				smvec3d(-WK_TALWIDTH/2,WK_PRELEN,WK_HEIGHT),
				smvec3d( WK_TALWIDTH/2,WK_PRELEN+WK_TALLEN-WK_WING,0),
				0xFFCCCCCC,30);
	//PRE
	configureVertex(q.v,0,-WK_TALWIDTH/2,0        ,WK_HEIGHT);
	configureVertex(q.v,1, WK_TALWIDTH/2,0        ,WK_HEIGHT);
	configureVertex(q.v,2, WK_TALWIDTH/2,WK_PRELEN,WK_HEIGHT);
	configureVertex(q.v,3,-WK_TALWIDTH/2,WK_PRELEN,WK_HEIGHT);
	wkcf->pushSurface(q);wkeb->pushSurface(q);wkd->pushSurface(q);
	wkg->pushSurface(q);wka->pushSurface(q);
	wkcf->pushCube(
				smvec3d(-WK_TALWIDTH/2,0,WK_HEIGHT),
				smvec3d(-WK_TALWIDTH/2+WK_PREWIDTH,WK_PRELEN,0),
				0xFFCCCCCC,46);
	wkeb->pushCube(
				smvec3d(WK_TALWIDTH/2-WK_PREWIDTH,0,WK_HEIGHT),
				smvec3d(WK_TALWIDTH/2,WK_PRELEN,0),
				0xFFCCCCCC,46);
	wkd->pushCube(
				smvec3d(-WK_TALWIDTH/2+(WK_TALWIDTH-WK_PREWIDTH)/2,0,WK_HEIGHT),
				smvec3d( WK_TALWIDTH/2-(WK_TALWIDTH-WK_PREWIDTH)/2,WK_PRELEN,0),
				0xFFCCCCCC,46);
	wkg->pushCube(
				smvec3d(-WK_TALWIDTH/2+WK_WING+WK_TALWIDTH/12,0,WK_HEIGHT),
				smvec3d(-WK_TALWIDTH/2+WK_WING+WK_TALWIDTH/12+WK_PREWIDTH,WK_PRELEN,0),
				0xFFCCCCCC,46);
	wka->pushCube(
				smvec3d(WK_TALWIDTH/2-WK_WING-WK_TALWIDTH/24,0,WK_HEIGHT),
				smvec3d(WK_TALWIDTH/2-WK_WING-WK_TALWIDTH/24-WK_PREWIDTH,WK_PRELEN,0),
				0xFFCCCCCC,46);

	bk=new smEntity3D();
	for(int i=0;i<4;++i)q.v[i].col=0xFF000000;
	bk->pushCube(
				smvec3d(-BK_WIDTH/2,0,BK_HEIGHT+BK_BOTTOM),
				smvec3d( BK_WIDTH/2,BK_PRELEN,BK_BOTTOM),
				0xFF000000,47);
	configureVertex(q.v,0,-BK_WIDTH/2,BK_PRELEN,BK_HEIGHT+BK_BOTTOM);
	configureVertex(q.v,1, BK_WIDTH/2,BK_PRELEN,BK_HEIGHT+BK_BOTTOM);
	configureVertex(q.v,2, BK_WIDTH/2,WK_PRELEN*0.95,BK_DBOTTOM+BK_BOTTOM);
	configureVertex(q.v,3,-BK_WIDTH/2,WK_PRELEN*0.95,BK_DBOTTOM+BK_BOTTOM);
	bk->pushSurface(q);
	configureVertex(q.v,0,-BK_WIDTH/2,BK_PRELEN,BK_HEIGHT+BK_BOTTOM);
	configureVertex(q.v,1,-BK_WIDTH/2,WK_PRELEN*0.95,BK_DBOTTOM+BK_BOTTOM);
	configureVertex(q.v,2,-BK_WIDTH/2,WK_PRELEN*0.95,BK_BOTTOM);
	configureVertex(q.v,3,-BK_WIDTH/2,BK_PRELEN,BK_BOTTOM);
	bk->pushSurface(q);
	configureVertex(q.v,0, BK_WIDTH/2,BK_PRELEN,BK_HEIGHT+BK_BOTTOM);
	configureVertex(q.v,1, BK_WIDTH/2,WK_PRELEN*0.95,BK_DBOTTOM+BK_BOTTOM);
	configureVertex(q.v,2, BK_WIDTH/2,WK_PRELEN*0.95,BK_BOTTOM);
	configureVertex(q.v,3, BK_WIDTH/2,BK_PRELEN,BK_BOTTOM);
	bk->pushSurface(q);
	configureVertex(q.v,0,-BK_WIDTH/2,WK_PRELEN*0.95,BK_DBOTTOM+BK_BOTTOM);
	configureVertex(q.v,1, BK_WIDTH/2,WK_PRELEN*0.95,BK_DBOTTOM+BK_BOTTOM);
	configureVertex(q.v,2, BK_WIDTH/2,WK_PRELEN*0.95,BK_BOTTOM);
	configureVertex(q.v,3,-BK_WIDTH/2,WK_PRELEN*0.95,BK_BOTTOM);
	bk->pushSurface(q);
	configureVertex(q.v,0,-BK_WIDTH/2,BK_PRELEN,BK_BOTTOM);
	configureVertex(q.v,1, BK_WIDTH/2,BK_PRELEN,BK_BOTTOM);
	configureVertex(q.v,2, BK_WIDTH/2,WK_PRELEN*0.95,BK_BOTTOM);
	configureVertex(q.v,3,-BK_WIDTH/2,WK_PRELEN*0.95,BK_BOTTOM);
	bk->pushSurface(q);
}
