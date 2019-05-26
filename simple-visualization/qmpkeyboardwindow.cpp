#include <QVBoxLayout>
#include <QCloseEvent>
#include "qmpkeyboardwindow.hpp"

qmpKeyboardWindow::qmpKeyboardWindow(qmpPluginAPI *_api,QWidget *parent):
	QWidget(parent,Qt::Dialog),api(_api)
{
	setLayout(new QVBoxLayout());
	for(int ch=0;ch<16;++ch)
		layout()->addWidget(pw[ch]=new qmpPianoWidget(this));
	hide();
	eh=api->registerEventHandler(
		[this](const void* ee,void*){
			const SEvent *e=(const SEvent*)ee;
			if((e->type&0xF0)==0x80||((e->type&0xF0)==0x90&&e->p2==0))
				emit keystateupdated(e->type&0xF,e->p1,false);
			if((e->type&0xF0)==0x90&&e->p2>0)
				emit keystateupdated(e->type&0xF,e->p1,e->p2>0);
		}
	,nullptr);
	connect(this,&qmpKeyboardWindow::keystateupdated,this,&qmpKeyboardWindow::onkeystatesupdate);
}
qmpKeyboardWindow::~qmpKeyboardWindow()
{
	api->unregisterEventHandler(eh);
}
void qmpKeyboardWindow::closeEvent(QCloseEvent *event)
{
	api->setFuncState("Keyboard",false);
	event->accept();
}
void qmpKeyboardWindow::onkeystatesupdate(int ch,int key,bool state)
{pw[ch]->setKeyState(key,state);}
void qmpKeyboardWindow::resetAll()
{for(int ch=0;ch<16;++ch)pw[ch]->reset();}
