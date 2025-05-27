#ifndef SERIAL_H
#define SERIAL_H

#include "log_parser.h"
#include "mainwindow.h"
#include "enum_types.h"

#define RX_BUFFER_LEN 1024
#define BR_115200 115200

#include "command_line.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

class CommandLine;
class MainWindow;
class LogParser;

class Serial: public QObject {
    Q_OBJECT
public:
    explicit Serial(QObject *parent = nullptr);
    ~Serial();

    QTimer *connection_check_timer;
    QSerialPort *serial;

    void sendData(QString);
    void setMainWindow(MainWindow*);
    void connectSerial();
    void detectOm106Devices();

    void checkConnectionStatus();
    void whenConnectionLost();
    void readSerial();
private:
    MainWindow *mainWindow;

    char uart_rx_buffer[RX_BUFFER_LEN];
    uint16_t uart_buffer_index;

    qint32 baud_rate;

    QString selected_port_name;
    QString line;

    uint8_t uartLineProcess(char*);
    QString readBytes(QSerialPort*);
    void checkPortConnection(QSerialPort*, const QString&, int);
};

#endif // SERIAL_H
