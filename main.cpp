
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
    qDebug() << "See log file " + logfilePath;
    QFile outFile(logfilePath);
    outFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    output_ts.setDevice(&outFile);
    qInstallMessageHandler(myMessageHandler);

    qDebug() << "Audio Volume Normal start";
    qDebug() << "SysInfo: "<< QSysInfo::prettyProductName() << " " << QSysInfo::currentCpuArchitecture();
    qDebug() << "Default font size:"<<f.pointSize();
    qDebug() << "Set font size 12";

//    qDebug() << "palette.name QPalette::Text:" << QApplication::palette().color(QPalette::Active, QPalette::Text).name();
//    qDebug() << "palette.name QPalette::WindowText:" << QApplication::palette().color(QPalette::Active, QPalette::WindowText).name();
//    qDebug() << "palette.value QPalette::Text:" << QApplication::palette().color(QPalette::Active, QPalette::Text).value();
//    qDebug() << "palette.value QPalette::WindowText:" << QApplication::palette().color(QPalette::Active, QPalette::WindowText).value();


//Debug: 10:19:47.270 palette QPalette::Text: "#1f1c1b" ()
//Debug: 10:19:47.270 palette QPalette::WindowText: "#221f1e" ()
//Debug: 11:02:21.485 palette.value QPalette::Text: 31 ()
//Debug: 11:02:21.485 palette.value QPalette::WindowText: 34 ()


    f.setPointSize(12);
    app.setFont(f);

    QCoreApplication::setOrganizationName("Vladimir-N-sk");
    QCoreApplication::setApplicationName("Audio Volume Normal");
    QCoreApplication::setApplicationVersion("2.0");
    QCoreApplication::setOrganizationDomain("alvladnik@gmail.com");
    Window AudioWin;

     AudioWin.setMinimumSize(1280, 720);
//     AudioWin.setMinimumSize(640,480);

    AudioWin.show();
    return app.exec();
}
