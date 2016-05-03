#ifndef QMPSETTINGSWINDOW_H
#define QMPSETTINGSWINDOW_H

#include <map>
#include <QDialog>
#include <QCloseEvent>
#include <QSettings>
#include <QListWidget>
#include <QComboBox>
#include <QFormLayout>
#include "qmpplugin.hpp"

namespace Ui {
	class qmpSettingsWindow;
}

struct qmpCustomOption
{
	QWidget* widget;
	std::string desc;int type;
	QVariant defaultval,minv,maxv;
};

class qmpSettingsWindow:public QDialog
{
	Q_OBJECT

	public:
		explicit qmpSettingsWindow(QWidget *parent=0);
		~qmpSettingsWindow();
		void closeEvent(QCloseEvent *event);
		void settingsInit();
		QListWidget* getSFWidget();
		void updatePluginList(qmpPluginManager *pmgr);
		void registerOptionInt(std::string tab,std::string desc,std::string key,int min,int max,int defaultval);
		int getOptionInt(std::string key);
		void setOptionInt(std::string key,int val);
		void registerOptionUint(std::string tab,std::string desc,std::string key,unsigned min,unsigned max,unsigned defaultval);
		unsigned getOptionUint(std::string key);
		void setOptionUint(std::string key,unsigned val);
		void registerOptionBool(std::string tab,std::string desc,std::string key,bool defaultval);
		bool getOptionBool(std::string key);
		void setOptionBool(std::string key,bool val);
		void registerOptionDouble(std::string tab,std::string desc,std::string key,double min,double max,double defaultval);
		double getOptionDouble(std::string key);
		void setOptionDouble(std::string key,double val);
		void registerOptionString(std::string tab,std::string desc,std::string key,std::string defaultval);
		std::string getOptionString(std::string key);
		void setOptionString(std::string key,std::string val);
	signals:
		void dialogClosing();

	private slots:
		void on_buttonBox_accepted();
		void on_buttonBox_rejected();

		void on_cbBufSize_currentTextChanged(const QString &s);
		void on_cbBufCnt_currentTextChanged(const QString &s);

		void on_pbAdd_clicked();
		void on_pbRemove_clicked();
		void on_pbUp_clicked();
		void on_pbDown_clicked();

		void on_cbAutoBS_stateChanged();

	private:
		Ui::qmpSettingsWindow *ui;
		void settingsUpdate();
		std::map<std::string,qmpCustomOption> customOptions;
		std::map<std::string,QGridLayout*> customOptPages;
		void updateCustomOptions();
		static QSettings *settings;
		static QComboBox* outwidget;
	public:
		static QSettings* getSettingsIntf(){return settings;}
		static QComboBox* getDefaultOutWidget();
};

#endif // QMPSETTINGSWINDOW_H
