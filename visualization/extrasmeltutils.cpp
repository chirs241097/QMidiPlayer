#include <cstdio>
#include <cstdarg>
#include <algorithm>
#include <smcolor.hpp>
#include "extrasmeltutils.hpp"
SMELT* smEntity3DBuffer::sm=nullptr;
SMELT* smParticle::sm=nullptr;
SMELT* smParticleSystem::sm=nullptr;
smVertex makeVertex(float x,float y,float z,DWORD color,float tx,float ty)
{smVertex v;v.x=x;v.y=y;v.z=z;v.col=color;v.tx=tx;v.ty=ty;return v;}
void smEntity3D::addVertices(size_t n,...)
{
	va_list vl;va_start(vl,n);
	for(int i=0;i<n;++i)
	{
		smVertex v=va_arg(vl,smVertex);
		vertices.push_back(v);
	}
	va_end(vl);
}
void smEntity3D::addIndices(size_t n,...)
{
	va_list vl;va_start(vl,n);
	for(int i=0;i<n;++i)
	{
		int idx=va_arg(vl,int);
		indices.push_back((WORD)idx);
	}
	va_end(vl);
}
smVertex smEntity3D::vertex(size_t idx)const
{
	if(idx>0&&idx<vertices.size())return vertices[idx];
	return smVertex();
}
WORD smEntity3D::index(size_t idx)const
{
	if(idx>0&&idx<indices.size())return indices[idx];
	return 0;
}
void smEntity3D::setVertex(size_t idx,smVertex v)
{
	if(idx>0&&idx<vertices.size())vertices[idx]=v;
}
void smEntity3D::setIndex(size_t idx,WORD i)
{
	if(idx>0&&idx<indices.size())indices[idx]=i;
}

smEntity3D smEntity3D::cube(smvec3d a,smvec3d b,DWORD color,int faces)
{
	smEntity3D ret;
	ret.addVertices(8,
		makeVertex(a.x,a.y,a.z,color,0,0),makeVertex(b.x,a.y,a.z,color,0,0),
		makeVertex(b.x,b.y,a.z,color,0,0),makeVertex(a.x,b.y,a.z,color,0,0),
		makeVertex(a.x,a.y,b.z,color,0,0),makeVertex(b.x,a.y,b.z,color,0,0),
		makeVertex(b.x,b.y,b.z,color,0,0),makeVertex(a.x,b.y,b.z,color,0,0));
	if(faces&0x1)//a.z
		ret.addIndices(6, 0,1,3, 1,2,3);
	if(faces&0x2)//b.z
		ret.addIndices(6, 4,5,7, 5,6,7);
	if(faces&0x4)//a.x
		ret.addIndices(6, 0,3,7, 0,4,7);
	if(faces&0x8)//b.x
		ret.addIndices(6, 1,2,6, 1,5,6);
	if(faces&0x10)//a.y
		ret.addIndices(6, 0,1,4, 1,4,5);
	if(faces&0x20)//b.y
		ret.addIndices(6, 2,3,7, 2,6,7);
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
	if(!vertices.size())return;
	sm->smDrawCustomIndexedVertices(&vertices[0],&indices[0],vertices.size(),indices.size(),BLEND_ALPHABLEND,0);
	vertices.clear();indices.clear();
}
smParticle::smParticle()
{sm=smGetInterface(SMELT_APILEVEL);dead=false;clifespan=0;}
smParticle::~smParticle(){sm->smRelease();}
void smParticle::render()
{sm->smRenderQuad(&q);}
void smParticle::update()
{
	clifespan+=sm->smGetDelta();if(clifespan>lifespan){dead=true;return;}
	vel=vel+accel;pos=pos+vel;rotv=rotv+rota;rot=rot+rotv;
	size=clifespan/lifespan*(finalsize-initsize)+initsize;
	smColorRGBA fc(finalcolor),ic(initcolor),cc;
	cc.a=clifespan/lifespan*(fc.a-ic.a)+ic.a;
	cc.r=clifespan/lifespan*(fc.r-ic.r)+ic.r;
	cc.g=clifespan/lifespan*(fc.g-ic.g)+ic.g;
	cc.b=clifespan/lifespan*(fc.b-ic.b)+ic.b;
	color=cc.getHWColor();
	for(int i=0;i<4;++i)q.v[i].col=color;
	smMatrix m;m.loadIdentity();
	if(lookat)m.lookat(pos,lookatpos,smvec3d(0,0,1));else
	{m.rotate(rot.x,1,0,0);m.rotate(rot.y,0,1,0);m.rotate(rot.z,0,0,1);}
	smvec3d v0=m*smvec3d(-size,-size,0),v1=m*smvec3d(size,-size,0);
	smvec3d v2=m*smvec3d(size,size,0),v3=m*smvec3d(-size,size,0);
	q.v[0].x=v0.x+pos.x;q.v[0].y=v0.y+pos.y;q.v[0].z=v0.z+pos.z;
	q.v[1].x=v1.x+pos.x;q.v[1].y=v1.y+pos.y;q.v[1].z=v1.z+pos.z;
	q.v[2].x=v2.x+pos.x;q.v[2].y=v2.y+pos.y;q.v[2].z=v2.z+pos.z;
	q.v[3].x=v3.x+pos.x;q.v[3].y=v3.y+pos.y;q.v[3].z=v3.z+pos.z;
}
smParticleSystem::smParticleSystem()
{sm=smGetInterface(SMELT_APILEVEL);particles.clear();posGenerator=nullptr;active=false;}
smParticleSystem::~smParticleSystem()
{for(unsigned i=0;i<particles.size();++i)delete particles[i];particles.clear();}
void smParticleSystem::setParticleSystemInfo(smParticleSystemInfo _psinfo)
{psinfo=_psinfo;}
void smParticleSystem::setPos(smvec3d _pos){pos=_pos;}
void smParticleSystem::setPSEmissionPosGen(smPSEmissionPositionGenerator *_gen)
{posGenerator=_gen;}
void smParticleSystem::setPSLookAt(smvec3d at){lookat=true;lookatpos=at;}
void smParticleSystem::unsetPSLookAt(){lookat=false;}
void smParticleSystem::startPS()
{active=true;nemdelay=0;re.setSeed(time(nullptr));}
void smParticleSystem::stopPS()
{active=false;}
void smParticleSystem::updatePS()
{
	cemdelay+=sm->smGetDelta();
	if(active&&cemdelay>nemdelay&&(int)particles.size()<psinfo.maxcount)
	{
		int ec=re.nextInt(psinfo.emissioncount-psinfo.ecvar,psinfo.emissioncount+psinfo.ecvar);
		for(int i=0;i<ec;++i)
		{
			smParticle *p=new smParticle();
			p->pos=pos+(posGenerator?posGenerator->genPos():smvec3d(0,0,0));
			p->vel=smvec3d(
				re.nextDouble(psinfo.vel.x-psinfo.velvar.x,psinfo.vel.x+psinfo.velvar.x),
				re.nextDouble(psinfo.vel.y-psinfo.velvar.y,psinfo.vel.y+psinfo.velvar.y),
				re.nextDouble(psinfo.vel.z-psinfo.velvar.z,psinfo.vel.z+psinfo.velvar.z));
			p->accel=smvec3d(
				re.nextDouble(psinfo.acc.x-psinfo.accvar.x,psinfo.acc.x+psinfo.accvar.x),
				re.nextDouble(psinfo.acc.y-psinfo.accvar.y,psinfo.acc.y+psinfo.accvar.y),
				re.nextDouble(psinfo.acc.z-psinfo.accvar.z,psinfo.acc.z+psinfo.accvar.z));
			p->rotv=smvec3d(
				re.nextDouble(psinfo.rotv.x-psinfo.rotvvar.x,psinfo.rotv.x+psinfo.rotvvar.x),
				re.nextDouble(psinfo.rotv.y-psinfo.rotvvar.y,psinfo.rotv.y+psinfo.rotvvar.y),
				re.nextDouble(psinfo.rotv.z-psinfo.rotvvar.z,psinfo.rotv.z+psinfo.rotvvar.z));
			p->rota=smvec3d(
				re.nextDouble(psinfo.rota.x-psinfo.rotavar.x,psinfo.rota.x+psinfo.rotavar.x),
				re.nextDouble(psinfo.rota.y-psinfo.rotavar.y,psinfo.rota.y+psinfo.rotavar.y),
				re.nextDouble(psinfo.rota.z-psinfo.rotavar.z,psinfo.rota.z+psinfo.rotavar.z));
			p->rot=smvec3d(0,0,0);if(lookat)p->lookat=true,p->lookatpos=lookatpos;else p->lookat=false;
			p->lifespan=re.nextDouble(psinfo.lifespan-psinfo.lifespanvar,psinfo.lifespan+psinfo.lifespanvar);
			p->initsize=re.nextDouble(psinfo.initsize-psinfo.initsizevar,psinfo.initsize+psinfo.initsizevar);
			p->finalsize=re.nextDouble(psinfo.finalsize-psinfo.finalsizevar,psinfo.finalsize+psinfo.finalsizevar);
			p->size=p->initsize;
			p->initcolor=ARGB(
				re.nextInt(GETA(psinfo.initcolor)-GETA(psinfo.initcolorvar),GETA(psinfo.initcolor)+GETA(psinfo.initcolorvar)),
				re.nextInt(GETR(psinfo.initcolor)-GETR(psinfo.initcolorvar),GETR(psinfo.initcolor)+GETR(psinfo.initcolorvar)),
				re.nextInt(GETG(psinfo.initcolor)-GETG(psinfo.initcolorvar),GETG(psinfo.initcolor)+GETG(psinfo.initcolorvar)),
				re.nextInt(GETB(psinfo.initcolor)-GETB(psinfo.initcolorvar),GETB(psinfo.initcolor)+GETB(psinfo.initcolorvar)));
			p->finalcolor=ARGB(
				re.nextInt(GETA(psinfo.finalcolor)-GETA(psinfo.finalcolorvar),GETA(psinfo.finalcolor)+GETA(psinfo.finalcolorvar)),
				re.nextInt(GETR(psinfo.finalcolor)-GETR(psinfo.finalcolorvar),GETR(psinfo.finalcolor)+GETR(psinfo.finalcolorvar)),
				re.nextInt(GETG(psinfo.finalcolor)-GETG(psinfo.finalcolorvar),GETG(psinfo.finalcolor)+GETG(psinfo.finalcolorvar)),
				re.nextInt(GETB(psinfo.finalcolor)-GETB(psinfo.finalcolorvar),GETB(psinfo.finalcolor)+GETB(psinfo.finalcolorvar)));
			p->color=p->initcolor;p->q.tex=psinfo.texture;p->q.blend=psinfo.blend;
			p->q.v[0].tx=p->q.v[3].tx=0;p->q.v[0].ty=p->q.v[1].ty=0;
			p->q.v[1].tx=p->q.v[2].tx=1;p->q.v[2].ty=p->q.v[3].ty=1;
			particles.push_back(p);
		}
		cemdelay=0;
		nemdelay=re.nextDouble(psinfo.emissiondelay-psinfo.edvar,psinfo.emissiondelay+psinfo.edvar);
	}
	for(unsigned i=0,j;i<particles.size()&&!particles[i]->dead;++i)
	{
		particles[i]->update();
		if(particles[i]->dead)
		{
			for(j=particles.size()-1;j>i&&particles[j]->dead;--j);
			std::swap(particles[i],particles[j]);
		}
	}
	while(!particles.empty()&&particles.back()->dead)
	{delete particles.back();particles.back()=nullptr;particles.pop_back();}
}
void smParticleSystem::renderPS()
{for(unsigned i=0;i<particles.size();++i)particles[i]->render();}

smColor::smColor()
{r=g=b=h=s=v=a=0;}

void smColor::update_rgb()
{
	auto f=[this](float n){
		float k=fmodf(n+6.0f*this->h,6.0f);
		return this->v-this->v*this->s*max(0.0f,min(1.0f,min(k,4.0f-k)));
	};
	r=f(5);
	g=f(3);
	b=f(1);
}
void smColor::update_hsv()
{
	v=max(r,max(g,b));
	float vm=min(r,min(g,b));
	float chroma=v-vm;
	if(v-vm<EPSF)h=0;
	else if(v-r<EPSF)h=(0.0f+(g-b)/chroma)/6.0f;
	else if(v-g<EPSF)h=(2.0f+(b-r)/chroma)/6.0f;
	else if(v-b<EPSF)h=(4.0f+(r-g)/chroma)/6.0f;
	if(v<EPSF)s=0;else s=chroma/v;
}

void smColor::clamp(bool hsv)
{
	if(hsv)
	{
		h=min(1.0f,max(0.0f,h));
		s=min(1.0f,max(0.0f,s));
		v=min(1.0f,max(0.0f,v));
		update_rgb();
	}
	else
	{
		r=min(1.0f,max(0.0f,r));
		g=min(1.0f,max(0.0f,g));
		b=min(1.0f,max(0.0f,b));
		update_hsv();
	}
}
float smColor::alpha()const
{return a;}
float smColor::red()const
{return r;}
float smColor::green()const
{return g;}
float smColor::blue()const
{return b;}
float smColor::hue()const
{return h;}
float smColor::saturation()const
{return s;}
float smColor::hslSaturation()const
{
	float l=lightness();
	if(fabsf(l)<EPSF||fabsf(l-1)<EPSF)return 0;
	return (v-l)/min(l,1-l);
}
float smColor::value()const
{return v;}
float smColor::lightness()const
{return v-v*s/2;}

void smColor::setAlpha(float alpha)
{a=alpha;}
void smColor::setRed(float red)
{
	if(fabsf(r-red)>EPSF)
	{
		r=red;
		update_hsv();
	}
}
void smColor::setGreen(float green)
{
	if(fabsf(g-green)>EPSF)
	{
		g=green;
		update_hsv();
	}
}
void smColor::setBlue(float blue)
{
	if(fabsf(b-blue)>EPSF)
	{
		b=blue;
		update_hsv();
	}
}
void smColor::setHue(float hue)
{
	if(fabsf(h-hue)>EPSF)
	{
		h=hue;
		update_rgb();
	}
}
void smColor::setSaturation(float saturation)
{
	if(fabsf(s-saturation)>EPSF)
	{
		s=saturation;
		update_rgb();
	}
}
void smColor::setHSLSaturation(float saturation)
{
	float ss=hslSaturation();
	float l=lightness();
	if(fabsf(ss-saturation)>EPSF)
	{
		ss=saturation;
		v=l+ss*min(l,1-l);
		if(v<EPSF)s=0;
		else s=2-2*l/v;
		update_rgb();
	}
}
void smColor::setValue(float value)
{
	if(fabsf(v-value)>EPSF)
	{
		v=value;
		update_rgb();
	}
}
void smColor::setLightness(float lightness)
{
	float ss=hslSaturation();
	float l=this->lightness();
	if(fabsf(l-lightness)>EPSF)
	{
		l=lightness;
		v=l+ss*min(l,1-l);
		if(v<EPSF)s=0;
		else s=2-2*l/v;
		update_rgb();
	}
}
smColor smColor::lighter(int factor)
{
	smColor ret(*this);
	ret.setValue(v*(factor/100.0f));
	ret.clamp(true);
	return ret;
}
smColor smColor::darker(int factor)
{
	smColor ret(*this);
	ret.setValue(factor?(v/(factor/100.0f)):1.);
	ret.clamp(true);
	return ret;
}

uint32_t smColor::toHWColor()
{
	return RGBA(r*255,g*255,b*255,a*255);
}
smColor smColor::fromHWColor(uint32_t color)
{
	smColor ret;
	ret.r=GETR(color)/255.0f;
	ret.g=GETG(color)/255.0f;
	ret.b=GETB(color)/255.0f;
	ret.a=GETA(color)/255.0f;
	ret.update_hsv();
	return ret;
}
