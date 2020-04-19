#ifndef QMPSETTINGSWINDOW_H
#define QMPSETTINGSWINDOW_H

#include <string>
#include <map>
#include <vector>
#include <QDialog>
#include <QCloseEvent>
#include <QHideEvent>
#include <QSettings>
#include <QTableWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QFormLayout>
#include "qmpsettings.hpp"
#include "qmpplugin.hpp"
#include "qmpcustomizewindow.hpp"
#include "qmpdevpropdialog.hpp"

namespace Ui {
	class qmpSettingsWindow;
}

class QLineEdit;
class QToolButton;
class QFileEdit:public QWidget
{
	Q_OBJECT
	private:
		QLineEdit *le;
		QToolButton *tb;
	private slots:
		void chooseFile();
	public:
		QFileEdit(QWidget* par=nullptr);
		QString text();
		void setText(const QString& s);
};

class QHexSpinBox:public QSpinBox
{
	Q_OBJECT
	public:
		QHexSpinBox(QWidget *parent=0):QSpinBox(parent)
		{
			setPrefix("0x");
			setDisplayIntegerBase(16);
			setRange(-0x80000000,0x7FFFFFFF);
		}
	protected:
		QString textFromValue(int value)const
		{
			return QString::number(u(value),16).toUpper();
		}
		int valueFromText(const QString &text)const
		{
			return i(text.toUInt(nullptr,16));
		}
		QValidator::State validate(QString &input,int &pos)const
		{
			QString t=input;
			if(t.startsWith("0x"))t.remove(0,2);
			pos-=t.size()-t.trimmed().size();t=t.trimmed();
			if(t.isEmpty())return QValidator::Intermediate;
			input=QString("0x")+t.toUpper();
			bool okay;t.toUInt(&okay,16);
			if(!okay)return QValidator::Invalid;
			return QValidator::Acceptable;
		}
		inline unsigned int u(int i)const
		{return reinterpret_cast<unsigned&>(i);}
		inline int i(unsigned int u)const
		{return reinterpret_cast<int&>(u);}
};

class qmpDevicePriorityDialog;

class qmpSettingsWindow:public QDialog
{
	Q_OBJECT

	public:
		explicit qmpSettingsWindow(qmpSettings *qmpsettings,QWidget *parent=nullptr);
		~qmpSettingsWindow();
		void closeEvent(QCloseEvent *event);
		void hideEvent(QHideEvent *event);
		void updatePluginList(qmpPluginManager *pmgr);
		void postInit();
		void registerCustomizeWidgetOptions();
		void registerSoundFontOption();
		void registerPluginOption(qmpPluginManager *pmgr);
		void registerExtraMidiOptions();
	signals:
		void dialogClosing();

	private slots:
		void on_buttonBox_accepted();
		void on_buttonBox_rejected();

	private:
		Ui::qmpSettingsWindow *ui;
		std::map<std::string,qmpOption> customOptions;
		std::map<std::string,QGridLayout*> customOptPages;
		void saveOption(std::string key=std::string());
		void loadOption(std::string key=std::string());
		void setupWidgets();
		QGridLayout* pageForTab(std::string tab);
		qmpCustomizeWindow *cwt,*cwa;
		qmpDevPropDialog *dps;
		qmpDevicePriorityDialog *devpriod;
		qmpSettings *settings;
};

#endif // QMPSETTINGSWINDOW_H
