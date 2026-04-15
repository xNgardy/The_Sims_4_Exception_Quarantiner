#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>

class QDragEnterEvent;
class QDropEvent;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void runQuarantine();
    void restoreQuarantine();
    void selectModsFolder();
    void selectExceptionFile();

private:
    void setExceptionFilePath(const QString &filePath);

    QLineEdit *modsPathInput;
    QLineEdit *exceptionPathInput;
    QPushButton *runButton;
    QPushButton *restoreButton;
    QPushButton *browseModsButton;
    QPushButton *browseExceptionButton;
    QCheckBox *dryRunCheckBox;
    QLabel *dropHintLabel;
};

#endif
