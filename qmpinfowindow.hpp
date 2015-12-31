#ifndef QMPINFOWINDOW_HPP
#define QMPINFOWINDOW_HPP

#include <QDialog>

namespace Ui {
	class qmpInfoWindow;
}

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
