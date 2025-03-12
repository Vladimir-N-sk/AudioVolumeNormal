
#include "window.h"
#include "audio.h"

void Audio::set_audio_level(QStringList process_args )
{

    qDebug() <<"Start SET_AUDIO_LEVEL";

    int msecDurTime=1, msecFrameTime=0;
    stop = false;

    emit set_pS(0);

    QString dirFFmpeg = QCoreApplication::applicationDirPath()+"/lib";

    if ( !QFile(dirFFmpeg+"/libffmpeg").exists() ) {
        qDebug() << "Not found file lib/libffmpeg" ;
        exit(1);
    }

    QProcess *process = new QProcess(parent());
    process->setProcessChannelMode(QProcess::MergedChannels);
    connect(process, &QProcess::errorOccurred, this, &Audio::log_prog );

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LD_LIBRARY_PATH", dirFFmpeg); // Add an environment variable
    process->setProcessEnvironment(env);

    process->start( (dirFFmpeg+"/libffmpeg"), process_args);

//    qDebug() << "Process arguments:";
//    for (const QString &str : process_args) {
//        qDebug() << str;
//    }


    if( !process->waitForStarted(1000) ) {
        qDebug() <<"ERROR START SET_AUDIO_LEVEL args: "<<(dirFFmpeg+"/libffmpeg");
        //                    << "-y"<< "-hide_banner" << "-i" << fileNameIn <<"-map"<< "0:v"<< "-c:v"<< "copy"<< "-map"<< "0:a:0"<< "-c:a"<< codec
        //                    <<"-map"<< "0:a:1?"<< "-c:a"<< codec<< "-map"<< "0:a:2?"<< "-c:a"<< codec
        //                    <<"-af" << strDb <<"-c:s"<< "copy"<<"-c:v"<< "copy" <<"-strict"<< "experimental"<<fileNameOut;
        return;
    }

    emit set_pS(msecFrameTime*100/msecDurTime);

    while (process->waitForReadyRead(-1)) {
        if (stop) {
            process->close();
            delete process;
            return;
        }
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
    emit set_pS(100);
    qDebug() << "SET_AUDIO_LEVEL END process!";
}


void Audio::audio_level(QString fileName )
{

    int msecDurTime=1, msecFrameTime=0, streamAudio=0, kodek=0;
    int mFT[] {0,0,0};
    stop = false;

    QString dirFFmpeg = QCoreApplication::applicationDirPath()+"/lib";

    if ( !QFile(dirFFmpeg+"/libffmpeg").exists() ) {
        qDebug() << "Not found file lib/libffmpeg" ;
        exit(1);
    }

    QProcess *process = new QProcess(parent());
    process->setProcessChannelMode(QProcess::MergedChannels);

    connect(process, &QProcess::errorOccurred, this, &Audio::log_prog );

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LD_LIBRARY_PATH", dirFFmpeg); // Add an environment variable
    process->setProcessEnvironment(env);

    //Определяем кол-во аудио потоков
    process->start( (dirFFmpeg+"/libffmpeg"), QStringList()
                    << "-hide_banner" << "-i" << fileName);

    if( !process->waitForStarted(1000) ) {
        qDebug() <<"ERROR START AUDIO_LEVEL args: "<<(dirFFmpeg+"/libffmpeg")
                << " -hide_banner " << " -i " << fileName;
        return;
    }

    emit set_pS(0);
    while (process->waitForReadyRead(-1)) {
        while(process->canReadLine()){
            QString line = QString(process->readLine() );
            //            qDebug()<< "Line from ffmpeg:" << line;
            if ( line.contains("Audio:"))  streamAudio++;

        }           //process->canReadLine
    }               // rocess->waitForReadyRead

    //больше 3-х (0,1,2) потоков не обрабатываем
        if (streamAudio>3) streamAudio=3;

    qDebug()<< "Count Audio stream:" << streamAudio;

    //Опрос аудио потоков

    for (int nn=0; nn<streamAudio; nn++) {

        QStringList list_args;
        list_args<< "-hide_banner" << "-i" << fileName;

        QString ann="0:a:" + QString::number(nn);
        list_args << "-map" << ann;

        list_args<<"-af"<<filter_vol_detect<<"-vn"<< "-sn"<< "-dn"<<"-f"<< "null"<<"/dev/null";

        process->start( (dirFFmpeg+"/libffmpeg"), list_args);

        if( !process->waitForStarted(1000) ) {
            qDebug() <<"ERROR START AUDIO_LEVEL args: "<<(dirFFmpeg+"/libffmpeg")
                    << " -hide_banner " << " -i " << fileName
                    << "-map" << ann
                    <<" -af " << filter_vol_detect
                   <<" -vn "<< " -sn "<< " -dn "<<" -f " << " null " << " /dev/null";
            return;
        }

//        qDebug() <<"START AUDIO_LEVEL args: "<<(dirFFmpeg+"/libffmpeg")
//                << " -hide_banner " << " -i " << fileName
//                << "-map" << ann
//                <<" -af " << filter_vol_detect
//                <<" -vn "<< " -sn "<< " -dn "<<" -f " << " null " << " /dev/null";

        emit set_pD( ((mFT[0]+mFT[1]+mFT[2])*100)/(msecDurTime*streamAudio) );
        QCoreApplication::processEvents();

        while (process->waitForReadyRead(-1)) {
            if (stop) {
                qDebug()<< "!!! Resv signal STOP Process !!!" ;
                process->close();
                delete process;
                return;
            }
            while(process->canReadLine()){

                QString line = QString(process->readLine() );

//                qDebug()<< "Line from ffmpeg:" << line;

                if ( line.contains("time=") ){
                    int tt=line.indexOf("bit",0)-(line.indexOf("time",0)+5) ;
                    QString tim = line.mid(line.indexOf("time",0)+5,tt);
                    QTime frameTime = QTime::fromString(tim);
                    mFT[nn] = QTime(0, 0, 0).msecsTo(frameTime);
                } else if ( line.contains("Duration:") ){
                    if (msecDurTime == 1) {
                        int nn=line.indexOf(",",0)-(line.indexOf("Dur",0)+10) ;
                        QString dur = line.mid(12,nn);
                        QTime durTime = QTime::fromString(dur);
                        msecDurTime = QTime(0, 0, 0).msecsTo(durTime);
                    }
                } else if (line.contains("max_volume:")) {

                    int mm=line.indexOf("dB",0)-(line.indexOf("max_vol",0)+11) ;
                    QString max = line.mid(line.indexOf("max_vol",0)+11 ,mm).trimmed();

                    switch (nn) {
                    case 0:
//                        qDebug()<< "!!! send_max_vol1:"<< max;
                        emit send_max_vol1(fileName, max);
                        break;
                    case 1:
//                        qDebug()<< "!!! send_max_vol2:"<< max;
                        emit send_max_vol2(fileName, max);
                        break;
                    case 2:
//                        qDebug()<< "!!! send_max_vol3:"<< max;
                        emit send_max_vol3(fileName, max);
                        break;
                    default:
                        break;
                    }
                    QCoreApplication::processEvents();
                } else if ( line.contains("Audio:") && kodek == 0  ) {
                    kodek++;
                    int aa=line.indexOf("Audio:",0);
                    int bb=line.indexOf(QRegExp("[^0-9a-z-A-Z]"), aa+8);
                    int cc = bb-aa-7;
                    QString cod = line.mid(aa+7,cc).trimmed();
                    emit send_codec(fileName, cod);
                    QCoreApplication::processEvents();
                }
                emit set_pD( ((mFT[0]+mFT[1]+mFT[2])*100)/(msecDurTime*streamAudio) );
                QCoreApplication::processEvents();
            }           //process->canReadLine
        }               // rocess->waitForReadyRead
    }//end for
    emit set_pD(100);
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
}

void Audio::recv_cancel_pD(){
    stop = true;
}

