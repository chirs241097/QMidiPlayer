#ifndef QMPINFOWINDOW_HPP
#define QMPINFOWINDOW_HPP

#include <QDialog>
#include <QLabel>
#include <QMouseEvent>
#include <QApplication>
#include <QClipboard>

namespace Ui {
	class qmpInfoWindow;
}

class QClickableLabel : public QLabel
{
	Q_OBJECT
	public:
		explicit QClickableLabel(QWidget *parent=0):QLabel(parent){}
	protected:
		void mousePressEvent(QMouseEvent *e)
		{
			QLabel::mousePressEvent(e);
			if(e->buttons()&Qt::LeftButton)
				QApplication::clipboard()->setText(text());
		}
};

class qmpInfoWindow : public QDialog
{
	Q_OBJECT

	public:
		explicit qmpInfoWindow(QWidget *parent = 0);
		~qmpInfoWindow();
	public slots:
		void updateInfo();

	private:
		Ui::qmpInfoWindow *ui;
};

#endif // QMPINFOWINDOW_HPP
