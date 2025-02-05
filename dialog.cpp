
#include <QtWidgets>

#include "dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    createProgressGroupBox();
    createButtonBox();

    mainLayout = new QGridLayout;
    mainLayout->addWidget(progressGroupBox, 0, 0);
    mainLayout->addWidget(buttonBox, 1, 0);
    setLayout(mainLayout);

//    mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
//    mainLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        mainLayout->setSizeConstraint(QLayout::SetMaximumSize);

    setWindowTitle(tr("Процесс выполнения"));
}


void Dialog::set_name_file(QString nf)
{
    label2->setText(QString(tr("Нахождение уровня аудио в файле: "))+nf);
    label3->setText(QString(tr("Изменение уровня аудио в файле: "))+nf);
}


void Dialog::set_pb_all(int v)
{
 pbar1->setValue(v);
}


void Dialog::set_pb_audio(int v)
{
 pbar2->setValue(v);
//    qDebug() << "set_pb_audio:" << v;
}

void Dialog::set_pb_change(int v)
{
 pbar3->setValue(v);
}

void Dialog::createProgressGroupBox()
{
    progressGroupBox = new QGroupBox(tr("Индикаторы выполнения"));

    label1 = new QLabel(tr("Всего файлов"));

    label2 = new QLabel(tr("2"));
    label3 = new QLabel(tr("3"));

    pbar1 = new QProgressBar();

    pbar2 = new QProgressBar();
    pbar2->setRange(0, 100);
    //вызов метода setAlignment() с параметром Qt::AlignCenter
    //переводит индикатор в режим отображения процентов в центре
    pbar2->setAlignment(Qt::AlignCenter);

    pbar3 = new QProgressBar();

    QVBoxLayout *vbl = new QVBoxLayout();

    vbl->addWidget(label1,Qt::AlignHCenter);
//    vbl->addWidget(label1,0);
    vbl->addWidget(pbar1,0);
    vbl->addWidget(label2,0,Qt::AlignHCenter);
    vbl->addWidget(pbar2,0);
    vbl->addWidget(label3,0,Qt::AlignHCenter);
    vbl->addWidget(pbar3,0);

    progressGroupBox->setLayout(vbl);
}

void Dialog::createButtonBox()
{
            qDebug() << "start createButtonBox";
    buttonBox = new QDialogButtonBox;
    closeButton = buttonBox->addButton(QDialogButtonBox::Cancel);
    connect(closeButton, &QPushButton::clicked, this, &Dialog::close);
            qDebug() << "end createButtonBox";
}


