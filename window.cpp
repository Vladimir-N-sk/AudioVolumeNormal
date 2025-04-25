
#include <QtWidgets>

#include "window.h"
#include "audio.h"

enum { absoluteFileNameRole = Qt::UserRole + 1 };

static inline QString fileNameOfItem(const QTableWidgetItem *item)
{
    return item->data(absoluteFileNameRole).toString();
}

static inline void openFile(const QString &fileName)
{
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

Window::Window(QWidget *parent)
    : QWidget(parent)
{
    extern QString avn_ver;
    setWindowTitle(tr("Audio Volume Normal ")+avn_ver );

    const QIcon folderIcon = QIcon::fromTheme("folder-cyan");

    QPushButton *browseButton = new QPushButton(folderIcon, tr("Папки..."), this);
    browseButton->setStyleSheet("color: black;" "background-color: #00FF55;"
                              "selection-color: #00FF55;" "selection-background-color: black;");
    connect(browseButton, &QAbstractButton::clicked, this, &Window::browse);
    findButton = new QPushButton(tr(" Найти файлы "), this);
    findButton->setStyleSheet("color: black;" "background-color: #00FF55;"
                              "selection-color: #00FF55;" "selection-background-color: black;");
    connect(findButton, &QAbstractButton::clicked, this, &Window::find);
    workButton = new QPushButton(tr("Начать"), this);
    workButton->setStyleSheet("color: black;" "background-color: #00FF55;"
                              "selection-color: #00FF55;" "selection-background-color: black;");
    connect(workButton, &QAbstractButton::clicked, this, &Window::work_list);
//    workButton->setDisabled(true);
    workButton->hide();

    directoryComboBox = createComboBox(QDir::toNativeSeparators(QDir::currentPath()));
    connect(directoryComboBox->lineEdit(), &QLineEdit::returnPressed,
            this, &Window::animateFindClick);

    filesFoundLabel = new QLabel;
    sb = new QStatusBar;
//    sb1 = new QLabel(sb);

    createFilesTable();

    QGridLayout *mainLayout = new QGridLayout(this);

/*
Метод addWidget() имеет пять параметров:
первый параметр — это дочерний виджет, метка в нашем случае;
следующие два параметра — это строка и столбец в сетке компоновки, куда мы помещаем наш виджет;
последние параметры определяют, сколько строк будет занимать текущий виджет
(в нашем случае метка будет охватывать только один столбец и одну строку).
*/

    mainLayout->addWidget(new QLabel(tr("Найти файлы")), 0, 0, 1, 1);
    mainLayout->addWidget(new QLabel(tr("формата медиа: mkv avi mp4 ts")), 0, 1, 1, 1, Qt::AlignLeft);

#ifdef Q_OS_LINUX
    mainLayout->addWidget(new QLabel(tr("<Ctrl-Q> - выход")), 0, 2, 1, 1, Qt::AlignRight);
#elif defined(Q_OS_WIN)
    mainLayout->addWidget(new QLabel(tr("<Alt-F4> - выход")), 0, 2, 1, 1, Qt::AlignRight);
#else
  qDebug() << "We don't support that version OS";
#endif

    mainLayout->addWidget(new QLabel(tr("В папке:")), 1, 0);
    mainLayout->addWidget(directoryComboBox, 1, 1);

    mainLayout->addWidget(browseButton, 1, 2);
    mainLayout->addWidget(filesTable, 2, 0, 1, 3);
    mainLayout->addWidget(sb, 3, 0, 1, 2);
    mainLayout->addWidget(findButton, 3, 2);
    mainLayout->addWidget(workButton, 4, 2);

    connect(new QShortcut(QKeySequence::Quit, this), &QShortcut::activated,
        qApp, &QApplication::quit);


    gb1 = new QGroupBox(tr("Задание"));

    //фон светло-серый
//    gb1->setStyleSheet("color: black;" "background-color: #B6B6B6;" );
    gb1->setAlignment(Qt::AlignHCenter);
    gb1->setFlat(true);

    rb1 = new QRadioButton(tr("Узнать данные аудио"), gb1);
    rb_avn = new QRadioButton(tr("Изменить уровень аудио"), gb1);
    rb_5t2 = new QRadioButton(tr("Изменить формат 5.1 в stereo"), gb1);
    rb1->setChecked(true);
    rb1->setDisabled(true);
    rb_avn->setDisabled(true);
    rb_5t2->setDisabled(true);

    // Менеджер размещения для радиокнопок:
    QVBoxLayout *vbl1 = new QVBoxLayout();
    vbl1->addWidget(rb1, 0);
    vbl1->addWidget(rb_avn, 0);
    vbl1->addWidget(rb_5t2, 0);
    gb1->setLayout(vbl1);

    gb2 = new QGroupBox(tr("Кодек"));
    gb2->setAlignment(Qt::AlignHCenter);
    gb2->setFlat(false);

    rb_def = new QRadioButton(tr("Оставить без изменений"), gb2);
    rb_ac3 = new QRadioButton(tr("Преобразовать в AC3"), gb2);
    rb_aac = new QRadioButton(tr("Преобразовать в AAC"), gb2);
    rb_def->setChecked(true);
    rb_def->setDisabled(true);
    rb_ac3->setDisabled(true);
    rb_aac->setDisabled(true);

    QVBoxLayout *vbl2 = new QVBoxLayout();
    vbl2->addWidget(rb_def, 0);
    vbl2->addWidget(rb_ac3, 0);
    vbl2->addWidget(rb_aac, 0);

    gb2->setLayout(vbl2);

    QHBoxLayout *hbl = new QHBoxLayout();

    hbl->addWidget(gb1);
    hbl->addWidget(gb2);

    mainLayout->addLayout(hbl, 4, 1);

    connect(rb1, &QRadioButton::toggled, this, &Window::codec_activ );

    audio = new Audio();
    connect(audio,&Audio::send_max_vol1, this, &Window::recv_max_vol1 );
    connect(audio,&Audio::send_max_vol2, this, &Window::recv_max_vol2 );
    connect(audio,&Audio::send_max_vol3, this, &Window::recv_max_vol3 );
    connect(audio,&Audio::send_codec, this, &Window::recv_codec );
    connect(audio,&Audio::send_channel1, this, &Window::recv_channel1 );
    connect(audio,&Audio::send_channel2, this, &Window::recv_channel2 );
    connect(audio,&Audio::send_channel3, this, &Window::recv_channel3 );

    pbD = new pbDialog();
    connect(audio,&Audio::set_pD, pbD, &pbDialog::pBarAudio_valueChanged );
    connect(this,&Window::send_file_name, pbD, &pbDialog::lbAudio_setText );
    connect(this,&Window::send_file_percent, pbD, &pbDialog::pBarAll_valueChanged );
    connect(this,&Window::send_file_count, pbD, &pbDialog::lbAll_setText );
    connect(pbD,&pbDialog::rejected, audio, &Audio::recv_cancel_pD );
    connect(pbD,&pbDialog::rejected, this, &Window::recv_stop );

    pbD3 = new pbDialog3();
    connect(audio,&Audio::set_pS, pbD3, &pbDialog3::pBarChange_valueChanged );
    connect(this,&Window::send_file_avn_name, pbD3, &pbDialog3::lbChange_setText );
    connect(audio,&Audio::set_pD, pbD3, &pbDialog3::pBarAudio_valueChanged );
    connect(this,&Window::send_file_name, pbD3, &pbDialog3::lbAudio_setText );
    connect(this,&Window::send_file_percent, pbD3, &pbDialog3::pBarAll_valueChanged );
    connect(this,&Window::send_file_count, pbD3, &pbDialog3::lbAll_setText );
    connect(pbD3,&pbDialog3::rejected, audio, &Audio::recv_cancel_pD );
    connect(pbD3,&pbDialog::rejected, this, &Window::recv_stop );

    colorText = QApplication::palette().color(QPalette::Active, QPalette::Text).value();

    sb->showMessage(tr("Для начала работы нажмите <Найти файлы>"));
    setLayout(mainLayout);
}


void Window::work()
{

    if ( vyborFilesList.isEmpty() ){
        QMessageBox::information(this, tr("Выбран файл"),
                                 tr("<h2 align=\"center\">Внимание!</h2>\n"
                                    "Ни один файл не выбран."),
                                 QMessageBox::Ok, QMessageBox::NoButton);
    } else {

        QString fff="";
        switch (vyborFilesList.size()) {
        case 1: fff = "Выбран 1 файл.";
        case 2:
        case 3:
        case 4:
            fff = "Выбрано " + (QString::number(vyborFilesList.size())) + " файла.";
            break;
        default:
            fff = "Выбрано " + (QString::number(vyborFilesList.size())) + " файлов.";
            break;
        }
        sb->showMessage(fff);

        if ( rb1->isChecked()) {

            stop = false;
            pbD->setModal(true);
            pbD->show();
            for (int i = 0; i < vyborFilesList.size(); ++i) {
                if (stop) {
                    sb->showMessage(tr("Процесс прерван."));
                    break;
                }
                emit send_file_name( QFileInfo(vyborFilesList[i]).fileName() );
                emit send_file_percent(i*100/vyborFilesList.size());
                emit send_file_count(QString::number(i));

                audio->audio_level(vyborFilesList[i] );

                qInfo()<< "File: " << vyborFilesList[i];
                qInfo()<< "Volume audio1: " << FileVolume1.value(vyborFilesList[i]);
                qInfo()<< "Volume audio2: " << FileVolume2.value(vyborFilesList[i]);
                qInfo()<< "Volume audio3: " << FileVolume3.value(vyborFilesList[i]);
                qInfo()<< "Codec: " << FileCodec.value(vyborFilesList[i]);
            }
            pbD->hide();
        }
        if ( rb_avn->isChecked()){
            bool ok;
            double dDb1, dDb2, dDb3, max;
            stop = false;

            pbD3->show();
            for (int i = 0; i < vyborFilesList.size(); ++i) {

                if (stop) {
                    sb->showMessage(tr("Процесс прерван."));
                    break;
                }

                // Посылаем на форму имя обрабатываемого файла
                emit send_file_name( QFileInfo(vyborFilesList[i]).fileName() );

                // Посылаем на форму %% обработанных файлов
                emit send_file_percent(i*100/vyborFilesList.size());

                // Посылаем на форму NN обработанных файлов
                emit send_file_count(QString::number(i));

                QString inFile = vyborFilesList[i];

                QString avnFile = QFileInfo(inFile).canonicalPath() +
                        "/AVN_" + QFileInfo(inFile).fileName();

                // Посылаем на форму имя создаваемого файла
                emit send_file_avn_name( QString("AVN_" + QFileInfo(inFile).fileName()) );


                QFile testFile(avnFile);
                if (  !(testFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) ) {
                    qDebug()<< "Not open output file: " << avnFile;
                    QMessageBox::critical(this, tr("Предупреждение"), tr("<h2>Внимание</h2>\n"
                                                                         "Что-то пошло не так!\n"
                                                                         "Не удалось создать тестовый файл."),
                                          QMessageBox::Ok, QMessageBox::NoButton);
                }
                testFile.close();

                if ( QFile::exists(avnFile) ) {
                    sb->showMessage(tr("Удаляем старую копию ")+avnFile );
                    if ( !QFile::remove(avnFile)) {
                        QMessageBox::critical(this, tr("Выбран файл"), tr("<h2>Внимание!</h2>\n"
                                                                          "Не удалось удалить старую копию файла"),
                                              QMessageBox::Ok, QMessageBox::NoButton);
                    }
                }

                if ( FileVolume1.value(inFile) == "-.-" ) {

                    sb->showMessage(tr("Находим текущий уровень аудио в файле: ")+inFile );
                    audio->audio_level(inFile);
                    if (stop) {
                        sb->showMessage(tr("Процесс прерван."));
                        break;
                    }

                    showMapFiles();

                    qInfo()<< "File: " << inFile;
                    qInfo()<< "Volume audio1: " << FileVolume1.value(inFile);
                    qInfo()<< "Volume audio2: " << FileVolume2.value(inFile);
                    qInfo()<< "Volume audio3: " << FileVolume3.value(inFile);
                    qInfo()<< "Codec: " << FileCodec.value(inFile);

                    dDb1=dDb2=dDb3=1.0;
                    max=0.0;
                    if ( FileVolume1.value(inFile) != "-.-" ) {
                        dDb1=FileVolume1.value(inFile).trimmed().toDouble(&ok);
                        if (!ok) qDebug()<< "Audio1 value Conversion double ERROR! Value:" << FileVolume1.value(inFile);
                        FileCheck1.insert(inFile, true );
                    }
                    if ( FileVolume2.value(inFile) != "-.-" ){
                        dDb2=FileVolume2.value(inFile).trimmed().toDouble(&ok);
                        if (!ok) qDebug()<< "Audio2 value Conversion double ERROR! Value:" << FileVolume2.value(inFile);
                        FileCheck2.insert(inFile, true );
                    }
                    if ( FileVolume3.value(inFile) != "-.-" ) {
                        dDb3=FileVolume3.value(inFile).trimmed().toDouble(&ok);
                        if (!ok) qDebug()<< "Audio3 value Conversion double ERROR! Value:" << FileVolume3.value(inFile);
                        FileCheck3.insert(inFile, true );
                    }

                    if ( (dDb1 < 1) && (dDb2 < 1) && (dDb3 < 1) ) {
                        max = (dDb2>dDb1)? dDb2 : dDb1;
                        max = (max >dDb3)?  max : dDb3;
                    } else if  ((dDb1 < 1) && (dDb2 < 1)) {
                        max = (dDb2>dDb1)? dDb2 : dDb1;
                    } else if ((dDb2 < 1) && (dDb3 < 1)) {
                        max = (dDb2>dDb3)? dDb2 : dDb3;
                    } else if ((dDb1 < 1) && (dDb3 < 1)) {
                        max = (dDb1>dDb3)? dDb1 : dDb3;
                    } else if (dDb1 < 1) {
                        max=dDb1;
                    } else if (dDb2 < 1) {
                        max=dDb2;
                    } else if (dDb3 < 1) {
                        max=dDb3;
                    }
                    max = -1 * max;

                    QString strDb="volume=" + QString::number(max)+"dB";

                    QStringList list_args;
                    list_args<< "-y" << "-hide_banner"<< "-i" << inFile
                             <<"-map"<< "0:v:0"<< "-c:v"<< "copy";

//                    if ( rb_def->isChecked()) codec="copy";
                    if ( rb_ac3->isChecked()) codec=codec_ac3;
                    if ( rb_aac->isChecked()) codec=codec_aac;

                    if ( max > 1) {
//                        if (FileCheck1.value(inFile)) list_args<< "-map"<< "0:a:0"<< "-c:a"<< FileCodec.value(inFile);
//                        if (FileCheck2.value(inFile)) list_args<< "-map"<< "0:a:1"<< "-c:a"<< FileCodec.value(inFile);
//                        if (FileCheck3.value(inFile)) list_args<< "-map"<< "0:a:2"<< "-c:a"<< FileCodec.value(inFile);
                        if (FileCheck1.value(inFile)) list_args<< "-map"<< "0:a:0"<< "-c:a"<< codec;
                        if (FileCheck2.value(inFile)) list_args<< "-map"<< "0:a:1"<< "-c:a"<< codec;
                        if (FileCheck3.value(inFile)) list_args<< "-map"<< "0:a:2"<< "-c:a"<< codec;
                        list_args<<"-af" << strDb;
                        sb->showMessage(tr("Изменяем уровень аудио в файле: ")+QString("AVN_" + QFileInfo(inFile).fileName()) );
                    } else {
                        sb->showMessage(tr("Уровень аудио в файле ")
                                        +QString("AVN_" + QFileInfo(inFile).fileName())
                                        +tr(" не будет изменён.") );
                        qInfo() << tr("Уровень аудио в файле ")
                                <<avnFile
                               << tr(" не будет изменён.");
                        list_args<< "-map"<< "0:a" <<"-c:a"<< codec;
                    }
//                    list_args<<"-c:s"<< "copy"<<"-strict"<< "experimental" <<avnFile;
                    list_args <<"-map"<<"0:s?"<<"-c:s"<< "copy" <<avnFile;

//                    qDebug() << "Process arguments:";
//                    for (const QString &str : list_args) {
//                        qDebug() << str;
//                    }

                    audio->set_audio_level( list_args );

                } else {

                    qInfo()<< "File: " << inFile;
                    qInfo()<< "Volume audio1: " << FileVolume1.value(inFile);
                    qInfo()<< "Volume audio2: " << FileVolume2.value(inFile);
                    qInfo()<< "Volume audio3: " << FileVolume3.value(inFile);
                    qInfo()<< "Codec: " << FileCodec.value(inFile);

                    dDb1=dDb2=dDb3=1.0;
                    max=0.0;
                    if ( FileCheck1.value(inFile) ) {
                        qInfo() << "Audio 1 is checked.";
                        dDb1=FileVolume1.value(inFile).trimmed().toDouble(&ok);
                        if (!ok) qDebug()<< "Audio1 value Conversion double ERROR! Value:" << FileVolume1.value(inFile);
                    }
                    if ( FileCheck2.value(inFile) ){
                        qInfo() << "Audio 2 is checked.";
                        dDb2=FileVolume2.value(inFile).trimmed().toDouble(&ok);
                        if (!ok) qDebug()<< "Audio2 value Conversion double ERROR! Value:" << FileVolume2.value(inFile);
                    }
                    if ( FileCheck3.value(inFile) ) {
                        qInfo() << "Audio 3 is checked.";
                        dDb3=FileVolume3.value(inFile).trimmed().toDouble(&ok);
                        if (!ok) qDebug()<< "Audio3 value Conversion double ERROR! Value:" << FileVolume3.value(inFile);
                    }

                    if ( !FileCheck1.value(inFile) && !FileCheck2.value(inFile) && !FileCheck3.value(inFile)) {
                        sb->showMessage(tr("ВНИМАНИЕ! Для файла ")+
                                        QDir::toNativeSeparators(currentDir.relativeFilePath(inFile)) + tr(" не был выбран ни один аудио.") );
                        qDebug() << tr("ВНИМАНИЕ! Для файла ")
                                 <<QDir::toNativeSeparators(currentDir.relativeFilePath(inFile)) << tr(" не был выбран ни один аудио.");
                        continue;
                    }

                    if ( (dDb1 < 1) && (dDb2 < 1) && (dDb3 < 1) ) {
                        max = (dDb2>dDb1)? dDb2 : dDb1;
                        max = (max >dDb3)?  max : dDb3;
                    } else if  ((dDb1 < 1) && (dDb2 < 1)) {
                        max = (dDb2>dDb1)? dDb2 : dDb1;
                    } else if ((dDb2 < 1) && (dDb3 < 1)) {
                        max = (dDb2>dDb3)? dDb2 : dDb3;
                    } else if ((dDb1 < 1) && (dDb3 < 1)) {
                        max = (dDb1>dDb3)? dDb1 : dDb3;
                    } else if (dDb1 < 1) {
                        max=dDb1;
                    } else if (dDb2 < 1) {
                        max=dDb2;
                    } else if (dDb3 < 1) {
                        max=dDb3;
                    }
                    max = -1 * max;

                    QString strDb="volume=" + QString::number(max)+"dB";
//                    qDebug()<< "Filter: " << strDb;

                    QStringList list_args;
                    list_args<< "-y" << "-hide_banner"<< "-i" << inFile
                             <<"-map"<< "0:v:0"<< "-c:v"<< "copy";

//                    if ( rb_def->isChecked()) codec="copy";
                    if ( rb_ac3->isChecked()) codec=codec_ac3;
                    if ( rb_aac->isChecked()) codec=codec_aac;

                    if ( max > 1) {
//                        if (FileCheck1.value(inFile)) list_args<< "-map"<< "0:a:0"<< "-c:a"<< FileCodec.value(inFile);
//                        if (FileCheck2.value(inFile)) list_args<< "-map"<< "0:a:1"<< "-c:a"<< FileCodec.value(inFile);
//                        if (FileCheck3.value(inFile)) list_args<< "-map"<< "0:a:2"<< "-c:a"<< FileCodec.value(inFile);
                        if (FileCheck1.value(inFile)) list_args<< "-map"<< "0:a:0"<< "-c:a"<< codec;
                        if (FileCheck2.value(inFile)) list_args<< "-map"<< "0:a:1"<< "-c:a"<< codec;
                        if (FileCheck3.value(inFile)) list_args<< "-map"<< "0:a:2"<< "-c:a"<< codec;

                        list_args<<"-af" << strDb;
                        sb->showMessage(tr("Изменяем уровень аудио в файле: ")+avnFile );
                    } else {
                        sb->showMessage(tr("Уровень аудио в файле ")
//                                        +QDir::toNativeSeparators(currentDir.relativeFilePath(avnFile))
                                        +avnFile
                                        +tr(" не будет изменён.") );
                        qInfo() << tr("Уровень аудио в файле ")
//                                <<QDir::toNativeSeparators(currentDir.relativeFilePath(avnFile))
                               <<avnFile
                               << tr(" не будет изменён.");
                        list_args<< "-map"<< "0:a" <<"-c:a"<< codec;
                    }
//                    list_args<<"-c:s"<< "copy"<<"-strict"<< "experimental" <<avnFile;
                    list_args <<"-map"<<"0:s?"<<"-c:s"<< "copy" <<avnFile;

                    audio->set_audio_level( list_args );
                }
            }
            pbD3->hide();
        }


/****** rb_5t2   ****************************************************************************/

        if ( rb_5t2->isChecked()){
//            bool ok;
            stop = false;

            pbD3->show();
            for (int i = 0; i < vyborFilesList.size(); ++i) {

                if (stop) {
                    sb->showMessage(tr("Процесс прерван."));
                    break;
                }

                // Посылаем на форму %% обработанных файлов
                emit send_file_percent(i*100/vyborFilesList.size());

                // Посылаем на форму NN обработанных файлов
                emit send_file_count(QString::number(i));

                QString inFile = vyborFilesList[i];

                QString avnFile = QFileInfo(inFile).canonicalPath() +
                        "/5t2_" + QFileInfo(inFile).fileName();

                // Посылаем на форму имя создаваемого файла
//                emit send_file_avn_name( avnFile );
                emit send_file_avn_name( QString("5t2_" + QFileInfo(inFile).fileName()) );

                QFile testFile(avnFile);
                if (  !(testFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) ) {
                    qDebug()<< "Not open output file: " << avnFile;
                    QMessageBox::critical(this, tr("Предупреждение"), tr("<h2>Внимание</h2>\n"
                                                                         "Что-то пошло не так!\n"
                                                                         "Не удалось создать тестовый файл."),
                                          QMessageBox::Ok, QMessageBox::NoButton);
                }
                testFile.close();

                if ( QFile::exists(avnFile) ) {
                    sb->showMessage(tr("Удаляем старую копию ")+avnFile );
                    if ( !QFile::remove(avnFile)) {
                        QMessageBox::critical(this, tr("Выбран файл"), tr("<h2>Внимание!</h2>\n"
                                                                          "Не удалось удалить старую копию файла"),
                                              QMessageBox::Ok, QMessageBox::NoButton);
                    }
                }

                if ( FileCodec.value(inFile) == "---" ) {

                    sb->showMessage(tr("ВНИМАНИЕ! Для файла ")+
                     QDir::toNativeSeparators(currentDir.relativeFilePath(inFile)) + tr(" нет исходных аудио данных.") );


                    QMessageBox::information(this, tr("Изменение формата"),
                                             tr("<h2 align=\"center\">Внимание!</h2>\n"
                                                "<p align=\"center\">Для изменения формата аудио необходимо"
                                                "<p align=\"center\">сначала узнать данные аудио"
                                                "<p align=\"center\">исходного файла."
                                                ),
                                             QMessageBox::Ok, QMessageBox::NoButton);

                } else {

                    qInfo()<< "File: " << inFile;
                    if ( FileCheck1.value(inFile) ) qInfo() << "Audio 1 is checked.";
                    if ( FileCheck2.value(inFile) ) qInfo() << "Audio 2 is checked.";
                    if ( FileCheck3.value(inFile) ) qInfo() << "Audio 3 is checked.";

                    qInfo()<< "Channel audio1: " << FileChannel1.value(inFile);
                    qInfo()<< "Channel audio2: " << FileChannel2.value(inFile);
                    qInfo()<< "Channel audio3: " << FileChannel3.value(inFile);
                    qInfo()<< "Codec: " << FileCodec.value(inFile);

                    if ( !FileCheck1.value(inFile) && !FileCheck2.value(inFile) && !FileCheck3.value(inFile)) {
                        qDebug() << tr("ВНИМАНИЕ! Для файла ")
                                 <<QDir::toNativeSeparators(currentDir.relativeFilePath(inFile)) << tr(" не был выбран ни один аудио поток.");
                        sb->showMessage(tr("ВНИМАНИЕ! Для файла ")+
                                        QDir::toNativeSeparators(currentDir.relativeFilePath(inFile)) + tr(" не был выбран ни один аудио поток.") );
                        continue;
                    }

                    QStringList list_args;

                    list_args<< "-y" << "-hide_banner"<< "-i" << inFile
                             <<"-map"<< "0:v:0"<< "-c:v"<< "copy";

                    if ( rb_ac3->isChecked()) codec=codec_ac3;
                    if ( rb_aac->isChecked()) codec=codec_aac;

//                    if (FileCheck1.value(inFile)) list_args<< "-map"<< "0:a:0"<<"-ac"<< "2"<<"-c:a"<< FileCodec.value(inFile);
//                    if (FileCheck2.value(inFile)) list_args<< "-map"<< "0:a:1"<<"-ac"<< "2"<< "-c:a"<< FileCodec.value(inFile);
//                    if (FileCheck3.value(inFile)) list_args<< "-map"<< "0:a:2"<<"-ac"<< "2"<< "-c:a"<< FileCodec.value(inFile);
                    if (FileCheck1.value(inFile)) list_args<< "-map"<< "0:a:0"<<"-ac"<< "2"<<"-c:a"<< codec;
                    if (FileCheck2.value(inFile)) list_args<< "-map"<< "0:a:1"<<"-ac"<< "2"<< "-c:a"<< codec;
                    if (FileCheck3.value(inFile)) list_args<< "-map"<< "0:a:2"<<"-ac"<< "2"<< "-c:a"<< codec;

                    list_args <<"-map"<<"0:s?"<<"-c:s"<< "copy" <<avnFile;
                    sb->showMessage(tr("Изменяем формат аудио в файле: ")+avnFile );

//                    qDebug() << "Process arguments:";
//                    for (const QString &str : list_args) {
//                        qDebug() << str;
//                    }

                    audio->set_audio_level( list_args );
                } // end else
            }
            pbD3->hide();
        }  // end rb_5t2-checked



    }  // vyborFilesList.isEmpty()
    //    }  // for
}

static void updateComboBox(QComboBox *comboBox)
{
    if (comboBox->findText(comboBox->currentText()) == -1)
        comboBox->addItem(comboBox->currentText());
}

void Window::find()
{
    filesTable->setRowCount(0);
    findFilesList.clear();

    QString path = QDir::cleanPath(directoryComboBox->currentText());
    currentDir = QDir(path);

    updateComboBox(directoryComboBox);

//Без субдиреториев (для скорости)
    QDirIterator it(path, {"*.mkv","*.avi","*.mp4","*.ts"},
                    QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot
                    );

    while (it.hasNext())
        findFilesList << it.next();

    if (!findFilesList.isEmpty()) {
        rb1->setDisabled(false);
        rb_avn->setDisabled(false);
        rb_5t2->setDisabled(false);
        createMapFiles(findFilesList);
        showMapFiles();
        workButton->setVisible(true);

        QString fff="";
        switch (findFilesList.size()) {
        case 1: fff = "Найден 1 файл.";
        case 2:
        case 3:
        case 4:
            fff = "Найдено " + (QString::number(findFilesList.size())) + " файла.";
            break;
        default:
            fff = "Найдено " + (QString::number(findFilesList.size())) + " файлов.";
            break;
        }
        sb->showMessage(fff + QString(" Выбираем файлы, <Задание>, <Кодек> и <Начать>"));
    } else {
        workButton->hide();
        rb1->setDisabled(true);
        rb_avn->setDisabled(true);
        rb_5t2->setDisabled(true);
        findFilesList.clear();
        vyborFile.clear();
        QMessageBox::information(this, tr("Поиск фалов"),
                                 tr("<h2>Упс!</h2>\n"
                                    "<p align=\"center\">В выбранной папке ни один медиафайл не найден."
                                    "<p align=\"center\">Попробуйте в другой папке."),
                                 QMessageBox::Ok, QMessageBox::NoButton);
    }
}

void Window::createMapFiles(const QStringList &findFileNames)
{
    FileSize.clear();
    FileVolume1.clear();
    FileVolume2.clear();
    FileVolume3.clear();
    FileCodec.clear();
    FileChannel1.clear();
    FileChannel2.clear();
    FileChannel3.clear();
    for (const QString &filePathName : findFileNames) {
        FileSize.insert(filePathName, QFileInfo(filePathName).size());
        FileVolume1.insert(filePathName, "-.-");
        FileVolume2.insert(filePathName, "-.-");
        FileVolume3.insert(filePathName, "-.-");
        FileCodec.insert(filePathName, "---");
        FileChannel1.insert(filePathName, "---");
        FileChannel2.insert(filePathName, "---");
        FileChannel3.insert(filePathName, "---");
    }
}

void Window::showMapFiles()
{

    filesTable->clearContents();
    int rt = filesTable->rowCount();
    for (int i = rt ; i >= 0; i--) filesTable->removeRow(i);

    QMapIterator<QString, qint64> fs(FileSize);

    while (fs.hasNext()) {
        fs.next();
        const QString toolTip = "Двойной клик и начнется выполнение задания над этим файлом.";
        const QString tTC = "Включить или не включить этот аудиопоток в выходной файл";
        const QString relativePath = QDir::toNativeSeparators(currentDir.relativeFilePath(fs.key()));
        const qint64 size = QFileInfo(fs.key()).size();

        QTableWidgetItem *fileNameItem = new QTableWidgetItem(relativePath);
        fileNameItem->setData(absoluteFileNameRole, QVariant(fs.key()));
        fileNameItem->setToolTip(toolTip);
        fileNameItem->setFlags(fileNameItem->flags() ^ Qt::ItemIsEditable);

        QTableWidgetItem *sizeItem = new QTableWidgetItem(QLocale().formattedDataSize(size));
        sizeItem->setData(absoluteFileNameRole, QVariant(fs.key()));
        sizeItem->setToolTip(toolTip);
        sizeItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        sizeItem->setFlags(sizeItem->flags() ^ Qt::ItemIsEditable);

        QTableWidgetItem *codecItem = new QTableWidgetItem( FileCodec[fs.key()] );
        codecItem->setData(absoluteFileNameRole, QVariant(fs.key()));
        codecItem->setToolTip(toolTip);
        codecItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        codecItem->setFlags(codecItem->flags() ^ Qt::ItemIsEditable);
        if ( colorText < 100 ) {
//            codecItem->setBackground(QColor(QColorConstants::Svg::lightsteelblue));
            codecItem->setBackground(QColor(QColorConstants::Svg::lavender));
        } else {
            codecItem->setBackground(QColor(QColorConstants::Svg::lightslategrey));
        }


/**/
        QTableWidgetItem *channelItem1 = new QTableWidgetItem( FileChannel1[fs.key()] );
        channelItem1->setToolTip(toolTip);
        channelItem1->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        channelItem1->setFlags(channelItem1->flags() ^ Qt::ItemIsEditable);
        if ( colorText < 100 ) {
            channelItem1->setBackground(QColor(QColorConstants::Svg::aqua));
        } else {
            channelItem1->setBackground(QColor(QColorConstants::Svg::blue));
        }

        QTableWidgetItem *volumeItem1 = new QTableWidgetItem( FileVolume1[fs.key()] );
        volumeItem1->setToolTip(toolTip);
        volumeItem1->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        volumeItem1->setFlags(volumeItem1->flags() ^ Qt::ItemIsEditable);
        if ( colorText < 100 ) {
            volumeItem1->setBackground(QColor(QColorConstants::Svg::aqua));
        } else {
            volumeItem1->setBackground(QColor(QColorConstants::Svg::blue));
        }

        QTableWidgetItem *checkItem1 = new QTableWidgetItem( Qt::CheckStateRole );
        checkItem1->setToolTip(tTC);
        checkItem1->setSizeHint(QSize());
        if ( colorText < 100 ) {
            checkItem1->setBackground(QColor(QColorConstants::Svg::aqua));
        } else {
            checkItem1->setBackground(QColor(QColorConstants::Svg::blue));
        }
        if ( FileVolume1[fs.key()] == "-.-" ) {
            checkItem1->setCheckState(Qt::Unchecked);
        } else {
            checkItem1->setCheckState(Qt::Checked);
        }

/**/
        QTableWidgetItem *channelItem2 = new QTableWidgetItem( FileChannel2[fs.key()] );
        channelItem2->setToolTip(toolTip);
        channelItem2->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        channelItem2->setFlags(channelItem2->flags() ^ Qt::ItemIsEditable);
        if ( colorText < 100 ) {
            channelItem2->setBackground(QColor(QColorConstants::Svg::bisque));
        } else {
            channelItem2->setBackground(QColor(QColorConstants::Svg::blueviolet));
        }

        QTableWidgetItem *volumeItem2 = new QTableWidgetItem( FileVolume2[fs.key()] );
        volumeItem2->setToolTip(toolTip);
        volumeItem2->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        volumeItem2->setFlags(volumeItem2->flags() ^ Qt::ItemIsEditable);
        if ( colorText < 100 ) {
            volumeItem2->setBackground(QColor(QColorConstants::Svg::bisque));
        } else {
            volumeItem2->setBackground(QColor(QColorConstants::Svg::blueviolet));
        }

        QTableWidgetItem *checkItem2 = new QTableWidgetItem( Qt::CheckStateRole );
        checkItem2->setToolTip(tTC);
        if ( colorText < 100 ) {
            checkItem2->setBackground(QColor(QColorConstants::Svg::bisque));
        } else {
            checkItem2->setBackground(QColor(QColorConstants::Svg::blueviolet));
        }
        if ( FileVolume2[fs.key()] == "-.-" ) {
            checkItem2->setCheckState(Qt::Unchecked);
        } else {
            checkItem2->setCheckState(Qt::Checked);
        }


        /**/
        QTableWidgetItem *channelItem3 = new QTableWidgetItem( FileChannel3[fs.key()] );
        channelItem3->setToolTip(toolTip);
        channelItem3->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        channelItem3->setFlags(channelItem3->flags() ^ Qt::ItemIsEditable);
        if ( colorText < 100 ) {
            channelItem3->setBackground(QColor(QColorConstants::Svg::palegreen));
        } else {
            channelItem3->setBackground(QColor(QColorConstants::Svg::darkgreen));
        }

        QTableWidgetItem *volumeItem3 = new QTableWidgetItem( FileVolume3[fs.key()] );
        volumeItem3->setToolTip(toolTip);
        volumeItem3->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        volumeItem3->setFlags(volumeItem3->flags() ^ Qt::ItemIsEditable);
        if ( colorText < 100 ) {
            volumeItem3->setBackground(QColor(QColorConstants::Svg::palegreen));
        } else {
            volumeItem3->setBackground(QColor(QColorConstants::Svg::darkgreen));
        }

        QTableWidgetItem *checkItem3 = new QTableWidgetItem( Qt::CheckStateRole );
        checkItem3->setToolTip(tTC);
        if ( colorText < 100 ) {
            checkItem3->setBackground(QColor(QColorConstants::Svg::palegreen));
        } else {
            checkItem3->setBackground(QColor(QColorConstants::Svg::darkgreen));
        }
        if ( FileVolume3[fs.key()] == "-.-" ) {
            checkItem3->setCheckState(Qt::Unchecked);
        } else {
            checkItem3->setCheckState(Qt::Checked);
        }

        int row = filesTable->rowCount();
        filesTable->insertRow(row);
        filesTable->setItem(row, 0, fileNameItem);
        filesTable->setItem(row, 1, sizeItem);
        filesTable->setItem(row, 2, codecItem);
        filesTable->setItem(row, 3, channelItem1);
        filesTable->setItem(row, 4, volumeItem1);
        filesTable->setItem(row, 5, checkItem1);
        filesTable->setItem(row, 6, channelItem2);
        filesTable->setItem(row, 7, volumeItem2);
        filesTable->setItem(row, 8, checkItem2);
        filesTable->setItem(row, 9, channelItem3);
        filesTable->setItem(row, 10, volumeItem3);
        filesTable->setItem(row, 11, checkItem3);
    }
}

QComboBox *Window::createComboBox(const QString &text)
{
    QComboBox *comboBox = new QComboBox;
    comboBox->setEditable(true);
    comboBox->addItem(text);
    comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return comboBox;
}

void Window::createFilesTable()
{

    filesTable = new QTableWidget(0, 12);
    filesTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList labels;

    labels << tr("File") << tr(" Size ")<< tr(" Codec ")
           << tr(" Audio1 ")<< tr(" Lvl1 ")<< tr("v")
           << tr(" Audio2 ")<< tr(" Lvl2 ")<< tr("v")
           << tr(" Audio3 ")<< tr(" Lvl3 ")<< tr("v");

    filesTable->setHorizontalHeaderLabels(labels);

    filesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    for (int i=1; i<12;i++){
        filesTable->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }

    filesTable->verticalHeader()->hide();
    filesTable->setShowGrid(false);
    filesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);


    filesTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(filesTable, &QTableWidget::customContextMenuRequested,
            this, &Window::contextMenu);
    connect(filesTable, &QTableWidget::cellActivated,
            this, &Window::openFileOfItem);
}


void Window::openFileOfItem(int row, int /* column */)
{
    const QTableWidgetItem *item = filesTable->item(row, 0);
    const QTableWidgetItem *item5 = filesTable->item(row, 5);
    const QTableWidgetItem *item8 = filesTable->item(row, 8);
    const QTableWidgetItem *item11 = filesTable->item(row, 11);

    vyborFile = fileNameOfItem(item);
    vyborFilesList.clear();
    vyborFilesList << vyborFile;

    FileCheck1.clear();
    FileCheck2.clear();
    FileCheck3.clear();

    FileCheck1.insert(vyborFile, item5->checkState() );
    FileCheck2.insert(vyborFile, item8->checkState() );
    FileCheck3.insert(vyborFile, item11->checkState() );

    work();
    showMapFiles();
}

void Window::work_list()
{
    // колличество итемов = строка * столбец
    listItem.clear();
    listItem = filesTable->selectedItems();

    if (!listItem.isEmpty()) {
    Q_ASSERT(listItem.count() % 12 == 0);
        // прыгаем через 12 элементов, кол-во столбцов в строке
        vyborFilesList.clear();
        FileCheck1.clear();
        FileCheck2.clear();
        FileCheck3.clear();
        for (int i=0; i<listItem.size() ;i+=12) {
            vyborFile=listItem.at(i)->data(absoluteFileNameRole).toString();
            vyborFilesList << vyborFile;
            FileCheck1.insert(vyborFile, listItem.at(i+5)->checkState() );
            FileCheck2.insert(vyborFile, listItem.at(i+8)->checkState() );
            FileCheck3.insert(vyborFile, listItem.at(i+11)->checkState() );
        }
        work();
        showMapFiles();
    } else {
        QMessageBox::information(this, tr("Выбор файла"),
                                 tr("<h2 align=\"center\">Внимание!</h2>"
                                    "<p align=\"center\">Ни один файл не выбран."),
                                 QMessageBox::Ok, QMessageBox::NoButton);
    }
}


void Window::animateFindClick()
{
    findButton->animateClick();
}


void Window::contextMenu(const QPoint &pos)
{
    const QTableWidgetItem *item = filesTable->itemAt(pos);

    const QTableWidgetItem *item5 = filesTable->item(item->row(), 5);
    const QTableWidgetItem *item8 = filesTable->item(item->row(), 8);
    const QTableWidgetItem *item11 = filesTable->item(item->row(), 11);

    if (!item)
        return;
    QMenu menu;
#ifndef QT_NO_CLIPBOARD
    QAction *copyAction = menu.addAction("Копировать имя файла в буфер");
#endif
    QAction *openAction = menu.addAction("Начать процесс");
    QAction *action = menu.exec(filesTable->mapToGlobal(pos));
    if (!action)
        return;

    vyborFile = fileNameOfItem(item);
    if (action == openAction) {
        vyborFilesList.clear();
        vyborFilesList << vyborFile;

        FileCheck1.clear();
        FileCheck2.clear();
        FileCheck3.clear();
        FileCheck1.insert(vyborFile, item5->checkState() );
        FileCheck2.insert(vyborFile, item8->checkState() );
        FileCheck3.insert(vyborFile, item11->checkState() );

        work();
        showMapFiles();
    }
#ifndef QT_NO_CLIPBOARD
    else if (action == copyAction)
        QGuiApplication::clipboard()->setText(QDir::toNativeSeparators(vyborFile));
#endif
}

void Window::browse()
{
//    QString directory =
//        QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this,
//                                 tr("Выбрать папку"),
//                                 QDir::currentPath() ));

    QString directory =
            QDir::toNativeSeparators(
                QFileInfo(
                    QFileDialog::getOpenFileName(this,
                                                 tr("Выбрать файл"),
                                                 QDir::currentPath(),
                                                 tr("*.avi *.mkv *.mp4 *.ts"))
                    ).canonicalPath()
                );

    if (!directory.isEmpty()) {
        if (directoryComboBox->findText(directory) == -1)
            directoryComboBox->addItem(directory);
        directoryComboBox->setCurrentIndex(directoryComboBox->findText(directory));
        FileSize.clear();
        FileVolume1.clear();
        FileVolume2.clear();
        FileVolume3.clear();
        FileChannel1.clear();
        FileChannel2.clear();
        FileChannel3.clear();
        FileCodec.clear();
        workButton->hide();
        sb->showMessage(tr("Для начала работы нажмите <Найти файлы>"));
        showMapFiles();
    }
}

void Window::recv_max_vol1(QString fName, QString strMVol){
    FileVolume1.insert(fName, strMVol);
}
void Window::recv_max_vol2(QString fName, QString strMVol){
    FileVolume2.insert(fName, strMVol);
}
void Window::recv_max_vol3(QString fName, QString strMVol){
    FileVolume3.insert(fName, strMVol);
}

void Window::recv_codec(QString fName, QString strCodec){
    FileCodec.insert(fName, strCodec);
}

void Window::recv_channel1(QString fName, QString strChannel){
    FileChannel1.insert(fName, strChannel);
}
void Window::recv_channel2(QString fName, QString strChannel){
    FileChannel2.insert(fName, strChannel);
}
void Window::recv_channel3(QString fName, QString strChannel){
    FileChannel3.insert(fName, strChannel);
}

void Window::recv_stop(){
    stop = true;
}

void Window::codec_activ(bool checked){
    if (checked) {
        rb_def->setDisabled(true);
        rb_ac3->setDisabled(true);
        rb_aac->setDisabled(true);
    } else {
        rb_def->setDisabled(false);
        rb_ac3->setDisabled(false);
        rb_aac->setDisabled(false);
    }
}
