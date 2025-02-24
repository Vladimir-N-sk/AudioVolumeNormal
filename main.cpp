
#include <QApplication>
#include <QtPlugin>

#include "window.h"

//QString logFilePath = "avn.log";
//bool logToFile = false;

//void customMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
//{
//    QHash<QtMsgType, QString> msgLevelHash({{QtDebugMsg, "Debug"}, {QtInfoMsg, "Info"}, {QtWarningMsg, "Warning"}, {QtCriticalMsg, "Critical"}, {QtFatalMsg, "Fatal"}});
//    QByteArray localMsg = msg.toLocal8Bit();
//    QTime time = QTime::currentTime();
//    QString formattedTime = time.toString("hh:mm:ss.zzz");
//    QByteArray formattedTimeMsg = formattedTime.toLocal8Bit();
//    QString logLevelName = msgLevelHash[type];
//    QByteArray logLevelMsg = logLevelName.toLocal8Bit();

//    if (logToFile) {
//        QString txt = QString("%1 %2: %3 (%4)").arg(formattedTime, logLevelName, msg,  context.file);
//        QFile outFile(logFilePath);
//        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
//        QTextStream ts(&outFile);
//        ts << txt << Qt::endl;
//        outFile.close();
//    } else {
//        fprintf(stderr, "%s %s: %s (%s:%u, %s)\n", formattedTimeMsg.constData(), logLevelMsg.constData(), localMsg.constData(), context.file, context.line, context.function);
//        fflush(stderr);
//    }

//    if (type == QtFatalMsg)
//        abort();
//}


static QTextStream output_ts;

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString& msg)
{
        QTime time = QTime::currentTime();
        QString formattedTime = time.toString("hh:mm:ss.zzz");
        QByteArray formattedTimeMsg = formattedTime.toLocal8Bit();

    switch (type) {
    case QtDebugMsg:
        output_ts << QString("Debug: %1 %2 (%3)").arg(formattedTime,msg, context.file) << Qt::endl;
        break;
    case QtInfoMsg:
        output_ts << QString("Info: %1 %2 (%3)").arg(formattedTime,msg, context.file) << Qt::endl;
        break;
    case QtWarningMsg:
        output_ts << QString("Warning: %1 %2 (%3)").arg(formattedTime,msg, context.file) << Qt::endl;
        break;
    case QtCriticalMsg:
        output_ts << QString("Critical: %1 %2 (%3)").arg(formattedTime,msg, context.file) << Qt::endl;
        break;
    case QtFatalMsg:
        output_ts << QString("Fatal: %1 %2 (%3)").arg(formattedTime,msg, context.file) << Qt::endl;
        abort();
    }
}

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);

    qDebug() << "See log file avn.log";

//    QByteArray envVar = qgetenv("QTDIR");       //  check if the app is ran in Qt Creator
//    if (envVar.isEmpty())
//        logToFile = true;
//    qInstallMessageHandler(customMessageOutput); // custom message handler for debugging


//    QString logfilePath = QStringLiteral("/home/monsys/avn.log");
    QString logfilePath = QCoreApplication::applicationDirPath()+"/avn.log";
    QFile outFile(logfilePath);
    outFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    output_ts.setDevice(&outFile);
    qInstallMessageHandler(myMessageHandler);


//    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Vladimir-N-sk");
    QCoreApplication::setApplicationName("Audio Volume Normal");
    QCoreApplication::setApplicationVersion("1.1");
    QCoreApplication::setOrganizationDomain("alvladnik@gmail.com");
    Window AudioWin;

//     AudioWin.setFixedSize(640,480);
     AudioWin.setMinimumSize(1280, 720);
//     AudioWin.setMinimumSize(640,480);

    AudioWin.show();
    return app.exec();
}
