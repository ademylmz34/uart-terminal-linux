#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QLabel>

#include "log_parser.h"
#include "creator.h"
#include "serial.h"
#include "enum_types.h"

#define NUM_OF_CAL_POINTS 10
#define NUM_OF_SENSOR_BOARD 15

class CommandLine;
class CalibrationBoard;
class Serial;
class LogParser;

struct CalibrationValLabels {
    QLabel* cal_point;
    QLabel* cal_point_start_time;
    QLabel* cal_next_cal_start_duration;
    QLabel* cal_sensitivity;
    QLabel* cal_point_end_time;
    QLabel* cal_status;
    QLabel* cal_o3_average;
    QLabel* cal_zero_cal_conc;
    QLabel* cal_start_from_zero_checked;
    QLabel* cal_stabilization_duration;
    QLabel* cal_const_cal_temp;
    QLabel* cal_clean_air_duration;
};

struct MainWindowHeaderValLabels {
    QLabel* cabin_info;
    QLabel* calibration_start_date;
    QLabel* calibration_repeat_val;
    QLabel* cabin_temp_val;
    QLabel* cabin_o3_val;
    QLabel* current_date_time;
};

extern CalibrationValLabels cal_val_labels;
extern MainWindowHeaderValLabels main_window_header_labels;

extern QMap<uint8_t, QFrame*> sensor_frames;
extern QMap<uint16_t, uint32_t> sensors_serial_no;
extern QMap<uint8_t, uint8_t> sensors_eeprom_is_data_exist;

extern QDateTime current_dt;

extern QMap<Request, QString> request_commands;
extern QMap<Request, uint8_t> request_data_status;
extern QMap<CalibrationStates, QString> calibration_state_str;

extern QMap<uint8_t, QLabel*> header_labels;
extern QMap<uint8_t, QLabel*> temp_labels;
extern QMap<uint8_t, QLabel*> hum_labels;
extern QMap<uint8_t, QLabel*> r1_labels;
extern QMap<uint8_t, QLabel*> r2_labels;
extern QMap<uint8_t, QLabel*> r3_labels;

extern uint8_t sensor_module_status[NUM_OF_SENSOR_BOARD];
extern uint8_t active_sensor_count;

extern QMap<QString, uint8_t> sensor_module_map;
extern QMap<QString, QString> log_folder_names;

extern Creator file_folder_creator;

extern uint8_t is_main_folder_created;
extern uint8_t is_oml_log_folder_created;

extern uint16_t calibration_points[NUM_OF_CAL_POINTS];
extern Request current_request;
extern Request serial_no_request;

extern QString line;

extern QTimer *get_calibration_status_timer;

//extern QStringList sensors_folder;

extern Serial *serial;
extern CommandLine *command_line;
extern CalibrationBoard *calibration_board;
extern LogParser *uart_log_parser;

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
        void disableBaudCmb();
        void disableConnectionButton();
        void setLineEditText(const QString&);
        void Log2LinePlainText(const QString &);
        ~MainWindow();

    private slots:
        void onBtnClearClicked();

    private:
        Ui::MainWindow *ui;
        QTimer* current_date_time_timer;

        void setLabels();
        void sendHeartBeat();
        void aboutToExit();
    protected:
        void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H
