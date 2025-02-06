#include "pbdialog.h"
#include "ui_pbdialog.h"
#include <QDebug>

pbDialog::pbDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::pbDialog)
{
    ui->setupUi(this);
}

pbDialog::~pbDialog()
{
    delete ui;
}

void pbDialog::on_pBarAudio_valueChanged(int value)
{
    ui->pBarAudio->setValue(value);
    qDebug()<< "on_pBarAudio_valueChanged" << value;

}
