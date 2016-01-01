#include "qmpsettingswindow.hpp"
#include "ui_qmpsettingswindow.h"
#include "qmpmainwindow.hpp"

qmpSettingsWindow::qmpSettingsWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpSettingsWindow)
{
	ui->setupUi(this);
	connect(this,SIGNAL(dialogClosing()),parent,SLOT(dialogClosed()));
}

qmpSettingsWindow::~qmpSettingsWindow()
{
	delete ui;
}

void qmpSettingsWindow::closeEvent(QCloseEvent *event)
{
	setVisible(false);
	emit dialogClosing();
	event->accept();
}

void qmpSettingsWindow::on_buttonBox_accepted()
{
	emit dialogClosing();
}

void qmpSettingsWindow::on_buttonBox_rejected()
{
	emit dialogClosing();
}
