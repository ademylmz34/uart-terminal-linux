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
#define NUM_OF_OM106L_DEVICE 2
#define NUM_OF_SENSOR_BOARD 15

enum Request {
    CAL_REPEAT_COUNT,
    ACTIVE_SENSOR_COUNT,
    CAL_POINTS,
    NONE
};

enum Om106l_Devices {
    DEVICE_1,
    DEVICE_2
};

enum Command {
    CMD_CSF,
    CMD_CSLF,
    CMD_COMF,
    CMD_COMLF,
    CMD_GCD,
    CMD_GABC,
    CMD_R,
    CMD_SC,
    CMD_SM
};

enum calibration_states {
    WAIT_STATE,
    CLEAN_AIR_STATE,
    SET_ENVIRONMENT_CONDITIONS_STATE,
    ZERO_CALIBRATION_STATE,
    SPAN_CALIBRATION_START_STATE,
    SPAN_CALIBRATION_MID_STATE,
    SPAN_CALIBRATION_END_STATE,
    RETURN_TO_ZERO_STATE,
    REPEAT_CALIBRATION_STATE
};

struct SensorFiles {
    QString sensor_id;
    QFile* log_file;
    QFile* kal_log_file;
    QFile* kal_end_log_file;

    QTextStream* log_stream;
    QTextStream* kal_stream;
    QTextStream* kal_end_stream;
};

struct Om106Files {
    Om106l_Devices device_id;
    QFile* om106_log_file;
    QTextStream* om106_stream;
};

extern uint8_t om106l_device_status[NUM_OF_OM106L_DEVICE];
extern uint8_t sensor_module_status[NUM_OF_SENSOR_BOARD];
extern uint8_t active_sensor_count;

extern QMap<uint16_t, SensorFiles> sensor_map;
extern QMap<Om106l_Devices, Om106Files> om106_map;
extern QMap<QString, uint8_t> sensor_folder_create_status;
extern QMap<QString, uint8_t> sensor_log_folder_create_status;

extern QFile* main_log_file;
extern QTextStream* main_log_stream;

extern QFile* calibration_log_file;
extern QTextStream* calibration_stream;

extern Creator file_folder_creator;
extern int calibration_repeat_count;

extern calibration_states calibration_state;

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

extern QStringList sensor_ids;
//extern QStringList sensors_folder;

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
        QSerialPort *serial_2;
        QTimer *connection_check_timer;
        QTimer *connection_check_timer_2;
        QTimer *time_check;
        QString selected_port_name;
        QString selected_port_name_2;
        QString line;
        QString request_command;
        QStringList sensor_numbers;
        QString mcu_command;

        LogParser* uart_log_parser;
        LogParser::Packet packet;

        char uart_rx_buffer[RX_BUFFER_LEN];
        uint16_t uart_buffer_index;
        uint8_t data_received_time;
        uint8_t is_main_folder_created;
        uint8_t is_oml_log_folder_created;

        uint8_t uartLineProcess(char*);
        uint8_t isArrayEmpty(const uint8_t*, size_t);
        uint8_t parseLineEditInput(const QStringList&, QStringList&);
        uint8_t processCommand(Command);

        uint8_t createCalibrationFolders();
        uint8_t createSensorFolders();

        void startCalibrationProcess();

        QStringList getSensorFolderNames();

        QString readFullResponse(QSerialPort&, int);
        void detectOm106Devices();
        void checkPortConnection(QSerialPort*, const QString&, int);
        void parseCommand(QString);
        void getDataFromMCU();
        void onAppExit();
        void checkTime();
        void checkConnectionStatus();
        void checkConnectionStatus_2();
        void Log2LinePlainText(QString);
        void connectSerial();
        void readSerial();
        void commandLineProcess();
        void sendData(QString);
};
#endif // MAINWINDOW_H
