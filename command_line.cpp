#include "command_line.h"
#include <QMessageBox>
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

void CommandLine::startCalibrationRequest(Request request, QString request_cmd) {
    current_request = request;
    request_command = request_cmd;
    data_received_timeout = 10;
    request_data_status[current_request] = 0;
    get_calibration_data_timer->start(1000);
}

void CommandLine::getActiveBoardCount() { startCalibrationRequest(ACTIVE_SENSOR_COUNT, request_commands[ACTIVE_SENSOR_COUNT]); }

void CommandLine::getCalStatus() { startCalibrationRequest(CAL_STATUS, request_commands[CAL_STATUS]); }

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
            } else if (status == 2) {
                mainWindow->setLineEditText("Sensör klasörleri zaten var.");
            }
            break;

        case CMD_GCD:
            //getCalibrationData();
            break;

        case CMD_GABC:
            getActiveBoardCount();
            break;

        case CMD_SC:
            calibration_board->startCalibrationProcess();
            break;

        case CMD_R:
            serial->sendData(mcu_command);
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

void CommandLine::messageBox(QString message)
{
    if (cal_status_t.calibration_state == WAIT_STATE && log_folder_names.isEmpty()) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(mainWindow,
                                  "Onay",
                                  message,
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        qDebug() << "Kullanıcı EVET dedi.";
        if (!log_folder_names.isEmpty()) {
            for (const QString &key : log_folder_names.keys()) {
                QString folderPath = log_folder_names.value(key);
                QDir dir(folderPath);

                if (dir.exists()) {
                    bool success = dir.removeRecursively();
                    qDebug() << key << (success ? "silindi" : "silinemedi") << folderPath;
                } else {
                    qDebug() << key << "zaten yok" << folderPath;
                }
            }
            is_oml_log_folder_created = 0;
            is_calibration_folders_created = 0;
            sensor_log_folder_create_status.clear();
            log_folder_names.clear();
        } else {
            qDebug() << "Log klasörleri zaten oluşturulmamış";
        }
    } else if (reply == QMessageBox::No) {
        qDebug() << "Kullanıcı HAYIR dedi.";
        // işlemleri burada yap
    } else {
        QMessageBox::warning(nullptr, "Zorunlu Seçim", "Lütfen bir seçim yapın.");
    }
    get_calibration_status_timer->stop();
    mcu_uart_connection_status_timer->stop();
    mainWindow->close();
}

void CommandLine::commandLineProcess()
{
    QString command_cl = mainWindow->getLineEditText();
    mainWindow->Log2LinePlainText(command_cl);
    parseCommand(command_cl);
}
