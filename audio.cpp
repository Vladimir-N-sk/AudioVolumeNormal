
#include "window.h"
#include "audio.h"

void Audio::set_audio_level(QStringList process_args )
{

    qInfo() <<"Start SET_AUDIO_LEVEL";

    int msecDurTime=1, msecFrameTime=0;
    stop = false;

    emit set_pS(0);

    QString fileFFmpeg = QDir::toNativeSeparators( (QCoreApplication::applicationDirPath()+"/libffmpeg") );

    if ( !QFile(fileFFmpeg).exists() ) {
        qDebug() << "Not found file libffmpeg" ;
        exit(1);
    }

    QProcess *process = new QProcess(parent());
    process->setProcessChannelMode(QProcess::MergedChannels);
    connect(process, &QProcess::errorOccurred, this, &Audio::log_prog );

    process->start( (fileFFmpeg), process_args);

    if( !process->waitForStarted(1000) ) {
        qDebug() <<"ERROR START SET_AUDIO_LEVEL args: "<<(fileFFmpeg);
        qDebug() << process_args;
        return;
    } else {
        qInfo() <<"START SET_AUDIO_LEVEL args: "<<(fileFFmpeg);
        qInfo() << process_args;
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

            }
//            else if (line.contains("max_volume:")) {
//            }
            emit set_pS(msecFrameTime*100/msecDurTime);
            QCoreApplication::processEvents();
        }           //process->canReadLine
    }               // rocess->waitForReadyRead

    process->close();
    delete process;
    emit set_pS(100);
    qInfo() << "End SET_AUDIO_LEVEL";
}


void Audio::audio_level(QString fileName )
{

    qInfo() << "Start AUDIO_LEVEL";
    int msecDurTime=1, streamAudio=0, kodek=0;
    int mFT[] {0,0,0};
    stop = false;

    QString fileFFmpeg = QDir::toNativeSeparators( (QCoreApplication::applicationDirPath()+"/libffmpeg") );

    if ( !QFile(fileFFmpeg).exists() ) {
        qDebug() << "Not found file libffmpeg" ;
        exit(1);
    }


    QProcess *process = new QProcess(parent());
    process->setProcessChannelMode(QProcess::MergedChannels);

    connect(process, &QProcess::errorOccurred, this, &Audio::log_prog );

    //Определяем кол-во аудио потоков
    process->start( (fileFFmpeg), QStringList()
                    << "-hide_banner" << "-i" << fileName);

    if( !process->waitForStarted(1000) ) {
        qDebug() <<"ERROR START AUDIO_LEVEL args: "<<(fileFFmpeg)
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

    qInfo()<< "Count Audio stream:" << streamAudio;

    //Опрос аудио потоков

    for (int numberStreamAudio=0; numberStreamAudio<streamAudio; numberStreamAudio++) {

        QStringList list_args;
        list_args<< "-hide_banner" << "-i" << fileName;

        QString ann="0:a:" + QString::number(numberStreamAudio);
        list_args << "-map" << ann;

        list_args<<"-af"<<filter_vol_detect<<"-vn"<< "-sn"<< "-dn"<<"-f"<< "null"<<"/dev/null";

        process->start( (fileFFmpeg), list_args);

        if( !process->waitForStarted(1000) ) {
            qDebug() <<"ERROR START AUDIO_LEVEL args: "<<(fileFFmpeg)
                    << " -hide_banner " << " -i " << fileName
                    << "-map" << ann
                    <<" -af " << filter_vol_detect
                   <<" -vn "<< " -sn "<< " -dn "<<" -f " << " null " << " /dev/null";
            return;
        }

        qDebug() <<"START AUDIO_LEVEL args: "<<(fileFFmpeg)
                << " -hide_banner " << " -i " << fileName
                << "-map" << ann
                <<" -af " << filter_vol_detect
                <<" -vn "<< " -sn "<< " -dn "<<" -f " << " null " << " /dev/null";

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
                    mFT[numberStreamAudio] = QTime(0, 0, 0).msecsTo(frameTime);
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

                    switch (numberStreamAudio) {
                    case 0:
                        emit send_max_vol1(fileName, max);
                        break;
                    case 1:
                        emit send_max_vol2(fileName, max);
                        break;
                    case 2:
                        emit send_max_vol3(fileName, max);
                        break;
                    default:
                        break;
                    }
                    QCoreApplication::processEvents();
                } else if ( line.contains("Audio:") && (numberStreamAudio == 0) ) {
                    if (kodek == 0) {
                        kodek++;
                        //Позиция (1) 'A'udio от начала строки
                        int aa=line.indexOf("Audio:",0);
                        //Позиция (2) любого не буквы и цифры от 'А' + 8
                        int bb=line.indexOf(QRegExp("[^0-9a-z-A-Z]"), aa+8);
                        //Длинна подстроки cc=(2)-(1)-('Audio: ')
                        int cc = bb-aa-7;
                        QString cod = line.mid(aa+7,cc).trimmed();
                        emit send_codec(fileName, cod);
                        QCoreApplication::processEvents();
                    }

//                    qDebug()<< "Line from ffmpeg:" << line;

                    if(numberStreamAudio == 0){

                        int z1=line.indexOf(QChar(','), (line.indexOf(QChar(','),0))+1 );
                        int z2=line.indexOf(QChar(','), z1+1 );
                        QString channel = line.mid( (z1+1),(z2-z1-1) ).trimmed();

                        z1=line.mid( (line.indexOf(QChar(':'),0))+1,1).toInt();

//                        qDebug() << "***Audio Channel:" << channel;
                        switch (z1) {
                        case 1:
//                            qDebug() << "*****Number Audio Channel:"<<z1 <<" format:" << channel;
                            emit send_channel1(fileName, channel);
                            break;
                        case 2:
//                            qDebug() << "*****Number Audio Channel:"<<z1 <<" format:" << channel;
                            emit send_channel2(fileName, channel);
                            break;
                        case 3:
//                            qDebug() << "*****Number Audio Channel:"<<z1 <<" format:" << channel;
                            emit send_channel3(fileName, channel);
                            break;
                        default:
                            break;
                        }
                    }
                    QCoreApplication::processEvents();
                }
                emit set_pD( ((mFT[0]+mFT[1]+mFT[2])*100)/(msecDurTime*streamAudio) );
                QCoreApplication::processEvents();
            }           //process->canReadLine
        }               // rocess->waitForReadyRead
    }//end for
    emit set_pD(100);
    qInfo() << "End AUDIO_LEVEL";
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

Audio::~Audio()
{
}

void Audio::recv_cancel_pD(){
    stop = true;
}

