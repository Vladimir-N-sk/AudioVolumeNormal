#include "pbdialog3.h"
#include "ui_pbdialog3.h"

pbDialog3::pbDialog3(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::pbDialog3)
{
    ui->setupUi(this);
    ui->buttonBox->setToolTip(QString(tr("Прервать процесс")));
}

pbDialog3::~pbDialog3()
{
    delete ui;
}

void pbDialog3::pBarAll_valueChanged(int value)
{
    ui->pBarAll->setValue(value);
}

void pbDialog3::pBarAudio_valueChanged(int value)
{
    ui->pBarAudio->setValue(value);
}

void pbDialog3::pBarChange_valueChanged(int value)
{
        ui->pBarChange->setValue(value);
}

void pbDialog3::lbAudio_setText(QString txt)
{
    ui->lbAudio->setText(QString(tr("Поиск в файле: "))+txt );
}

void pbDialog3::lbAll_setText(QString num)
{
    ui->lbAll->setText(QString(tr("Обработано файлов: "))+num );
}

void pbDialog3::lbChange_setText(QString txt)
{
    ui->lbChange->setText(QString(tr("Создаем файл: "))+txt );
}
