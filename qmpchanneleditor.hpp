#ifndef QMPCHANNELEDITOR_HPP
#define QMPCHANNELEDITOR_HPP

#include <QDialog>

namespace Ui {
	class qmpchanneleditor;
}

class qmpchanneleditor : public QDialog
{
	Q_OBJECT

	public:
	explicit qmpchanneleditor(QWidget *parent = 0);
	~qmpchanneleditor();

	private:
	Ui::qmpchanneleditor *ui;
};

#endif // QMPCHANNELEDITOR_HPP
