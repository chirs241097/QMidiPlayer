#ifndef EXTRASMELTUTILS_H
#define EXTRASMELTUTILS_H
#include <ctime>
#include <vector>
#include <smelt.hpp>
#include <smmath.hpp>
#include <smrandom.hpp>
class smEntity3D
{
	friend class smEntity3DBuffer;
	private:
		std::vector<smVertex> vertices;
		std::vector<WORD> indices;
	public:
		smEntity3D(){vertices.clear();indices.clear();}
		void addVertices(size_t n,...);
		void addIndices(size_t n,...);
		smVertex vertex(size_t idx)const;
		WORD index(size_t idx)const;
		void setVertex(size_t idx,smVertex v);
		void setIndex(size_t idx,WORD i);
		static smEntity3D cube(smvec3d tl,smvec3d br,DWORD color,int faces=63);
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
class smPSEmissionPositionGenerator
{
	public:
		virtual smvec3d genPos(){return smvec3d(0,0,0);}
};
class smXLinePSGenerator:public smPSEmissionPositionGenerator
{
	private:
		smRandomEngine re;
		double var;
	public:
		smXLinePSGenerator(double _var){re.setSeed(time(nullptr));var=_var;}
		smvec3d genPos(){return smvec3d(re.nextDouble(-var,var),0,0);}
};
class smParticleSystemInfo
{
	public:
		smvec3d vel,velvar,acc,accvar;
		smvec3d rotv,rotvvar,rota,rotavar;
		double lifespan,lifespanvar;
		int maxcount,emissioncount,ecvar;
		double emissiondelay,edvar;
		double initsize,initsizevar;
		double finalsize,finalsizevar;
		DWORD initcolor,initcolorvar;
		DWORD finalcolor,finalcolorvar;
		SMTEX texture;int blend;
};
class smParticle
{
	friend class smParticleSystem;
	private:
		static SMELT* sm;
		smvec3d pos,rot,lookatpos;
		smvec3d vel,accel,rotv,rota;
		double lifespan,clifespan;
		double initsize,finalsize,size;
		DWORD color,initcolor,finalcolor;
		smQuad q;
		bool dead,lookat;
	public:
		smParticle();
		~smParticle();
		void render();
		void update();
};
class smParticleSystem
{
	private:
		static SMELT* sm;
		std::vector<smParticle*> particles;
		smParticleSystemInfo psinfo;
		smvec3d pos,lookatpos;
		smRandomEngine re;
		smPSEmissionPositionGenerator* posGenerator;
		bool active,lookat;
		double cemdelay,nemdelay;
	public:
		smParticleSystem();
		~smParticleSystem();
		void setParticleSystemInfo(smParticleSystemInfo _psinfo);
		void setPos(smvec3d _pos);
		void setPSEmissionPosGen(smPSEmissionPositionGenerator* _gen);
		void setPSLookAt(smvec3d at);
		void unsetPSLookAt();
		void startPS();
		void stopPS();
		void updatePS();
		void renderPS();
};
extern smVertex makeVertex(float x,float y,float z,DWORD color,float tx,float ty);
#endif // EXTRASMELTUTILS_H
