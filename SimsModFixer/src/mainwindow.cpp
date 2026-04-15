#include "mainwindow.h"

#include <QDir>
#include <QDirIterator>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMimeData>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>
#include <QTextStream>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

namespace {

struct AnalysisResult {
    QStringList suspectFileList;
    QStringList suspectFolderList;
    QStringList foundLocations;
    QStringList filesToMoveRelative;
};

QString normalizeModHint(const QString &value) {
    QString normalized = value.toLower();
    QString result;
    result.reserve(normalized.size());

    for (QChar ch : normalized) {
        if (ch.isLetterOrNumber()) {
            result.append(ch);
        }
    }

    return result;
}

QString cleanPath(const QString &path) {
    return QDir::cleanPath(QDir::fromNativeSeparators(path));
}

bool isSupportedExceptionFile(const QString &filePath) {
    const QString lowerPath = filePath.toLower();
    return lowerPath.endsWith(".txt") || lowerPath.endsWith(".html");
}

AnalysisResult analyzeSuspects(const QString &modsPath, const QString &exceptionPath) {
    AnalysisResult result;

    QDir modsDir(modsPath);
    QDir quarantineDir(modsDir.absoluteFilePath("Quarantine_Mods"));
    const QString quarantineRoot = cleanPath(quarantineDir.absolutePath());

    QFile file(exceptionPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return result;
    }

    QTextStream in(&file);
    const QString content = in.readAll();
    file.close();

    QRegularExpression modFileRe("([a-zA-Z0-9_\\-\\s\\[\\]\\(\\)]+\\.(?:package|ts4script))");
    QRegularExpressionMatchIterator fileMatches = modFileRe.globalMatch(content);

    QSet<QString> suspectMods;
    while (fileMatches.hasNext()) {
        const QRegularExpressionMatch match = fileMatches.next();
        suspectMods.insert(match.captured(1));
    }

    QRegularExpression modFolderRe("\\.\\\\([^\\\\/:*?\"<>|\\r\\n]+)");
    QRegularExpressionMatchIterator folderMatches = modFolderRe.globalMatch(content);

    QSet<QString> suspectFolders;
    while (folderMatches.hasNext()) {
        const QRegularExpressionMatch match = folderMatches.next();
        suspectFolders.insert(match.captured(1));
    }

    result.suspectFileList = suspectMods.values();
    result.suspectFileList.sort(Qt::CaseInsensitive);

    result.suspectFolderList = suspectFolders.values();
    result.suspectFolderList.sort(Qt::CaseInsensitive);

    if (suspectMods.isEmpty() && suspectFolders.isEmpty()) {
        return result;
    }

    QSet<QString> exactMatchedFiles;
    QSet<QString> seenLocations;
    QSet<QString> filesToMoveAbsolute;
    QSet<QString> normalizedFolderHints;

    for (const QString &folderName : suspectFolders) {
        normalizedFolderHints.insert(normalizeModHint(folderName));
    }

    QDirIterator exactFileIt(modsPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (exactFileIt.hasNext()) {
        const QString filePath = exactFileIt.next();
        const QFileInfo fileInfo(filePath);

        if (cleanPath(fileInfo.absolutePath()).startsWith(quarantineRoot, Qt::CaseInsensitive)) {
            continue;
        }

        if (suspectMods.contains(fileInfo.fileName())) {
            exactMatchedFiles.insert(filePath);
        }
    }

    QDirIterator folderIt(modsPath, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (folderIt.hasNext()) {
        const QString folderPath = folderIt.next();
        const QFileInfo folderInfo(folderPath);

        if (cleanPath(folderPath).startsWith(quarantineRoot, Qt::CaseInsensitive)) {
            continue;
        }

        const QString normalizedFolderName = normalizeModHint(folderInfo.fileName());
        bool matchesHint = false;

        for (const QString &hint : normalizedFolderHints) {
            if (!hint.isEmpty() &&
                (normalizedFolderName.contains(hint) || hint.contains(normalizedFolderName))) {
                matchesHint = true;
                break;
            }
        }

        if (matchesHint && !seenLocations.contains(folderPath)) {
            seenLocations.insert(folderPath);
        }
    }

    QDirIterator clueFileIt(modsPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (clueFileIt.hasNext()) {
        const QString filePath = clueFileIt.next();
        const QFileInfo fileInfo(filePath);

        if (cleanPath(fileInfo.absolutePath()).startsWith(quarantineRoot, Qt::CaseInsensitive)) {
            continue;
        }

        const QString normalizedFileName = normalizeModHint(fileInfo.fileName());
        bool matchesHint = false;

        for (const QString &hint : normalizedFolderHints) {
            if (!hint.isEmpty() &&
                (normalizedFileName.contains(hint) || hint.contains(normalizedFileName))) {
                matchesHint = true;
                break;
            }
        }

        if (matchesHint) {
            const QString parentFolder = fileInfo.absolutePath();
            if (!seenLocations.contains(parentFolder)) {
                seenLocations.insert(parentFolder);
            }
        }
    }

    for (const QString &matchedFile : exactMatchedFiles) {
        filesToMoveAbsolute.insert(matchedFile);
    }

    QStringList locationList = seenLocations.values();
    locationList.sort(Qt::CaseInsensitive);
    result.foundLocations = locationList;

    for (const QString &location : locationList) {
        const QFileInfo locationInfo(location);
        if (locationInfo.isDir()) {
            QDirIterator locationFiles(location, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (locationFiles.hasNext()) {
                const QString filePath = locationFiles.next();
                const QFileInfo fileInfo(filePath);
                if (cleanPath(fileInfo.absolutePath()).startsWith(quarantineRoot, Qt::CaseInsensitive)) {
                    continue;
                }
                filesToMoveAbsolute.insert(filePath);
            }
        } else if (locationInfo.isFile()) {
            filesToMoveAbsolute.insert(location);
        }
    }

    QStringList relativeFiles;
    for (const QString &filePath : filesToMoveAbsolute) {
        relativeFiles << QDir::cleanPath(modsDir.relativeFilePath(filePath));
    }

    relativeFiles.sort(Qt::CaseInsensitive);
    result.filesToMoveRelative = relativeFiles;
    return result;
}

QString buildAnalysisSummary(const AnalysisResult &analysis, bool dryRun, int movedCount = 0, int skippedCount = 0) {
    QString summary;

    if (dryRun) {
        summary = QString("Dry run completed.\n\nWould move %1 files to quarantine.")
                      .arg(analysis.filesToMoveRelative.size());
    } else {
        summary = QString("Process completed.\n\nMoved %1 files to quarantine.")
                      .arg(movedCount);
        if (skippedCount > 0) {
            summary += QString("\nSkipped %1 files because a destination already existed.").arg(skippedCount);
        }
    }

    if (!analysis.suspectFileList.isEmpty()) {
        summary += "\n\nDetected mod filenames:\n- " + analysis.suspectFileList.join("\n- ");
    }

    if (!analysis.suspectFolderList.isEmpty()) {
        summary += "\n\nDetected mod folder clues:\n- " + analysis.suspectFolderList.join("\n- ");
    }

    if (!analysis.foundLocations.isEmpty()) {
        summary += "\n\nFound locations:\n- " + analysis.foundLocations.join("\n- ");
    } else if (!analysis.suspectFolderList.isEmpty()) {
        summary += "\n\nNo matching folders were found inside the selected Mods directory.";
    }

    if (!analysis.filesToMoveRelative.isEmpty()) {
        summary += dryRun ? "\n\nFiles that would be moved:\n- " : "\n\nMoved files:\n- ";
        summary += analysis.filesToMoveRelative.join("\n- ");
    }

    return summary;
}

}  // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setAcceptDrops(true);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(10);

    QLabel *introLabel = new QLabel(
        "Find suspicious Sims 4 mods from an exception report and move them into Mods/Quarantine_Mods.",
        this);
    introLabel->setWordWrap(true);

    QLabel *modsLabel = new QLabel("1. Select your Sims 4 Mods Folder:", this);
    modsPathInput = new QLineEdit(this);
    modsPathInput->setReadOnly(true);
    modsPathInput->setPlaceholderText("Choose your The Sims 4/Mods folder");
    browseModsButton = new QPushButton("Browse Mods Folder", this);

    QLabel *exceptionLabel = new QLabel("2. Select or drop the Exception File (.txt or .html):", this);
    exceptionPathInput = new QLineEdit(this);
    exceptionPathInput->setReadOnly(true);
    exceptionPathInput->setPlaceholderText("You can also drag and drop the exception file onto this window.");
    browseExceptionButton = new QPushButton("Browse Exception File", this);

    dryRunCheckBox = new QCheckBox("Preview only (dry run, do not move files)", this);
    dryRunCheckBox->setChecked(true);

    runButton = new QPushButton("Analyze And Quarantine", this);
    runButton->setMinimumHeight(40);
    runButton->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");

    restoreButton = new QPushButton("Restore Quarantined Files", this);
    restoreButton->setMinimumHeight(36);

    dropHintLabel = new QLabel(
        "Tip: drag a Better Exceptions .txt or .html file anywhere onto this window.",
        this);
    dropHintLabel->setWordWrap(true);

    QFrame *actionsDivider = new QFrame(this);
    actionsDivider->setFrameShape(QFrame::HLine);
    actionsDivider->setFrameShadow(QFrame::Sunken);

    QHBoxLayout *actionButtonsLayout = new QHBoxLayout();
    actionButtonsLayout->setSpacing(10);
    actionButtonsLayout->addWidget(runButton, 2);
    actionButtonsLayout->addWidget(restoreButton, 1);

    layout->addWidget(introLabel);
    layout->addSpacing(4);
    layout->addWidget(modsLabel);
    layout->addWidget(modsPathInput);
    layout->addWidget(browseModsButton);
    layout->addSpacing(15);
    layout->addWidget(exceptionLabel);
    layout->addWidget(exceptionPathInput);
    layout->addWidget(browseExceptionButton);
    layout->addWidget(dryRunCheckBox);
    layout->addSpacing(6);
    layout->addWidget(actionsDivider);
    layout->addLayout(actionButtonsLayout);
    layout->addSpacing(10);
    layout->addWidget(dropHintLabel);

    setCentralWidget(centralWidget);

    connect(browseModsButton, &QPushButton::clicked, this, &MainWindow::selectModsFolder);
    connect(browseExceptionButton, &QPushButton::clicked, this, &MainWindow::selectExceptionFile);
    connect(runButton, &QPushButton::clicked, this, &MainWindow::runQuarantine);
    connect(restoreButton, &QPushButton::clicked, this, &MainWindow::restoreQuarantine);
}

MainWindow::~MainWindow() {}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty() && urls.first().isLocalFile() && isSupportedExceptionFile(urls.first().toLocalFile())) {
            event->acceptProposedAction();
            return;
        }
    }

    event->ignore();
}

void MainWindow::dropEvent(QDropEvent *event) {
    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty() || !urls.first().isLocalFile()) {
        event->ignore();
        return;
    }

    const QString filePath = urls.first().toLocalFile();
    if (!isSupportedExceptionFile(filePath)) {
        QMessageBox::warning(this, "Unsupported File",
                             "Please drop a .txt or .html exception file.");
        event->ignore();
        return;
    }

    setExceptionFilePath(filePath);
    event->acceptProposedAction();
}

void MainWindow::selectModsFolder() {
    const QString folderPath = QFileDialog::getExistingDirectory(this, "Select Mods Folder");
    if (!folderPath.isEmpty()) {
        modsPathInput->setText(QDir::toNativeSeparators(folderPath));
    }
}

void MainWindow::selectExceptionFile() {
    const QString filePath = QFileDialog::getOpenFileName(
        this, "Select Exception File", "", "Text and HTML files (*.txt *.html)");
    if (!filePath.isEmpty()) {
        setExceptionFilePath(filePath);
    }
}

void MainWindow::setExceptionFilePath(const QString &filePath) {
    exceptionPathInput->setText(QDir::toNativeSeparators(filePath));
}

void MainWindow::runQuarantine() {
    const QString modsPath = modsPathInput->text();
    const QString exceptionPath = exceptionPathInput->text();
    const bool dryRun = dryRunCheckBox->isChecked();

    if (modsPath.isEmpty() || exceptionPath.isEmpty()) {
        QMessageBox::warning(this, "Missing Information",
                             "Please select both the Mods folder and the exception file.");
        return;
    }

    const AnalysisResult analysis = analyzeSuspects(modsPath, exceptionPath);
    if (analysis.suspectFileList.isEmpty() && analysis.suspectFolderList.isEmpty()) {
        QMessageBox::information(this, "Result",
                                 "No specific mod filename or mod folder clue was found in the exception file.\n"
                                 "The issue might be the game itself or an untraceable mod.");
        return;
    }

    if (analysis.filesToMoveRelative.isEmpty()) {
        QMessageBox::information(this, dryRun ? "Dry Run" : "Success",
                                 buildAnalysisSummary(analysis, dryRun));
        return;
    }

    if (dryRun) {
        QMessageBox::information(this, "Dry Run", buildAnalysisSummary(analysis, true));
        return;
    }

    QDir modsDir(modsPath);
    QDir quarantineDir(modsDir.absoluteFilePath("Quarantine_Mods"));
    if (!quarantineDir.exists()) {
        quarantineDir.mkpath(".");
    }

    int movedCount = 0;
    int skippedCount = 0;

    for (const QString &relativePath : analysis.filesToMoveRelative) {
        const QString sourcePath = modsDir.absoluteFilePath(relativePath);
        const QString destinationPath = quarantineDir.absoluteFilePath(relativePath);
        const QFileInfo sourceInfo(sourcePath);
        const QFileInfo destinationInfo(destinationPath);

        if (!sourceInfo.exists()) {
            skippedCount++;
            continue;
        }

        QDir().mkpath(destinationInfo.absolutePath());

        if (QFile::exists(destinationPath)) {
            skippedCount++;
            continue;
        }

        if (QFile::rename(sourcePath, destinationPath)) {
            movedCount++;
        } else {
            skippedCount++;
        }
    }

    QMessageBox::information(this, "Success",
                             buildAnalysisSummary(analysis, false, movedCount, skippedCount));
}

void MainWindow::restoreQuarantine() {
    const QString modsPath = modsPathInput->text();
    if (modsPath.isEmpty()) {
        QMessageBox::warning(this, "Missing Information",
                             "Please select the Mods folder before restoring quarantine.");
        return;
    }

    QDir modsDir(modsPath);
    QDir quarantineDir(modsDir.absoluteFilePath("Quarantine_Mods"));

    if (!quarantineDir.exists()) {
        QMessageBox::information(this, "Restore Quarantine",
                                 "No Quarantine_Mods folder was found inside the selected Mods directory.");
        return;
    }

    QStringList quarantinedFiles;
    QDirIterator it(quarantineDir.absolutePath(), QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        quarantinedFiles << cleanPath(it.next());
    }

    quarantinedFiles.sort(Qt::CaseInsensitive);

    if (quarantinedFiles.isEmpty()) {
        QMessageBox::information(this, "Restore Quarantine",
                                 "Quarantine_Mods exists, but there are no files to restore.");
        return;
    }

    int restoredCount = 0;
    int skippedCount = 0;
    QStringList restoredFiles;

    for (const QString &filePath : quarantinedFiles) {
        const QString relativePath = QDir::cleanPath(quarantineDir.relativeFilePath(filePath));
        const QString destinationPath = modsDir.absoluteFilePath(relativePath);
        const QFileInfo destinationInfo(destinationPath);

        QDir().mkpath(destinationInfo.absolutePath());

        if (QFile::exists(destinationPath)) {
            skippedCount++;
            continue;
        }

        if (QFile::rename(filePath, destinationPath)) {
            restoredCount++;
            restoredFiles << relativePath;
        } else {
            skippedCount++;
        }
    }

    restoredFiles.sort(Qt::CaseInsensitive);

    QString summary = QString("Restore completed.\n\nRestored %1 files from quarantine.")
                          .arg(restoredCount);

    if (skippedCount > 0) {
        summary += QString("\nSkipped %1 files because a destination already existed or the move failed.")
                       .arg(skippedCount);
    }

    if (!restoredFiles.isEmpty()) {
        summary += "\n\nRestored files:\n- " + restoredFiles.join("\n- ");
    }

    QMessageBox::information(this, "Restore Quarantine", summary);
}
