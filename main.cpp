#include "mainwindow.h"
#include <exception>
#include <QApplication>

#include <csignal>

void handleCrash(int sig) {
    command_line->messageBox("Uygulama çöktü, son log dosyalarının silinmesini istiyor musunuz?");
    serial->sendData("?r");
    if (!serial->serial->waitForBytesWritten(500)) {
        qDebug() << "Veri yazılamadı!";
    } else {
        qDebug() << "Veri gönderildi!";
    }
    exit(sig);
}

int main(int argc, char *argv[])
{
    std::signal(SIGSEGV, handleCrash); // segmentation fault
    std::signal(SIGABRT, handleCrash); // abort
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
