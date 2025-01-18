
#include <QtWidgets>

#include "window.h"
#include "audio.h"

enum { absoluteFileNameRole = Qt::UserRole + 1 };

static inline QString fileNameOfItem(const QTableWidgetItem *item)
{
    return item->data(absoluteFileNameRole).toString();
}

//! [14] вызываем внешний обработчик файла
static inline void openFile(const QString &fileName)
{
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

Window::Window(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(tr("Audio Level"));

    const QIcon folderIcon = QIcon::fromTheme("folder-music");

    QPushButton *browseButton = new QPushButton(folderIcon, tr("Папки..."), this);
    connect(browseButton, &QAbstractButton::clicked, this, &Window::browse);
    findButton = new QPushButton(tr(" Найти файлы "), this);
    connect(findButton, &QAbstractButton::clicked, this, &Window::find);
    workButton = new QPushButton(tr("Начать"), this);
    connect(workButton, &QAbstractButton::clicked, this, &Window::work);
    workButton->setDisabled(true);


    directoryComboBox = createComboBox(QDir::toNativeSeparators(QDir::currentPath()));
    connect(directoryComboBox->lineEdit(), &QLineEdit::returnPressed,
            this, &Window::animateFindClick);

    filesFoundLabel = new QLabel;

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

    mainLayout->addWidget(new QLabel(tr("В папке:")), 1, 0);
    mainLayout->addWidget(directoryComboBox, 1, 1);

    mainLayout->addWidget(browseButton, 1, 2);
    mainLayout->addWidget(filesTable, 2, 0, 1, 3);
    mainLayout->addWidget(filesFoundLabel, 3, 0, 1, 2);
    mainLayout->addWidget(findButton, 3, 2);
    mainLayout->addWidget(workButton, 4, 2);

    connect(new QShortcut(QKeySequence::Quit, this), &QShortcut::activated,
        qApp, &QApplication::quit);
/**/
    gb = new QGroupBox(this);

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
      mainLayout->addWidget(gb, 4, 1, 1, 1);

      audio = new Audio();
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
    }
}


static void updateComboBox(QComboBox *comboBox)
{
    if (comboBox->findText(comboBox->currentText()) == -1)
        comboBox->addItem(comboBox->currentText());
}

void Window::step_pD(int step)
{
    pD->setValue(step);
}

void Window::recv_max_vol(std::string str){
    QString file_max_vol = QString::fromStdString(str);
            QTextStream output(stdout);
            output <<"File Max Volume:" <<file_max_vol<< "\n\n";
}

void Window::work()
{

    if ( vyborFile.isEmpty() ){
        QMessageBox::information(this, tr("Выбран файл"),
                                 tr("<h2>Внимание!</h2>\n"
                                    "Ни один файл не выбран."),
                                 QMessageBox::Ok, QMessageBox::NoButton);
    } else {

        connect(audio,&Audio::set_pD, this, &Window::step_pD );
        connect(audio,&Audio::send_max_vol, this, &Window::recv_max_vol );

        if ( rb1->isChecked()) {
            pD = new QProgressDialog("Выполнение процесса", "Стоп", 0, 100);
            pD->setMinimumWidth(400);
            pD->setMinimumDuration(0);
            pD->setValue(0);
            pD->setModal(true);
            pD->setWindowTitle(tr("Вычисление уровня аудио") );

            audio->audio_level(vyborFile.toUtf8().constData());

            delete pD;
        }
        if ( rb2->isChecked()){
            pD = new QProgressDialog("Выполнение процесса", "Стоп", 0, 100);
            pD->setMinimumWidth(400);
            pD->setMinimumDuration(0);
            pD->setValue(0);
            pD->setModal(true);
            pD->setWindowTitle(tr("Вычисление уровня аудио") );

            audio->audio_level(vyborFile.toUtf8().constData());

            delete pD;

/*****************************************************************************************/

            pD = new QProgressDialog("Выполнение процесса", "Стоп", 0, 100);
            pD->setMinimumWidth(400);
            pD->setMinimumDuration(0);
            pD->setValue(0);
            pD->setModal(true);
            pD->setWindowTitle(tr("Вычисление уровня аудио") );

            audio->audio_level(vyborFile.toUtf8().constData());

            delete pD;


        }
    }
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
//        files << it.next();
        findFilesList << it.next();

//    QString text = "AVN";
//    if (!text.isEmpty())
//        files = findFiles(files, text);
//    files.sort();

    if (!findFilesList.isEmpty()) {
//        workButton->setDisabled(false);
        rb1->setDisabled(false);
        rb2->setDisabled(false);
        showFiles(findFilesList);
    } else {
        workButton->setDisabled(true);
        rb1->setDisabled(true);
        rb2->setDisabled(true);
        findFilesList.clear();
        vyborFile.clear();
    }

}

void Window::animateFindClick()
{
    findButton->animateClick();
}

/* поиск файлов с подстрокой */
QStringList Window::findFiles(const QStringList &files, const QString &text)
{
    QProgressDialog progressDialog(this);
    progressDialog.setCancelButtonText(tr("&Cancel"));
    progressDialog.setRange(0, files.size());
    progressDialog.setWindowTitle(tr("Find Files"));
    progressDialog.setMinimumDuration(0);

    QMimeDatabase mimeDatabase;
    QStringList foundFiles;

    for (int i = 0; i < files.size(); ++i) {
        progressDialog.setValue(i);
        progressDialog.setLabelText(tr("Searching file number %1 of %n...", nullptr, files.size()).arg(i));
        QCoreApplication::processEvents();

        if (progressDialog.wasCanceled())
            break;

        const QString fileName = files.at(i);
        const QMimeType mimeType = mimeDatabase.mimeTypeForFile(fileName);
//        if (mimeType.isValid() && !mimeType.inherits(QStringLiteral("text/plain"))) {
//            qWarning() << "Not searching binary file " << QDir::toNativeSeparators(fileName);
//            continue;
//        }
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QString line;
//            QTextStream in(&file);
//            while (!in.atEnd()) {
//                if (progressDialog.wasCanceled())
//                    break;
//                line = in.readLine();
//                if (line.contains(text, Qt::CaseInsensitive)) {
//                    foundFiles << files[i];
//                    break;
//                }
//            } //end while
        }
    }
//    return foundFiles;
    return files;
}
/**/

void Window::showFiles(const QStringList &paths)
{
    for (const QString &filePath : paths) {
        const QString toolTip = QDir::toNativeSeparators(filePath);
        const QString relativePath = QDir::toNativeSeparators(currentDir.relativeFilePath(filePath));
        const qint64 size = QFileInfo(filePath).size();
        QTableWidgetItem *fileNameItem = new QTableWidgetItem(relativePath);
        fileNameItem->setData(absoluteFileNameRole, QVariant(filePath));
        fileNameItem->setToolTip(toolTip);
        fileNameItem->setFlags(fileNameItem->flags() ^ Qt::ItemIsEditable);
        QTableWidgetItem *sizeItem = new QTableWidgetItem(QLocale().formattedDataSize(size));
        sizeItem->setData(absoluteFileNameRole, QVariant(filePath));
        sizeItem->setToolTip(toolTip);
        sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        sizeItem->setFlags(sizeItem->flags() ^ Qt::ItemIsEditable);

        int row = filesTable->rowCount();
        filesTable->insertRow(row);
        filesTable->setItem(row, 0, fileNameItem);
        filesTable->setItem(row, 1, sizeItem);
    }
    filesFoundLabel->setText(tr("Найдено %n медиа файлов", nullptr, paths.size()));
    filesFoundLabel->setWordWrap(true);
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
    labels << tr("Filename") << tr("Size")<< tr("Level");
    filesTable->setHorizontalHeaderLabels(labels);
    filesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    filesTable->verticalHeader()->hide();
    filesTable->setShowGrid(false);

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
    work();
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
    const QString fileName = fileNameOfItem(item);
    if (action == openAction) {
        vyborFile = fileName;
        work();
    }
#ifndef QT_NO_CLIPBOARD
    else if (action == copyAction)
        QGuiApplication::clipboard()->setText(QDir::toNativeSeparators(fileName));
#endif
}
