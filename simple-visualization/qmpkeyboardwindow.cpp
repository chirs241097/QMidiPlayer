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
	api->registerEventHandlerIntf(ec=new EventCallback(),this);
	connect(ec,&EventCallback::keystateupdated,this,&qmpKeyboardWindow::onkeystatesupdate);
}
qmpKeyboardWindow::~qmpKeyboardWindow()
{
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

void EventCallback::callBack(void* callerdata,void* userdata)
{
	qmpKeyboardWindow *w=(qmpKeyboardWindow*)userdata;
	SEventCallBackData *cbd=(SEventCallBackData*)callerdata;
	if((cbd->type&0xF0)==0x80)
		emit keystateupdated(cbd->type&0xF,cbd->p1,false);
	if((cbd->type&0xF0)==0x90)
		emit keystateupdated(cbd->type&0xF,cbd->p1,cbd->p2>0);
}
