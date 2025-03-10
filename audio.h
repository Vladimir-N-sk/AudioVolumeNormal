#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>
#include <QWidget>
#include <QTextStream>
#include <QString>
#include <QDebug>
#include <QProcess>
#include <QByteArray>
#include <QTime>
#include <QProcessEnvironment>
#include <QCoreApplication>
#include <QMessageBox>
#include <QRegExp>

class Audio : public QObject
{
    Q_OBJECT
public:
    explicit Audio(QObject *parent = nullptr);
    virtual ~Audio();

    void audio_level(QString fileName );
    void set_audio_level(QString fileNameIn, QString fileNameOut, QString strDb, QString codec);
    void set_audio_level(QStringList process_args);

    void log_prog(QProcess::ProcessError process_error);

    int current_frame;
    int all_frame;
    int perFrame;
    int audio_stream_index;

    QString file_max_vol;

    friend class Window;

public slots:
    void recv_cancel_pD();

signals:
    void set_pD(int);
    void set_pS(int);
    void send_max_vol1(QString,QString);
    void send_max_vol2(QString,QString);
    void send_max_vol3(QString,QString);
    void send_codec(QString,QString);

protected:
    QProcess *process;
    QString filter_vol_detect = "volumedetect";
    QString filter_volume = "volume=";

private:
    bool stop;
//    int numA;

};

#endif // AUDIO_H
