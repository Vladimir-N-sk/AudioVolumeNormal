
#include "window.h"
#include "audio.h"
//#include <string>

void Audio::set_audio_level(QString fileNameIn, QString fileNameOut, QString setDB, QString codec )
{

//    qDebug() <<"Start SET_AUDIO_LEVEL";

    int msecDurTime=1, msecFrameTime=0;
    stop = false;

    bool ok;
    double dDb=setDB.trimmed().toDouble(&ok);

    if (!ok) qDebug()<< "SET_AUDIO_LEVEL Conversion double ERROR! Value:" << setDB;
    emit set_pS(0);
    dDb = -1 * dDb;
    if (dDb > 1) {
        QString strDb=QString::number(dDb)+"dB";

        strDb = filter_volume+strDb;

        QString dirFFmpeg = QCoreApplication::applicationDirPath()+"/lib";
        QProcess *process = new QProcess(parent());
        process->setProcessChannelMode(QProcess::MergedChannels);
        connect(process, &QProcess::errorOccurred, this, &Audio::log_prog );

        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("LD_LIBRARY_PATH", dirFFmpeg); // Add an environment variable
        process->setProcessEnvironment(env);

        process->start( (dirFFmpeg+"/libffmpeg"), QStringList()
                        << "-y"
                        << "-hide_banner"
                        << "-i" << fileNameIn
                        <<"-af" << strDb
                        <<"-c:s"<< "copy"<<"-c:v"<< "copy"
                        << "-c:a"<< codec
                        <<"-strict"<< "experimental"
                        <<fileNameOut
                        );

//-metadata comment="Audio Volume Normal"
//        -c:s copy

        if( !process->waitForStarted(1000) ) {
            qDebug() <<"ERROR START SET_AUDIO_LEVEL args: "<<(dirFFmpeg+"/libffmpeg") << " -hide_banner "<< " -i " << fileNameIn<<" -af "
                    << strDb<<" -c:s "<< " copy " <<" -c:v "<< " copy "<< " -c:a "<< codec<<" -strict experimental "<<fileNameOut;
            return;
        }

        qDebug() <<"START SET_AUDIO_LEVEL args: "  << (dirFFmpeg+"/libffmpeg")  << " -hide_banner "<< " -i " << fileNameIn<<" -af "
                << strDb<<" -c:s "<< " copy "  <<" -c:v "<< " copy "<< " -c:a "<< codec<<" -strict experimental "<<fileNameOut;


        emit set_pS(msecFrameTime*100/msecDurTime);

        while (process->waitForReadyRead(-1)) {
            if (stop) break;
            while(process->canReadLine()){

                QString line = QString(process->readLine() );

                //            qDebug()<< "Line from ffmpeg:" << line;

                if ( line.contains("time=") ){
                    int tt=line.indexOf("bit",0)-(line.indexOf("time",0)+5) ;
                    QString tim = line.mid(line.indexOf("time",0)+5,tt);
                    QTime frameTime = QTime::fromString(tim);

                    msecFrameTime = QTime(0, 0, 0).msecsTo(frameTime);

                } else if ( line.contains("Duration:") ){
                    int nn=line.indexOf(",",0)-(line.indexOf("Dur",0)+10) ;
                    QString dur = line.mid(12,nn);
                    QTime durTime = QTime::fromString(dur);
                    msecDurTime = QTime(0, 0, 0).msecsTo(durTime);

                } else if (line.contains("max_volume:")) {
                }
                emit set_pS(msecFrameTime*100/msecDurTime);
                QCoreApplication::processEvents();
            }           //process->canReadLine
        }               // rocess->waitForReadyRead

        process->close();
        delete process;
    } else { qDebug()<< "AUDIO_LEVEL < 1dB:" << setDB; }   // >1
    emit set_pS(100);
    qDebug() << "SET_AUDIO_LEVEL END process!";
}


void Audio::audio_level(QString fileName )
{

//    qDebug() <<"*** Start AUDIO_LEVEL";
    int msecDurTime=1, msecFrameTime=0, streamAudio=0;
    stop = false;

    QString dirFFmpeg = QCoreApplication::applicationDirPath()+"/lib";

    QProcess *process = new QProcess(parent());
    process->setProcessChannelMode(QProcess::MergedChannels);

    connect(process, &QProcess::errorOccurred, this, &Audio::log_prog );

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LD_LIBRARY_PATH", dirFFmpeg); // Add an environment variable
    process->setProcessEnvironment(env);

//    if ( !QFile::exists(dirFFmpeg+"/libffmpeg") ) {
//        exit(1);
//    }

    process->start( (dirFFmpeg+"/libffmpeg"), QStringList()
                    << "-hide_banner" << "-i" << fileName <<"-af" << filter_vol_detect
                    <<"-vn"<< "-sn"<< "-dn"<<"-f" << "null" << "/dev/null");

//    qDebug() <<"AUDIO_LEVEL Start new process";

    if( !process->waitForStarted(1000) ) {
        qDebug() <<"ERROR START AUDIO_LEVEL args: "<<(dirFFmpeg+"/libffmpeg")
                << " -hide_banner " << " -i " << fileName <<" -af " << filter_vol_detect
                <<" -vn "<< " -sn "<< " -dn "<<" -f " << " null " << " /dev/null";
        return;
    }

    qDebug() <<"START AUDIO_LEVEL args: "<<(dirFFmpeg+"/libffmpeg")
            << " -hide_banner " << " -i " << fileName <<" -af " << filter_vol_detect
            <<" -vn "<< " -sn "<< " -dn "<<" -f " << " null " << " /dev/null";

    emit set_pD(msecFrameTime*100/msecDurTime);
//        qDebug() << "set_pD:"<< (msecFrameTime*100/msecDurTime);
        QCoreApplication::processEvents();
    while (process->waitForReadyRead(-1)) {
        if (stop) break;
        while(process->canReadLine()){
            QString line = QString(process->readLine() );

//            qDebug()<< "Line from ffmpeg:" << line;

            if ( line.contains("time=") ){
                int tt=line.indexOf("bit",0)-(line.indexOf("time",0)+5) ;
                QString tim = line.mid(line.indexOf("time",0)+5,tt);
                QTime frameTime = QTime::fromString(tim);
                msecFrameTime = QTime(0, 0, 0).msecsTo(frameTime);
            } else if ( line.contains("Duration:") ){
                int nn=line.indexOf(",",0)-(line.indexOf("Dur",0)+10) ;
                QString dur = line.mid(12,nn);
                QTime durTime = QTime::fromString(dur);
                msecDurTime = QTime(0, 0, 0).msecsTo(durTime);

            } else if (line.contains("max_volume:")) {
                int mm=line.indexOf("dB",0)-(line.indexOf("max_vol",0)+11) ;
//                QString max = line.mid(52,mm).trimmed();
                QString max = line.mid(line.indexOf("max_vol",0)+11 ,mm).trimmed();
                emit send_max_vol(fileName, max);
            } else if ( line.contains("Audio:") && streamAudio == 0  ) {
                streamAudio++;
                int aa=line.indexOf("Audio:",0);
                int bb=line.indexOf(" ", aa+8);
                int cc = bb-aa-7;
                QString cod = line.mid(aa+7,cc).trimmed();
                emit send_codec(fileName, cod);
            }

            emit set_pD(msecFrameTime*100/msecDurTime);
//            qDebug() << "set_pD"<< (msecFrameTime*100/msecDurTime);
            QCoreApplication::processEvents();
        }           //process->canReadLine
    }               // rocess->waitForReadyRead

    emit set_pD(100);
//    qDebug() << "set_pD 100";
    qDebug() << "AUDIO_LEVEL End process.";
    process->close();
    delete process;
}

void Audio::log_prog(QProcess::ProcessError process_error)
{

    switch (process_error) {
    case QProcess::FailedToStart:
        qDebug() << "FFMPEG - The process failed to start. Either the invoked program is missing, "
                << "or you may have insufficient permissions to invoke the program.";
        break;
    case QProcess::Crashed:
        qDebug() << "FFMPEG - The process crashed some time after starting successfully.";
        break;
    case QProcess::Timedout:
        qDebug() << "FFMPEG - The last waitFor...() function timed out."
                << "The state of QProcess is unchanged, and you can try calling waitFor...() again.";
        break;
    case QProcess::WriteError:
        qDebug() << "FFMPEG - An error occurred when attempting to write to the process."
                << " For example, the process may not be running, or it may have closed its input channel.";
        break;
    case QProcess::ReadError:
        qDebug() << "FFMPEG - An error occurred when attempting to read from the process."
                << " For example, the process may not be running.";
        break;
    default:
        qDebug() << "FFMPEG - Не известная ошибка ";
    }
}



Audio::Audio(QObject *parent)
    : QObject{parent}
{
        setObjectName("Audio");
        stop=false;
}

//---------------------------------------------------------------------------------------
Audio::~Audio()
{
//    stop();
}

//void get_max_vol(void *ptr, int, const char *fmt, va_list vl)
//{
//    char str[100];
//    std::string s1, s2;

//    vsprintf(str, fmt, vl);
////    fprintf(stdout,"Строка:%s",str);
//    s1=str;

//    s2=s1.substr(0, s1.find(":") );

//    if ( s2 == "max_volume" )
//    {
//        s2=s1.substr(s1.find(":")+1, s1.find("dB")-(s1.find(":")+1) );
//        s2 = trim(s2);
//        audio_max_vol = s2;
//        std::cout << "GET_MAX_VOLUME max_volume:"<< audio_max_vol << std::endl;
//    }
//}

//std::string trim( const std::string& s)
//{
//    const std::string blank = " \t\r\n";
//    size_t p1, p2, start;

//    if ( std::string::npos == (p1 = s.find_first_not_of( blank)))
//        return "";
//    start = p1;
//    do {
//        if ( std::string::npos == (p2 = s.find_first_of( blank, p1))) {
//            return s.substr( start);
//        }
//    } while ( std::string::npos != ( p1 = s.find_first_not_of( blank, p2)));
//    return s.substr( start, p2-start);
//}


void Audio::recv_cancel_pD(){
    stop = true;
}

//void Audio::recv_max_vol(std::string str){
////    a_m_v = str;
//    file_max_vol = QString::fromStdString(str).replace(',','.');

//    QTextStream output(stdout);
//    output <<"RECV_MAX_VOL file_max_vol:" <<file_max_vol<< "\n";

//}

