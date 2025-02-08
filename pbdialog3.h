#ifndef PBDIALOG3_H
#define PBDIALOG3_H

#include <QDialog>

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
    void on_pBarAll_valueChanged(int value);
    void on_pBarAudio_valueChanged(int value);
    void on_pBarChange_valueChanged(int value);

    void on_lbAudio_setText(QString txt);
    void on_lbAll_setText(QString num);
    void on_lbChange_setText(QString txt);

private:
    Ui::pbDialog3 *ui;
};

#endif // PBDIALOG3_H
