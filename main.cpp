/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
//    QByteArray envVar = qgetenv("QTDIR");       //  check if the app is ran in Qt Creator
//    if (envVar.isEmpty())
//        logToFile = true;
//    qInstallMessageHandler(customMessageOutput); // custom message handler for debugging


    QString logfilePath = QStringLiteral("/home/monsys/avn.log");
//    QString logfilePath = QCoreApplication::applicationDirPath()+"/avn.log";
    QFile outFile(logfilePath);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    output_ts.setDevice(&outFile);
    qInstallMessageHandler(myMessageHandler);


    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Vladimir-N-sk");
    QCoreApplication::setApplicationName("Audio Volume Normal");
    QCoreApplication::setApplicationVersion("1.0");
    QCoreApplication::setOrganizationDomain("alvladnik@gmail.com");
    Window AudioWin;

//     AudioWin.setFixedSize(640,480);
     AudioWin.setMinimumSize(1280, 720);
//     AudioWin.setMinimumSize(640,480);

    AudioWin.show();
    return app.exec();
}
