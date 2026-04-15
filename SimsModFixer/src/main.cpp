#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    MainWindow window;
    window.setWindowTitle("Sims 4 Mod Quarantine");
    window.resize(680, 380);
    window.setMinimumSize(620, 340);
    window.show();
    
    return app.exec();
}
