#include <cstdio>
#include <functional>
#include <set>
#include <QCheckBox>
#include <QPushButton>
#include <QComboBox>
#include <QTimer>
#include "qmpchannelswindow.hpp"
#include "ui_qmpchannelswindow.h"
#include "qmpmainwindow.hpp"

qmpChannelsModel::qmpChannelsModel(QObject*parent):QAbstractTableModel(parent)
{
	evh=qmpMainWindow::getInstance()->getPlayer()->registerEventHandler(
		[this](const void* _e,void*){
			if(!updatequeued)
			{
				updatequeued=true;
				const SEvent *e=(const SEvent*)(_e);
				if((e->p1&0xF0)==0xC0)
					emit dataChanged(index(e->p1&0xF0,4),index(e->p1&0xF0,4),{Qt::ItemDataRole::DisplayRole});
				QMetaObject::invokeMethod(this, &qmpChannelsModel::updateChannelActivity, Qt::ConnectionType::QueuedConnection);
			}
		}
	,nullptr);
	QTimer*t=new QTimer(this);
	t->setInterval(500);
	t->setSingleShot(false);
	connect(t,&QTimer::timeout,[this](){emit this->dataChanged(this->index(0,4),this->index(15,4),{Qt::ItemDataRole::DisplayRole});});
	memset(mute,0,sizeof(mute));
	memset(solo,0,sizeof(solo));
}
int qmpChannelsModel::columnCount(const QModelIndex&parent)const
{return parent.isValid()?0:6;}
int qmpChannelsModel::rowCount(const QModelIndex&parent)const
{return parent.isValid()?0:16;}
QModelIndex qmpChannelsModel::parent(const QModelIndex&child)const
{
	Q_UNUSED(child)
	return QModelIndex();
}
QVariant qmpChannelsModel::data(const QModelIndex&index,int role)const
{
	switch(index.column())
	{
		case 0:
			if(role==Qt::ItemDataRole::DecorationRole)
			{
				using namespace std::chrono_literals;
				bool lit=(std::chrono::system_clock::now()-qmpMainWindow::getInstance()->getPlayer()->getLastEventTS()[index.row()])<50ms;
				return lit?QIcon(":/img/ledon.svg"):QIcon(":/img/ledoff.svg");
			}
		break;
		case 1:
			if(role==Qt::ItemDataRole::CheckStateRole)
				return mute[index.row()]?Qt::CheckState::Checked:Qt::CheckState::Unchecked;
		break;
		case 2:
			if(role==Qt::ItemDataRole::CheckStateRole)
				return solo[index.row()]?Qt::CheckState::Checked:Qt::CheckState::Unchecked;
		break;
		case 3:
			if(role==Qt::ItemDataRole::DisplayRole)
			{
				std::vector<std::string> devs=qmpMainWindow::getInstance()->getPlayer()->getMidiOutDevices();
				return QString::fromStdString(devs[qmpMainWindow::getInstance()->getPlayer()->getChannelOutput(index.row())]);
			}
		break;
		case 4:
		{
			if(role==Qt::ItemDataRole::DisplayRole)
			{
				int ch=index.row();
				uint16_t b;uint8_t p;
				std::string nm;
				char data[256];
				CMidiPlayer *plyr=qmpMainWindow::getInstance()->getPlayer();
				bool r=plyr->getChannelOutputDevice(ch)->getChannelPreset(ch,&b,&p,nm);
				sprintf(data,"%03d:%03d %s",b,p,nm.c_str());
				if(!r)
				{
					nm=plyr->getChannelOutputDevice(ch)->getPresetName(plyr->getCC(ch,0)<<7|plyr->getCC(ch,32),plyr->getCC(ch,128));
					sprintf(data,"%03d:%03d:%03d %s",plyr->getCC(ch,0),plyr->getCC(ch,32),plyr->getCC(ch,128),nm.c_str());
				}
				return QString(data);
			}
		}
		break;
		case 5:
			if(role==Qt::ItemDataRole::DisplayRole)
				return "...";
			if(role==Qt::ItemDataRole::TextAlignmentRole)
				return Qt::AlignmentFlag::AlignCenter;
		break;
	}
	return QVariant();
}
bool qmpChannelsModel::setData(const QModelIndex&index,const QVariant&value,int role)
{
	if(index.column()==3)
	{
		if(role!=Qt::ItemDataRole::DisplayRole)return false;
		std::vector<std::string> dsv=CMidiPlayer::getInstance()->getMidiOutDevices();
		int idx=std::find(dsv.begin(),dsv.end(),value.toString().toStdString())-dsv.begin();
		if(idx==CMidiPlayer::getInstance()->getChannelOutput(index.row()))return false;
		CMidiPlayer::getInstance()->setChannelOutput(index.row(),idx);
		emit dataChanged(index,index,{Qt::DisplayRole});
		return true;
	}
	return false;
}
QVariant qmpChannelsModel::headerData(int section,Qt::Orientation orientation,int role)const
{
	if(role!=Qt::ItemDataRole::DisplayRole)return QVariant();
	if(orientation==Qt::Orientation::Vertical)
		return section+1;
	switch(section)
	{
		case 0:return QString("A");
		case 1:return QString("M");
		case 2:return QString("S");
		case 3:return QString("Device");
		case 4:return QString("Preset");
		case 5:return QString("...");
	}
	return QString();
}
Qt::ItemFlags qmpChannelsModel::flags(const QModelIndex&idx)const
{
	Qt::ItemFlags ret=Qt::ItemFlag::ItemIsEnabled|Qt::ItemFlag::ItemIsSelectable;
	if(idx.column()==1||idx.column()==2)
		ret|=Qt::ItemFlag::ItemIsUserCheckable;
	if(idx.column()==3)
		ret|=Qt::ItemFlag::ItemIsEditable;
	return ret;
}
void qmpChannelsModel::updateChannelActivity()
{
	emit dataChanged(index(0,0),index(15,0),{Qt::ItemDataRole::DecorationRole});
	updatequeued=false;
}
void qmpChannelsModel::channelMSClicked(const QModelIndex&idx)
{
	bool*x[3]={nullptr,mute,solo};
	if(x[idx.column()][idx.row()]^=1)
		x[3-idx.column()][idx.row()]=0;
	qmpMainWindow::getInstance()->getPlayer()->setMute(idx.row(),mute[idx.row()]);
	qmpMainWindow::getInstance()->getPlayer()->setSolo(idx.row(),solo[idx.row()]);
	emit dataChanged(index(idx.row(),1),index(idx.row(),2),{Qt::ItemDataRole::CheckStateRole});
}
void qmpChannelsModel::channelMSClearAll(int type)
{
	if(type==1)
	{
		memset(mute,0,sizeof(mute));
		for(int i=0;i<16;++i)
			qmpMainWindow::getInstance()->getPlayer()->setMute(i,0);
		emit dataChanged(index(0,1),index(15,1),{Qt::ItemDataRole::CheckStateRole});
	}
	if(type==2)
	{
		memset(solo,0,sizeof(solo));
		for(int i=0;i<16;++i)
			qmpMainWindow::getInstance()->getPlayer()->setSolo(i,0);
		emit dataChanged(index(0,2),index(15,2),{Qt::ItemDataRole::CheckStateRole});
	}
}

qmpDeviceItemDelegate::qmpDeviceItemDelegate(bool ignoreInternal,QWidget*parent):
	QStyledItemDelegate(parent),par(parent),nofs(ignoreInternal){}
void qmpDeviceItemDelegate::paint(QPainter*painter,const QStyleOptionViewItem&option,const QModelIndex&index)const
{
	QStyleOptionViewItem opt;
	initStyleOption(&opt,index);
	QStyleOptionComboBox socb;
	socb.currentText=opt.text;
	socb.editable=false;
	socb.rect=option.rect;
	socb.state=opt.state;
	par->style()->drawComplexControl(QStyle::ComplexControl::CC_ComboBox,&socb,painter);
	par->style()->drawControl(QStyle::CE_ComboBoxLabel,&socb,painter);
}
QSize qmpDeviceItemDelegate::sizeHint(const QStyleOptionViewItem&option,const QModelIndex&index)const
{
	QStyleOptionViewItem opt;
	initStyleOption(&opt,index);
	QStyleOptionComboBox socb;
	socb.currentText=opt.text;
	socb.editable=false;
	socb.rect=option.rect;
	QSize sz=par->fontMetrics().size(Qt::TextFlag::TextSingleLine,socb.currentText);
	return par->style()->sizeFromContents(QStyle::ContentsType::CT_ComboBox,&socb,sz);
}
QWidget* qmpDeviceItemDelegate::createEditor(QWidget*parent,const QStyleOptionViewItem&option,const QModelIndex&index)const
{
	Q_UNUSED(option)
	Q_UNUSED(index)
	QComboBox *cb=new QComboBox(parent);
	cb->setEditable(false);
	connect(cb,static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,[index,cb](int){
		const_cast<QAbstractItemModel*>(index.model())->setData(index,cb->currentText(),Qt::ItemDataRole::DisplayRole);
		cb->hidePopup();
	});
	return cb;
}
void qmpDeviceItemDelegate::setEditorData(QWidget*widget,const QModelIndex&index)const
{
	/*
	 * We want to quit editing as soon as the popup of the combobox is closed.
	 * Unfortunately QTableView does not do that. And I don't feel like sub-classing
	 * it. So here are some dirty tricks to make it work that way.
	 */
	QComboBox *cb=qobject_cast<QComboBox*>(widget);
	QSignalBlocker sblk(cb);
	cb->clear();
	std::vector<std::string> devs=qmpMainWindow::getInstance()->getPlayer()->getMidiOutDevices();
	for(auto s:devs)
		if(!nofs||(nofs&&s!="Internal FluidSynth"))
			cb->addItem(QString::fromStdString(s));
	cb->setCurrentText(index.data().toString());
	cb->showPopup();
}
void qmpDeviceItemDelegate::setModelData(QWidget*editor,QAbstractItemModel*model,const QModelIndex&index)const
{
	QComboBox *cb=qobject_cast<QComboBox*>(editor);
	model->setData(index,cb->currentText(),Qt::ItemDataRole::DisplayRole);
}
void qmpDeviceItemDelegate::updateEditorGeometry(QWidget*editor,const QStyleOptionViewItem&option,const QModelIndex&index)const
{
	Q_UNUSED(index)
	editor->setGeometry(option.rect);
}

qmpChannelsWindow::qmpChannelsWindow(QWidget *parent) :
	QWidget(parent,Qt::Dialog),
	ui(new Ui::qmpChannelsWindow)
{
	ui->setupUi(this);
	ui->tvChannels->setHorizontalHeader(new QHeaderView(Qt::Orientation::Horizontal));
	ui->tvChannels->setModel(chmodel=new qmpChannelsModel);
	ui->tvChannels->setItemDelegateForColumn(3,new qmpDeviceItemDelegate(false,ui->tvChannels));
	ui->tvChannels->setAlternatingRowColors(true);
	ui->tvChannels->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	ui->tvChannels->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
	ui->tvChannels->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeMode::Stretch);
	connect(ui->tvChannels,&QTableView::clicked,[this](const QModelIndex&idx){
		if(idx.column()==1||idx.column()==2)
			this->chmodel->channelMSClicked(idx);
		if(idx.column()==3)
			this->ui->tvChannels->edit(idx);
		if(idx.column()==5)
			this->showChannelEditorWindow(idx.row());
	});
	connect(ui->tvChannels,&QTableView::activated,[this](const QModelIndex&idx){
		if(idx.column()==4)
		{
			pselectw->show();
			pselectw->setupWindow(idx.row());
		}
	});
	pselectw=new qmpPresetSelector(this);
	ceditw=new qmpChannelEditor(this);
	cha=new QIcon(":/img/ledon.svg");chi=new QIcon(":/img/ledoff.svg");
	eh=qmpMainWindow::getInstance()->getPlayer()->registerEventHandler(
		[this](const void *ee,void*){
			const SEvent *e=(const SEvent*)ee;
			if((e->type&0xF0)==0x90&&e->p2>0&&(e->flags&0x01))
				emit this->noteOn();
		}
	,nullptr);
	std::vector<std::string> devs=qmpMainWindow::getInstance()->getPlayer()->getMidiOutDevices();
	size_t devc=devs.size();
	std::set<std::string> devset;
	for(auto dev:devs)devset.insert(dev);
	std::string selecteddev;
	for(auto setdev:qmpSettingsWindow::getSettingsIntf()->value("Midi/DevicePriority",QList<QVariant>{"Internal FluidSynth"}).toList())
		if(devset.find(setdev.toString().toStdString())!=devset.end())
		{
			selecteddev=setdev.toString().toStdString();
			break;
		}
	for(int ch=0;ch<16;++ch)
	{
		for(size_t j=0;j<devc;++j)
		{
			if(selecteddev==devs[j])
				qmpMainWindow::getInstance()->getPlayer()->setChannelOutput(ch,j);
		}
	}
	qmpMainWindow::getInstance()->registerFunctionality(
		chnlf=new qmpChannelFunc(this),
		std::string("Channel"),
		tr("Channel").toStdString(),
		getThemedIconc(":/img/channel.svg"),
		0,
		true
	);
	if(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/ChnlW",QRect(-999,-999,999,999)).toRect()!=QRect(-999,-999,999,999))
		setGeometry(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/ChnlW",QRect(-999,-999,999,999)).toRect());
	if(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/ChnlWShown",0).toInt())
	{show();qmpMainWindow::getInstance()->setFuncState("Channel",true);}
}

void qmpChannelsWindow::showEvent(QShowEvent *event)
{
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
	{
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/ChnlWShown",1);
	}
	if(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/ChnlW",QRect(-999,-999,999,999)).toRect()!=QRect(-999,-999,999,999))
		setGeometry(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/ChnlW",QRect(-999,-999,999,999)).toRect());
	event->accept();
}

void qmpChannelsWindow::closeEvent(QCloseEvent *event)
{
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
	{
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/ChnlW",geometry());
	}
	setVisible(false);
	if(!qmpMainWindow::getInstance()->isFinalizing()&&qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
	{
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/ChnlWShown",0);
	}
	qmpMainWindow::getInstance()->setFuncState("Channel",false);
	event->accept();
}

qmpChannelsWindow::~qmpChannelsWindow()
{
	qmpMainWindow::getInstance()->unregisterFunctionality("Channel");
	qmpMainWindow::getInstance()->getPlayer()->unregisterEventHandler(eh);
	delete chnlf;
	delete chi;delete cha;
	delete ui;
}

void qmpChannelsWindow::on_pbUnmute_clicked()
{
	chmodel->channelMSClearAll(1);
}

void qmpChannelsWindow::on_pbUnsolo_clicked()
{
	chmodel->channelMSClearAll(2);
}

void qmpChannelsWindow::showChannelEditorWindow(int chid)
{
	ceditw->show();
	ceditw->setupWindow(chid);
}

qmpChannelFunc::qmpChannelFunc(qmpChannelsWindow *par)
{p=par;}
void qmpChannelFunc::show()
{p->show();}
void qmpChannelFunc::close()
{p->close();}
