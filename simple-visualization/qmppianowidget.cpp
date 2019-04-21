#include <cstring>
#include <QPainter>
#include "qmppianowidget.hpp"

qmpPianoWidget::qmpPianoWidget(QWidget *parent) : QWidget(parent)
{
	memset(keystates,0,sizeof(keystates));
}
void qmpPianoWidget::setKeyState(int key,bool state)
{
	keystates[key]=state;
	update();
}
void qmpPianoWidget::reset()
{
	memset(keystates,0,sizeof(keystates));
	update();
}
QSize qmpPianoWidget::minimumSizeHint()const
{
	return QSize(320,22);
}

void qmpPianoWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event)
	for(int i=0;i<128;++i)
	{
		QRectF r=getKeyRect(i);
		paintKey(r,QColor(keystates[i]?0xff66cc:0x66ccff));
	}
}

QRectF qmpPianoWidget::getKeyRect(int key)
{
	int octave=key/12;key%=12;
	bool is_black=(key<5&&(key&1))||(key>5&&((key&1)^1));
	double key_width=width()/75.;
	QRectF ret(0,0,key_width,height()/2.);
	if(!is_black)
	{
		ret.moveTop(height()/2.);
		int shift=(key+(key>=5))>>1;
		ret.moveLeft((octave*7+shift)*key_width);
	}
	else
		ret.moveLeft((octave*7+(key+(key>=5))/2.)*key_width);
	return ret;
}
void qmpPianoWidget::paintKey(QRectF keyrect,QColor keycolor)
{
	QColor bordercolor(keycolor);
	if(keycolor.valueF()>0.5)
		bordercolor=bordercolor.darker(150);
	else
		bordercolor=bordercolor.lighter(150);
	QPainter *p=new QPainter(this);
	p->setPen(bordercolor);
	p->setBrush(QBrush(keycolor));
	p->drawRect(keyrect.adjusted(1,1,-1,-1));
	delete p;
}
