#include "mainwindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDir>
#include <QDirIterator>
#include <QSet>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // Create central widget and layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    // Mods Folder UI
    QLabel *modsLabel = new QLabel("1. Select your Sims 4 Mods Folder:", this);
    modsPathInput = new QLineEdit(this);
    modsPathInput->setReadOnly(true);
    browseModsButton = new QPushButton("Browse Mods Folder", this);

    // Exception File UI
    QLabel *exceptionLabel = new QLabel("2. Select the Exception File (.txt or .html):", this);
    exceptionPathInput = new QLineEdit(this);
    exceptionPathInput->setReadOnly(true);
    browseExceptionButton = new QPushButton("Browse Exception File", this);

    // Run Button
    runButton = new QPushButton("RUN QUARANTINE", this);
    runButton->setMinimumHeight(40);
    runButton->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");

    // Add widgets to layout
    layout->addWidget(modsLabel);
    layout->addWidget(modsPathInput);
    layout->addWidget(browseModsButton);
    layout->addSpacing(15);
    layout->addWidget(exceptionLabel);
    layout->addWidget(exceptionPathInput);
    layout->addWidget(browseExceptionButton);
    layout->addSpacing(20);
    layout->addWidget(runButton);

    setCentralWidget(centralWidget);

    // Connect buttons to functions
    connect(browseModsButton, &QPushButton::clicked, this, &MainWindow::selectModsFolder);
    connect(browseExceptionButton, &QPushButton::clicked, this, &MainWindow::selectExceptionFile);
    connect(runButton, &QPushButton::clicked, this, &MainWindow::runQuarantine);
}

MainWindow::~MainWindow() {}

void MainWindow::selectModsFolder() {
    QString folderPath = QFileDialog::getExistingDirectory(this, "Select Mods Folder");
    if (!folderPath.isEmpty()) {
        modsPathInput->setText(folderPath);
    }
}

void MainWindow::selectExceptionFile() {
    QString filePath = QFileDialog::getOpenFileName(this, "Select Exception File", "", "Text and HTML files (*.txt *.html)");
    if (!filePath.isEmpty()) {
        exceptionPathInput->setText(filePath);
    }
}

void MainWindow::runQuarantine() {
    QString modsPath = modsPathInput->text();
    QString exceptionPath = exceptionPathInput->text();

    if (modsPath.isEmpty() || exceptionPath.isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Please select both the Mods folder and the exception file.");
        return;
    }

    // Determine the quarantine directory (one level up from Mods folder)
    QDir modsDir(modsPath);
    QDir quarantineDir(modsDir.absoluteFilePath("../Quarantine_Mods"));

    // Create the directory if it does not exist
    if (!quarantineDir.exists()) {
        quarantineDir.mkpath(".");
    }

    // Read the exception file
    QFile file(exceptionPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Cannot read the exception file.");
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // Regex to find .package or .ts4script extensions
    QRegularExpression re("([a-zA-Z0-9_\\-\\s\\[\\]\\(\\)]+\\.(?:package|ts4script))");
    QRegularExpressionMatchIterator i = re.globalMatch(content);

    // Use QSet to store unique suspect mods
    QSet<QString> suspectMods;
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        suspectMods.insert(match.captured(1));
    }

    if (suspectMods.isEmpty()) {
        QMessageBox::information(this, "Result", "No specific mod name found in the exception file. The issue might be the game itself or an untraceable mod.");
        return;
    }

    int movedCount = 0;
    
    // Iterate through the Mods folder including subdirectories
    QDirIterator it(modsPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    
    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);
        
        // Check if the current file is in our list of suspect mods
        if (suspectMods.contains(fileInfo.fileName())) {
            QString destination = quarantineDir.absoluteFilePath(fileInfo.fileName());
            
            // Move the file
            if (QFile::rename(filePath, destination)) {
                movedCount++;
            }
        }
    }

    QMessageBox::information(this, "Success", QString("Process completed successfully.\nMoved %1 files to quarantine.").arg(movedCount));
}