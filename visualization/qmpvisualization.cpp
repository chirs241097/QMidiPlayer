#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include "qmpvisualization.hpp"

const int viewdist=100;
const int notestretch=100;//length of quarter note
const int minnotelength=100;
const int noteappearance=1;
DWORD chcolors[]={0XFFFF0000,0XFFFF8000,0XFFFFBF00,0XFFFFFF00,
				  0XFFBFFF00,0XFF80FF00,0XFF00FF00,0XFF00FFBF,
				  0XFF00FFFF,0XFF333333,0XFF00BFFF,0XFF007FFF,
				  0XFF0000FF,0XFF7F00FF,0XFFBF00FF,0XFFFF00BF};

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
	sm=smGetInterface(SMELT_APILEVEL);
	sm->smVidMode(800,600,true);
	sm->smUpdateFunc(h);sm->smQuitFunc(closeh);
	sm->smWinTitle("QMidiPlayer Visualization");
	sm->smSetFPS(FPS_VSYNC);
	sm->smNoSuspend(true);
	sm->smInit();shouldclose=false;
	sm->smTextureOpt(TPOT_POT,TFLT_LINEAR);
	chequer=sm->smTextureLoad("chequerboard.png");
	tdscn=sm->smTargetCreate(800,600);
	if(!font.loadTTF("/usr/share/fonts/truetype/freefont/FreeMono.ttf",16))
	printf("W: Font load failed.\n");
	pos[0]=-0;pos[1]=70;pos[2]=20;
	rot[0]=0;rot[1]=90;rot[2]=90;ctk=0;
	sm->smMainLoop();
}
void qmpVisualization::show()
{
	rendererTh=new std::thread(&qmpVisualization::showThread,this);
}
void qmpVisualization::close()
{
	shouldclose=true;
	rendererTh->join();
	sm->smFinale();
	font.releaseTTF();
	sm->smTextureFree(chequer);
	sm->smTargetFree(tdscn);
	sm->smRelease();
}
void qmpVisualization::reset()
{
	for(unsigned i=0;i<pool.size();++i)delete pool[i];
	pool.clear();elb=ctk=0;
	for(int i=0;i<16;++i)for(int j=0;j<128;++j)
	{
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
	for(int i=0;i<4;++i)
	{q.v[i].col=0xFF999999;q.v[i].z=0;}
	q.tex=chequer;q.blend=BLEND_ALPHABLEND;
	q.v[0].x=q.v[3].x=-60;q.v[1].x=q.v[2].x=60;
	q.v[0].y=q.v[1].y=-60;q.v[2].y=q.v[3].y=60;
	q.v[0].tx=q.v[3].tx=0;q.v[1].tx=q.v[2].tx=15;
	q.v[0].ty=q.v[1].ty=0;q.v[2].ty=q.v[3].ty=15;
	sm->smRenderBegin3D(60,tdscn);
	sm->sm3DCamera6f2v(pos,rot);
	sm->smClrscr(0xFF666666);
	sm->smRenderQuad(&q);
	if(sm->smGetKeyState(SMK_D))pos[0]+=cos(smMath::deg2rad(rot[2]-90)),pos[1]+=sin(smMath::deg2rad(rot[2]-90));
	if(sm->smGetKeyState(SMK_A))pos[0]-=cos(smMath::deg2rad(rot[2]-90)),pos[1]-=sin(smMath::deg2rad(rot[2]-90));
	if(sm->smGetKeyState(SMK_S))pos[0]+=cos(smMath::deg2rad(rot[2])),pos[1]+=sin(smMath::deg2rad(rot[2]));
	if(sm->smGetKeyState(SMK_W))pos[0]-=cos(smMath::deg2rad(rot[2])),pos[1]-=sin(smMath::deg2rad(rot[2]));
	if(sm->smGetKeyState(SMK_Q))pos[2]+=1;
	if(sm->smGetKeyState(SMK_E))pos[2]-=1;
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
	double lpt=(double)notestretch/api->getDivision()/10.;
	for(uint32_t i=elb;i<pool.size();++i)
	{
		if(((double)pool[i]->tcs-ctk)*lpt>viewdist*2)break;
		if(fabs((double)pool[i]->tcs-ctk)*lpt<viewdist*2||fabs((double)pool[i]->tce-ctk)*lpt<viewdist*2)
		{
			smvec3d a(((double)pool[i]->key-64),pool[i]->ch*-2.,((double)pool[i]->tce-ctk)*lpt);
			smvec3d b(((double)pool[i]->key-64)+.9,pool[i]->ch*-2.+.6,((double)pool[i]->tcs-ctk)*lpt);
			if(((double)pool[i]->tce-pool[i]->tcs)*lpt<minnotelength/100.)a.z=((double)pool[i]->tcs-ctk)*lpt-minnotelength/100.;
			drawCube(a,b,SETA(chcolors[pool[i]->ch],pool[i]->vel),0);
		}
	}
	if(playing)ctk+=(int)(1e6/(api->getRawTempo()/api->getDivision())*sm->smGetDelta());
	while(pool.size()&&((double)ctk-pool[elb]->tce)*lpt>viewdist*2)++elb;
	//if(ctk>fintk)return true;
	sm->smRenderEnd();
	for(int i=0;i<4;++i){q.v[i].col=0xFFFFFFFF;q.v[i].z=0;}
	q.tex=sm->smTargetTexture(tdscn);
	sm->smRenderBegin2D();
	sm->smClrscr(0xFF000000);
	q.v[0].tx=q.v[3].tx=0;q.v[1].tx=q.v[2].tx=1;
	q.v[0].ty=q.v[1].ty=0;q.v[2].ty=q.v[3].ty=1;
	q.v[0].x=q.v[1].x=0;q.v[2].x=q.v[3].x=800;
	q.v[0].y=q.v[3].y=0;q.v[1].y=q.v[2].y=600;
	sm->smRenderQuad(&q);
	font.updateString(L"Tempo: %.2f",api->getRealTempo());
	font.render(1,556,0xFFFFFFFF,ALIGN_LEFT);
	font.render(0,555,0xFF000000,ALIGN_LEFT);
	font.updateString(L"Current tick: %d",ctk);
	font.render(1,576,0xFFFFFFFF,ALIGN_LEFT);
	font.render(0,575,0xFF000000,ALIGN_LEFT);
	font.updateString(L"FPS: %.2f",sm->smGetFPS());
	font.render(1,596,0xFFFFFFFF,ALIGN_LEFT);
	font.render(0,595,0xFF000000,ALIGN_LEFT);
	sm->smRenderEnd();
	return shouldclose;
}

void qmpVisualization::drawCube(smvec3d a,smvec3d b,DWORD col,SMTEX tex)
{
	smQuad q;q.blend=BLEND_ALPHABLEND;
	q.tex=tex;for(int i=0;i<4;++i)q.v[i].col=col;
	if(noteappearance==1)
	{
		//top
		q.v[0].x=a.x;q.v[0].y=a.y;q.v[0].z=a.z;
		q.v[1].x=b.x;q.v[1].y=a.y;q.v[1].z=a.z;
		q.v[2].x=b.x;q.v[2].y=b.y;q.v[2].z=a.z;
		q.v[3].x=a.x;q.v[3].y=b.y;q.v[3].z=a.z;
		sm->smRenderQuad(&q);
		//bottom
		q.v[0].x=a.x;q.v[0].y=a.y;q.v[0].z=b.z;
		q.v[1].x=b.x;q.v[1].y=a.y;q.v[1].z=b.z;
		q.v[2].x=b.x;q.v[2].y=b.y;q.v[2].z=b.z;
		q.v[3].x=a.x;q.v[3].y=b.y;q.v[3].z=b.z;
		sm->smRenderQuad(&q);
		//left
		q.v[0].x=a.x;q.v[0].y=b.y;q.v[0].z=a.z;
		q.v[1].x=a.x;q.v[1].y=b.y;q.v[1].z=b.z;
		q.v[2].x=a.x;q.v[2].y=a.y;q.v[2].z=b.z;
		q.v[3].x=a.x;q.v[3].y=a.y;q.v[3].z=a.z;
		sm->smRenderQuad(&q);
		//right
		q.v[0].x=b.x;q.v[0].y=b.y;q.v[0].z=a.z;
		q.v[1].x=b.x;q.v[1].y=b.y;q.v[1].z=b.z;
		q.v[2].x=b.x;q.v[2].y=a.y;q.v[2].z=b.z;
		q.v[3].x=b.x;q.v[3].y=a.y;q.v[3].z=a.z;
		sm->smRenderQuad(&q);
		//front
		q.v[0].x=a.x;q.v[0].y=b.y;q.v[0].z=a.z;
		q.v[1].x=b.x;q.v[1].y=b.y;q.v[1].z=a.z;
		q.v[2].x=b.x;q.v[2].y=b.y;q.v[2].z=b.z;
		q.v[3].x=a.x;q.v[3].y=b.y;q.v[3].z=b.z;
		sm->smRenderQuad(&q);
		//back
		q.v[0].x=a.x;q.v[0].y=a.y;q.v[0].z=a.z;
		q.v[1].x=b.x;q.v[1].y=a.y;q.v[1].z=a.z;
		q.v[2].x=b.x;q.v[2].y=a.y;q.v[2].z=b.z;
		q.v[3].x=a.x;q.v[3].y=a.y;q.v[3].z=b.z;
		sm->smRenderQuad(&q);
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
	closeh=new RefuseCloseHandler();
	api->registerVisualizationIntf(vi);
	api->registerEventReaderIntf(cb,NULL);
	api->registerEventHandlerIntf(hcb,NULL);
}
void qmpVisualization::deinit()
{
	close();
	delete cb;delete hcb;delete vi;
	delete h;delete closeh;
}
const char* qmpVisualization::pluginGetName()
{return "QMidiPlayer Default Visualization Plugin";}
const char* qmpVisualization::pluginGetVersion()
{return "0.7.8";}

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

//dummy implementations of the api...
uint32_t qmpPluginAPI::getDivision(){return 0;}
uint32_t qmpPluginAPI::getRawTempo(){return 0;}
double qmpPluginAPI::getRealTempo(){return 0;}
uint32_t qmpPluginAPI::getTimeSig(){return 0;}
int qmpPluginAPI::getKeySig(){return 0;}
uint32_t qmpPluginAPI::getNoteCount(){return 0;}
uint32_t qmpPluginAPI::getCurrentPolyphone(){return 0;}
uint32_t qmpPluginAPI::getMaxPolyphone(){return 0;}
uint32_t qmpPluginAPI::getCurrentTimeStamp(){return 0;}
int qmpPluginAPI::registerEventHandlerIntf(IMidiCallBack*,void*){return -1;}
void qmpPluginAPI::unregisterEventHandlerIntf(int){}
int qmpPluginAPI::registerEventReaderIntf(IMidiCallBack*,void*){return -1;}
void qmpPluginAPI::unregisterEventReaderIntf(int){}
int qmpPluginAPI::registerVisualizationIntf(qmpVisualizationIntf*){return 0;}
void qmpPluginAPI::unregisterVisualizationIntf(int){}
void qmpPluginAPI::registerOptionInt(std::string,std::string,int){}
int qmpPluginAPI::getOptionInt(std::string){return 0;}
void qmpPluginAPI::registerOptionDouble(std::string,std::string,double){}
double qmpPluginAPI::getOptionDouble(std::string){return 0;}
void qmpPluginAPI::registerOptionString(std::string,std::string,std::string){}
std::string qmpPluginAPI::getOptionString(std::string){return "";}
