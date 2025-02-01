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

//std::string trim( const std::string&);
//static std::string audio_max_vol;

class Audio : public QObject
{
    Q_OBJECT
public:
    explicit Audio(QObject *parent = nullptr);
    virtual ~Audio();

//    void audio_level(const char* fileName );
    void audio_level(QString fileName );
    void set_audio_level(QString fileNameIn, QString fileNameOut, QString setDB);

//    int exit_prog(int);
//    void logging(const char *fmt, ...);

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
    void send_max_vol(QString,QString);

protected:
    QProcess *process;
    QString filter_vol_detect = "volumedetect";
    QString filter_volume = "volume=";


private:
    bool stop;

};

//void get_max_vol(void *ptr, int, const char *fmt, va_list vl);


#endif // AUDIO_H
