#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

#include "log_parser.h"
#include "creator.h"
#include "serial.h"
#include "enum_types.h"

#define NUM_OF_CAL_POINTS 10
#define NUM_OF_OM106L_DEVICE 2
#define NUM_OF_SENSOR_BOARD 15

class CommandLine;
class CalibrationBoard;
class Serial;

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

struct CalibrationStatus {
    float o3_average;
    uint16_t calibration_ppb;
    uint16_t calibration_state;
    uint16_t calibration_duration;
    uint16_t stabilization_timer;
    uint16_t repeat_calibration;
    uint16_t pwm_duty_cycle;
    uint16_t pwm_period;
};

extern CalibrationStatus cal_status_t;

extern QDateTime calibration_start_dt;
extern QDateTime calibration_end_dt;
extern QDateTime calibration_ppb_start_dt;
extern QDateTime calibration_ppb_end_dt;

extern QDateTime current_dt;

extern int cal_ppb_cal_time;

extern QMap<Request, QString> request_commands;
extern QMap<Request, uint8_t> request_data_status;
extern QMap<CalibrationStates, QString> calibration_state_str;

extern uint8_t om106l_device_status[NUM_OF_OM106L_DEVICE];
extern uint8_t sensor_module_status[NUM_OF_SENSOR_BOARD];
extern uint8_t active_sensor_count;

extern QMap<uint16_t, SensorFiles> sensor_map;
extern QMap<Om106l_Devices, Om106Files> om106_map;
extern QMap<QString, uint8_t> sensor_folder_create_status;
extern QMap<QString, uint8_t> sensor_log_folder_create_status;
extern QMap<QString, uint8_t> sensor_module_map;
extern QMap<QString, QString> log_folder_names;

extern QFile* main_log_file;
extern QTextStream* main_log_stream;

extern QFile* calibration_log_file;
extern QTextStream* calibration_stream;

extern Creator file_folder_creator;
extern int calibration_repeat_count;

extern uint8_t is_main_folder_created;

extern int kal_point;
extern int kal_point_val;
extern uint8_t cal_repeat_count_data_received;
extern uint8_t active_sensor_count_data_received;
extern uint8_t cal_points_data_received;

extern uint16_t calibration_points[NUM_OF_CAL_POINTS];
extern Request current_request;

extern QString request_command;
extern QStringList sensor_numbers;
extern QString mcu_command;

extern QStringList sensor_ids;

extern QTimer *get_calibration_data_timer;
extern QTimer *get_calibration_status_timer;
extern QTimer *mcu_uart_connection_status_timer;

extern uint8_t data_received_timeout;
extern uint8_t is_calibration_folders_created;
//extern QStringList sensors_folder;

extern Serial *serial;
extern CommandLine *command_line;
extern CalibrationBoard *calibration_board;

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE
class CommandLine;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(QWidget *parent = nullptr);
        int getCmbBaudRateValue();

        QString getLineEditText() const;
        void setLineEditText(const QString&);
        void Log2LinePlainText(const QString &);
        ~MainWindow();

    private slots:
        void onBtnClearClicked();

    private:
        Ui::MainWindow *ui;

        void sendHeartBeat();
        void aboutToExit();
    protected:
        void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H
