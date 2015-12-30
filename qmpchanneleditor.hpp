#ifndef QMPCHANNELEDITOR_H
#define QMPCHANNELEDITOR_H

#include <QDialog>
#include <QShowEvent>
#include <QCloseEvent>

namespace Ui {
	class qmpchanneleditor;
}

class qmpchanneleditor:public QDialog
{
	Q_OBJECT

	public:
		explicit qmpchanneleditor(QWidget *parent=0);
		~qmpchanneleditor();
	protected:
		void showEvent(QShowEvent *e);
		void closeEvent(QCloseEvent *e);
	public slots:
		void setupWindow(int chid=-1);

	private slots:
		void on_pbChLeft_clicked();
		void on_pbChRight_clicked();
		void on_dCut_sliderPressed();
		void on_dReso_sliderPressed();
		void on_dReverb_sliderPressed();
		void on_dChorus_sliderPressed();
		void on_dVol_sliderPressed();
		void on_dPan_sliderPressed();
		void on_dAttack_sliderPressed();
		void on_dDecay_sliderPressed();
		void on_dRelease_sliderPressed();
		void on_dRate_sliderPressed();
		void on_dDepth_sliderPressed();
		void on_dDelay_sliderPressed();
		void on_dAttack_sliderReleased();
		void on_dDecay_sliderReleased();
		void on_dRelease_sliderReleased();
		void on_dRate_sliderReleased();
		void on_dDepth_sliderReleased();
		void on_dDelay_sliderReleased();
		void on_dCut_sliderReleased();
		void on_dReso_sliderReleased();
		void on_dReverb_sliderReleased();
		void on_dChorus_sliderReleased();
		void on_dVol_sliderReleased();
		void on_dPan_sliderReleased();
		void on_dCut_valueChanged();
		void on_dReso_valueChanged();
		void on_dReverb_valueChanged();
		void on_dChorus_valueChanged();
		void on_dVol_valueChanged();
		void on_dPan_valueChanged();
		void on_dAttack_valueChanged();
		void on_dDecay_valueChanged();
		void on_dRelease_valueChanged();
		void on_dRate_valueChanged();
		void on_dDepth_valueChanged();
		void on_dDelay_valueChanged();

	private:
		Ui::qmpchanneleditor *ui;
		int ch;
		void sendCC();
};

#endif // QMPCHANNELEDITOR_H
