
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
    setWindowTitle(tr("Audio Volume Normal v.2.0"));

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
/**/
    gb = new QGroupBox(tr("Задание"));
    gb->setAlignment(Qt::AlignHCenter);

//    gb->setStyleSheet("color: black;" "background-color: #70D4E5;" );
//    gb->setStyleSheet("color: black;" "background-color: #D6D2D0;" );

    //фон светло-серый
        gb->setStyleSheet("color: black;" "background-color: #B6B6B6;" );

    rb1 = new QRadioButton(tr("Узнать уровень аудио"), gb);
    rb2 = new QRadioButton(tr("Поменять уровень аудио"), gb);
    rb1->setChecked(true);
    rb1->setDisabled(true);
    rb2->setDisabled(true);

    // Менеджер размещения для радиокнопок:
    QHBoxLayout *hbl = new QHBoxLayout();
    hbl->addWidget(rb1, 0);
    hbl->addWidget(rb2, 0);
    gb->setLayout(hbl);
    mainLayout->addWidget(gb, 4, 0, 1, 2);

    audio = new Audio();
    connect(audio,&Audio::send_max_vol1, this, &Window::recv_max_vol1 );
    connect(audio,&Audio::send_max_vol2, this, &Window::recv_max_vol2 );
    connect(audio,&Audio::send_max_vol3, this, &Window::recv_max_vol3 );
    connect(audio,&Audio::send_codec, this, &Window::recv_codec );

    pbD = new pbDialog();
    connect(audio,&Audio::set_pD, pbD, &pbDialog::pBarAudio_valueChanged );
    connect(this,&Window::send_file_name, pbD, &pbDialog::lbAudio_setText );
    connect(this,&Window::send_file_percent, pbD, &pbDialog::pBarAll_valueChanged );
    connect(this,&Window::send_file_count, pbD, &pbDialog::lbAll_setText );
    connect(pbD,&pbDialog::rejected, audio, &Audio::recv_cancel_pD );
    connect(pbD,&pbDialog::rejected, this, &Window::recv_stop );

    pbD3 = new pbDialog3();
    connect(audio,&Audio::set_pS, pbD3, &pbDialog3::pBarChange_valueChanged );
    connect(this,&Window::send_file_name, pbD3, &pbDialog3::lbChange_setText );
    connect(audio,&Audio::set_pD, pbD3, &pbDialog3::pBarAudio_valueChanged );
    connect(this,&Window::send_file_name, pbD3, &pbDialog3::lbAudio_setText );
    connect(this,&Window::send_file_percent, pbD3, &pbDialog3::pBarAll_valueChanged );
    connect(this,&Window::send_file_count, pbD3, &pbDialog3::lbAll_setText );
    connect(pbD3,&pbDialog3::rejected, audio, &Audio::recv_cancel_pD );
    connect(pbD3,&pbDialog::rejected, this, &Window::recv_stop );

    colorText = QApplication::palette().color(QPalette::Active, QPalette::Text).value();

    sb->showMessage(tr("Для начала работы нажмите <Найти файлы>"));
}


void Window::work()
{

    if ( vyborFilesList.isEmpty() ){
        QMessageBox::information(this, tr("Выбран файл"),
                                 tr("<h2>Внимание!</h2>\n"
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

                qDebug()<< "File: " << vyborFilesList[i];
                qDebug()<< "Volume audio1: " << FileVolume1.value(vyborFilesList[i]);
                qDebug()<< "Volume audio2: " << FileVolume2.value(vyborFilesList[i]);
                qDebug()<< "Volume audio3: " << FileVolume3.value(vyborFilesList[i]);
                qDebug()<< "Codec: " << FileCodec.value(vyborFilesList[i]);
            }
            pbD->hide();
        }
        if ( rb2->isChecked()){
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

                if ( FileCheck1.value(inFile) ) qDebug() << "FileCheck1 is checked.";
                if ( FileCheck2.value(inFile) ) qDebug() << "FileCheck2 is checked.";
                if ( FileCheck3.value(inFile) ) qDebug() << "FileCheck3 is checked.";

                if ( FileVolume1.value(inFile) == "-.-" ) {

                    sb->showMessage(tr("Находим текущий уровень аудио в файле: ")+inFile );
                    audio->audio_level(inFile);
                    if (stop) {
                        sb->showMessage(tr("Процесс прерван."));
                        break;
                    }

                    showMapFiles();

                    qDebug()<< "File: " << inFile;
                    qDebug()<< "Volume audio1: " << FileVolume1.value(inFile);
                    qDebug()<< "Volume audio2: " << FileVolume2.value(inFile);
                    qDebug()<< "Volume audio3: " << FileVolume3.value(inFile);
                    qDebug()<< "Codec: " << FileCodec.value(inFile);

                    dDb1=dDb2=dDb3=max=0.0;
                    dDb1=FileVolume1.value(inFile).trimmed().toDouble(&ok);
                    if (!ok) {
                        qDebug()<< "Audio 1 value Conversion double ERROR! Value:" << FileVolume1.value(inFile);
                    } else {
                        FileCheck1.insert(inFile, true );
                    }

                    dDb2=FileVolume2.value(inFile).trimmed().toDouble(&ok);
                    if (!ok) {
                        qDebug()<< "Audio 2 value Conversion double ERROR! Value:" << FileVolume2.value(inFile);
                    } else {
                        FileCheck2.insert(inFile, true );
                    }

                    dDb3=FileVolume3.value(inFile).trimmed().toDouble(&ok);
                    if (!ok) {
                        qDebug()<< "Audio 3 value Conversion double ERROR! Value:" << FileVolume3.value(inFile);
                    } else {
                        FileCheck3.insert(inFile, true );
                    }

                    if ( (dDb1 < -1) && (dDb2 < -1) && (dDb3 < -1) ) {
                        max = (dDb2>dDb1)? dDb2 : dDb1;
                        max = (max >dDb3)?  max : dDb3;
                    } else if  ((dDb1 < -1) && (dDb2 < -1)) {
                        max = (dDb2>dDb1)? dDb2 : dDb1;
                    } else if ((dDb2 < -1) && (dDb3 < -1)) {
                        max = (dDb2>dDb3)? dDb2 : dDb3;
                    } else if ((dDb1 < -1) && (dDb3 < -1)) {
                        max = (dDb1>dDb3)? dDb1 : dDb3;
                    } else if (dDb1 < -1) {
                        max=dDb1;
                    } else if (dDb2 < -1) {
                        max=dDb2;
                    } else if (dDb3 < -1) {
                        max=dDb3;
                    }
                    max = -1 * max;
                    if ( max<1) {
                        sb->showMessage(tr("Уровень аудио в файле ")
                                        +QDir::toNativeSeparators(currentDir.relativeFilePath(inFile))
                                        +tr(" не будет изменён.") );
                        qDebug() << tr("Уровень аудио в файле ")
                                 <<QDir::toNativeSeparators(currentDir.relativeFilePath(inFile))
                                << tr(" не будет изменён.");
                        continue;
                    }

                    QString strDb="volume=" + QString::number(max)+"dB";
                    qDebug()<< "Filter: " << strDb;

                    QStringList list_args;
                    list_args<< "-y" << "-hide_banner"<< "-i" << inFile
                             <<"-map"<< "0:v"<< "-c:v"<< "copy";

                    if (FileCheck1.value(inFile)) list_args<< "-map"<< "0:a:0"<< "-c:a"<< FileCodec.value(inFile);
                    if (FileCheck2.value(inFile)) list_args<< "-map"<< "0:a:1"<< "-c:a"<< FileCodec.value(inFile);
                    if (FileCheck3.value(inFile)) list_args<< "-map"<< "0:a:2"<< "-c:a"<< FileCodec.value(inFile);

                    list_args<<"-af" << strDb <<"-c:s"<< "copy"<<"-c:v"<< "copy"
                            <<"-strict"<< "experimental" <<avnFile;

                    qDebug() << "Process arguments:";
                    for (const QString &str : list_args) {
                        qDebug() << str;
                    }

                    sb->showMessage(tr("Изменяем уровень аудио в файле: ")+avnFile );
                    audio->set_audio_level( list_args );

                } else {

                    qDebug()<< "File: " << inFile;
                    qDebug()<< "Volume audio1: " << FileVolume1.value(inFile);
                    qDebug()<< "Volume audio2: " << FileVolume2.value(inFile);
                    qDebug()<< "Volume audio3: " << FileVolume3.value(inFile);
                    qDebug()<< "Codec: " << FileCodec.value(inFile);

                    dDb1=dDb2=dDb3=max=0.0;
                    if ( FileCheck1.value(inFile) ) {
                        qDebug() << "Audio 1 is checked.";
                        dDb1=FileVolume1.value(inFile).trimmed().toDouble(&ok);
                        if (!ok) qDebug()<< "Audio1 value Conversion double ERROR! Value:" << FileVolume1.value(inFile);
                    }
                    if ( FileCheck2.value(inFile) ){
                        qDebug() << "Audio 2 is checked.";
                        dDb2=FileVolume2.value(inFile).trimmed().toDouble(&ok);
                        if (!ok) qDebug()<< "Audio2 value Conversion double ERROR! Value:" << FileVolume2.value(inFile);
                    }
                    if ( FileCheck3.value(inFile) ) {
                        qDebug() << "Audio 3 is checked.";
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

                    if ( (dDb1 < -1) && (dDb2 < -1) && (dDb3 < -1) ) {
                        max = (dDb2>dDb1)? dDb2 : dDb1;
                        max = (max >dDb3)?  max : dDb3;
                    } else if  ((dDb1 < -1) && (dDb2 < -1)) {
                        max = (dDb2>dDb1)? dDb2 : dDb1;
                    } else if ((dDb2 < -1) && (dDb3 < -1)) {
                        max = (dDb2>dDb3)? dDb2 : dDb3;
                    } else if ((dDb1 < -1) && (dDb3 < -1)) {
                        max = (dDb1>dDb3)? dDb1 : dDb3;
                    } else if (dDb1 < -1) {
                        max=dDb1;
                    } else if (dDb2 < -1) {
                        max=dDb2;
                    } else if (dDb3 < -1) {
                        max=dDb3;
                    }
                    max = -1 * max;
                    if ( max<1) {
                        sb->showMessage(tr("Уровень аудио в файле ")+
                                        QDir::toNativeSeparators(currentDir.relativeFilePath(inFile)) + tr(" не будет изменён.") );
                        qDebug() << tr("Уровень аудио в файле ")
                                 <<QDir::toNativeSeparators(currentDir.relativeFilePath(inFile)) << tr(" не будет изменён.");
                        continue;
                    }

                    QString strDb="volume=" + QString::number(max)+"dB";
                    qDebug()<< "Filter: " << strDb;

                    QStringList list_args;
                    list_args<< "-y" << "-hide_banner"<< "-i" << inFile
                             <<"-map"<< "0:v"<< "-c:v"<< "copy";

                    if (FileCheck1.value(inFile)) list_args<< "-map"<< "0:a:0"<< "-c:a"<< FileCodec.value(inFile);
                    if (FileCheck2.value(inFile)) list_args<< "-map"<< "0:a:1"<< "-c:a"<< FileCodec.value(inFile);
                    if (FileCheck3.value(inFile)) list_args<< "-map"<< "0:a:2"<< "-c:a"<< FileCodec.value(inFile);

                    list_args<<"-af" << strDb <<"-c:s"<< "copy"<<"-c:v"<< "copy"
                            <<"-strict"<< "experimental" <<avnFile;

                    qDebug() << "Process arguments:";
                    for (const QString &str : list_args) {
                        qDebug() << str;
                    }

//exit(0);

                    sb->showMessage(tr("Изменяем уровень аудио в файле: ")+avnFile );

                    audio->set_audio_level( list_args );
                }
            }
            pbD3->hide();
        }
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
        rb2->setDisabled(false);
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
        sb->showMessage(fff + QString(" Выбираем файлы, <Задание> и <Начать>"));
    } else {
        workButton->hide();
        rb1->setDisabled(true);
        rb2->setDisabled(true);
        findFilesList.clear();
        vyborFile.clear();
        QMessageBox::information(this, tr("Поиск фалов"),
                                 tr("<h2>Упс!</h2>\n"
                                    "<p>В выбранной папке ни один медиафайл не найден."
                                    "<p>Попробуйте в другой папке."),
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
    for (const QString &filePathName : findFileNames) {
        FileSize.insert(filePathName, QFileInfo(filePathName).size());
        FileVolume1.insert(filePathName, "-.-");
        FileVolume2.insert(filePathName, "-.-");
        FileVolume3.insert(filePathName, "-.-");
        FileCodec.insert(filePathName, "---");
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
        const QString tTC = "Выбор обрабатывать или не обрабатывать этот аудио поток";
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
        filesTable->setItem(row, 3, volumeItem1);
        filesTable->setItem(row, 4, checkItem1);
        filesTable->setItem(row, 5, volumeItem2);
        filesTable->setItem(row, 6, checkItem2);
        filesTable->setItem(row, 7, volumeItem3);
        filesTable->setItem(row, 8, checkItem3);
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

    filesTable = new QTableWidget(0, 9);
    filesTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList labels;
    labels << tr("Файл") << tr("Размер")<< tr("Кодек")
           << tr("Аудио1")<< tr("Выбор")<< tr("Аудио2")<< tr("Выбор")<< tr("Аудио3")<< tr("Выбор");
    filesTable->setHorizontalHeaderLabels(labels);
    filesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
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
    const QTableWidgetItem *item4 = filesTable->item(row, 4);
    const QTableWidgetItem *item6 = filesTable->item(row, 6);
    const QTableWidgetItem *item8 = filesTable->item(row, 8);

    vyborFile = fileNameOfItem(item);
    vyborFilesList.clear();
    vyborFilesList << vyborFile;

    FileCheck1.clear();
    FileCheck2.clear();
    FileCheck3.clear();

    FileCheck1.insert(vyborFile, item4->checkState() );
    FileCheck2.insert(vyborFile, item6->checkState() );
    FileCheck3.insert(vyborFile, item8->checkState() );

    work();
    showMapFiles();
}

void Window::work_list()
{
    // колличество итемов = строка * столбец
    listItem.clear();
    listItem = filesTable->selectedItems();

    if (!listItem.isEmpty()) {
    Q_ASSERT(listItem.count() % 9 == 0);
        // прыгаем через 9 элементов, кол-во столбцов в строке
        vyborFilesList.clear();
        FileCheck1.clear();
        FileCheck2.clear();
        FileCheck3.clear();
        for (int i=0; i<listItem.size() ;i+=9) {
            vyborFile=listItem.at(i)->data(absoluteFileNameRole).toString();
            vyborFilesList << vyborFile;
            FileCheck1.insert(vyborFile, listItem.at(i+4)->checkState() );
            FileCheck2.insert(vyborFile, listItem.at(i+6)->checkState() );
            FileCheck3.insert(vyborFile, listItem.at(i+8)->checkState() );
        }
        work();
        showMapFiles();
    } else {
        QMessageBox::information(this, tr("Выбор файла"),
                                 tr("<h2>Внимание!</h2>"
                                    "<p>Ни один файл не выбран."),
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

    const QTableWidgetItem *item4 = filesTable->item(item->row(), 4);
    const QTableWidgetItem *item6 = filesTable->item(item->row(), 6);
    const QTableWidgetItem *item8 = filesTable->item(item->row(), 8);

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
        FileCheck1.insert(vyborFile, item4->checkState() );
        FileCheck2.insert(vyborFile, item6->checkState() );
        FileCheck3.insert(vyborFile, item8->checkState() );

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
    QString directory =
        QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this,
                                 tr("Выбрать папку"),
                                 QDir::currentPath() ));

    if (!directory.isEmpty()) {
        if (directoryComboBox->findText(directory) == -1)
            directoryComboBox->addItem(directory);
        directoryComboBox->setCurrentIndex(directoryComboBox->findText(directory));
        FileSize.clear();
        FileVolume1.clear();
        FileVolume2.clear();
        FileVolume3.clear();
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

void Window::recv_stop(){
    stop = true;
}
