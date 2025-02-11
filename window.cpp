
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
    setWindowTitle(tr("Audio Volume Normal"));

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
    mainLayout->addWidget(new QLabel(tr("формата медиа: mkv avi mp4 mp3")), 0, 1, 1, 1, Qt::AlignLeft);
    mainLayout->addWidget(new QLabel(tr("<Ctrl-Q> - выход")), 0, 2, 1, 1, Qt::AlignRight);

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
    gb = new QGroupBox(this);
    gb->setTitle("Задание");
    gb->setAlignment(Qt::AlignHCenter);

    gb->setStyleSheet("color: black;" "background-color: #70D4E5;");

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
    connect(audio,&Audio::send_max_vol, this, &Window::recv_max_vol );

    pbD = new pbDialog();
    connect(audio,&Audio::set_pD, pbD, &pbDialog::on_pBarAudio_valueChanged );
    connect(this,&Window::send_file_name, pbD, &pbDialog::on_lbAudio_setText );
    connect(this,&Window::send_file_percent, pbD, &pbDialog::on_pBarAll_valueChanged );
    connect(this,&Window::send_file_count, pbD, &pbDialog::on_lbAll_setText );

    pbD3 = new pbDialog3();
    connect(audio,&Audio::set_pS, pbD3, &pbDialog3::on_pBarChange_valueChanged );
    connect(this,&Window::send_file_name, pbD3, &pbDialog3::on_lbChange_setText );
    connect(audio,&Audio::set_pD, pbD3, &pbDialog3::on_pBarAudio_valueChanged );
    connect(this,&Window::send_file_name, pbD3, &pbDialog3::on_lbAudio_setText );
    connect(this,&Window::send_file_percent, pbD3, &pbDialog3::on_pBarAll_valueChanged );
    connect(this,&Window::send_file_count, pbD3, &pbDialog3::on_lbAll_setText );

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
        sb->showMessage(QString("Выбрано ")+ (QString::number(vyborFilesList.size())) + QString(" файл(ов)."));
        if ( rb1->isChecked()) {
            pbD->show();
            for (int i = 0; i < vyborFilesList.size(); ++i) {

                emit send_file_name(vyborFilesList[i]);
                emit send_file_percent(i*100/vyborFilesList.size());
                emit send_file_count(QString::number(i));

                audio->audio_level(vyborFilesList[i] );

            }
            pbD->hide();
        }
        if ( rb2->isChecked()){
            pbD3->show();
            for (int i = 0; i < vyborFilesList.size(); ++i) {

                emit send_file_name(vyborFilesList[i]);
                emit send_file_percent(i*100/vyborFilesList.size());
                emit send_file_count(QString::number(i));

                QString outFile = vyborFilesList[i];
                QString avnFile = outFile + ".avn";

                if ( QFile::exists(avnFile) ) {
                    if ( !QFile::remove(avnFile)) {
                        QMessageBox::critical(this, tr("Выбран файл"), tr("<h2>Внимание!</h2>\n"
                                                                         "Не удалось удалить старую копию файла"),
                                             QMessageBox::Ok, QMessageBox::NoButton);
                    }
                }

                if ( FileVolume.value(outFile) == "-.-" ) {

                    audio->audio_level(outFile);

                    if ( QFile::rename(vyborFilesList[i], avnFile) ){
                        audio->set_audio_level(avnFile, outFile, FileVolume.value(outFile));

                    }else {
                        QMessageBox::critical(this, tr("Выбран файл"), tr("<h2>Внимание!</h2>\n"
                                                                         "Не удалось создать копию файла"),
                                             QMessageBox::Ok, QMessageBox::NoButton);
                    }
                } else {
                    if ( QFile::rename(vyborFilesList[i], avnFile) ){
                        audio->set_audio_level(avnFile, outFile ,FileVolume.value(outFile));

                    }else {
                        QMessageBox::warning(this, tr("Выбран файл"), tr("<h2>Внимание!</h2>\n"
                                                                         "Не удалось создать копию файла"),
                                             QMessageBox::Ok, QMessageBox::NoButton);
                    }
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

    QDirIterator it(path, {"*.mkv","*.avi","*.mp4","*.mp3"},
                    QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);

    while (it.hasNext())
        findFilesList << it.next();

//    QString text = "AVN";
//    if (!text.isEmpty())
//        files = findFiles(files, text);
//    files.sort();

    if (!findFilesList.isEmpty()) {
        rb1->setDisabled(false);
        rb2->setDisabled(false);
        createMapFiles(findFilesList);
        showMapFiles();
//        workButton->setDisabled(false);
        workButton->setVisible(true);
       sb->showMessage(QString("Найдено ")+ (QString::number(findFilesList.size())) + QString(" файлов. ")
                       + QString("Выбираем файлы, <Задание> и <Начать>"));
    } else {
//        workButton->setDisabled(true);
        workButton->hide();
        rb1->setDisabled(true);
        rb2->setDisabled(true);
        findFilesList.clear();
        vyborFile.clear();
        QMessageBox::information(this, tr("Поиск фалов"),
                                 tr("<h2>Внимание!</h2>\n"
                                    "<p>В выбранной папке"
                                    "<p>ни один медиа-файл не найден."),
                                 QMessageBox::Ok, QMessageBox::NoButton);
    }
}

void Window::createMapFiles(const QStringList &findFileNames)
{
    FileSize.clear();
    FileVolume.clear();
    for (const QString &filePathName : findFileNames) {
        FileSize.insert(filePathName, QFileInfo(filePathName).size());
        FileVolume.insert(filePathName, "-.-");
    }
}

void Window::showMapFiles()
{

    filesTable->clearContents();
    int rt = filesTable->rowCount();
    for (int i = rt ; i >= 0; i--) filesTable->removeRow(i);

    QMapIterator<QString, qint64> fs(FileSize);
    QMapIterator<QString, QString> fv(FileVolume);

    while (fs.hasNext()) {
        fs.next();
//        const QString toolTip = QDir::toNativeSeparators(fs.key());
        const QString toolTip = "Двойной клик и начнется выполнение задания над этим файлом.";
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

        QTableWidgetItem *volumeItem = new QTableWidgetItem( FileVolume[fs.key()] );
        volumeItem->setData(absoluteFileNameRole, QVariant(fs.key()));
        volumeItem->setToolTip(toolTip);
        volumeItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        volumeItem->setFlags(fileNameItem->flags() ^ Qt::ItemIsEditable);
        int row = filesTable->rowCount();
        filesTable->insertRow(row);
        filesTable->setItem(row, 0, fileNameItem);
        filesTable->setItem(row, 1, sizeItem);
        filesTable->setItem(row, 2, volumeItem);
    }
//    filesFoundLabel->setText(tr("Найдено %n медиа файлов", nullptr, FileSize.size() ));
//    filesFoundLabel->setWordWrap(true);
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
    filesTable = new QTableWidget(0, 3);
    filesTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList labels;
    labels << tr("Filename") << tr("Size")<< tr("Volume (dB)");
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
    vyborFile = fileNameOfItem(item);
    vyborFilesList.clear();
    vyborFilesList << vyborFile;
    work();
    showMapFiles();
}

void Window::work_list()
{
    // колличество итемов = строка * столбец
    listItem.clear();
    listItem = filesTable->selectedItems();

    if (!listItem.isEmpty()) {
    Q_ASSERT(listItem.count() % 3== 0);
        // прыгаем через 3 элемента
        vyborFilesList.clear();
        for (int i=0; i<listItem.size() ;i+=3) {
            vyborFile =listItem.at(i)->data(absoluteFileNameRole).toString();
//            qDebug()<<"I:"<< i <<"***File from listItem:"<<vyborFile;
            vyborFilesList << vyborFile;
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
//    const QString fileName = fileNameOfItem(item);
    vyborFile = fileNameOfItem(item);
    if (action == openAction) {
//        vyborFile = fileName;
        vyborFilesList.clear();
        vyborFilesList << vyborFile;
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
                                 QDir::currentPath()));

    if (!directory.isEmpty()) {
        if (directoryComboBox->findText(directory) == -1)
            directoryComboBox->addItem(directory);
        directoryComboBox->setCurrentIndex(directoryComboBox->findText(directory));
        FileSize.clear();
        FileVolume.clear();
        workButton->hide();
        sb->showMessage(tr("Для начала работы нажмите <Найти файлы>"));
        showMapFiles();
    }
}


void Window::recv_max_vol(QString fName, QString strMVol){
    FileVolume.insert(fName, strMVol);
}
