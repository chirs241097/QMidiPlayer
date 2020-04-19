#include <QLineEdit>
#include <QToolButton>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#include <QHeaderView>
#include <QCheckBox>
#include <set>
#include "qmpsettingswindow.hpp"
#include "qmpdeviceprioritydialog.hpp"
#include "ui_qmpsettingswindow.h"
#include "qmpmainwindow.hpp"

qmpSettingsWindow::qmpSettingsWindow(qmpSettings *qmpsettings,QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpSettingsWindow)
{
	ui->setupUi(this);customOptions.clear();customOptPages.clear();
	connect(this,&qmpSettingsWindow::dialogClosing,(qmpMainWindow*)parent,&qmpMainWindow::dialogClosed);
	settings=qmpsettings;
	cwt=new qmpCustomizeWindow(this);
	cwa=new qmpCustomizeWindow(this);
	dps=new qmpDevPropDialog(this);
	devpriod=new qmpDevicePriorityDialog(this);
}

qmpSettingsWindow::~qmpSettingsWindow()
{
	delete ui;
}

void qmpSettingsWindow::closeEvent(QCloseEvent *event)
{
	setVisible(false);
	loadOption();
	emit dialogClosing();
	event->accept();
}
void qmpSettingsWindow::hideEvent(QHideEvent *event)
{
	emit dialogClosing();
	event->accept();
}

void qmpSettingsWindow::on_buttonBox_accepted()
{
	saveOption();
	qmpMainWindow::getInstance()->setupWidget();
	emit dialogClosing();
}

void qmpSettingsWindow::on_buttonBox_rejected()
{
	loadOption();
	emit dialogClosing();
}

void qmpSettingsWindow::updatePluginList(qmpPluginManager *pmgr)
{
	std::vector<qmpPlugin> *plugins=pmgr->getPlugins();
	QVariant *data=static_cast<QVariant*>(settings->getOptionCustom("DisabledPlugins"));
	QList<QVariant> disabled_plugins_l=static_cast<QVariant*>(data)->toList();
	delete data;
	std::set<std::string> disabled_plugins_s;
	for(auto &i:disabled_plugins_l)
		disabled_plugins_s.insert(i.toString().toStdString());
	for(unsigned i=0;i<plugins->size();++i)
	{
		bool enabled=disabled_plugins_s.find(plugins->at(i).name)==disabled_plugins_s.end();
		plugins->at(i).enabled=enabled;
	}
}

void qmpSettingsWindow::postInit()
{
	setupWidgets();
	int sf=0;
	QVariant *data=static_cast<QVariant*>(settings->getOptionCustom("FluidSynth/SoundFonts"));
	for(auto i:data->toList())
		if(!i.toString().startsWith('#'))
		{
			sf=1;
			break;
		}
	delete data;
	std::string selecteddev;
	std::vector<std::string> devs=qmpMainWindow::getInstance()->getPlayer()->getMidiOutDevices();
	std::set<std::string> devset;
	for(auto dev:devs)devset.insert(dev);
	QVariant *devpriov=static_cast<QVariant*>(qmpMainWindow::getInstance()->getSettings()->getOptionCustom("Midi/DevicePriority"));
	QList<QVariant> devprio=devpriov->toList();
	delete devpriov;
	for(auto &setdev:devprio)
		if(devset.find(setdev.toString().toStdString())!=devset.end())
		{
			selecteddev=setdev.toString().toStdString();
			break;
		}
	if(selecteddev=="Internal FluidSynth"&&!sf)
	{
		if(QMessageBox::question(this,
		tr("No soundfont loaded"),
		tr("Internal fluidsynth is the only available MIDI output but it has no soundfont set. "
		   "Would you like to setup soundfonts now? You may have to reload the internal synth afterwards."))==QMessageBox::Yes)
		{
			show();
			ui->tabWidget->setCurrentWidget(qobject_cast<QWidget*>(pageForTab("SoundFonts")->parent()));
		}
	}
}

void qmpSettingsWindow::registerCustomizeWidgetOptions()
{
	QPushButton *pbCustomizeToolbar=new QPushButton(tr("Customize..."));
	QPushButton *pbCustomizeActions=new QPushButton(tr("Customize..."));
	QVariant toolbar_def_val=QList<QVariant>({"Channel","Playlist","Effects","Visualization"});
	QVariant actions_def_val=QList<QVariant>({"FileInfo","Render","Panic","ReloadSynth"});
	settings->registerOptionCustom("Behavior","Customize toolbar","Behavior/Toolbar",pbCustomizeToolbar,&toolbar_def_val,std::bind(&qmpCustomizeWindow::save,cwt),std::bind(&qmpCustomizeWindow::load,cwt,std::placeholders::_1));
	settings->registerOptionCustom("Behavior","Customize actions","Behavior/Actions",pbCustomizeActions,&actions_def_val,std::bind(&qmpCustomizeWindow::save,cwa),std::bind(&qmpCustomizeWindow::load,cwa,std::placeholders::_1));
	connect(pbCustomizeToolbar,&QPushButton::clicked,[this]{loadOption("Behavior/Toolbar");cwt->show();});
	connect(pbCustomizeActions,&QPushButton::clicked,[this]{loadOption("Behavior/Actions");cwa->show();});
	connect(cwt,&QDialog::accepted,[this]{saveOption("Behavior/Toolbar");qmpMainWindow::getInstance()->setupWidget();});
	connect(cwa,&QDialog::accepted,[this]{saveOption("Behavior/Actions");qmpMainWindow::getInstance()->setupWidget();});
	connect(cwt,&QDialog::rejected,[this]{loadOption("Behavior/Toolbar");});
	connect(cwa,&QDialog::rejected,[this]{loadOption("Behavior/Actions");});
	qmpMainWindow::getInstance()->setupWidget();
}

void qmpSettingsWindow::registerSoundFontOption()
{
	QWidget *sfpanel=new QWidget();
	sfpanel->setLayout(new QVBoxLayout);
	sfpanel->layout()->setMargin(0);
	QTableWidget *twsf=new QTableWidget();
	twsf->setColumnCount(2);
	twsf->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
	twsf->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	twsf->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	twsf->setHorizontalHeaderLabels({tr("E"),tr("Path")});
	twsf->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
	twsf->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
	sfpanel->layout()->addWidget(twsf);
	QWidget *controls=new QWidget();
	controls->setLayout(new QHBoxLayout);
	controls->layout()->setMargin(0);
	QPushButton *pbsfadd=new QPushButton(style()->standardIcon(QStyle::StandardPixmap::SP_DialogOpenButton),QString());
	QPushButton *pbsfrem=new QPushButton(style()->standardIcon(QStyle::StandardPixmap::SP_DialogDiscardButton),QString());
	QPushButton *pbsfmup=new QPushButton(style()->standardIcon(QStyle::StandardPixmap::SP_ArrowUp),QString());
	QPushButton *pbsfmdn=new QPushButton(style()->standardIcon(QStyle::StandardPixmap::SP_ArrowDown),QString());
	controls->layout()->addWidget(pbsfadd);
	controls->layout()->addWidget(pbsfrem);
	controls->layout()->addWidget(pbsfmup);
	controls->layout()->addWidget(pbsfmdn);
	sfpanel->layout()->addWidget(controls);

	connect(pbsfadd,&QPushButton::clicked,[twsf,this]{
		QStringList sl=QFileDialog::getOpenFileNames(this,"Add File","","SoundFont files (*.sf2)");
		for(int i=0;i<sl.size();++i){
			twsf->insertRow(twsf->rowCount());
			QTableWidgetItem *sfn,*sfe;
			twsf->setItem(twsf->rowCount()-1,1,sfn=new QTableWidgetItem(sl[i]));
			twsf->setItem(twsf->rowCount()-1,0,sfe=new QTableWidgetItem());
			sfe->setCheckState(Qt::CheckState::Unchecked);
			sfn->setFlags(Qt::ItemFlag::ItemIsEnabled|Qt::ItemFlag::ItemIsSelectable);
			sfe->setFlags(Qt::ItemFlag::ItemIsEnabled|Qt::ItemFlag::ItemIsSelectable|Qt::ItemFlag::ItemIsUserCheckable);
		}
	});
	connect(pbsfrem,&QPushButton::clicked,[twsf]{
		QList<QTableWidgetItem*> sl=twsf->selectedItems();
		for(int i=0;i<sl.size();++i)
			twsf->removeRow(twsf->row(sl[i]));
	});
	connect(pbsfmup,&QPushButton::clicked,[twsf]{
		int cid=twsf->currentRow();if(!cid)return;
		QTableWidgetItem *ci=twsf->takeItem(cid,1);
		QTableWidgetItem *ce=twsf->takeItem(cid,0);
		twsf->removeRow(cid);
		twsf->insertRow(cid-1);
		twsf->setItem(cid-1,0,ce);
		twsf->setItem(cid-1,1,ci);
		twsf->setCurrentCell(cid-1,1);
	});
	connect(pbsfmdn,&QPushButton::clicked,[twsf]{
		int cid=twsf->currentRow();if(cid==twsf->rowCount()-1)return;
		QTableWidgetItem *ci=twsf->takeItem(cid,1);
		QTableWidgetItem *ce=twsf->takeItem(cid,0);
		twsf->removeRow(cid);
		twsf->insertRow(cid+1);
		twsf->setItem(cid+1,0,ce);
		twsf->setItem(cid+1,1,ci);
		twsf->setCurrentCell(cid+1,1);
	});

	QVariant sf_def_val=QList<QVariant>();
	auto save_func=[twsf]()->void*{
		QList<QVariant> sflist;
		for(int i=0;i<twsf->rowCount();++i)
		{
			QString sfs=twsf->item(i,1)->text();
			if(twsf->item(i,0)->checkState()==Qt::CheckState::Unchecked)
				sfs="#"+sfs;
			sflist.push_back(sfs);
		}
		return new QVariant(sflist);
	};
	auto load_func=[twsf](void* data){
		QList<QVariant> sflist=static_cast<QVariant*>(data)->toList();
		twsf->clearContents();
		twsf->setRowCount(0);
		for(int i=0;i<sflist.size();++i)
		{
			twsf->insertRow(i);
			QTableWidgetItem *sfn,*sfe;
			QString sf=sflist[i].toString();
			bool enabled=!sf.startsWith('#');
			if(!enabled)sf=sf.mid(1);
			twsf->setItem(i,1,sfn=new QTableWidgetItem(sf));
			twsf->setItem(i,0,sfe=new QTableWidgetItem());
			sfn->setFlags(Qt::ItemFlag::ItemIsEnabled|Qt::ItemFlag::ItemIsSelectable);
			sfe->setFlags(Qt::ItemFlag::ItemIsEnabled|Qt::ItemFlag::ItemIsSelectable|Qt::ItemFlag::ItemIsUserCheckable);
			sfe->setCheckState(enabled?Qt::CheckState::Checked:Qt::CheckState::Unchecked);
		}
	};
	settings->registerOptionCustom("SoundFonts","","FluidSynth/SoundFonts",sfpanel,&sf_def_val,save_func,load_func);
}

void qmpSettingsWindow::registerPluginOption(qmpPluginManager *pmgr)
{
	QTableWidget *twplugins=new QTableWidget();
	twplugins->setColumnCount(4);
	twplugins->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
	twplugins->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	twplugins->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	twplugins->setHorizontalHeaderLabels({tr("E"),tr("Plugin Name"),tr("Version"),tr("Path")});
	twplugins->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
	twplugins->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
	QVariant ep_def_val=QList<QVariant>();
	auto save_func=[twplugins,this]()->void*{
		QVariant *data=static_cast<QVariant*>(settings->getOptionCustom("DisabledPlugins"));
		QList<QVariant> disabled_plugins_ol=static_cast<QVariant*>(data)->toList();
		delete data;
		std::set<std::string> disabled_plugins_s;
		for(auto &i:disabled_plugins_ol)
			disabled_plugins_s.insert(i.toString().toStdString());
		for(int i=0;i<twplugins->rowCount();++i)
		{
			QString pn=twplugins->item(i,1)->text();
			if(twplugins->item(i,0)->checkState()==Qt::CheckState::Unchecked)
				disabled_plugins_s.insert(pn.toStdString());
			else
				disabled_plugins_s.erase(pn.toStdString());
		}
		QList<QVariant> disabled_plugins;
		for(auto &i:disabled_plugins_s)
			disabled_plugins.push_back(QString(i.c_str()));
		return new QVariant(disabled_plugins);
	};
	auto load_func=[twplugins,pmgr](void* data){
		QList<QVariant> disabled_plugins_l=static_cast<QVariant*>(data)->toList();
		std::set<std::string> disabled_plugins;
		for(auto i:disabled_plugins_l)
			disabled_plugins.insert(i.toString().toStdString());

		twplugins->clearContents();
		twplugins->setRowCount(0);

		std::vector<qmpPlugin> *plugins=pmgr->getPlugins();
		for(int i=0;static_cast<size_t>(i)<plugins->size();++i)
		{
			twplugins->insertRow(i);
			qmpPlugin &p=plugins->at(static_cast<size_t>(i));
			QTableWidgetItem *icb;
			twplugins->setItem(i,0,icb=new QTableWidgetItem());
			bool enabled=disabled_plugins.find(p.name)==disabled_plugins.end();
			icb->setCheckState(enabled?Qt::CheckState::Checked:Qt::CheckState::Unchecked);
			icb->setFlags(Qt::ItemFlag::ItemIsEnabled|Qt::ItemFlag::ItemIsSelectable|Qt::ItemFlag::ItemIsUserCheckable);
			twplugins->setItem(i,1,new QTableWidgetItem(p.name.c_str()));
			twplugins->setItem(i,2,new QTableWidgetItem(p.version.c_str()));
			twplugins->setItem(i,3,new QTableWidgetItem(p.path.c_str()));
			for(int j=1;j<=3;++j)
			twplugins->item(i,j)->setFlags(Qt::ItemFlag::ItemIsEnabled|Qt::ItemFlag::ItemIsSelectable);
		}
	};
	settings->registerOptionCustom("Plugins","","DisabledPlugins",twplugins,&ep_def_val,save_func,load_func);
}

void qmpSettingsWindow::registerExtraMidiOptions()
{
	QPushButton *pbDevPrio=new QPushButton("...");
	connect(pbDevPrio,&QPushButton::clicked,[this]{loadOption("Midi/DevicePriority");devpriod->show();});
	connect(devpriod,&QDialog::accepted,[this]{saveOption("Midi/DevicePriority");});
	connect(devpriod,&QDialog::rejected,[this]{loadOption("Midi/DevicePriority");});
	QVariant devprio_def_val=QList<QVariant>({"Internal FluidSynth"});
	settings->registerOptionCustom("MIDI","Select MIDI output devices","Midi/DevicePriority",pbDevPrio,&devprio_def_val,std::bind(&qmpDevicePriorityDialog::save,devpriod),std::bind(&qmpDevicePriorityDialog::load,devpriod,std::placeholders::_1));

	QPushButton *pbDevProp=new QPushButton("...");
	connect(pbDevProp,&QPushButton::clicked,[this]{loadOption("Midi/DeviceInitializationFiles");dps->show();});
	connect(dps,&QDialog::accepted,[this]{saveOption("Midi/DeviceInitializationFiles");});
	connect(dps,&QDialog::rejected,[this]{loadOption("Midi/DeviceInitializationFiles");});
	QVariant devprop_def_val=QList<QVariant>({});
	settings->registerOptionCustom("MIDI","External MIDI device setup","Midi/DeviceInitializationFiles",pbDevProp,&devprop_def_val,std::bind(&qmpDevPropDialog::save,dps),std::bind(&qmpDevPropDialog::load,dps,std::placeholders::_1));
}

void qmpSettingsWindow::saveOption(std::string key)
{
	auto save_opt=[this](std::string& key)->QVariant
	{
		qmpOption &o=settings->options[key];
		QVariant ret;
		switch(o.type)
		{
			case qmpOption::ParameterType::parameter_int:
			{
				QSpinBox *sb=qobject_cast<QSpinBox*>(o.widget);
				if(sb)
					ret=sb->value();
			}
			break;
			case qmpOption::ParameterType::parameter_uint:
			{
				QHexSpinBox *sb=qobject_cast<QHexSpinBox*>(o.widget);
				if(sb)
				{
					int val=sb->value();
					ret=reinterpret_cast<unsigned&>(val);
				}
			}
			break;
			case qmpOption::ParameterType::parameter_bool:
			{
				QCheckBox *cb=qobject_cast<QCheckBox*>(o.widget);
				if(cb)
					ret=cb->isChecked();
			}
			break;
			case qmpOption::ParameterType::parameter_double:
			{
				QDoubleSpinBox *sb=qobject_cast<QDoubleSpinBox*>(o.widget);
				if(sb)
					ret=sb->value();
			}
			break;
			case qmpOption::ParameterType::parameter_str:
			{
				QLineEdit *le=qobject_cast<QLineEdit*>(o.widget);
				if(le)
					ret=le->text();
			}
			break;
			case qmpOption::ParameterType::parameter_enum:
			{
				QComboBox *cb=qobject_cast<QComboBox*>(o.widget);
				if(cb)
					ret=cb->currentText();
			}
			break;
			case qmpOption::ParameterType::parameter_url:
			{
				QFileEdit *fe=qobject_cast<QFileEdit*>(o.widget);
				if(fe)
					ret=fe->text();
			}
			break;
			default:
				if(o.save_func)
				{
					QVariant* var=static_cast<QVariant*>(o.save_func());
					ret=QVariant(*var);
					delete var;
				}
			break;
		}
		return ret;
	};
	if(key.length())
	{
		QVariant r=save_opt(key);
		if(r.isValid())
			settings->settings->setValue(QString(key.c_str()),r);
	}
	else for(std::string& key:settings->optionlist)
	{
		QVariant r=save_opt(key);
		if(r.isValid())
			settings->settings->setValue(QString(key.c_str()),r);
	}
	settings->settings->sync();
}

void qmpSettingsWindow::loadOption(std::string key)
{
	auto load_opt=[this](std::string& key)
	{
		qmpOption &o=settings->options[key];
		switch(o.type)
		{
			case qmpOption::ParameterType::parameter_int:
			{
				QSpinBox *sb=qobject_cast<QSpinBox*>(o.widget);
				if(sb)
					sb->setValue(settings->getOptionInt(key));
			}
			break;
			case qmpOption::ParameterType::parameter_uint:
			{
				QHexSpinBox *sb=qobject_cast<QHexSpinBox*>(o.widget);
				if(sb)
					sb->setValue(settings->getOptionUint(key));
			}
			break;
			case qmpOption::ParameterType::parameter_bool:
			{
				QCheckBox *cb=qobject_cast<QCheckBox*>(o.widget);
				if(cb)
					cb->setChecked(settings->getOptionBool(key));
			}
			break;
			case qmpOption::ParameterType::parameter_double:
			{
				QDoubleSpinBox *sb=qobject_cast<QDoubleSpinBox*>(o.widget);
				if(sb)
					sb->setValue(settings->getOptionDouble(key));
			}
			break;
			case qmpOption::ParameterType::parameter_str:
			{
				QLineEdit *le=qobject_cast<QLineEdit*>(o.widget);
				if(le)
					le->setText(QString(settings->getOptionString(key).c_str()));
			}
			break;
			case qmpOption::ParameterType::parameter_enum:
			{
				QComboBox *cb=qobject_cast<QComboBox*>(o.widget);
				if(cb)
					cb->setCurrentIndex(settings->getOptionEnumInt(key));
			}
			break;
			case qmpOption::ParameterType::parameter_url:
			{
				QFileEdit *fe=qobject_cast<QFileEdit*>(o.widget);
				if(fe)
					fe->setText(QString(settings->getOptionString(key).c_str()));
			}
			break;
			default:
				if(o.load_func)
				{
					void *var=settings->getOptionCustom(key);
					o.load_func(var);
					delete static_cast<QVariant*>(var);
				}
			break;
		}
	};
	if(key.length())load_opt(key);
	else for(std::string& key:settings->optionlist)
		load_opt(key);
}

void qmpSettingsWindow::setupWidgets()
{
	for(std::string& key:settings->optionlist)
	{
		if(!settings->options[key].desc.length()&&settings->options[key].type!=qmpOption::ParameterType::parameter_custom)
			continue;
		QWidget *optw=nullptr;
		qmpOption &o=settings->options[key];
		switch(o.type)
		{
			case qmpOption::ParameterType::parameter_int:
			{
				QSpinBox *sb=new QSpinBox;
				sb->setMinimum(o.minv.toInt());
				sb->setMaximum(o.maxv.toInt());
				sb->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
				optw=sb;
			}
			break;
			case qmpOption::ParameterType::parameter_uint:
			{
				QHexSpinBox *sb=new QHexSpinBox;
				sb->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
				optw=sb;
			}
			break;
			case qmpOption::ParameterType::parameter_bool:
			{
				QCheckBox *cb=new QCheckBox(QString(o.desc.c_str()));
				cb->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
				optw=cb;
			}
			break;
			case qmpOption::ParameterType::parameter_double:
			{
				QDoubleSpinBox *sb=new QDoubleSpinBox;
				sb->setMinimum(o.minv.toDouble());
				sb->setMaximum(o.maxv.toDouble());
				sb->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
				optw=sb;
			}
			break;
			case qmpOption::ParameterType::parameter_str:
			{
				QLineEdit* te=new QLineEdit();
				te->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
				optw=te;
			}
			break;
			case qmpOption::ParameterType::parameter_enum:
			{
				QComboBox* cb=new QComboBox();
				for(std::string& item:o.enumlist)cb->addItem(QString(item.c_str()));
				cb->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
				optw=cb;
			}
			break;
			case qmpOption::ParameterType::parameter_url:
			{
				QFileEdit* fe=new QFileEdit();
				fe->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
				optw=fe;
			}
			break;
			default:
				optw=o.widget;
			break;
		}
		o.widget=optw;
		QGridLayout* page=pageForTab(o.tab);
		if(o.type==qmpOption::ParameterType::parameter_bool||
			(o.type==qmpOption::parameter_custom&&!o.desc.length()))
		{
			int row=page->rowCount();
			page->addWidget(o.widget,row,0,1,2);
		}
		else
		{
			QLabel* lb=new QLabel(o.desc.c_str(),page->parentWidget());
			lb->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
			int row=page->rowCount();
			page->addWidget(lb,row,0);
			page->addWidget(o.widget,row,1);
		}
	}
	loadOption();
}

QGridLayout* qmpSettingsWindow::pageForTab(std::string tab)
{
	if(customOptPages.find(tab)!=customOptPages.end())
		return customOptPages[tab];
	QWidget* w=new QWidget;
	QGridLayout* page=new QGridLayout(w);
	w->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	ui->tabWidget->addTab(w,QString(tab.c_str()));
	customOptPages[tab]=page;
	return page;
}

QFileEdit::QFileEdit(QWidget *par):QWidget(par)
{
	QHBoxLayout *layout=new QHBoxLayout(this);
	layout->setMargin(0);
	le=new QLineEdit(this);
	layout->addWidget(le);
	tb=new QToolButton(this);
	tb->setText("...");
	layout->addWidget(tb);
	connect(tb,&QToolButton::clicked,this,&QFileEdit::chooseFile);
}
QString QFileEdit::text(){return le->text();}
void QFileEdit::setText(const QString& s){le->setText(s);}
void QFileEdit::chooseFile()
{
	QString s=QFileDialog::getOpenFileName(nullptr,tr("Select a file"),QFileInfo(text()).dir().absolutePath());
	if(s.length())setText(s);
}
