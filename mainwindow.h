#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

#include "log_parser.h"
#include "creator.h"

#define RX_BUFFER_LEN 1024
#define NUM_OF_CAL_POINTS 10

enum Request {
    CAL_REPEAT_COUNT,
    ACTIVE_SENSOR_COUNT,
    CAL_POINTS,
    NONE
};

extern Creator file_folder_creator;
extern int calibrationRepeatCount;
extern int activeSensorCount;
extern int kalPoint;
extern int kalPointVal;
extern uint16_t calibrationPoints[NUM_OF_CAL_POINTS];
extern QString cal_repeat_count_command;
extern QString active_sensor_count_command;
extern QString cal_points_request_command;
extern Request currentRequest;

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
    QString line;

    LogParser* uart_log_parser;
    LogParser::Packet packet;

    char uart_rx_buffer[RX_BUFFER_LEN];
    uint16_t uart_buffer_index;

    int8_t uart_line_process(char*);
    void onTimeout();
    void getDataFromMCU(QString, Request);
    void handleReceivedData();
    void onAppExit();
    void checkConnectionStatus();
    void Log2LinePlainText(QString);
    void connectSerial();
    void readSerial();
    void sendData();
};
#endif // MAINWINDOW_H
