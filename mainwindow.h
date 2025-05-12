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
extern int calibration_repeat_count;

extern int active_sensor_count;
extern int kal_point;
extern int kal_point_val;
extern uint8_t cal_repeat_count_data_received;
extern uint8_t active_sensor_count_data_received;
extern uint8_t cal_points_data_received;

extern uint16_t calibration_points[NUM_OF_CAL_POINTS];
extern QString cal_repeat_count_command;
extern QString active_sensor_count_command;
extern QString cal_points_request_command;
extern Request current_request;

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
    void onBtnClearClicked();

private:
    Ui::MainWindow *ui;
    QSerialPort *serial;
    QTimer *connection_check_timer;
    QTimer *time_check;
    QString selected_port_name;
    QString line;

    LogParser* uart_log_parser;
    LogParser::Packet packet;

    char uart_rx_buffer[RX_BUFFER_LEN];
    uint16_t uart_buffer_index;
    uint8_t data_received_time;

    int8_t uartLineProcess(char*);
    void onTimeout();
    void getDataFromMCU();
    void onAppExit();
    void checkTime();
    void checkConnectionStatus();
    void Log2LinePlainText(QString);
    void connectSerial();
    void readSerial();
    void sendData();
};
#endif // MAINWINDOW_H
