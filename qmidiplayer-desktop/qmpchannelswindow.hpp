#ifndef QMPCHANNELSWINDOW_H
#define QMPCHANNELSWINDOW_H

#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QComboBox>
#include <QShowEvent>
#include <QCloseEvent>
#include <QMoveEvent>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include "qmppresetselect.hpp"
#include "qmpchanneleditor.hpp"
#include "../core/qmpmidiplay.hpp"
#include "../core/qmpmidioutrtmidi.hpp"

namespace Ui {
	class qmpChannelsWindow;
}

class QDCPushButton:public QPushButton
{
	Q_OBJECT
	private:
		int id;
	protected:
		void mousePressEvent(QMouseEvent *event){QPushButton::mousePressEvent(event);emit onClick(id);}
	public:
		QDCPushButton(QString s):QPushButton(s){id=-1;}
		void setID(int _id){id=_id;}
		QSize sizeHint()const{return QSize();}
	signals:
		void onClick(int id);
};

class QDCComboBox:public QComboBox
{
	Q_OBJECT
	private:
		int id;
	public:
		QDCComboBox():QComboBox(){id=-1;connect(this,SIGNAL(currentIndexChanged(int)),this,SLOT(indexChangedSlot(int)));}
		void setID(int _id){id=_id;}
		QSize sizeHint()const{return QSize();}
		QSize minimumSizeHint()const{return QSize();}
	signals:
		void onChange(int id,int idx);
	public slots:
		void indexChangedSlot(int idx){emit(onChange(id,idx));}
};

class qmpChannelsWindow;

class qmpChannelFunc:public qmpFuncBaseIntf
{
	private:
		qmpChannelsWindow *p;
	public:
		qmpChannelFunc(qmpChannelsWindow *par);
		void show();
		void close();
};

class qmpChannelsModel:public QAbstractTableModel
{
	Q_OBJECT
	public:
		explicit qmpChannelsModel(QObject*parent=nullptr);
		int columnCount(const QModelIndex&parent=QModelIndex())const override;
		int rowCount(const QModelIndex&parent=QModelIndex())const override;
		QModelIndex parent(const QModelIndex&child)const override;
		QVariant data(const QModelIndex&index,int role=Qt::ItemDataRole::DisplayRole)const override;
		QVariant headerData(int section,Qt::Orientation orientation,int role=Qt::ItemDataRole::DisplayRole)const override;
		Qt::ItemFlags flags(const QModelIndex&idx)const override;
	public slots:
		void updateChannelActivity();
		void channelMSClicked(const QModelIndex&idx);
		void channelMSClearAll(int type);
	private:
		int evh;
		bool updatequeued;
		bool mute[16],solo[16];
};

class qmpDeviceItemDelegate:public QStyledItemDelegate
{
	Q_OBJECT
	public:
		explicit qmpDeviceItemDelegate(QWidget*parent=nullptr);
		void paint(QPainter*painter,const QStyleOptionViewItem&option,const QModelIndex&index)const override;
		QSize sizeHint(const QStyleOptionViewItem&option,const QModelIndex&index)const override;
		QWidget* createEditor(QWidget*parent,const QStyleOptionViewItem&option,const QModelIndex&index)const override;
		void setEditorData(QWidget*editor,const QModelIndex&index)const override;
		void setModelData(QWidget*editor,QAbstractItemModel*model,const QModelIndex&index)const override;
		void updateEditorGeometry(QWidget*editor,const QStyleOptionViewItem&option,const QModelIndex&index)const override;
	private:
		QWidget *par;
};

class qmpChannelsWindow:public QWidget
{
	Q_OBJECT

	public:
		explicit qmpChannelsWindow(QWidget *parent=nullptr);
		~qmpChannelsWindow();
		void showEvent(QShowEvent *event);
		void closeEvent(QCloseEvent *event);
	public slots:
		void showChannelEditorWindow(int chid);
		void on_pbUnmute_clicked();
		void on_pbUnsolo_clicked();

	signals:
		void noteOn();

	private:
		Ui::qmpChannelsWindow *ui;
		qmpPresetSelector *pselectw;
		qmpChannelEditor *ceditw;
		qmpChannelsModel *chmodel;
		QIcon *cha,*chi;
		qmpChannelFunc *chnlf;
		int eh;
};

#endif // QMPCHANNELSWINDOW_H
