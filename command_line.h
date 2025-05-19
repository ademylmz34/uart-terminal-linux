#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include "mainwindow.h"
#include "calibration_board.h"

#include <stdint.h>

#include <QString>

class CalibrationBoard;

enum Command {
    CMD_CSF,
    CMD_CSLF,
    CMD_COMF,
    CMD_COMLF,
    CMD_GCD,
    CMD_GABC,
    CMD_R,
    CMD_SC,
    CMD_SM,
    CMD_NONE
};

class CommandLine : public QObject {

    Q_OBJECT

public:
    explicit CommandLine(QObject *parent = nullptr);
    ~CommandLine();

    void setMainWindow(MainWindow*);
    void commandLineProcess();

private:
    MainWindow *mainWindow = nullptr;

    void parseCommand(QString);
    uint8_t processCommand(Command);
};

#endif // COMMAND_LINE_H
