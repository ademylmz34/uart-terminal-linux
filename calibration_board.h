#ifndef CALIBRATION_BOARD_H
#define CALIBRATION_BOARD_H

#include "mainwindow.h"
#include "serial.h"
#include "enum_types.h"

#include <QTimer>
#include <QString>
#include <QRegularExpression>

class MainWindow;

extern QString request_command;
extern QString serial_no_request_command;
extern QStringList command_line_parameters;
extern QString mcu_command;

extern QTimer *get_calibration_data_timer;
extern QTimer *get_sensors_serial_no_timer;

extern uint8_t data_received_timeout;
extern uint8_t serial_no_data_received_timeout;
extern uint8_t is_calibration_folders_created;

extern QStringList sensor_ids;

extern QMap<QString, uint8_t> sensor_folder_create_status;
extern QMap<QString, uint8_t> sensor_log_folder_create_status;

class CalibrationBoard : public QObject {
    Q_OBJECT

public:
    explicit CalibrationBoard(QObject *parent = nullptr);
    ~CalibrationBoard();

    QString findSensorFolderNameByValue(int);
    uint8_t readLogDirectoryPaths();
    uint8_t createSensorFolders();
    void setMainWindow(MainWindow*);
    void startCalibrationProcess();
    void clearLogDirectoryPathsFile();
private:
    MainWindow *mainWindow = nullptr;

    uint8_t writeLogDirectoryPaths(const QMap<QString, QString>&);
    uint8_t createCalibrationFolders();
    uint8_t isArrayEmpty(const uint8_t*, size_t);
    QStringList getSensorFolderNames();

    void getCalibrationData();
    void getSensorSerialNoData();
    void getSerialNoDataFromMCU();
    void getDataFromMCU();
};

#endif // CALIBRATION_BOARD_H
