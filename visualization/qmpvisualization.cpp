#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <set>
#include "qmpvisualization.hpp"

int viewdist=100;
int notestretch=100;//length of quarter note
int minnotelength=100;
int noteappearance=1,showpiano=1,stairpiano=1,savevp=1,showlabel=1;
int wwidth=800,wheight=600,wsupersample=1,wmultisample=0,showparticle=1;
int horizontal=1,flat=0,osdpos=0,fontsize=16,showmeasure=1;
int fov=60,vsync=1,tfps=60,usespectrum=0;
DWORD chkrtint=0xFF999999;
const wchar_t* minors=L"abebbbf c g d a e b f#c#g#d#a#";
const wchar_t* majors=L"CbGbDbAbEbBbF C G D A E B F#C#";
double fpoffsets[]={1,18,28,50,55,82,98,109,130,137,161,164,191};
double froffsets[]={0,18,33,50,65,82,98,113,130,145,161,176,191};
DWORD iccolors[]={0XFFFF0000,0XFFFF8000,0XFFFFBF00,0XFFF0F000,
				  0XFFB2EE00,0XFF80FF00,0XFF00FF00,0XFF00EEB2,
				  0XFF00EEEE,0XFF333333,0XFF00BFFF,0XFF007FFF,
				  0XFF0000FF,0XFF7F00FF,0XFFBF00FF,0XFFFF00BF};
DWORD accolors[]={0XFFFF9999,0XFFFFCC99,0XFFFFF4D4,0XFFFFFFDD,
				  0XFFF0FFC2,0XFFDDFFBB,0XFFBBFFBB,0XFFAAFFEA,
				  0XFFBBFFFF,0XFF999999,0XFF99EEFF,0XFF99CCFF,
				  0XFF9999FF,0XFFCC99FF,0XFFEE99FF,0XFFFF99EE};

std::set<BYTE> sustaininst={16,17,18,19,20,21,22,23,
							40,41,42,43,44,45,48,49,
							50,51,52,53,54,56,57,58,
							59,60,61,62,63,64,65,66,
							67,68,69,70,71,72,73,74,
							75,76,77,78,79,80,81,82,
							83,84,85,86,87,89,90,91,
							92,93,94,95,97,101,109,110,111};

bool cmp(MidiVisualEvent* a,MidiVisualEvent* b)
{
	if(a->tcs<b->tcs)return true;if(a->tcs>b->tcs)return false;
	if(a->tce<b->tce)return true;return false;
}
void qmpVisualization::showThread()
{
	wwidth=api->getOptionInt("Visualization/wwidth");
	wheight=api->getOptionInt("Visualization/wheight");
	wsupersample=api->getOptionInt("Visualization/supersampling");
	wmultisample=api->getOptionInt("Visualization/multisampling");
	fov=api->getOptionInt("Visualization/fov");
	noteappearance=api->getOptionBool("Visualization/3dnotes");
	showpiano=api->getOptionBool("Visualization/showpiano");
	stairpiano=api->getOptionBool("Visualization/stairpiano");
	showlabel=api->getOptionBool("Visualization/showlabel");
	showparticle=api->getOptionBool("Visualization/showparticle");
	horizontal=api->getOptionBool("Visualization/horizontal");
	flat=api->getOptionBool("Visualization/flat");
	showmeasure=api->getOptionBool("Visualization/showmeasure");
	savevp=api->getOptionBool("Visualization/savevp");
	vsync=api->getOptionBool("Visualization/vsync");
	tfps=api->getOptionInt("Visualization/tfps");
	osdpos=api->getOptionEnumInt("Visualization/osdpos");
	fontsize=api->getOptionInt("Visualization/fontsize");
	viewdist=api->getOptionInt("Visualization/viewdist");
	notestretch=api->getOptionInt("Visualization/notestretch");
	minnotelength=api->getOptionInt("Visualization/minnotelen");
	chkrtint=api->getOptionUint("Visualization/chkrtint");
	usespectrum=api->getOptionBool("Visualization/usespectrum");
	for(int i=0;i<16;++i)
	{
		accolors[i]=api->getOptionUint("Visualization/chActiveColor"+std::to_string(i));
		iccolors[i]=api->getOptionUint("Visualization/chInactiveColor"+std::to_string(i));
	}
	sm=smGetInterface(SMELT_APILEVEL);
	sm->smVidMode(wwidth,wheight,true,!hidewindow);
	sm->smUpdateFunc(h);sm->smQuitFunc(closeh);
	sm->smWinTitle("QMidiPlayer Visualization");
	sm->smSetFPS(vsync?FPS_VSYNC:tfps);
	sm->smNoSuspend(true);
	sm->smInit();shouldclose=false;
	sm->smTextureOpt(TPOT_POT,TFLT_LINEAR);
	chequer=sm->smTextureLoad("chequerboard.png");if(!chequer)
	chequer=sm->smTextureLoad("/usr/share/qmidiplayer/img/chequerboard.png");
	pianotex=sm->smTextureLoad("kb_128.png");if(!pianotex)
	pianotex=sm->smTextureLoad("/usr/share/qmidiplayer/img/kb_128.png");
	particletex=sm->smTextureLoad("particle.png");if(!particletex)
	particletex=sm->smTextureLoad("/usr/share/qmidiplayer/img/particle.png");
	bgtex=sm->smTextureLoad(api->getOptionString("Visualization/background").c_str());
	if(rendermode)
		fbcont=new DWORD[wwidth*wheight];
	if(showparticle&&!horizontal)
	{
		smParticleSystemInfo psinfo;
		psinfo.acc=smvec3d(0,0,-0.05);psinfo.accvar=smvec3d(0,0,0.005);
		psinfo.vel=smvec3d(0,0,0.5);psinfo.velvar=smvec3d(0.1,0.1,0.2);
		psinfo.rotv=psinfo.rota=psinfo.rotavar=smvec3d(0,0,0);psinfo.rotvvar=smvec3d(0.04,0.04,0.04);
		psinfo.lifespan=1;psinfo.lifespanvar=0.5;psinfo.maxcount=1000;psinfo.emissioncount=5;psinfo.ecvar=2;
		psinfo.emissiondelay=0.1;psinfo.edvar=0;psinfo.initsize=0.8;psinfo.initsizevar=0.1;
		psinfo.finalsize=0.1;psinfo.finalsizevar=0.05;psinfo.initcolor=0xFFFFFFFF;psinfo.finalcolor=0x00FFFFFF;
		psinfo.initcolorvar=psinfo.finalcolorvar=0;psinfo.texture=particletex;psinfo.blend=BLEND_ALPHAADD;
		psepg=new smXLinePSGenerator(.6);
		for(int i=0;i<16;++i)for(int j=0;j<128;++j)
		{
			pss[i][j]=new smParticleSystem();
			pss[i][j]->setPSEmissionPosGen(psepg);
			psinfo.initcolor=accolors[i];psinfo.finalcolor=SETA(accolors[i],0);
			pss[i][j]->setParticleSystemInfo(psinfo);
			pss[i][j]->setPos(smvec3d(0.756*((double)j-64)+.48,(stairpiano?(56-i*7.):(64-i*8.)),stairpiano*i*2+0.1));
		}
	}else memset(pss,0,sizeof(pss));
	if(showpiano&&!horizontal)for(int i=0;i<16;++i)p3d[i]=new qmpVirtualPiano3D();
	memset(traveld,0,sizeof(traveld));
	nebuf=new smEntity3DBuffer();
	tdscn=sm->smTargetCreate(wwidth*wsupersample,wheight*wsupersample,wmultisample);
	tdparticles=sm->smTargetCreate(wwidth*wsupersample,wheight*wsupersample,wmultisample);
	if(!api->getOptionString("Visualization/font2").length()||!font.loadTTF(api->getOptionString("Visualization/font2").c_str(),fontsize))
	if(!font.loadTTF("/usr/share/fonts/truetype/freefont/FreeMono.ttf",fontsize))
	if(!font.loadTTF("/usr/share/fonts/gnu-free/FreeMono.otf",fontsize))
	if(!font.loadTTF((std::string(getenv("windir")?getenv("windir"):"")+"/Fonts/cour.ttf").c_str(),fontsize))
	fprintf(stderr,"W: Font load failed.\n");
	if(!api->getOptionString("Visualization/font2").length()||!fonthdpi.loadTTF(api->getOptionString("Visualization/font2").c_str(),180))
	if(!fonthdpi.loadTTF("/usr/share/fonts/truetype/freefont/FreeMono.ttf",180))
	if(!fonthdpi.loadTTF("/usr/share/fonts/gnu-free/FreeMono.otf",180))
	if(!fonthdpi.loadTTF((std::string(getenv("windir")?getenv("windir"):"")+"/Fonts/cour.ttf").c_str(),180))
	fprintf(stderr,"W: Font load failed.\n");
	if(!api->getOptionString("Visualization/font1").length()||!font2.loadTTF(api->getOptionString("Visualization/font1").c_str(),fontsize))
	if(!font2.loadTTF("/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",fontsize))
	if(!font2.loadTTF("/usr/share/fonts/wenquanyi/wqy-microhei/wqy-microhei.ttc",fontsize))
	if(!font2.loadTTF((std::string(getenv("windir")?getenv("windir"):"")+"/Fonts/msyh.ttc").c_str(),fontsize))
	if(!font2.loadTTF((std::string(getenv("windir")?getenv("windir"):"")+"/Fonts/segoeui.ttf").c_str(),fontsize))
	fprintf(stderr,"W: Font load failed.\n");
	if(pos[0]<-1e8)
	{
		if(horizontal)
		{
			pos[0]=-20;pos[1]=45;pos[2]=0;
			rot[0]=0;rot[1]=90;rot[2]=90;
		}
		else
		{
			pos[0]=0;pos[1]=120;pos[2]=70;
			rot[0]=0;rot[1]=75;rot[2]=90;
		}
	}
	debug=false;
	ctk=api->getCurrentTimeStamp();
	lst=std::chrono::steady_clock::now();
	sm->smMainLoop();
	sm->smFinale();
}
void qmpVisualization::show()
{
	rendererTh=new std::thread(&qmpVisualization::showThread,this);
}
void qmpVisualization::close()
{
	shouldclose=true;
	if(rendererTh)
	{
		rendererTh->join();
		delete rendererTh;
		rendererTh=nullptr;
	}else return;

	if(showpiano&&!horizontal)for(int i=0;i<16;++i)delete p3d[i];
	if(showparticle&&!horizontal)for(int i=0;i>16;++i)for(int j=0;j<128;++j){delete pss[i][j];pss[i][j]=0;}
	delete nebuf;
	if(savevp)
	{
		api->setOptionDouble("Visualization/px",pos[0]);
		api->setOptionDouble("Visualization/py",pos[1]);
		api->setOptionDouble("Visualization/pz",pos[2]);
		api->setOptionDouble("Visualization/rx",rot[0]);
		api->setOptionDouble("Visualization/ry",rot[1]);
		api->setOptionDouble("Visualization/rz",rot[2]);
	}
	if(rendermode)
		delete[] fbcont;
	font.releaseTTF();
	font2.releaseTTF();
	fonthdpi.releaseTTF();
	sm->smTextureFree(chequer);
	sm->smTextureFree(pianotex);
	sm->smTextureFree(particletex);
	if(bgtex)sm->smTextureFree(bgtex);
	sm->smTargetFree(tdscn);
	sm->smTargetFree(tdparticles);
	sm->smRelease();
}
void qmpVisualization::reset()
{
	for(unsigned i=0;i<pool.size();++i)delete pool[i];
	pool.clear();elb=ctk=lstk=cfr=0;tspool.clear();
	cts=0x0402;cks=0;ctp=500000;
	for(int i=0;i<16;++i)
	{cpbr[i]=2;cpw[i]=8192;}
	for(int i=0;i<16;++i)for(int j=0;j<128;++j)
	{
		if(showparticle&&!horizontal&&pss[i][j])pss[i][j]->stopPS();
		while(!pendingt[i][j].empty())pendingt[i][j].pop();
		while(!pendingv[i][j].empty())pendingv[i][j].pop();
	}
}

void qmpVisualization::switchToRenderMode(void(*frameCallback)(void*,size_t,uint32_t,uint32_t),bool _hidewindow)
{
	rendermode=true;
	framecb=frameCallback;
	hidewindow=_hidewindow;
}
void qmpVisualization::start(){playing=true;}
void qmpVisualization::stop(){playing=false;}
void qmpVisualization::pause(){playing=!playing;}
void qmpVisualization::updateVisualization3D()
{
	smQuad q;
	if(!rendermode)
	{
		if(sm->smGetKeyState(SMK_D))pos[0]+=cos(smMath::deg2rad(rot[2]-90)),pos[1]+=sin(smMath::deg2rad(rot[2]-90));
		if(sm->smGetKeyState(SMK_A))pos[0]-=cos(smMath::deg2rad(rot[2]-90)),pos[1]-=sin(smMath::deg2rad(rot[2]-90));
		if(sm->smGetKeyState(SMK_S))pos[0]+=cos(smMath::deg2rad(rot[2])),pos[1]+=sin(smMath::deg2rad(rot[2]));
		if(sm->smGetKeyState(SMK_W))pos[0]-=cos(smMath::deg2rad(rot[2])),pos[1]-=sin(smMath::deg2rad(rot[2]));
		if(sm->smGetKeyState(SMK_Q))pos[2]+=1;
		if(sm->smGetKeyState(SMK_E))pos[2]-=1;
		if(sm->smGetKeyState(SMK_R))
		{
			if(horizontal)
			{
				pos[0]=-20;pos[1]=45;pos[2]=0;
				rot[0]=0;rot[1]=90;rot[2]=90;
			}
			else
			{
				pos[0]=0;pos[1]=120;pos[2]=70;
				rot[0]=0;rot[1]=75;rot[2]=90;
			}
		}
		if(sm->smGetKeyState(SMK_LBUTTON)==SMKST_HIT)
		sm->smSetMouseGrab(true),sm->smGetMouse2f(&lastx,&lasty);
		if(sm->smGetKeyState(SMK_LBUTTON)==SMKST_KEEP)
		{
			float x,y;
			sm->smGetMouse2f(&x,&y);
			rot[1]-=(y-lasty)*0.01;
			rot[2]+=(x-lastx)*0.01;
			while(rot[1]>360)rot[1]-=360;
			while(rot[1]<0)rot[1]+=360;
			while(rot[2]>360)rot[2]-=360;
			while(rot[2]<0)rot[2]+=360;
		}
		if(sm->smGetKeyState(SMK_LBUTTON)==SMKST_RELEASE)
		{
			sm->smSetMouseGrab(false);
			sm->smSetMouse2f(wwidth/2,wheight/2);
		}
		if(sm->smGetKeyState(SMK_I))rot[1]+=1;
		if(sm->smGetKeyState(SMK_K))rot[1]-=1;
		if(sm->smGetKeyState(SMK_L))rot[0]+=1;
		if(sm->smGetKeyState(SMK_J))rot[0]-=1;
		if(sm->smGetKeyState(SMK_U))rot[2]+=1;
		if(sm->smGetKeyState(SMK_O))rot[2]-=1;
	}
	for(int i=0;i<4;++i)
	{q.v[i].col=chkrtint;q.v[i].z=(showpiano&&!horizontal)?-5:0;}
	q.tex=chequer;q.blend=BLEND_ALPHABLEND;
	q.v[0].x=q.v[3].x=-120;q.v[1].x=q.v[2].x=120;
	q.v[0].y=q.v[1].y=-120;q.v[2].y=q.v[3].y=120;
	if(horizontal)
	{
		for(int i=0;i<4;++i)q.v[i].x=-20;
		q.v[0].y=q.v[3].y=-120;q.v[1].y=q.v[2].y=120;
		q.v[0].z=q.v[1].z=-120;q.v[2].z=q.v[3].z=120;
	}
	q.v[0].tx=q.v[3].tx=0;q.v[1].tx=q.v[2].tx=30;
	q.v[0].ty=q.v[1].ty=0;q.v[2].ty=q.v[3].ty=30;
	sm->smRenderBegin3D(fov,true,tdscn);
	sm->sm3DCamera6f2v(pos,rot);
	sm->smClrscr(0,1,1);
	sm->smRenderQuad(&q);
	double lpt=(double)notestretch/api->getDivision()/10.*(horizontal?0.25:1);
	memcpy(lastnotestatus,notestatus,sizeof(notestatus));
	memset(notestatus,0,sizeof(notestatus));
	for(uint32_t i=elb;i<pool.size();++i)
	{
		if(((double)pool[i]->tcs-ctk)*lpt>viewdist*2)break;
		if(fabs((double)pool[i]->tcs-ctk)*lpt<viewdist*2||fabs((double)pool[i]->tce-ctk)*lpt<viewdist*2)
		{
			if(pool[i]->ch==999){
				smvec3d a(0.63*(-64)+.1-10,(stairpiano?(56-0*7.):(64-0*8.))+10,((double)pool[i]->tcs-ctk)*lpt-minnotelength*.005);
				smvec3d b(0.63*64+.7+10,(stairpiano?(56-15*7.):(64-15*8.))+.4-10,((double)pool[i]->tcs-ctk)*lpt+minnotelength*.005);
				if(horizontal){
					a=smvec3d(((double)pool[i]->tcs-ctk)*lpt-20-minnotelength*.001,(16- 0*2.)+2.4,0.63*(-64)+.1);
					b=smvec3d(((double)pool[i]->tcs-ctk)*lpt-20+minnotelength*.001,(16-15*2.)+0.4,0.63*64+.7);
				}
				smMatrix I;I.loadIdentity();
				smEntity3D c=smEntity3D::cube(a,b,0xFF000000,horizontal?51:60);
				if(stairpiano&&showpiano&&!horizontal)
				{
					std::vector<size_t> il={2,3,6,7};
					for(size_t ti:il)
					{
						smVertex t=c.vertex(ti);
						t.z+=30;
						c.setVertex(ti,t);
					}
				}
				if(showmeasure)
					nebuf->addTransformedEntity(&c,I,smvec3d(0,0,0));
				continue;
			}
			if(pool[i]->ch>=990)continue;
			if(api->getChannelMask(pool[i]->ch))continue;
			smvec3d a(0.63*((double)pool[i]->key-64)+.1,(stairpiano?(56-pool[i]->ch*7.):(64-pool[i]->ch*8.)),((double)pool[i]->tce-ctk)*lpt+(stairpiano&&showpiano&&!horizontal)*pool[i]->ch*2.);
			smvec3d b(0.63*((double)pool[i]->key-64)+.7,(stairpiano?(56-pool[i]->ch*7.):(64-pool[i]->ch*8.))+.4,((double)pool[i]->tcs-ctk)*lpt+(stairpiano&&showpiano&&!horizontal)*pool[i]->ch*2.);
			bool isnoteon=pool[i]->tcs<=ctk&&pool[i]->tce>=ctk;
			double pb=((int)cpw[pool[i]->ch]-8192)/8192.*cpbr[pool[i]->ch];
			if(isnoteon)
			{
				a.x=0.63*((double)pool[i]->key-64+pb)+.1;
				b.x=0.63*((double)pool[i]->key-64+pb)+.7;
			}
			notestatus[pool[i]->ch][pool[i]->key]|=isnoteon;a.x*=1.2;b.x*=1.2;
			if(horizontal)
			{
				a=smvec3d(((double)pool[i]->tcs-ctk)*lpt-20,(16-pool[i]->ch*2.),0.63*((double)pool[i]->key-64)+.1);
				b=smvec3d(((double)pool[i]->tce-ctk)*lpt-20,(16-pool[i]->ch*2.)+.4,0.63*((double)pool[i]->key-64)+.7);
				if(isnoteon)
				{
					a.z=0.63*((double)pool[i]->key-64+pb)+.1;
					b.z=0.63*((double)pool[i]->key-64+pb)+.7;
				}
			}
			if(showparticle&&!horizontal)
			{
				if(notestatus[pool[i]->ch][pool[i]->key]&&!lastnotestatus[pool[i]->ch][pool[i]->key])
				{
					pss[pool[i]->ch][pool[i]->key]->startPS();
					pss[pool[i]->ch][pool[i]->key]->setPos(smvec3d(0.756*((double)pool[i]->key-64)+.48,(stairpiano?(56-pool[i]->ch*7.):(64-pool[i]->ch*8.)),stairpiano*pool[i]->ch*2+0.1));
				}
				else pss[pool[i]->ch][pool[i]->key]->stopPS();
			}
			if(((double)pool[i]->tce-pool[i]->tcs)*lpt<minnotelength*(horizontal?0.0025:0.01))
			{
				if(horizontal)
					a.x=((double)pool[i]->tcs-ctk)*lpt-minnotelength/400.-20;
				else
					a.z=((double)pool[i]->tcs-ctk)*lpt-minnotelength/100.+stairpiano*pool[i]->ch*2;
			}
			if(usespectrum)
			{
				if(notestatus[pool[i]->ch][pool[i]->key]&&!lastnotestatus[pool[i]->ch][pool[i]->key])
					spectra[pool[i]->ch][pool[i]->key]=pool[i]->vel*(api->getChannelCC(pool[i]->ch,7)/127.);
			}
			else
			{
				smColor col=smColor::fromHWColor(isnoteon?accolors[pool[i]->ch]:iccolors[pool[i]->ch]);
				drawCube(a,b,col.lighter(37+pool[i]->vel/2).toHWColor(),0);
			}
		}
	}
	if(usespectrum&&playing)
	for(int i=0;i<16;++i)for(int j=0;j<128;++j)
	{
		if(sustaininst.find(api->getChannelPreset(i))!=sustaininst.end())
		{
			if(!notestatus[i][j]&&spectra[i][j])
				spectra[i][j]=.95*spectra[i][j];
		}else if(spectra[i][j])spectra[i][j]=.95*spectra[i][j];
		if(spectrar[i][j]<spectra[i][j]*0.9)spectrar[i][j]+=spectra[i][j]*0.2;
		else spectrar[i][j]=spectra[i][j];
		if(spectrar[i][j])
		{
			double pb=((int)cpw[i]-8192)/8192.*cpbr[i];
			smvec3d a(0.756*((double)j-64+pb)+.12,
					  (stairpiano?(56-i*7.):(64-i*8.)),
					  spectrar[i][j]*1.2*(1+0.02*sin(sm->smGetTime()*32))+(stairpiano&&showpiano&&!horizontal)*i*2.);
			smvec3d b(0.756*((double)j-64+pb)+.84,
					  (stairpiano?(56-i*7.):(64-i*8.))+.4,
					  (stairpiano&&showpiano&&!horizontal)*i*2.);
			drawCube(a,b,SETA(iccolors[i],204),0);
		}
	}
	nebuf->drawBatch();
	if(showpiano&&!horizontal)
	for(int i=0;i<16;++i)
	{
		for(int j=0;j<128;++j)
		{
			if(notestatus[i][j])
			if(traveld[i][j]<10)traveld[i][j]+=2;else traveld[i][j]=10;
			else
			if(traveld[i][j]>0)traveld[i][j]-=2;else traveld[i][j]=0;
			p3d[i]->setKeyTravelDist(j,traveld[i][j]/10.);
		}
		double pb=((int)cpw[i]-8192)/8192.*cpbr[i];
		p3d[i]->render(smvec3d(0.756*pb,stairpiano?55-i*7:62-i*8,stairpiano*i*2));
	}
	for(int i=0;i<16;++i)
		if(showlabel)
		{
			std::string s=api->getChannelPresetString(i);
			wchar_t ws[1024];mbstowcs(ws,s.c_str(),1024);
			fonthdpi.updateString(ws);
			fonthdpi.render(-49,stairpiano?56-i*7:63-i*8,stairpiano*i*2+0.1,0xFFFFFFFF,ALIGN_RIGHT,.008,0.01);
			fonthdpi.render(-49.05,stairpiano?56.05-i*7:63.05-i*8,stairpiano*i*2+0.2,0xFF000000,ALIGN_RIGHT,.008,0.01);
		}
	while(pool.size()&&elb<pool.size()&&((double)ctk-pool[elb]->tce)*lpt>viewdist*2)++elb;
	sm->smRenderEnd();
	if(showparticle&&!horizontal)
	{
		sm->smRenderBegin3D(fov,false,tdparticles);
		sm->sm3DCamera6f2v(pos,rot);
		sm->smClrscr(0,1,1);
		for(int i=0;i<16;++i)for(int j=0;j<128;++j)
		{
			pss[i][j]->setPSLookAt(smvec3d(pos[0],pos[1],pos[2]));
			pss[i][j]->updatePS();pss[i][j]->renderPS();
		}
		sm->smRenderEnd();
	}
}
void qmpVisualization::updateVisualization2D()
{
	double lpt=-(double)notestretch/api->getDivision()/2.;
	memset(notestatus,0,sizeof(notestatus));
	double notew=wwidth/128,nh=showpiano?wwidth/2048.*172.:0;
	if(horizontal){notew=wheight/128;nh=showpiano?wheight/2048.*172.:0;lpt=-lpt;}
	smQuad nq;nq.blend=BLEND_ALPHABLEND;nq.tex=0;
	for(int i=0;i<4;++i)nq.v[i].z=0,nq.v[i].tx=nq.v[i].ty=0;
	for(uint32_t i=elb;i<pool.size();++i)
	{
		bool upperbound=((double)pool[i]->tcs-ctk)*lpt+wheight-nh<0;
		bool lowerbound=fabs((double)pool[i]->tce-ctk)*lpt+wheight-nh<wheight;
		if(horizontal)
		{
			upperbound=((double)pool[i]->tcs-ctk)*lpt+nh>wwidth;
			lowerbound=fabs((double)pool[i]->tce-ctk)*lpt+nh>0;
		}
		if(upperbound)break;
		if(!upperbound||lowerbound)
		{
			if(pool[i]->ch==999){
				smvec2d a(0,((double)pool[i]->tcs-ctk)*lpt+wheight-nh-minnotelength*0.02);
				smvec2d b(wwidth,((double)pool[i]->tcs-ctk)*lpt+wheight-nh);
				if(horizontal)
				{
					a=smvec2d(((double)pool[i]->tcs-ctk)*lpt+nh-minnotelength*0.02,0);
					b=smvec2d(((double)pool[i]->tcs-ctk)*lpt+nh,wheight);
				}
				nq.v[0].x=nq.v[3].x=a.x;nq.v[0].y=nq.v[1].y=a.y;
				nq.v[1].x=nq.v[2].x=b.x;nq.v[2].y=nq.v[3].y=b.y;
				for(int j=0;j<4;++j)nq.v[j].col=0xC0000000;
				if(showmeasure)sm->smRenderQuad(&nq);
				continue;
			}
			if(pool[i]->ch>=990)continue;
			if(api->getChannelMask(pool[i]->ch))continue;
			smvec2d a((froffsets[12]*(pool[i]->key/12)+froffsets[pool[i]->key%12])*wwidth/2048.,((double)pool[i]->tce-ctk)*lpt+wheight-nh);
			smvec2d b(a.x+notew*0.9,((double)pool[i]->tcs-ctk)*lpt+wheight-nh);
			if(horizontal)
			{
				a=smvec2d(((double)pool[i]->tce-ctk)*lpt+nh,(froffsets[12]*(pool[i]->key/12)+froffsets[pool[i]->key%12])*wheight/2048.);
				b=smvec2d(((double)pool[i]->tcs-ctk)*lpt+nh,a.y+notew*0.9);
			}
			bool isnoteon=pool[i]->tcs<=ctk&&pool[i]->tce>=ctk;
			if(isnoteon)
			{
				double pb=((int)cpw[pool[i]->ch]-8192)/8192.*cpbr[pool[i]->ch];
				uint32_t newkey=pool[i]->key+(int)floor(pb);
				double fpb=pb-floor(pb);
				if(horizontal)
				{
					a.y=(froffsets[12]*(newkey/12)+froffsets[newkey%12])*wheight/2048.+notew*fpb;
					b.y=a.y+notew*0.9;
				}
				else
				{
					a.x=(froffsets[12]*(newkey/12)+froffsets[newkey%12])*wwidth/2048.+notew*fpb;
					b.x=a.x+notew*0.9;
				}
			}
			if(horizontal)a.y=wheight-a.y,b.y=wheight-b.y;
			notestatus[pool[i]->ch][pool[i]->key]|=isnoteon;
			if(horizontal)
			{
				if(((double)pool[i]->tce-pool[i]->tcs)*lpt<minnotelength*0.04)
					a.x=((double)pool[i]->tcs-ctk)*lpt+nh-minnotelength*0.04;
			}
			else
			{
				if(((double)pool[i]->tcs-pool[i]->tce)*lpt<minnotelength*0.04)
					a.y=((double)pool[i]->tcs-ctk)*lpt+wheight-nh-minnotelength*0.04;
			}
			nq.v[0].x=nq.v[3].x=a.x;nq.v[0].y=nq.v[1].y=a.y;
			nq.v[1].x=nq.v[2].x=b.x;nq.v[2].y=nq.v[3].y=b.y;
			for(int j=0;j<4;++j)
			nq.v[j].col=SETA(isnoteon?accolors[pool[i]->ch]:iccolors[pool[i]->ch],int(pool[i]->vel*1.6+(isnoteon?52:32)));
			if(usespectrum)
			{
				if(notestatus[pool[i]->ch][pool[i]->key]&&!lastnotestatus[pool[i]->ch][pool[i]->key])
					spectra[pool[i]->ch][pool[i]->key]=pool[i]->vel*(api->getChannelCC(pool[i]->ch,7)/127.);
			}else sm->smRenderQuad(&nq);
		}
	}
	if(horizontal)
	while(pool.size()&&elb<pool.size()&&fabs((double)pool[elb]->tce-ctk)*lpt+nh<0)++elb;
	else
	while(pool.size()&&elb<pool.size()&&fabs((double)pool[elb]->tce-ctk)*lpt+wheight-nh>wheight)++elb;
	smQuad q;
	q.tex=pianotex;q.blend=BLEND_ALPHABLEND;
	for(int i=0;i<4;++i)q.v[i].col=0xFFFFFFFF,q.v[i].z=0;
	q.v[0].ty=q.v[3].ty=0;q.v[1].ty=q.v[2].ty=172./256.;
	q.v[0].tx=q.v[1].tx=0;q.v[2].tx=q.v[3].tx=1.;
	q.v[0].x=q.v[1].x=0;q.v[2].x=q.v[3].x=wwidth;
	q.v[0].y=q.v[3].y=wheight-nh;q.v[1].y=q.v[2].y=wheight;
	if(horizontal)
	{
		q.v[0].tx=q.v[3].tx=0;q.v[1].tx=q.v[2].tx=1;
		q.v[0].ty=q.v[1].ty=0;q.v[2].ty=q.v[3].ty=172./256.;
		q.v[0].x=q.v[1].x=nh;q.v[2].x=q.v[3].x=0;
		q.v[0].y=q.v[3].y=wheight;q.v[1].y=q.v[2].y=0;
	}
	sm->smRenderQuad(&q);
	for(int i=0,j;i<128;++i)
	{
		DWORD c=0;for(j=0;j<16;++j)if(notestatus[j][i]){c=SETA(iccolors[j],0xFF);break;}
		if(horizontal)
		{
			q.v[0].y=q.v[1].y=(fpoffsets[12]*(i/12)+fpoffsets[i%12])*wheight/2048.;
			q.v[2].y=q.v[3].y=q.v[0].y;
			q.v[0].x=q.v[3].x=nh;q.v[1].x=q.v[2].x=0;
		}
		else
		{
			q.v[0].x=q.v[1].x=(fpoffsets[12]*(i/12)+fpoffsets[i%12])*wwidth/2048.;
			q.v[2].x=q.v[3].x=q.v[0].x;
			q.v[0].y=q.v[3].y=wheight-nh;q.v[1].y=q.v[2].y=wheight;
		}
		if(!c)continue;for(int j=0;j<4;++j)q.v[j].col=c;
		switch(i%12)
		{
			case 1:case 3:case 6:case 8:case 10:
				if(horizontal)
				{
					q.v[1].x=q.v[2].x=nh-115*wheight/2048.;
					q.v[2].y+=15.*wheight/2048;q.v[3].y+=15.*wheight/2048;
				}
				else
				{
					q.v[1].y=q.v[2].y=wheight-nh+115*wwidth/2048.;
					q.v[2].x+=15.*wwidth/2048;q.v[3].x+=15.*wwidth/2048;
				}
				q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-15/256.;
				q.v[0].tx=q.v[3].tx=1344/2048.;q.v[1].tx=q.v[2].tx=1459/2048.;
			break;
			case 0:
				if(horizontal)
				{q.v[2].y+=27.*wheight/2048;q.v[3].y+=27.*wheight/2048;}
				else
				{q.v[2].x+=27.*wwidth/2048;q.v[3].x+=27.*wwidth/2048;}
				q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-27/256.;
				q.v[0].tx=q.v[3].tx=0/2048.;q.v[1].tx=q.v[2].tx=172/2048.;
			break;
			case 2:
				if(horizontal)
				{q.v[2].y+=29.*wheight/2048;q.v[3].y+=29.*wheight/2048;}
				else
				{q.v[2].x+=29.*wwidth/2048;q.v[3].x+=29.*wwidth/2048;}
				q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-29/256.;
				q.v[0].tx=q.v[3].tx=192/2048.;q.v[1].tx=q.v[2].tx=364/2048.;
			break;
			case 4:
				if(horizontal)
				{q.v[2].y+=28.*wheight/2048;q.v[3].y+=28.*wheight/2048;}
				else
				{q.v[2].x+=28.*wwidth/2048;q.v[3].x+=28.*wwidth/2048;}
				q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-28/256.;
				q.v[0].tx=q.v[3].tx=384/2048.;q.v[1].tx=q.v[2].tx=556/2048.;
			break;
			case 5:
				if(horizontal)
				{q.v[2].y+=28.*wheight/2048;q.v[3].y+=28.*wheight/2048;}
				else
				{q.v[2].x+=28.*wwidth/2048;q.v[3].x+=28.*wwidth/2048;}
				q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-28/256.;
				q.v[0].tx=q.v[3].tx=576/2048.;q.v[1].tx=q.v[2].tx=748/2048.;
			break;
			case 7:
				if(horizontal)
				{q.v[2].y+=29.*wheight/2048;q.v[3].y+=29.*wheight/2048;}
				else
				{q.v[2].x+=29.*wwidth/2048;q.v[3].x+=29.*wwidth/2048;}
				q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-29/256.;
				q.v[0].tx=q.v[3].tx=768/2048.;q.v[1].tx=q.v[2].tx=940/2048.;
			break;
			case 9:
				if(horizontal)
				{q.v[2].y+=28.*wheight/2048;q.v[3].y+=28.*wheight/2048;}
				else
				{q.v[2].x+=28.*wwidth/2048;q.v[3].x+=28.*wwidth/2048;}
				q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-28/256.;
				q.v[0].tx=q.v[3].tx=960/2048.;q.v[1].tx=q.v[2].tx=1132/2048.;
			break;
			case 11:
				if(horizontal)
				{q.v[2].y+=28.*wheight/2048;q.v[3].y+=28.*wheight/2048;}
				else
				{q.v[2].x+=28.*wwidth/2048;q.v[3].x+=28.*wwidth/2048;}
				q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-28/256.;
				q.v[0].tx=q.v[3].tx=1152/2048.;q.v[1].tx=q.v[2].tx=1324/2048.;
			break;
		}
		if(horizontal)for(int j=0;j<4;++j)q.v[j].y=wheight-q.v[j].y;
		sm->smRenderQuad(&q);
	}
	if(usespectrum&&playing)
	for(int i=0;i<16;++i)for(int j=0;j<128;++j)
	{
		if(sustaininst.find(api->getChannelPreset(i))!=sustaininst.end())
		{
			if(!notestatus[i][j]&&spectra[i][j])
				spectra[i][j]=.95*spectra[i][j];
		}else if(spectra[i][j])spectra[i][j]=.95*spectra[i][j];
		if(spectrar[i][j]<spectra[i][j]*0.9)spectrar[i][j]+=spectra[i][j]*0.2;
		else spectrar[i][j]=spectra[i][j];
		if(spectrar[i][j])
		{
			smvec2d a((froffsets[12]*(j/12)+froffsets[j%12])*wwidth/2048.,spectrar[i][j]/-128.*(wheight-nh)*(1+0.02*sin(sm->smGetTime()*32))+wheight-nh);
			smvec2d b(a.x+notew*0.9,lpt+wheight-nh);
			double pb=((int)cpw[i]-8192)/8192.*cpbr[i];
			uint32_t newkey=j+(int)floor(pb);
			double fpb=pb-floor(pb);
			if(horizontal)
			{
				a=smvec2d(spectrar[i][j]/128.*(wwidth-nh)*(1+0.02*sin(sm->smGetTime()*32))+nh,(froffsets[12]*(j/12)+froffsets[j%12])*wheight/2048.);
				b=smvec2d(nh,a.y+notew*0.9);
				a.y=(froffsets[12]*(newkey/12)+froffsets[newkey%12])*wheight/2048.+notew*fpb;
				b.y=a.y+notew*0.9;
				a.y=wheight-a.y;b.y=wheight-b.y;
			}
			else
			{
				a.x=(froffsets[12]*(newkey/12)+froffsets[newkey%12])*wwidth/2048.+notew*fpb;
				b.x=a.x+notew*0.9;
			}
			nq.v[0].x=nq.v[3].x=a.x;nq.v[0].y=nq.v[1].y=a.y;
			nq.v[1].x=nq.v[2].x=b.x;nq.v[2].y=nq.v[3].y=b.y;for(int j=0;j<4;++j)
			nq.v[j].col=SETA(iccolors[i],204);
			sm->smRenderQuad(&nq);
		}
	}
}
bool qmpVisualization::update()
{
	smQuad q;
	if(!rendermode)
	{
		if(sm->smGetKeyState(SMK_RIGHT)==SMKST_HIT)
			api->playerSeek(api->getCurrentPlaybackPercentage()+(sm->smGetKeyState(SMK_SHIFT)?5:1));
		if(sm->smGetKeyState(SMK_LEFT)==SMKST_HIT)
			api->playerSeek(api->getCurrentPlaybackPercentage()-(sm->smGetKeyState(SMK_SHIFT)?5:1));
		if(sm->smGetKeyState(SMK_B)==SMKST_HIT)
			debug^=1;
	}
	if(playing)
	{
		if(rendermode)
		{
			ctk=1e6*(cfr)/tfps/ctp*api->getDivision()+lstk;
			++cfr;
		}
		else
			ctk=lstk+std::chrono::duration_cast<std::chrono::duration<double,std::micro>>(std::chrono::steady_clock::now()-lst).count()/api->getRawTempo()*api->getDivision();
	}
	if(!flat)
		updateVisualization3D();
	sm->smRenderBegin2D();
	sm->smClrscr(0xFF666666);q.blend=BLEND_ALPHABLEND;
	for(int i=0;i<4;++i){q.v[i].col=0xFFFFFFFF;q.v[i].z=0;}
	if(bgtex)
	{
		q.tex=bgtex;
		q.v[0].x=q.v[3].x=0;q.v[1].x=q.v[2].x=wwidth;
		q.v[0].y=q.v[1].y=0;q.v[2].y=q.v[3].y=wheight;
		q.v[0].tx=q.v[3].tx=0;q.v[1].tx=q.v[2].tx=1;
		q.v[0].ty=q.v[1].ty=0;q.v[2].ty=q.v[3].ty=1;
		sm->smRenderQuad(&q);
	}
	if(flat)
		updateVisualization2D();
	else
	{
		q.tex=sm->smTargetTexture(tdscn);
		q.v[0].tx=q.v[3].tx=0;q.v[1].tx=q.v[2].tx=1;
		q.v[0].ty=q.v[1].ty=0;q.v[2].ty=q.v[3].ty=1;
		q.v[0].x=q.v[1].x=0;q.v[2].x=q.v[3].x=wwidth;
		q.v[0].y=q.v[3].y=0;q.v[1].y=q.v[2].y=wheight;
		sm->smRenderQuad(&q);
		if(showparticle&&!horizontal)
		{
			q.tex=sm->smTargetTexture(tdparticles);
			sm->smRenderQuad(&q);
		}
	}
	if(osdpos==4){sm->smRenderEnd();return shouldclose;}
	uint32_t ltpc=~0u;
	for(uint32_t i=elb;i<pool.size();++i)
	{
		if(pool[i]->tcs>ctk)break;
		if(pool[i]->ch==998)cts=pool[i]->key;
		if(pool[i]->ch==997)cks=pool[i]->key;
		if(pool[i]->ch==996)
			ltpc=i;
		if(pool[i]->ch==995)cpbr[pool[i]->vel]=pool[i]->key;
		if(pool[i]->ch==994)cpw[pool[i]->vel]=pool[i]->key;
	}
	if(~ltpc&&ctp!=pool[ltpc]->key)
	{
		ctp=pool[ltpc]->key;
		if(rendermode)
		{
			lstk=pool[ltpc]->tcs;
			cfr=1;
		}
	}
	int t,r;t=cks;r=(int8_t)((t>>8)&0xFF)+7;t&=0xFF;
	std::wstring ts(t?minors:majors,2*r,2);
	int step=int(1.33*fontsize);
	int xp=(osdpos&1)?wwidth-step-1:1;
	int yp=osdpos<2?wheight-step*5-4:step+4;
	int align=osdpos&1?ALIGN_RIGHT:ALIGN_LEFT;
	font2.updateString(L"Title: %ls",api->getWTitle().c_str());
	font2.render(xp,yp,0.5,0xFFFFFFFF,align);
	font2.render(xp-1,yp-1,0.5,0xFF000000,align);
	font.updateString(L"Time Sig: %d/%d",cts>>8,1<<(cts&0xFF));
	font.render(xp,yp+=step,0.5,0xFFFFFFFF,align);
	font.render(xp-1,yp-1,0.5,0xFF000000,align);
	font.updateString(L"Key Sig: %ls",ts.c_str());
	font.render(xp,yp+=step,0.5,0xFFFFFFFF,align);
	font.render(xp-1,yp-1,0.5,0xFF000000,align);
	font.updateString(L"Tempo: %.2f",60./(ctp/1e6));
	font.render(xp,yp+=step,0.5,0xFFFFFFFF,align);
	font.render(xp-1,yp-1,0.5,0xFF000000,align);
	font.updateString(L"Current tick: %d",ctk);
	font.render(xp,yp+=step,0.5,0xFFFFFFFF,align);
	font.render(xp-1,yp-1,0.5,0xFF000000,align);
	if(!rendermode)
	{
		font.updateString(L"FPS: %.2f",sm->smGetFPS());
		font.render(xp,yp+=step,0.5,0xFFFFFFFF,align);
		font.render(xp-1,yp-1,0.5,0xFF000000,align);
	}
	if(debug)
	{
		int dosdpos=(osdpos+1)%4;
		xp=(dosdpos&1)?wwidth-step-1:1;
		yp=dosdpos<2?wheight-step*5-4:step+4;
		align=dosdpos&1?ALIGN_RIGHT:ALIGN_LEFT;
		std::string tstr;
		tstr=std::string(sm->smGetOSInfo());
		font.updateString(L"OS: %ls",std::wstring({std::begin(tstr),std::end(tstr)}).c_str());
		font.render(xp,yp,0.5,0xFFFFFFFF,align);
		font.render(xp-1,yp-1,0.5,0xFF000000,align);
		tstr=std::string(sm->smGetCPUModel());
		font.updateString(L"CPU: %ls",std::wstring({std::begin(tstr),std::end(tstr)}).c_str());
		font.render(xp,yp+=step,0.5,0xFFFFFFFF,align);
		font.render(xp-1,yp-1,0.5,0xFF000000,align);
		tstr=std::string(sm->smGetDispDriver());
		font.updateString(L"Display %ls",std::wstring({std::begin(tstr),std::end(tstr)}).c_str());
		font.render(xp,yp+=3*step,0.5,0xFFFFFFFF,align);
		font.render(xp-1,yp-1,0.5,0xFF000000,align);
	}
	sm->smRenderEnd();
	if(rendermode)
	{
		if(ctk>api->getMaxTick())
			framecb(nullptr,0,ctk,api->getMaxTick());
		else
		{
			sm->smPixelCopy(0,0,wwidth,wheight,4*wwidth*wheight,fbcont);
			framecb(fbcont,4*wwidth*wheight,ctk,api->getMaxTick());
		}
	}
	return shouldclose;
}

void qmpVisualization::drawCube(smvec3d a,smvec3d b,DWORD col,SMTEX tex,int faces)
{
	smQuad q;q.blend=BLEND_ALPHABLEND;
	q.tex=tex;for(int i=0;i<4;++i)q.v[i].col=col;
	if(noteappearance==1)
	{
		smMatrix I;I.loadIdentity();smEntity3D c=smEntity3D::cube(a,b,col,faces);
		nebuf->addTransformedEntity(&c,I,smvec3d(0,0,0));
	}
	else
	{
		q.v[0].x=a.x;q.v[0].y=b.y;q.v[0].z=a.z;
		q.v[1].x=b.x;q.v[1].y=b.y;q.v[1].z=a.z;
		q.v[2].x=b.x;q.v[2].y=b.y;q.v[2].z=b.z;
		q.v[3].x=a.x;q.v[3].y=b.y;q.v[3].z=b.z;
		sm->smRenderQuad(&q);
	}
}

qmpVisualization* qmpVisualization::inst=nullptr;

qmpVisualization::qmpVisualization(qmpPluginAPI* _api)
{
	api=_api;
	inst=this;
	rendermode=false;
	hidewindow=false;
}
qmpVisualization::~qmpVisualization()
{
	api=nullptr;
	inst=nullptr;
}
void qmpVisualization::init()
{
	h=new CMidiVisualHandler(this);
	closeh=new CloseHandler(this);
	rendererTh=nullptr;playing=false;
	memset(rpnid,0xFF,sizeof(rpnid));
	memset(rpnval,0xFF,sizeof(rpnval));
	memset(spectra,0,sizeof(spectra));
	memset(spectrar,0,sizeof(spectrar));
	api->registerFunctionality(this,"Visualization","Visualization",api->isDarkTheme()?":/img/visualization_i.svg":":/img/visualization.svg",0,true);
	uihb=api->registerUIHook("main.start",[this](const void*,void*){this->start();},nullptr);
	uihs=api->registerUIHook("main.stop",[this](const void*,void*){this->stop();},nullptr);
	uihp=api->registerUIHook("main.pause",[this](const void*,void*){this->pause();},nullptr);
	uihr=api->registerUIHook("main.reset",[this](const void*,void*){this->reset();},nullptr);
	uihk=api->registerUIHook("main.seek",[this](const void*,void*){
		cts=api->getTimeSig();
		cks=api->getKeySig();
		ctp=api->getRawTempo();
		for(int i=0;i<16;++i)
		api->getPitchBendRaw(i,&cpw[i],&cpbr[i]);
	},nullptr);
	herh=api->registerEventReadHandler(
		[this](const void *ee,void*){
			const SEvent* e=(const SEvent*)ee;
			switch(e->type&0xF0)
			{
				case 0x80:
					this->pushNoteOff(e->time,e->type&0x0F,e->p1);
				break;
				case 0x90:
					if(e->p2)
					this->pushNoteOn(e->time,e->type&0x0F,e->p1,e->p2);
					else
					this->pushNoteOff(e->time,e->type&0x0F,e->p1);
				break;
				case 0xB0:
					if(e->p1==100)rpnid[e->type&0x0F]=e->p2;
					if(e->p1==6)rpnval[e->type&0x0F]=e->p2;
					if(~rpnid[e->type&0x0F]&&~rpnval[e->type&0x0F])
					{
						if(rpnid[e->type&0x0F]==0)
							this->pool.push_back(new MidiVisualEvent{e->time,e->time,rpnval[e->type&0x0F],e->type&0x0Fu,995});
						rpnval[e->type&0x0F]=~0u;
					}
				break;
				case 0xE0:
					this->pool.push_back(new MidiVisualEvent{e->time,e->time,(e->p1|(e->p2<<7))&0x3FFFu,e->type&0x0Fu,994});
				break;
				case 0xF0:
					if(e->type==0xFF&&e->p1==0x58)
					{
						this->tspool.push_back(std::make_pair(e->time,(e->str[0]<<24)|(e->str[1]<<16)));
						this->pool.push_back(new MidiVisualEvent{e->time,e->time,(e->str[0]&0xffu)<<8|(e->str[1]&0xffu),0,998});
					}
					else if(e->type==0xFF&&e->p1==0x59)
						this->pool.push_back(new MidiVisualEvent{e->time,e->time,(e->str[0]&0xffu)<<8|(e->str[1]&0xffu),0,997});
					else if(e->type==0xFF&&e->p1==0x51)
						this->pool.push_back(new MidiVisualEvent{e->time,e->time,(e->str[0]&0xffu)<<16|(e->str[1]&0xffu)<<8|(e->str[2]&0xffu),0,996});
				break;
			}
		}
	,nullptr);
	heh=api->registerEventHandler(
		[this](const void*,void*){
			if(this->ctk>this->api->getCurrentTimeStamp()+this->api->getDivision()/3)
				this->elb=0;
			/*if(abs((int)this->ctk-(int)this->api->getCurrentTimeStamp())>this->api->getDivision()/4)
				fprintf(stderr,"Visualization: out of sync! %u vs %u ad: %u\n",this->ctk,this->api->getCurrentTimeStamp());*/
			this->ctk=this->api->getCurrentTimeStamp();
			this->lstk=this->ctk;
			this->lst=std::chrono::steady_clock::now();
		}
	,nullptr);
	hfrf=api->registerFileReadFinishHook(
		[this](const void*,void*){
			memset(rpnval,0xFF,sizeof(rpnval));
			memset(rpnid,0xFF,sizeof(rpnid));
			std::sort(this->tspool.begin(),this->tspool.end());
			for(uint32_t tk=0,n=4,s=0;tk<=this->api->getMaxTick();){
				while(tk<(s>=this->tspool.size()?this->api->getMaxTick():this->tspool[s].first)){
					this->pool.push_back(new MidiVisualEvent{tk,tk,0,0,999});
					tk+=n*this->api->getDivision();
				}
				tk=(s>=this->tspool.size()?this->api->getMaxTick():this->tspool[s].first);
				if(tk==this->api->getMaxTick()){
					this->pool.push_back(new MidiVisualEvent{tk,tk,0,0,999});
					++tk;break;
				}
				else n=this->tspool[s++].second>>24;
			}
			std::sort(this->pool.begin(),this->pool.end(),cmp);
		}
	,nullptr);
	api->registerOptionBool("Visualization-Appearance","Show Piano","Visualization/showpiano",true);
	api->registerOptionBool("Visualization-Appearance","3D Notes","Visualization/3dnotes",true);
	api->registerOptionBool("Visualization-Appearance","Arrange channels on a stair","Visualization/stairpiano",true);
	api->registerOptionBool("Visualization-Appearance","Show channel labels","Visualization/showlabel",true);
	api->registerOptionBool("Visualization-Appearance","Show Particles","Visualization/showparticle",false);
	api->registerOptionBool("Visualization-Appearance","Horizontal Visualization","Visualization/horizontal",false);
	api->registerOptionBool("Visualization-Appearance","2D Visualization","Visualization/flat",false);
	api->registerOptionBool("Visualization-Appearance","Show Measure Indicator","Visualization/showmeasure",true);
	api->registerOptionBool("Visualization-Appearance","Use spectrum instead of piano roll","Visualization/usespectrum",false);
	api->registerOptionBool("Visualization-Video","Enable VSync","Visualization/vsync",true);
	api->registerOptionBool("Visualization-Video","Save Viewport","Visualization/savevp",true);
	api->registerOptionInt("Visualization-Video","Window Width","Visualization/wwidth",320,3200,800);
	api->registerOptionInt("Visualization-Video","Window Height","Visualization/wheight",320,3200,600);
	api->registerOptionInt("Visualization-Video","Target FPS","Visualization/tfps",5,1000,60);
	api->registerOptionInt("Visualization-Video","Supersampling","Visualization/supersampling",1,16,0);
	api->registerOptionInt("Visualization-Video","Multisampling","Visualization/multisampling",0,16,0);
	api->registerOptionInt("Visualization-Video","FOV","Visualization/fov",30,180,60);
	std::vector<std::string> tv;tv.push_back("Bottom left");tv.push_back("Bottom right");tv.push_back("Top left");tv.push_back("Top right");tv.push_back("Hidden");
	api->registerOptionEnumInt("Visualization-Video","OSD Position","Visualization/osdpos",tv,0);
	api->registerOptionInt("Visualization-Video","Font Size","Visualization/fontsize",6,180,16);
	api->registerOptionString("Visualization-Video","Custom Sans Font","Visualization/font1","",true);
	api->registerOptionString("Visualization-Video","Custom Monospace Font","Visualization/font2","",true);
	api->registerOptionInt("Visualization-Appearance","View distance","Visualization/viewdist",20,1000,100);
	api->registerOptionInt("Visualization-Appearance","Note stretch","Visualization/notestretch",20,500,100);
	api->registerOptionInt("Visualization-Appearance","Minimum note length","Visualization/minnotelen",20,500,100);
	api->registerOptionUint("Visualization-Appearance","Chequer board tint (AARRGGBB)","Visualization/chkrtint",0,0xFFFFFFFF,0xFF999999);
	api->registerOptionString("Visualization-Appearance","Background Image","Visualization/background","",true);
	api->registerOptionDouble("","","Visualization/px",-999999999,999999999,-1e9);
	api->registerOptionDouble("","","Visualization/py",-999999999,999999999,-1e9);
	api->registerOptionDouble("","","Visualization/pz",-999999999,999999999,-1e9);
	api->registerOptionDouble("","","Visualization/rx",-999999999,999999999,-1e9);
	api->registerOptionDouble("","","Visualization/ry",-999999999,999999999,-1e9);
	api->registerOptionDouble("","","Visualization/rz",-999999999,999999999,-1e9);
	for(int i=0;i<16;++i)
	{
		api->registerOptionUint("","","Visualization/chActiveColor"+std::to_string(i),0,0xFFFFFFFF,accolors[i]);
		api->registerOptionUint("","","Visualization/chInactiveColor"+std::to_string(i),0,0xFFFFFFFF,iccolors[i]);
	}
	wwidth=api->getOptionInt("Visualization/wwidth");
	wheight=api->getOptionInt("Visualization/wheight");
	wsupersample=api->getOptionInt("Visualization/supersampling");
	wmultisample=api->getOptionInt("Visualization/multisampling");
	fov=api->getOptionInt("Visualization/fov");
	noteappearance=api->getOptionBool("Visualization/3dnotes");
	showpiano=api->getOptionBool("Visualization/showpiano");
	stairpiano=api->getOptionBool("Visualization/stairpiano");
	showlabel=api->getOptionBool("Visualization/showlabel");
	showparticle=api->getOptionBool("Visualization/showparticle");
	horizontal=api->getOptionBool("Visualization/horizontal");
	flat=api->getOptionBool("Visualization/flat");
	savevp=api->getOptionBool("Visualization/savevp");
	vsync=api->getOptionBool("Visualization/vsync");
	tfps=api->getOptionInt("Visualization/tfps");
	osdpos=api->getOptionEnumInt("Visualization/osdpos");
	fontsize=api->getOptionInt("Visualization/fontsize");
	viewdist=api->getOptionInt("Visualization/viewdist");
	notestretch=api->getOptionInt("Visualization/notestretch");
	minnotelength=api->getOptionInt("Visualization/minnotelen");
	chkrtint=api->getOptionUint("Visualization/chkrtint");
	for(int i=0;i<16;++i)
	{
		accolors[i]=api->getOptionUint("Visualization/chActiveColor"+std::to_string(i));
		iccolors[i]=api->getOptionUint("Visualization/chInactiveColor"+std::to_string(i));
	}
	if(savevp)
	{
		pos[0]=api->getOptionDouble("Visualization/px");
		pos[1]=api->getOptionDouble("Visualization/py");
		pos[2]=api->getOptionDouble("Visualization/pz");
		rot[0]=api->getOptionDouble("Visualization/rx");
		rot[1]=api->getOptionDouble("Visualization/ry");
		rot[2]=api->getOptionDouble("Visualization/rz");
	}
	memset(pss,0,sizeof(pss));
}
void qmpVisualization::deinit()
{
	if(!api)return;close();tspool.clear();
	for(unsigned i=0;i<pool.size();++i)delete pool[i];pool.clear();
	api->unregisterUIHook("main.start",uihb);
	api->unregisterUIHook("main.stop",uihs);
	api->unregisterUIHook("main.pause",uihp);
	api->unregisterUIHook("main.reset",uihr);
	api->unregisterFunctionality("Visualization");
	api->unregisterEventReadHandler(herh);
	api->unregisterEventHandler(heh);
	api->unregisterFileReadFinishHook(hfrf);
	delete h;delete closeh;
}
const char* qmpVisualization::pluginGetName()
{return "QMidiPlayer Default Visualization Plugin";}
const char* qmpVisualization::pluginGetVersion()
{return PLUGIN_VERSION;}

qmpVisualization *qmpVisualization::instance()
{return inst;}

void qmpVisualization::pushNoteOn(uint32_t tc,uint32_t ch,uint32_t key,uint32_t vel)
{
	pendingt[ch][key].push(tc);
	pendingv[ch][key].push(vel);
}
void qmpVisualization::pushNoteOff(uint32_t tc,uint32_t ch,uint32_t key)
{
	if(pendingt[ch][key].size()<1)return;
	MidiVisualEvent *ne=new MidiVisualEvent();
	ne->tcs=pendingt[ch][key].top();pendingt[ch][key].pop();
	ne->tce=tc;ne->ch=ch;ne->key=key;
	ne->vel=pendingv[ch][key].top();pendingv[ch][key].pop();
	pool.push_back(ne);
}
