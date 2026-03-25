#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void runQuarantine();
    void selectModsFolder();
    void selectExceptionFile();

private:
    QLineEdit *modsPathInput;
    QLineEdit *exceptionPathInput;
    QPushButton *runButton;
    QPushButton *browseModsButton;
    QPushButton *browseExceptionButton;
};

#endif