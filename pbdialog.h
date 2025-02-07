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
    void on_pBarAll_valueChanged(int value);
    void on_pBarAudio_valueChanged(int value);

    void on_lbAudio_setText(QString txt);

private:
    Ui::pbDialog *ui;
};

#endif // PBDIALOG_H
