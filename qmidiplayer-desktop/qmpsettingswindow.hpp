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
		void SetOptionInt(std::string key,int val);
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
		std::map<std::string,QFormLayout*> customOptPages;
		void updateCustomeOptions();
		static QSettings *settings;
		static QComboBox* outwidget;
	public:
		static QSettings* getSettingsIntf(){return settings;}
		static QComboBox* getDefaultOutWidget();
};

#endif // QMPSETTINGSWINDOW_H
