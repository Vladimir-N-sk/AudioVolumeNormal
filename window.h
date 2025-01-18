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

#ifndef WINDOW_H
#define WINDOW_H

#include <QObject>
#include <QWidget>
#include <QDir>
#include <QRadioButton>
#include <QGroupBox>
#include <QStringList>
#include <QDialog>
#include <QProgressBar>

#include "audio.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;
class QProgressDialog;
QT_END_NAMESPACE


class Window : public QWidget
{
    Q_OBJECT

public:
    Window(QWidget *parent = nullptr);

    QStringList findFiles(const QStringList &files, const QString &text);
    QProgressDialog *pD;

friend class Audio;

protected:

public slots:
    void step_pD(int step);
    void recv_max_vol(std::string);

private slots:
    void browse();
    void find();
    void work();

    void animateFindClick();
    void openFileOfItem(int row, int column);
    void contextMenu(const QPoint &pos);


private:
    Audio *audio;
    QStringList findFilesList;
    QString vyborFile;
    QProgressBar *pb;

    void showFiles(const QStringList &paths);
    QComboBox *createComboBox(const QString &text = QString());

    void createFilesTable();

    QComboBox *directoryComboBox;
    QLabel *filesFoundLabel;
    QPushButton *findButton;
    QPushButton *workButton;
    QTableWidget *filesTable;

    QGroupBox  *gb;      // Рамка с надписью вокруг группы элементов.
    QRadioButton *rb1;   // Три
    QRadioButton *rb2;   // зависимых
    QRadioButton *rb3;   // переключателя.
    QDir currentDir;


/**********************************************************************************************/

    AVFormatContext *fmt_ctx;
    AVCodecContext *dec_ctx;
    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx;
    AVFilterGraph *filter_graph;

    AVPacket *packet;
    AVFrame *frame;
    AVFrame *filt_frame;

    void audio_level(const char* fileName);
    void logging(const char *fmt, ...);
//??    void get_max_vol(void *ptr, int, const char *fmt, va_list vl);
    int read_all_packets();
    int open_input_file(const char *filename);
    int init_filters(const char *filters_descr);
    int exit_prog(int err);

};

#endif
