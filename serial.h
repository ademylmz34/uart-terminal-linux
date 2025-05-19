#ifndef SERIAL_H
#define SERIAL_H

#define RX_BUFFER_LEN 1024

#include "mainwindow.h"
#include "log_parser.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

class MainWindow;

class Serial: public QObject {
    Q_OBJECT
public:
    explicit Serial(QObject *parent = nullptr);
    ~Serial();

    void sendData(QString);
    void setMainWindow(MainWindow*);
    void connectSerial();
    void detectOm106Devices();
private:
    MainWindow *mainWindow;
    LogParser* uart_log_parser;
    LogParser::Packet packet;

    char uart_rx_buffer[RX_BUFFER_LEN];
    uint16_t uart_buffer_index;

    QSerialPort *serial;
    QSerialPort *serial_2;
    QString selected_port_name;
    QString selected_port_name_2;
    QTimer *connection_check_timer;
    QTimer *connection_check_timer_2;
    QString line;

    uint8_t uartLineProcess(char*);
    QString readBytes(QSerialPort*);
    void readSerial();
    void checkConnectionStatus();
    void checkConnectionStatus_2();
    void checkPortConnection(QSerialPort*, const QString&, int);
};

#endif // SERIAL_H
