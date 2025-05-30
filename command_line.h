#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include "mainwindow.h"
#include "calibration_board.h"
#include "enum_types.h"

#include <stdint.h>

#include <QString>

class CalibrationBoard;
class MainWindow;

class CommandLine : public QObject {

    Q_OBJECT

public:
    explicit CommandLine(QObject *parent = nullptr);
    ~CommandLine();

    void setMainWindow(MainWindow*);
    void getFirstData();
    void getPeriodicData();
    void getSerialNoData();
    void commandLineProcess();
    void messageBox(QString);
    void startCalibrationRequest(Request, QString);
    void startSerialNoDataRequest(Request, QString);
private:
    MainWindow *mainWindow = nullptr;

    void getCalibrationData();
    void parseCommand(QString);
    uint8_t processCommand(Command);
    uint8_t parseLineCommandInput(Command command_type);

};

#endif // COMMAND_LINE_H
