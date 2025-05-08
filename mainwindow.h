#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

#include "log_parser.h"
#include "creator.h"

#define RX_BUFFER_LEN 1024

extern Creator file_folder_creator;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnClear_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort *serial;
    QTimer *connectionCheckTimer;
    QString selectedPortName;

    LogParser* uart_log_parser;
    LogParser::Packet packet;

    char uart_rx_buffer[RX_BUFFER_LEN];
    uint16_t uart_buffer_index;

    int8_t uart_line_process(char*);
    void onAppExit();
    void checkConnectionStatus();
    void Log2LinePlainText(QString);
    void connectSerial();
    void readSerial();
    void sendData();
};
#endif // MAINWINDOW_H
