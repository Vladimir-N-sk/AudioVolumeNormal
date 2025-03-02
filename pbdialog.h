#ifndef PBDIALOG_H
#define PBDIALOG_H

#include <QDialog>

namespace Ui {
class pbDialog;
}

class pbDialog : public QDialog
{
    Q_OBJECT

public:
    explicit pbDialog(QWidget *parent = 0);
    ~pbDialog();

public slots:
    void pBarAll_valueChanged(int value);
    void pBarAudio_valueChanged(int value);
    void lbAudio_setText(QString txt);
    void lbAll_setText(QString num);

private:
    Ui::pbDialog *ui;
};

#endif // PBDIALOG_H
