#ifndef SERIAL_H
#define SERIAL_H

#include "mainwindow.h"
#include "log_parser.h"
#include "enum_types.h"

#define RX_BUFFER_LEN 1024

#include "command_line.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

class CommandLine;
class MainWindow;

class Serial: public QObject {
    Q_OBJECT
public:
    explicit Serial(QObject *parent = nullptr);
    ~Serial();

    QTimer *connection_check_timer;
    QTimer *connection_check_timer_2;
    QSerialPort *serial;
    QSerialPort *serial_2;

    void sendData(QString);
    void setMainWindow(MainWindow*);
    void connectSerial();
    void detectOm106Devices();

    void checkConnectionStatus();
    void checkConnectionStatus_2();
    void readSerial();
private:
    MainWindow *mainWindow;
    LogParser* uart_log_parser;
    LogParser::Packet packet;

    char uart_rx_buffer[RX_BUFFER_LEN];
    uint16_t uart_buffer_index;

    qint32 baud_rate;

    QString selected_port_name;
    QString selected_port_name_2;
    QString line;

    uint8_t uartLineProcess(char*);
    QString readBytes(QSerialPort*);
    void checkPortConnection(QSerialPort*, const QString&, int);
};

#endif // SERIAL_H
