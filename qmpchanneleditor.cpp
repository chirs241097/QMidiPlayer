#include "qmpchanneleditor.hpp"
#include "ui_qmpchanneleditor.h"

qmpchanneleditor::qmpchanneleditor(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpchanneleditor)
{
	ui->setupUi(this);
}

qmpchanneleditor::~qmpchanneleditor()
{
	delete ui;
}
