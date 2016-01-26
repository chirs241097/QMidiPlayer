#ifndef QMPEFXWINDOW_HPP
#define QMPEFXWINDOW_HPP

#include <QDialog>
#include <QCloseEvent>
#include <QShowEvent>
#include <QMoveEvent>

#include "qdialskulpturestyle.hpp"

namespace Ui {
	class qmpEfxWindow;
}

class qmpEfxWindow : public QDialog
{
	Q_OBJECT

	public:
		explicit qmpEfxWindow(QWidget *parent=0);
		~qmpEfxWindow();
		void closeEvent(QCloseEvent *event);
		void showEvent(QShowEvent *event);
		void moveEvent(QMoveEvent *event);
		void sendEfxChange();

	signals:
		void dialogClosing();

	private slots:
		void on_dRoom_valueChanged();
		void on_dDamp_valueChanged();
		void on_dWidth_valueChanged();
		void on_dLevelR_valueChanged();
		void on_dFeedBack_valueChanged();
		void on_dRate_valueChanged();
		void on_dDepth_valueChanged();
		void on_dLevelC_valueChanged();
		void on_sbRoom_valueChanged(QString s);
		void on_sbDamp_valueChanged(QString s);
		void on_sbWidth_valueChanged(QString s);
		void on_sbLevelR_valueChanged(QString s);
		void on_sbFeedBack_valueChanged(QString s);
		void on_sbRate_valueChanged(QString s);
		void on_sbDepth_valueChanged(QString s);
		void on_sbLevelC_valueChanged(QString s);
		void on_cbEnabledC_stateChanged();
		void on_cbEnabledR_stateChanged();
		void on_rbSine_toggled();
		void on_rbTriangle_toggled();

	private:
		void dailValueChange();
		void spinValueChange();
		Ui::qmpEfxWindow *ui;
		double rr,rd,rw,rl;
		int cfb,ct,initialized;
		double cl,cr,cd;
		QCommonStyle* styl;
};

#endif // QMPEFXWINDOW_HPP
