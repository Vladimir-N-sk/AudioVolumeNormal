
#include <QApplication>
#include <QtPlugin>

#include "window.h"

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

    QFont f = app.font();

    QString logfilePath = QCoreApplication::applicationDirPath()+"/AVN.log";
    qInfo() << "See log file " + logfilePath;
    QFile outFile(logfilePath);
    outFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    output_ts.setDevice(&outFile);
    qInstallMessageHandler(myMessageHandler);

    qInfo() << "Audio Volume Normal start";
    qInfo() << "SysInfo: "<< QSysInfo::prettyProductName() << " " << QSysInfo::currentCpuArchitecture();
    qInfo() << "Default font size:"<<f.pointSize();
    qInfo() << "Set font size 12";

    f.setPointSize(12);
    app.setFont(f);

    QCoreApplication::setOrganizationName("Vladimir-N-sk");
    QCoreApplication::setApplicationName("Audio Volume Normal");
    QCoreApplication::setApplicationVersion("2.2");
    QCoreApplication::setOrganizationDomain("alvladnik@gmail.com");
    Window AudioWin;

//    AudioWin.setWindowIcon(QIcon("avn.svg"));
    AudioWin.setWindowIcon(QIcon(  QString(QCoreApplication::applicationDirPath()+"/lib/avn.svg") ));

     AudioWin.setMinimumSize(1280, 720);
//     AudioWin.setMinimumSize(640,480);

    AudioWin.show();
    return app.exec();
}
