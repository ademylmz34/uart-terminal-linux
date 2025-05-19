#include "command_line.h"

#include <QDebug>

CommandLine::CommandLine(QObject *parent): QObject(parent)  // üst sınıfa parametre gönderimi
{

}

CommandLine::~CommandLine()
{

}

void CommandLine::setMainWindow(MainWindow *mw) {
    mainWindow = mw;
}

void CommandLine::parseCommand(QString command)
{
    Command type;
    QString command_str;
    command = command.trimmed();
    type = CMD_NONE;

    if (command.startsWith("?")) mcu_command = command;

    if (command == "?r") type = CMD_R;
    else if (command == "?sc") type = CMD_SC;
    else if (command.startsWith("?") && command.isLower()) type = CMD_SM;

    if (command.startsWith("!")) command_str = command.mid(1);

    if (command_str.startsWith("csf")) {
        sensor_numbers = command_str.split(" ", Qt::SkipEmptyParts);
        qDebug() << sensor_numbers.size();
        if (sensor_numbers.size() == 1) {
            mainWindow->setLineEditText("Sensör numaralarini giriniz.");
            return;
        }
        sensor_numbers.removeFirst();
        type = CMD_CSF;
    } else if (command_str == "gcd") type = CMD_GCD;
    else if (command_str == "gabc") type = CMD_GABC;

    processCommand(type);
}

uint8_t CommandLine::processCommand(Command command_type)
{
    uint8_t status;
    switch (command_type) {
    case CMD_CSF:
        status = calibration_board->createSensorFolders();
        if (status == 1) {
            mainWindow->setLineEditText("Sensör klasörleri başarıyla oluşturuldu.");
        } else if (status == 2){
            mainWindow->setLineEditText("Sensör klasörleri zaten var.");
        }
        break;

    case CMD_GCD:
        current_request = CAL_REPEAT_COUNT;
        request_command = cal_repeat_count_command;
        cal_repeat_count_data_received = 0;
        active_sensor_count_data_received = 0;
        cal_points_data_received = 0;
        data_received_timeout = 10;
        get_calibration_data_timer->start(1000);
        break;

    case CMD_GABC:
        current_request = ACTIVE_SENSOR_COUNT;
        request_command = active_sensor_count_command;
        active_sensor_count_data_received = 0;
        data_received_timeout = 10;
        get_calibration_data_timer->start(1000);
        break;

    case CMD_SC:
        calibration_board->startCalibrationProcess();
        break;

    case CMD_R:
        break;

    case CMD_SM:
        serial->sendData(mcu_command);
        break;
    case CMD_NONE:
        break;
    default:
        break;
    }
    return 1;
}

void CommandLine::commandLineProcess()
{
    QString command_cl = mainWindow->getLineEditText();
    mainWindow->Log2LinePlainText(command_cl);
    parseCommand(command_cl);
}
