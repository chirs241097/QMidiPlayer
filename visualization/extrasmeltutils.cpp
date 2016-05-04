#include "extrasmeltutils.hpp"
SMELT *smEntity3D::sm=NULL;
smEntity3D::smEntity3D()
{
	sm=smGetInterface(SMELT_APILEVEL);
	surfaces.clear();
}
void smEntity3D::pushSurface(smQuad q)
{surfaces.push_back(q);}
void smEntity3D::pushCube(smvec3d a,smvec3d b,DWORD color,DWORD mask)
{
	//a: top left corner b: bottom right corner
	smQuad q;q.blend=BLEND_ALPHABLEND;q.tex=0;
	for(int i=0;i<4;++i)q.v[i].col=color,q.v[i].tx=q.v[i].ty=0;
	//top
	if(mask&1)
	{
		q.v[0].x=a.x;q.v[0].y=a.y;q.v[0].z=a.z;
		q.v[1].x=b.x;q.v[1].y=a.y;q.v[1].z=a.z;
		q.v[2].x=b.x;q.v[2].y=b.y;q.v[2].z=a.z;
		q.v[3].x=a.x;q.v[3].y=b.y;q.v[3].z=a.z;
		pushSurface(q);
	}
	//bottom
	if(mask&2)
	{
		q.v[0].x=a.x;q.v[0].y=a.y;q.v[0].z=b.z;
		q.v[1].x=b.x;q.v[1].y=a.y;q.v[1].z=b.z;
		q.v[2].x=b.x;q.v[2].y=b.y;q.v[2].z=b.z;
		q.v[3].x=a.x;q.v[3].y=b.y;q.v[3].z=b.z;
		pushSurface(q);
	}
	//left
	if(mask&4)
	{
		q.v[0].x=a.x;q.v[0].y=b.y;q.v[0].z=a.z;
		q.v[1].x=a.x;q.v[1].y=b.y;q.v[1].z=b.z;
		q.v[2].x=a.x;q.v[2].y=a.y;q.v[2].z=b.z;
		q.v[3].x=a.x;q.v[3].y=a.y;q.v[3].z=a.z;
		pushSurface(q);
	}
	//right
	if(mask&8)
	{
		q.v[0].x=b.x;q.v[0].y=b.y;q.v[0].z=a.z;
		q.v[1].x=b.x;q.v[1].y=b.y;q.v[1].z=b.z;
		q.v[2].x=b.x;q.v[2].y=a.y;q.v[2].z=b.z;
		q.v[3].x=b.x;q.v[3].y=a.y;q.v[3].z=a.z;
		pushSurface(q);
	}
	//front
	if(mask&16)
	{
		q.v[0].x=a.x;q.v[0].y=b.y;q.v[0].z=a.z;
		q.v[1].x=b.x;q.v[1].y=b.y;q.v[1].z=a.z;
		q.v[2].x=b.x;q.v[2].y=b.y;q.v[2].z=b.z;
		q.v[3].x=a.x;q.v[3].y=b.y;q.v[3].z=b.z;
		pushSurface(q);
	}
	//back
	if(mask&32)
	{
		q.v[0].x=a.x;q.v[0].y=a.y;q.v[0].z=a.z;
		q.v[1].x=b.x;q.v[1].y=a.y;q.v[1].z=a.z;
		q.v[2].x=b.x;q.v[2].y=a.y;q.v[2].z=b.z;
		q.v[3].x=a.x;q.v[3].y=a.y;q.v[3].z=b.z;
		pushSurface(q);
	}
}
void smEntity3D::drawAt(smvec3d p)
{
	for(unsigned i=0;i<surfaces.size();++i)
	{
		smQuad tq=surfaces[i];
		for(unsigned j=0;j<4;++j)tq.v[j].x+=p.x,tq.v[j].y+=p.y,tq.v[j].z+=p.z;
		sm->smRenderQuad(&tq);
	}
}
void smEntity3D::drawWithTransformation(smMatrix t,smvec3d p)
{
	for(unsigned i=0;i<surfaces.size();++i)
	{
		smQuad tq=surfaces[i];
		for(unsigned j=0;j<4;++j)
		{
			smvec3d tp=t*smvec3d(tq.v[j].x,tq.v[j].y,tq.v[j].z);
			tq.v[j].x=tp.x+p.x;tq.v[j].y=tp.y+p.y;tq.v[j].z=tp.z+p.z;
		}
		sm->smRenderQuad(&tq);
	}
}
