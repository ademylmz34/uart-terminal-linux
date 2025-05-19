#ifndef CALIBRATION_BOARD_H
#define CALIBRATION_BOARD_H

#include "mainwindow.h"
#include "serial.h"

#include <QTimer>
#include <QString>
#include <QRegularExpression>

class MainWindow;

class CalibrationBoard : public QObject {

    Q_OBJECT

public:
    explicit CalibrationBoard(QObject *parent = nullptr);
    ~CalibrationBoard();

    uint8_t createSensorFolders();
    void setMainWindow(MainWindow*);
    void startCalibrationProcess();
private:
    MainWindow *mainWindow = nullptr;
    uint8_t is_oml_log_folder_created;

    uint8_t createCalibrationFolders();
    uint8_t isArrayEmpty(const uint8_t*, size_t);
    uint8_t parseLineEditInput(const QStringList&, QStringList&);
    QStringList getSensorFolderNames();

    void checkTime();
    void getDataFromMCU();
};

#endif // CALIBRATION_BOARD_H
