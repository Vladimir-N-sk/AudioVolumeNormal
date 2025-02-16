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

class Audio : public QObject
{
    Q_OBJECT
public:
    explicit Audio(QObject *parent = nullptr);
    virtual ~Audio();

    void audio_level(QString fileName );
    void set_audio_level(QString fileNameIn, QString fileNameOut, QString setDB, QString codec);

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
    void send_max_vol(QString,QString);
    void send_codec(QString,QString);

protected:
    QProcess *process;
    QString filter_vol_detect = "volumedetect";
    QString filter_volume = "volume=";

private:
    bool stop;

};

#endif // AUDIO_H
