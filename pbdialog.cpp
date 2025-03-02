#include "pbdialog.h"
#include "ui_pbdialog.h"

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

void pbDialog::pBarAll_valueChanged(int value)
{
    ui->pBarAll->setValue(value);
}

void pbDialog::pBarAudio_valueChanged(int value)
{
    ui->pBarAudio->setValue(value);
}

void pbDialog::lbAudio_setText(QString txt)
{
    ui->lbAudio->setText(QString(tr("в файле: "))+txt );
}

void pbDialog::lbAll_setText(QString num)
{
    ui->lbAll->setText(QString(tr("Обработано файлов: "))+num );
}
