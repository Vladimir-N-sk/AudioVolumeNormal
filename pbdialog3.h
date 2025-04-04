#ifndef PBDIALOG3_H
#define PBDIALOG3_H

#include <QDialog>
#include <QDir>
#include <QIcon>
#include <QApplication>

namespace Ui {
class pbDialog3;
}

class pbDialog3 : public QDialog
{
    Q_OBJECT

public:
    explicit pbDialog3(QWidget *parent = 0);
    ~pbDialog3();

public slots:
    void pBarAll_valueChanged(int value);
    void pBarAudio_valueChanged(int value);
    void pBarChange_valueChanged(int value);

    void lbAudio_setText(QString txt);
    void lbAll_setText(QString num);
    void lbChange_setText(QString txt);

private:
    Ui::pbDialog3 *ui;
};

#endif // PBDIALOG3_H
