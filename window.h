
#ifndef WINDOW_H
#define WINDOW_H

#include <QObject>
#include <QWidget>
#include <QDir>
#include <QRadioButton>
#include <QGroupBox>
#include <QStringList>
#include <QDialog>
#include <QStatusBar>
#include <QMap>
#include <QHash>
#include <QList>
#include <QLocale>
#include <QtGlobal>
#include <QtDebug>
#include <QTextStream>
#include <QTextCodec>
#include <QTime>
#include <QFile>
#include <QSysInfo>
#include <QIcon>

#include "audio.h"
#include "pbdialog.h"
#include "pbdialog3.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;
class QProgressDialog;
class QLocale;
QT_END_NAMESPACE


class Window : public QWidget
{
    Q_OBJECT

public:
    Window(QWidget *parent = nullptr);

//    QStringList findFiles(const QStringList &files, const QString &text);

friend class Audio;

protected:
    QMap<QString, qint64> FileSize;
    QHash<QString, QString> FileCodec;

    QHash<QString, QString> FileVolume1;
    QHash<QString, QString> FileVolume2;
    QHash<QString, QString> FileVolume3;

    QHash<QString, QString> FileChannel1;
    QHash<QString, QString> FileChannel2;
    QHash<QString, QString> FileChannel3;

    QHash<QString, bool> FileCheck1;
    QHash<QString, bool> FileCheck2;
    QHash<QString, bool> FileCheck3;

    QList<QTableWidgetItem *> listItem;

signals:
void send_stop_pD();
void send_file_name(QString);
void send_file_avn_name(QString);
void send_file_count(QString);
void send_file_percent(int);

public slots:
void recv_max_vol1(QString, QString);
void recv_max_vol2(QString, QString);
void recv_max_vol3(QString, QString);
void recv_codec(QString, QString);

void recv_channel1(QString, QString);
void recv_channel2(QString, QString);
void recv_channel3(QString, QString);

void recv_stop();

private slots:
    void codec_activ(bool checked);

    void browse();
    void find();
    void work();
    void work_list();

    void animateFindClick();
    void openFileOfItem(int row, int column);
    void contextMenu(const QPoint &pos);


private:
    Audio *audio;
    pbDialog *pbD;
    pbDialog3 *pbD3;

    QStringList findFilesList;
    QStringList vyborFilesList;
    QString vyborFile, sb1;
    QString codec_aac="aac", codec_ac3="ac3", codec;
    QStatusBar *sb;

    void showMapFiles();
    void createMapFiles(const QStringList &findFileNames);
    QComboBox *createComboBox(const QString &text = QString());

    void createFilesTable();

    QComboBox *directoryComboBox;
    QLabel *filesFoundLabel;
    QPushButton *findButton;
    QPushButton *workButton;
    QTableWidget *filesTable;

    QGroupBox  *gb1;      // Рамка с надписью вокруг группы элементов.
    QGroupBox  *gb2;      // Рамка с надписью вокруг группы элементов.
    QRadioButton *rb1;   // Три
    QRadioButton *rb_avn;   // зависимых
    QRadioButton *rb_5t2;   // переключателя.
    QRadioButton *rb_def;   // Три
    QRadioButton *rb_ac3;   // зависимых
    QRadioButton *rb_aac;   // переключателя.

    QDir currentDir;

    bool stop;
    int colorText;
};

#endif
