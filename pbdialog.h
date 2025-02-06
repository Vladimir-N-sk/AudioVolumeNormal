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
    void on_pBarAudio_valueChanged(int value);

private:
    Ui::pbDialog *ui;
};

#endif // PBDIALOG_H
