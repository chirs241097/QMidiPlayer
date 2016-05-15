#include <cstdarg>
#include "extrasmeltutils.hpp"
SMELT* smEntity3DBuffer::sm=NULL;
SMELT* smParticle::sm=NULL;
smVertex makeVertex(float x,float y,float z,DWORD color,float tx,float ty)
{smVertex v;v.x=x;v.y=y;v.z=z;v.col=color;v.tx=tx;v.ty=ty;return v;}
void smEntity3D::addVerices(int n,...)
{
	va_list vl;va_start(vl,n);
	for(int i=0;i<n;++i)
	{
		smVertex v=va_arg(vl,smVertex);
		vertices.push_back(v);
	}
	va_end(vl);
}
void smEntity3D::addIndices(int n,...)
{
	va_list vl;va_start(vl,n);
	for(int i=0;i<n;++i)
	{
		int idx=va_arg(vl,int);
		indices.push_back((WORD)idx);
	}
	va_end(vl);
}
smEntity3D smEntity3D::cube(smvec3d a,smvec3d b,DWORD color)
{
	//a: top left corner b: bottom right corner
	smEntity3D ret;
	ret.addVerices(8,
		makeVertex(a.x,a.y,a.z,color,0,0),makeVertex(b.x,a.y,a.z,color,0,0),
		makeVertex(b.x,b.y,a.z,color,0,0),makeVertex(a.x,b.y,a.z,color,0,0),
		makeVertex(a.x,a.y,b.z,color,0,0),makeVertex(b.x,a.y,b.z,color,0,0),
		makeVertex(b.x,b.y,b.z,color,0,0),makeVertex(a.x,b.y,b.z,color,0,0));
	ret.addIndices(36,
		0,1,3,1,2,3, 4,5,7,5,6,7,
		0,3,7,0,4,7, 1,2,6,1,5,6,
		2,3,7,2,6,7, 0,1,4,1,4,5);
	return ret;
}
smEntity3DBuffer::smEntity3DBuffer()
{
	sm=smGetInterface(SMELT_APILEVEL);
	vertices.clear();indices.clear();
}
void smEntity3DBuffer::addTransformedEntity(smEntity3D *entity,smMatrix t,smvec3d p)
{
	if(entity->vertices.size()+vertices.size()>4000)drawBatch();
	for(unsigned i=0;i<entity->indices.size();++i)
	indices.push_back(entity->indices[i]+vertices.size());
	for(unsigned i=0;i<entity->vertices.size();++i)
	{
		smvec3d tp=smvec3d(entity->vertices[i].x,entity->vertices[i].y,entity->vertices[i].z);
		tp=t*tp;tp=tp+p;vertices.push_back(makeVertex(tp.x,tp.y,tp.z,entity->vertices[i].col,entity->vertices[i].tx,entity->vertices[i].ty));
	}
}
void smEntity3DBuffer::drawBatch()
{
	sm->smDrawCustomIndexedVertices(&vertices[0],&indices[0],vertices.size(),indices.size(),BLEND_ALPHABLEND,0);
	vertices.clear();indices.clear();
}
smParticle::smParticle(){sm=smGetInterface(SMELT_APILEVEL);}
smParticle::~smParticle(){sm->smRelease();}
void smParticle::render()
{sm->smRenderQuad(&q);}
void smParticle::update()
{
	clifespan+=sm->smGetDelta();
	vel=vel+accel;pos=pos+vel;rotv=rotv+rota;rot=rot+rotv;
	size=clifespan/lifespan*(finalsize-initsize)+initsize;
	color=ARGB(
		(DWORD)(clifespan/lifespan*(GETA(finalcolor)-GETA(initcolor)+GETA(initcolor))),
		(DWORD)(clifespan/lifespan*(GETR(finalcolor)-GETR(initcolor)+GETR(initcolor))),
		(DWORD)(clifespan/lifespan*(GETG(finalcolor)-GETG(initcolor)+GETG(initcolor))),
		(DWORD)(clifespan/lifespan*(GETB(finalcolor)-GETB(initcolor)+GETB(initcolor))));
	//set up the quad
}
smParticleSystem::smParticleSystem(){particles.clear();posGenerator=NULL;}
smParticleSystem::~smParticleSystem()
{for(int i=0;i<particles.size();++i)delete particles[i];particles.clear();}
void smParticleSystem::setParticleSystemInfo(smParticleSystemInfo _psinfo)
{psinfo=_psinfo;}
void smParticleSystem::setPos(smvec3d _pos){pos=_pos;}
void smParticleSystem::setPSEmissionPosGen(smPSEmissionPositionGenerator *_gen)
{posGenerator=_gen;}
void smParticleSystem::startPS()
{}
void smParticleSystem::stopPS()
{}
void smParticleSystem::updatePS()
{}
void smParticleSystem::renderPS()
{}
