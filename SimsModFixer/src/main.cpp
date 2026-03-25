#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    MainWindow window;
    window.setWindowTitle("Sims 4 Mod Exception Resolver");
    window.resize(550, 280);
    window.show();
    
    return app.exec();
}