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
int horizontal=1,flat=0,osdpos=0,fontsize=16;
int fov=60,vsync=1,tfps=60,usespectrum=0;
DWORD chkrtint=0xFF999999;
const wchar_t* minors=L"abebbbf c g d a e b f#c#g#d#a#";
const wchar_t* majors=L"CbGbDbAbEbBbF C G D A E B F#C#";
double fpoffsets[]={1,18,28,50,55,82,98,109,130,137,161,164,191};
double froffsets[]={0,18,33,50,65,82,98,113,130,145,161,176,191};
DWORD iccolors[]={0XFFFF0000,0XFFFF8000,0XFFFFBF00,0XFFFFFF00,
				  0XFFBFFF00,0XFF80FF00,0XFF00FF00,0XFF00FFBF,
				  0XFF00FFFF,0XFF333333,0XFF00BFFF,0XFF007FFF,
				  0XFF0000FF,0XFF7F00FF,0XFFBF00FF,0XFFFF00BF};
DWORD accolors[]={0XFFFF9999,0XFFFFCC99,0XFFFFEE99,0XFFFFFF99,
				  0XFFEEFF99,0XFFCCFF99,0XFF99FF99,0XFF99FFCC,
				  0XFF99FFFF,0XFF999999,0XFF99EEFF,0XFF99CCFF,
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
void CReaderCallBack::callBack(void *callerdata,void *)
{
	SEventCallBackData* cbd=(SEventCallBackData*)callerdata;
	switch(cbd->type&0xF0)
	{
		case 0x80:
			par->pushNoteOff(cbd->time,cbd->type&0x0F,cbd->p1);
		break;
		case 0x90:
			par->pushNoteOn(cbd->time,cbd->type&0x0F,cbd->p1,cbd->p2);
		break;
	}
}
void CHandlerCallBack::callBack(void*,void*)
{
	if(par->ctk>par->api->getCurrentTimeStamp()+par->api->getDivision()/3)
		par->elb=0;
	par->ctk=par->api->getCurrentTimeStamp();
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
	sm->smVidMode(wwidth,wheight,true);
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
	if(noteappearance==1)nebuf=new smEntity3DBuffer();else nebuf=NULL;
	tdscn=sm->smTargetCreate(wwidth*wsupersample,wheight*wsupersample,wmultisample);
	tdparticles=sm->smTargetCreate(wwidth*wsupersample,wheight*wsupersample,wmultisample);
	if(!font.loadTTF("/usr/share/fonts/truetype/freefont/FreeMono.ttf",fontsize))
	if(!font.loadTTF("/usr/share/fonts/gnu-free-fonts/FreeMono.otf",fontsize))
	if(!font.loadTTF("C:/Windows/Fonts/cour.ttf",fontsize))
	printf("W: Font load failed.\n");
	if(!fonthdpi.loadTTF("/usr/share/fonts/truetype/freefont/FreeMono.ttf",180))
	if(!fonthdpi.loadTTF("/usr/share/fonts/gnu-free-fonts/FreeMono.otf",180))
	if(!fonthdpi.loadTTF("C:/Windows/Fonts/cour.ttf",180))
	printf("W: Font load failed.\n");
	if(!font2.loadTTF("/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",fontsize))
	if(!font2.loadTTF("/usr/share/fonts/wenquanyi/wqy-microhei/wqy-microhei.ttc",fontsize))
	if(!font2.loadTTF("C:/Windows/Fonts/segoeui.ttf",fontsize))
	printf("W: Font load failed.\n");
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
	ctk=0;
	if(savevp)
	{
		pos[0]=api->getOptionDouble("Visualization/px");
		pos[1]=api->getOptionDouble("Visualization/py");
		pos[2]=api->getOptionDouble("Visualization/pz");
		rot[0]=api->getOptionDouble("Visualization/rx");
		rot[1]=api->getOptionDouble("Visualization/ry");
		rot[2]=api->getOptionDouble("Visualization/rz");
	}
	sm->smMainLoop();
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
		rendererTh=NULL;
	}else return;

	if(showpiano&&!horizontal)for(int i=0;i<16;++i)delete p3d[i];
	if(showparticle&&!horizontal)for(int i=0;i>16;++i)for(int j=0;j<128;++j){delete pss[i][j];pss[i][j]=0;}
	if(noteappearance==1)delete nebuf;
	sm->smFinale();
	if(savevp)
	{
		api->setOptionDouble("Visualization/px",pos[0]);
		api->setOptionDouble("Visualization/py",pos[1]);
		api->setOptionDouble("Visualization/pz",pos[2]);
		api->setOptionDouble("Visualization/rx",rot[0]);
		api->setOptionDouble("Visualization/ry",rot[1]);
		api->setOptionDouble("Visualization/rz",rot[2]);
	}
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
	pool.clear();elb=ctk=0;
	for(int i=0;i<16;++i)for(int j=0;j<128;++j)
	{
		if(showparticle&&!horizontal&&pss[i][j])pss[i][j]->stopPS();
		while(!pendingt[i][j].empty())pendingt[i][j].pop();
		while(!pendingv[i][j].empty())pendingv[i][j].pop();
	}
}
void qmpVisualization::start(){playing=true;std::sort(pool.begin(),pool.end(),cmp);}
void qmpVisualization::stop(){playing=false;}
void qmpVisualization::pause(){playing=!playing;}
bool qmpVisualization::update()
{
	smQuad q;
	if(sm->smGetKeyState(SMK_RIGHT)==SMKST_HIT)
		api->playerSeek(api->getCurrentPlaybackPercentage()+(sm->smGetKeyState(SMK_SHIFT)?5:1));
	if(sm->smGetKeyState(SMK_LEFT)==SMKST_HIT)
		api->playerSeek(api->getCurrentPlaybackPercentage()-(sm->smGetKeyState(SMK_SHIFT)?5:1));
	if(!flat)
	{
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
		sm->smSetMouseGrab(false);
		if(sm->smGetKeyState(SMK_I))rot[1]+=1;
		if(sm->smGetKeyState(SMK_K))rot[1]-=1;
		if(sm->smGetKeyState(SMK_L))rot[0]+=1;
		if(sm->smGetKeyState(SMK_J))rot[0]-=1;
		if(sm->smGetKeyState(SMK_U))rot[2]+=1;
		if(sm->smGetKeyState(SMK_O))rot[2]-=1;
		//printf("pos: %f %f %f\n",pos[0],pos[1],pos[2]);
		//printf("rot: %f %f %f\n",rot[0],rot[1],rot[2]);
		double lpt=(double)notestretch/api->getDivision()/10.*(horizontal?0.25:1);
		memcpy(lastnotestatus,notestatus,sizeof(notestatus));
		memset(notestatus,0,sizeof(notestatus));
		for(uint32_t i=elb;i<pool.size();++i)
		{
			if(((double)pool[i]->tcs-ctk)*lpt>viewdist*2)break;
			if(fabs((double)pool[i]->tcs-ctk)*lpt<viewdist*2||fabs((double)pool[i]->tce-ctk)*lpt<viewdist*2)
			{
				if(api->getChannelMask(pool[i]->ch))continue;
				smvec3d a(0.63*((double)pool[i]->key-64)+.1,(stairpiano?(56-pool[i]->ch*7.):(64-pool[i]->ch*8.)),((double)pool[i]->tce-ctk)*lpt+(stairpiano&&showpiano&&!horizontal)*pool[i]->ch*2.);
				smvec3d b(0.63*((double)pool[i]->key-64)+.7,(stairpiano?(56-pool[i]->ch*7.):(64-pool[i]->ch*8.))+.4,((double)pool[i]->tcs-ctk)*lpt+(stairpiano&&showpiano&&!horizontal)*pool[i]->ch*2.);
				bool isnoteon=pool[i]->tcs<=ctk&&pool[i]->tce>=ctk;if(isnoteon)
				a.x=0.63*((double)pool[i]->key-64+api->getPitchBend(pool[i]->ch))+.1,
				b.x=0.63*((double)pool[i]->key-64+api->getPitchBend(pool[i]->ch))+.7;
				notestatus[pool[i]->ch][pool[i]->key]|=isnoteon;a.x*=1.2;b.x*=1.2;
				if(horizontal)
				{
					a=smvec3d(((double)pool[i]->tcs-ctk)*lpt-20,(16-pool[i]->ch*2.),0.63*((double)pool[i]->key-64)+.1);
					b=smvec3d(((double)pool[i]->tce-ctk)*lpt-20,(16-pool[i]->ch*2.)+.4,0.63*((double)pool[i]->key-64)+.7);
					if(isnoteon)
						a.z=0.63*((double)pool[i]->key-64+api->getPitchBend(pool[i]->ch))+.1,
						b.z=0.63*((double)pool[i]->key-64+api->getPitchBend(pool[i]->ch))+.7;
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
				{if(horizontal)a.x=((double)pool[i]->tcs-ctk)*lpt-minnotelength/100.-20;
				else a.z=((double)pool[i]->tcs-ctk)*lpt-minnotelength/100.+stairpiano*pool[i]->ch*2;}
				if(usespectrum)
				{
					if(notestatus[pool[i]->ch][pool[i]->key]&&!lastnotestatus[pool[i]->ch][pool[i]->key])
						spectra[pool[i]->ch][pool[i]->key]=pool[i]->vel*(api->getChannelCC(pool[i]->ch,7)/127.);
				}
				else
				drawCube(a,b,SETA(isnoteon?accolors[pool[i]->ch]:iccolors[pool[i]->ch],int(pool[i]->vel*1.6+(isnoteon?52:32))),0);
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
				smvec3d a(0.756*((double)j-64+api->getPitchBend(i))+.12,
						  (stairpiano?(56-i*7.):(64-i*8.)),
						  spectrar[i][j]*1.2*(1+0.02*sin(sm->smGetTime()*32))+(stairpiano&&showpiano&&!horizontal)*i*2.);
				smvec3d b(0.756*((double)j-64+api->getPitchBend(i))+.84,
						  (stairpiano?(56-i*7.):(64-i*8.))+.4,
						  (stairpiano&&showpiano&&!horizontal)*i*2.);
				drawCube(a,b,SETA(iccolors[i],204),0);
			}
		}
		if(noteappearance)nebuf->drawBatch();
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
			p3d[i]->render(smvec3d(0.756*api->getPitchBend(i),stairpiano?55-i*7:62-i*8,stairpiano*i*2));
			if(showlabel)
			{
				std::string s=api->getChannelPresetString(i);
				wchar_t ws[1024];mbstowcs(ws,s.c_str(),1024);
				fonthdpi.updateString(ws);
				fonthdpi.render(-49,stairpiano?56-i*7:63-i*8,stairpiano*i*2+0.1,0xFFFFFFFF,ALIGN_RIGHT,.008,0.01);
				fonthdpi.render(-49.05,stairpiano?56.05-i*7:63.05-i*8,stairpiano*i*2+0.2,0xFF000000,ALIGN_RIGHT,.008,0.01);
			}
		}
		if(playing)ctk+=(int)(1e6/(api->getRawTempo()/api->getDivision())*sm->smGetDelta());
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
	{
		double lpt=-(double)notestretch/api->getDivision()/2.;
		memset(notestatus,0,sizeof(notestatus));
		if(!horizontal)
		{
			double notew=wwidth/128,nh=showpiano?wwidth/2048.*172.:0;
			smQuad nq;nq.blend=BLEND_ALPHABLEND;nq.tex=0;
			for(int i=0;i<4;++i)nq.v[i].z=0,nq.v[i].tx=nq.v[i].ty=0;
			for(uint32_t i=elb;i<pool.size();++i)
			{
				if(((double)pool[i]->tcs-ctk)*lpt+wheight-nh<0)break;
				if(fabs((double)pool[i]->tcs-ctk)*lpt+wheight-nh>0||fabs((double)pool[i]->tce-ctk)*lpt+wheight-nh<wheight)
				{
					if(api->getChannelMask(pool[i]->ch))continue;
					smvec2d a((froffsets[12]*(pool[i]->key/12)+froffsets[pool[i]->key%12])*wwidth/2048.,((double)pool[i]->tce-ctk)*lpt+wheight-nh);
					smvec2d b(a.x+notew*0.9,((double)pool[i]->tcs-ctk)*lpt+wheight-nh);
					bool isnoteon=pool[i]->tcs<=ctk&&pool[i]->tce>=ctk;if(isnoteon)
					{
						uint32_t newkey=pool[i]->key+(int)floor(api->getPitchBend(pool[i]->ch));
						double fpb=api->getPitchBend(pool[i]->ch)-floor(api->getPitchBend(pool[i]->ch));
						a.x=(froffsets[12]*(newkey/12)+froffsets[newkey%12])*wwidth/2048.+notew*fpb;
						b.x=a.x+notew*0.9;
					}
					notestatus[pool[i]->ch][pool[i]->key]|=isnoteon;
					if(((double)pool[i]->tcs-pool[i]->tce)*lpt<minnotelength*0.04)
					a.y=((double)pool[i]->tcs-ctk)*lpt+wheight-nh-minnotelength*0.04;
					nq.v[0].x=nq.v[3].x=a.x;nq.v[0].y=nq.v[1].y=a.y;
					nq.v[1].x=nq.v[2].x=b.x;nq.v[2].y=nq.v[3].y=b.y;for(int j=0;j<4;++j)
					nq.v[j].col=SETA(isnoteon?accolors[pool[i]->ch]:iccolors[pool[i]->ch],int(pool[i]->vel*1.6+(isnoteon?52:32)));
					if(usespectrum)
					{
						if(notestatus[pool[i]->ch][pool[i]->key]&&!lastnotestatus[pool[i]->ch][pool[i]->key])
							spectra[pool[i]->ch][pool[i]->key]=pool[i]->vel*(api->getChannelCC(pool[i]->ch,7)/127.);
					}
					else sm->smRenderQuad(&nq);
				}
			}
			while(pool.size()&&elb<pool.size()&&fabs((double)pool[elb]->tce-ctk)*lpt+wheight-nh>wheight)++elb;
			q.tex=pianotex;
			q.v[0].ty=q.v[3].ty=0;q.v[1].ty=q.v[2].ty=172./256.;
			q.v[0].tx=q.v[1].tx=0;q.v[2].tx=q.v[3].tx=1.;
			q.v[0].x=q.v[1].x=0;q.v[2].x=q.v[3].x=wwidth;
			q.v[0].y=q.v[3].y=wheight-nh;q.v[1].y=q.v[2].y=wheight;
			sm->smRenderQuad(&q);
			for(int i=0,j;i<128;++i)
			{
				DWORD c=0;for(j=0;j<16;++j)if(notestatus[j][i]){c=SETA(iccolors[j],0xFF);break;}
				q.v[0].x=q.v[1].x=(fpoffsets[12]*(i/12)+fpoffsets[i%12])*wwidth/2048.;
				q.v[2].x=q.v[3].x=q.v[0].x;
				q.v[0].y=q.v[3].y=wheight-nh;q.v[1].y=q.v[2].y=wheight;
				if(!c)continue;for(int j=0;j<4;++j)q.v[j].col=c;
				switch(i%12)
				{
					case 1:case 3:case 6:case 8:case 10:
						q.v[1].y=q.v[2].y=wheight-nh+115*wwidth/2048.;
						q.v[2].x+=15.*wwidth/2048;q.v[3].x+=15.*wwidth/2048;
						q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-15/256.;
						q.v[0].tx=q.v[3].tx=1344/2048.;q.v[1].tx=q.v[2].tx=1459/2048.;
					break;
					case 0:
						q.v[2].x+=27.*wwidth/2048;q.v[3].x+=27.*wwidth/2048;
						q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-27/256.;
						q.v[0].tx=q.v[3].tx=0/2048.;q.v[1].tx=q.v[2].tx=172/2048.;
					break;
					case 2:
						q.v[2].x+=29.*wwidth/2048;q.v[3].x+=29.*wwidth/2048;
						q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-29/256.;
						q.v[0].tx=q.v[3].tx=192/2048.;q.v[1].tx=q.v[2].tx=364/2048.;
					break;
					case 4:
						q.v[2].x+=28.*wwidth/2048;q.v[3].x+=28.*wwidth/2048;
						q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-28/256.;
						q.v[0].tx=q.v[3].tx=384/2048.;q.v[1].tx=q.v[2].tx=556/2048.;
					break;
					case 5:
						q.v[2].x+=28.*wwidth/2048;q.v[3].x+=28.*wwidth/2048;
						q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-28/256.;
						q.v[0].tx=q.v[3].tx=576/2048.;q.v[1].tx=q.v[2].tx=748/2048.;
					break;
					case 7:
						q.v[2].x+=29.*wwidth/2048;q.v[3].x+=29.*wwidth/2048;
						q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-29/256.;
						q.v[0].tx=q.v[3].tx=768/2048.;q.v[1].tx=q.v[2].tx=940/2048.;
					break;
					case 9:
						q.v[2].x+=28.*wwidth/2048;q.v[3].x+=28.*wwidth/2048;
						q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-28/256.;
						q.v[0].tx=q.v[3].tx=960/2048.;q.v[1].tx=q.v[2].tx=1132/2048.;
					break;
					case 11:
						q.v[2].x+=28.*wwidth/2048;q.v[3].x+=28.*wwidth/2048;
						q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-28/256.;
						q.v[0].tx=q.v[3].tx=1152/2048.;q.v[1].tx=q.v[2].tx=1324/2048.;
					break;
				}
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
					uint32_t newkey=j+(int)floor(api->getPitchBend(i));
					double fpb=api->getPitchBend(i)-floor(api->getPitchBend(i));
					a.x=(froffsets[12]*(newkey/12)+froffsets[newkey%12])*wwidth/2048.+notew*fpb;
					b.x=a.x+notew*0.9;
					nq.v[0].x=nq.v[3].x=a.x;nq.v[0].y=nq.v[1].y=a.y;
					nq.v[1].x=nq.v[2].x=b.x;nq.v[2].y=nq.v[3].y=b.y;for(int j=0;j<4;++j)
					nq.v[j].col=SETA(iccolors[i],204);
					sm->smRenderQuad(&nq);
				}
			}
		}
		else
		{
			double notew=wheight/128,nh=showpiano?wheight/2048.*172.:0;lpt=-lpt;
			smQuad nq;nq.blend=BLEND_ALPHABLEND;nq.tex=0;
			for(int i=0;i<4;++i)nq.v[i].z=0,nq.v[i].tx=nq.v[i].ty=0;
			for(uint32_t i=elb;i<pool.size();++i)
			{
				if(((double)pool[i]->tcs-ctk)*lpt+nh>wwidth)break;
				if(fabs((double)pool[i]->tcs-ctk)*lpt+nh<wwidth||fabs((double)pool[i]->tce-ctk)*lpt+nh>0)
				{
					if(api->getChannelMask(pool[i]->ch))continue;
					smvec2d a(((double)pool[i]->tce-ctk)*lpt+nh,(froffsets[12]*(pool[i]->key/12)+froffsets[pool[i]->key%12])*wheight/2048.);
					smvec2d b(((double)pool[i]->tcs-ctk)*lpt+nh,a.y+notew*0.9);
					bool isnoteon=pool[i]->tcs<=ctk&&pool[i]->tce>=ctk;if(isnoteon)
					{
						uint32_t newkey=pool[i]->key+(int)floor(api->getPitchBend(pool[i]->ch));
						double fpb=api->getPitchBend(pool[i]->ch)-floor(api->getPitchBend(pool[i]->ch));
						a.y=(froffsets[12]*(newkey/12)+froffsets[newkey%12])*wheight/2048.+notew*fpb;
						b.y=a.y+notew*0.9;
					}
					a.y=wheight-a.y;b.y=wheight-b.y;
					notestatus[pool[i]->ch][pool[i]->key]|=isnoteon;
					if(((double)pool[i]->tce-pool[i]->tcs)*lpt<minnotelength*0.04)
					a.x=((double)pool[i]->tcs-ctk)*lpt+nh-minnotelength*0.04;
					nq.v[0].x=nq.v[3].x=a.x;nq.v[0].y=nq.v[1].y=a.y;
					nq.v[1].x=nq.v[2].x=b.x;nq.v[2].y=nq.v[3].y=b.y;for(int j=0;j<4;++j)
					nq.v[j].col=SETA(isnoteon?accolors[pool[i]->ch]:iccolors[pool[i]->ch],int(pool[i]->vel*1.6+(isnoteon?52:32)));
					if(usespectrum)
					{
						if(notestatus[pool[i]->ch][pool[i]->key]&&!lastnotestatus[pool[i]->ch][pool[i]->key])
							spectra[pool[i]->ch][pool[i]->key]=pool[i]->vel*(api->getChannelCC(pool[i]->ch,7)/127.);
					}else sm->smRenderQuad(&nq);
				}
			}
			while(pool.size()&&elb<pool.size()&&fabs((double)pool[elb]->tce-ctk)*lpt+nh<0)++elb;
			q.tex=pianotex;
			q.v[0].tx=q.v[3].tx=0;q.v[1].tx=q.v[2].tx=1;
			q.v[0].ty=q.v[1].ty=0;q.v[2].ty=q.v[3].ty=172./256.;
			q.v[0].x=q.v[1].x=nh;q.v[2].x=q.v[3].x=0;
			q.v[0].y=q.v[3].y=wheight;q.v[1].y=q.v[2].y=0;
			sm->smRenderQuad(&q);
			for(int i=0,j;i<128;++i)
			{
				DWORD c=0;for(j=0;j<16;++j)if(notestatus[j][i]){c=SETA(iccolors[j],0xFF);break;}
				q.v[0].y=q.v[1].y=(fpoffsets[12]*(i/12)+fpoffsets[i%12])*wheight/2048.;
				q.v[2].y=q.v[3].y=q.v[0].y;
				q.v[0].x=q.v[3].x=nh;q.v[1].x=q.v[2].x=0;
				if(!c)continue;for(int j=0;j<4;++j)q.v[j].col=c;
				switch(i%12)
				{
					case 1:case 3:case 6:case 8:case 10:
						q.v[1].x=q.v[2].x=nh-115*wheight/2048.;
						q.v[2].y+=15.*wheight/2048;q.v[3].y+=15.*wheight/2048;
						q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-15/256.;
						q.v[0].tx=q.v[3].tx=1344/2048.;q.v[1].tx=q.v[2].tx=1459/2048.;
					break;
					case 0:
						q.v[2].y+=27.*wheight/2048;q.v[3].y+=27.*wheight/2048;
						q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-27/256.;
						q.v[0].tx=q.v[3].tx=0/2048.;q.v[1].tx=q.v[2].tx=172/2048.;
					break;
					case 2:
						q.v[2].y+=29.*wheight/2048;q.v[3].y+=29.*wheight/2048;
						q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-29/256.;
						q.v[0].tx=q.v[3].tx=192/2048.;q.v[1].tx=q.v[2].tx=364/2048.;
					break;
					case 4:
						q.v[2].y+=28.*wheight/2048;q.v[3].y+=28.*wheight/2048;
						q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-28/256.;
						q.v[0].tx=q.v[3].tx=384/2048.;q.v[1].tx=q.v[2].tx=556/2048.;
					break;
					case 5:
						q.v[2].y+=28.*wheight/2048;q.v[3].y+=28.*wheight/2048;
						q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-28/256.;
						q.v[0].tx=q.v[3].tx=576/2048.;q.v[1].tx=q.v[2].tx=748/2048.;
					break;
					case 7:
						q.v[2].y+=29.*wheight/2048;q.v[3].y+=29.*wheight/2048;
						q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-29/256.;
						q.v[0].tx=q.v[3].tx=768/2048.;q.v[1].tx=q.v[2].tx=940/2048.;
					break;
					case 9:
						q.v[2].y+=28.*wheight/2048;q.v[3].y+=28.*wheight/2048;
						q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-28/256.;
						q.v[0].tx=q.v[3].tx=960/2048.;q.v[1].tx=q.v[2].tx=1132/2048.;
					break;
					case 11:
						q.v[2].y+=28.*wheight/2048;q.v[3].y+=28.*wheight/2048;
						q.v[0].ty=q.v[1].ty=1.;q.v[2].ty=q.v[3].ty=1.-28/256.;
						q.v[0].tx=q.v[3].tx=1152/2048.;q.v[1].tx=q.v[2].tx=1324/2048.;
					break;
				}
				for(int j=0;j<4;++j)q.v[j].y=wheight-q.v[j].y;
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
					smvec2d a(spectrar[i][j]/128.*(wwidth-nh)*(1+0.02*sin(sm->smGetTime()*32))+nh,(froffsets[12]*(j/12)+froffsets[j%12])*wheight/2048.);
					smvec2d b(nh,a.y+notew*0.9);
					uint32_t newkey=j+(int)floor(api->getPitchBend(i));
					double fpb=api->getPitchBend(pool[i]->ch)-floor(api->getPitchBend(pool[i]->ch));
					a.y=(froffsets[12]*(newkey/12)+froffsets[newkey%12])*wheight/2048.+notew*fpb;
					b.y=a.y+notew*0.9;
					a.y=wheight-a.y;b.y=wheight-b.y;
					nq.v[0].x=nq.v[3].x=a.x;nq.v[0].y=nq.v[1].y=a.y;
					nq.v[1].x=nq.v[2].x=b.x;nq.v[2].y=nq.v[3].y=b.y;for(int j=0;j<4;++j)
					nq.v[j].col=SETA(iccolors[i],204);
					sm->smRenderQuad(&nq);
				}
			}
		}
		if(playing)ctk+=(int)(1e6/(api->getRawTempo()/api->getDivision())*sm->smGetDelta());
	}
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
	int t,r;t=api->getKeySig();r=(int8_t)((t>>8)&0xFF)+7;t&=0xFF;
	std::wstring ts(t?minors:majors);ts=ts.substr(2*r,2);
	int step=int(1.25*fontsize),xp=(osdpos&1)?wwidth-step-1:1,yp=osdpos<2?wheight-step*5-4:step+4,align=osdpos&1?ALIGN_RIGHT:ALIGN_LEFT;
	font2.updateString(L"Title: %ls",api->getWTitle().c_str());
	font2.render(xp,yp,0.5,0xFFFFFFFF,align);
	font2.render(xp-1,yp-1,0.5,0xFF000000,align);
	font.updateString(L"Time Sig: %d/%d",api->getTimeSig()>>8,1<<(api->getTimeSig()&0xFF));
	font.render(xp,yp+=step,0.5,0xFFFFFFFF,align);
	font.render(xp-1,yp-1,0.5,0xFF000000,align);
	font.updateString(L"Key Sig: %ls",ts.c_str());
	font.render(xp,yp+=step,0.5,0xFFFFFFFF,align);
	font.render(xp-1,yp-1,0.5,0xFF000000,align);
	font.updateString(L"Tempo: %.2f",api->getRealTempo());
	font.render(xp,yp+=step,0.5,0xFFFFFFFF,align);
	font.render(xp-1,yp-1,0.5,0xFF000000,align);
	font.updateString(L"Current tick: %d",ctk);
	font.render(xp,yp+=step,0.5,0xFFFFFFFF,align);
	font.render(xp-1,yp-1,0.5,0xFF000000,align);
	font.updateString(L"FPS: %.2f",sm->smGetFPS());
	font.render(xp,yp+=step,0.5,0xFFFFFFFF,align);
	font.render(xp-1,yp-1,0.5,0xFF000000,align);
	sm->smRenderEnd();
	return shouldclose;
}

void qmpVisualization::drawCube(smvec3d a,smvec3d b,DWORD col,SMTEX tex)
{
	smQuad q;q.blend=BLEND_ALPHABLEND;
	q.tex=tex;for(int i=0;i<4;++i)q.v[i].col=col;
	if(noteappearance==1)
	{
		smMatrix I;I.loadIdentity();smEntity3D c=smEntity3D::cube(a,b,col);
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

qmpVisualization::qmpVisualization(qmpPluginAPI* _api){api=_api;}
qmpVisualization::~qmpVisualization(){api=NULL;}
void qmpVisualization::init()
{
	cb=new CReaderCallBack(this);
	hcb=new CHandlerCallBack(this);
	vi=new CDemoVisualization(this);
	h=new CMidiVisualHandler(this);
	closeh=new CloseHandler(this);
	rendererTh=NULL;playing=false;
	memset(spectra,0,sizeof(spectra));
	memset(spectrar,0,sizeof(spectrar));
	hvif=api->registerVisualizationIntf(vi);
	herif=api->registerEventReaderIntf(cb,NULL);
	hehif=api->registerEventHandlerIntf(hcb,NULL);
	api->registerOptionBool("Visualization-Appearance","Show Piano","Visualization/showpiano",true);
	api->registerOptionBool("Visualization-Appearance","3D Notes","Visualization/3dnotes",true);
	api->registerOptionBool("Visualization-Appearance","Arrange channels on a stair","Visualization/stairpiano",true);
	api->registerOptionBool("Visualization-Appearance","Show channel labels","Visualization/showlabel",true);
	api->registerOptionBool("Visualization-Appearance","Show Particles","Visualization/showparticle",true);
	api->registerOptionBool("Visualization-Appearance","Horizontal Visualization","Visualization/horizontal",false);
	api->registerOptionBool("Visualization-Appearance","2D Visualization","Visualization/flat",false);
	api->registerOptionBool("Visualization-Appearance","Use spectrum instead of piano roll","Visualization/usespectrum",false);
	api->registerOptionBool("Visualization-Video","Enable VSync","Visualization/vsync",true);
	api->registerOptionBool("Visualization-Video","Save Viewport","Visualization/savevp",true);
	api->registerOptionInt("Visualization-Video","Window Width","Visualization/wwidth",320,3200,800);
	api->registerOptionInt("Visualization-Video","Window Height","Visualization/wheight",320,3200,600);
	api->registerOptionInt("Visualization-Video","FPS","Visualization/tfps",5,1000,60);
	api->registerOptionInt("Visualization-Video","Supersampling","Visualization/supersampling",1,16,0);
	api->registerOptionInt("Visualization-Video","Multisampling","Visualization/multisampling",0,16,0);
	api->registerOptionInt("Visualization-Video","FOV","Visualization/fov",30,180,60);
	std::vector<std::string> tv;tv.push_back("Bottom left");tv.push_back("Bottom right");tv.push_back("Top left");tv.push_back("Top right");tv.push_back("Hidden");
	api->registerOptionEnumInt("Visualization-Video","OSD Position","Visualization/osdpos",tv,0);
	api->registerOptionInt("Visualization-Video","Font Size","Visualization/fontsize",6,180,16);
	api->registerOptionInt("Visualization-Appearance","View distance","Visualization/viewdist",20,1000,100);
	api->registerOptionInt("Visualization-Appearance","Note stretch","Visualization/notestretch",20,500,100);
	api->registerOptionInt("Visualization-Appearance","Minimum note length","Visualization/minnotelen",20,500,100);
	api->registerOptionUint("Visualization-Appearance","Chequer board tint (AARRGGBB)","Visualization/chkrtint",0,0xFFFFFFFF,0xFF999999);
	api->registerOptionString("Visualization-Appearance","Background Image","Visualization/background","");
	api->registerOptionDouble("","","Visualization/px",-999999999,999999999,0);
	api->registerOptionDouble("","","Visualization/py",-999999999,999999999,120);
	api->registerOptionDouble("","","Visualization/pz",-999999999,999999999,70);
	api->registerOptionDouble("","","Visualization/rx",-999999999,999999999,0);
	api->registerOptionDouble("","","Visualization/ry",-999999999,999999999,75);
	api->registerOptionDouble("","","Visualization/rz",-999999999,999999999,90);
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
	memset(pss,0,sizeof(pss));
}
void qmpVisualization::deinit()
{
	if(!api)return;close();
	for(unsigned i=0;i<pool.size();++i)delete pool[i];pool.clear();
	api->unregisterVisualizationIntf(hvif);
	api->unregisterEventReaderIntf(herif);
	api->unregisterEventHandlerIntf(hehif);
	delete cb;delete hcb;delete vi;
	delete h;delete closeh;
}
const char* qmpVisualization::pluginGetName()
{return "QMidiPlayer Default Visualization Plugin";}
const char* qmpVisualization::pluginGetVersion()
{return "0.8.3";}

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
	if(tc>fintk)fintk=tc;
}
