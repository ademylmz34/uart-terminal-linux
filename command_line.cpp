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

void CommandLine::startSerialNoDataRequest(Request request, QString request_cmd) {
    serial_no_request = request;
    serial_no_request_command = request_cmd;
    serial_no_data_received_timeout = 10;
    request_data_status[serial_no_request] = 0;
    get_sensors_serial_no_timer->start(1000);
}

void CommandLine::getFirstData() { startCalibrationRequest(R_ACTIVE_SENSOR_COUNT, request_commands[R_ACTIVE_SENSOR_COUNT]); }

void CommandLine::getPeriodicData() { startCalibrationRequest(R_SENSOR_VALUES, request_commands[R_SENSOR_VALUES]); }

void CommandLine::getSerialNoData() { startSerialNoDataRequest(R_SENSOR_ID, request_commands[R_SENSOR_ID]); }

uint8_t CommandLine::parseLineCommandInput(Command command_type)
{
    QSet<QString> seenIds;
    QRegularExpression regex("^s\\d{1,9}$");
    uint8_t counter = 0;

    QString serialStr;
    bool ok;
    bool isNumeric;
    int s_no;

    switch (command_type) {
        case CMD_SPN:
            sensor_ids.clear();

            if (command_line_parameters.size() != NUM_OF_SENSOR_BOARD) {
                mainWindow->setLineEditText("Geçersiz giriş: 15 adet sXXXX.. formatında değer yok.");
                return false;
            }
            for (const QString& part : command_line_parameters) {
                if ((sensor_module_status[counter] == 0 && part != "0") || (sensor_module_status[counter] == 1 && part == "0")) {
                    mainWindow->setLineEditText("Sensör kartındaki slotlara göre değerleri giriniz.");
                    return false;
                }
                if (!regex.match(part).hasMatch() && part != "0") {
                    mainWindow->setLineEditText("Geçersiz format:" + part);
                    return false;
                }
                if (part != "0") {
                    if (seenIds.contains(part)) {
                        mainWindow->setLineEditText("Aynı sensör ID birden fazla kez girildi: " + part);
                        sensor_module_map.clear();
                        return false;
                    }
                    seenIds.insert(part);
                }
                if (sensor_module_status[counter]) {
                    if (part.mid(1).trimmed().remove('0').isEmpty()) {
                        mainWindow->setLineEditText("serial_no yalnızca sıfırlardan oluşamaz.:" + part + "counter: " + QString::number(counter));
                        sensor_module_map.clear();
                        counter = 0;
                        return false;
                    }
                    //sensor_module_map.insert(part, counter + 1);
                }
                counter++;
            }
            //sensor_ids = command_line_parameters;
            return true;
            break;
        case CMD_SPNC:
            if (command_line_parameters.size() != 2) {
                mainWindow->setLineEditText("Geçersiz format. Doğru kullanım: ?spnc <1-15> sXXXX");
                return false;
            }
            s_no = command_line_parameters[0].toInt(&ok);
            if (!ok || s_no < 1 || s_no > 15) {
                mainWindow->setLineEditText("sensor_no geçersiz. 1 ile 15 arasında olmalı.");
                return false;
            }
            if (sensor_module_status[s_no - 1] == false) {
                mainWindow->setLineEditText("sensor slotu bos olmayan sensor numarasi giriniz.");
                return false;
            }
            //sensor_no = static_cast<uint16_t>(s_no);

            /*QString serialStr = parts[1];
            if (!serialStr.startsWith('s')) {
                mainWindow->setLineEditText("serial_no 's' harfiyle başlamalı.");
                return false;
            } */

            serialStr = command_line_parameters[1]; // s3105 → 3105
            if (serialStr.trimmed().remove('0').isEmpty()) {
                mainWindow->setLineEditText("serial_no yalnızca sıfırlardan oluşamaz.");
                return false;
            }
            if (serialStr.length() < 1 || serialStr.length() > 9) {
                mainWindow->setLineEditText("serial_no uzunluğu 1 ile 9 hane arasında olmalı.");
                return false;
            }
            if (sensor_module_map.contains(QString("s%1").arg(serialStr))) {
                mainWindow->setLineEditText("serial_no kullanımda farklı bir serial no giriniz." + serialStr);
                return false;
            }

            serialStr.toUInt(&isNumeric);
            if (!isNumeric) {
                mainWindow->setLineEditText("serial_no sadece rakamlardan oluşmalı.");
                return false;
            }
            return true;
            break;
        default:
            break;
    }
    return true;
}

void CommandLine::parseCommand(QString command)
{
    Command type;
    QString command_str;
    command = command.trimmed();
    type = CMD_NONE;

    if (command.startsWith("?")) {
        mcu_command = command;

        if (command == "?r") type = CMD_R;
        else if (command == "?sc") type = CMD_SC;
        else if (command.startsWith("?spn") || command.startsWith("?spnc")) {
            command_line_parameters = command.split(" ", Qt::SkipEmptyParts);
            command_line_parameters.removeFirst();
            if (command.startsWith("?spnc")) type = CMD_SPNC;
            else if (command.startsWith("?spn")) type = CMD_SPN;
        }
        else if (command.startsWith("?") && command.isLower()) type = CMD_SM;

    } else if (command.startsWith("!")) {
        command_str = command.mid(1);
        if (command_str.startsWith("csf")) type = CMD_CSF;
        else if (command_str == "gcd") type = CMD_GCD;
        else if (command_str == "gabc") type = CMD_GABC;
    }
    processCommand(type);
}

uint8_t CommandLine::processCommand(Command command_type)
{
    uint8_t status;
    switch (command_type) {
        /*case CMD_CSF:
            status = calibration_board->createSensorFolders();
            if (status == 1) {
                mainWindow->setLineEditText("Sensör klasörleri başarıyla oluşturuldu.");
            } else if (status == 2) {
                mainWindow->setLineEditText("Sensör klasörleri zaten var.");
            }
            break;
        */
        case CMD_GCD:
            //getCalibrationData();
            break;

        case CMD_GABC:
            //getActiveBoardCount();
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

        case CMD_SPN:
            //?spn s31 0 s32 0 s345 s0 s378 s365 s147 s2354785 s655 s12 s13 s145 s3685
            sensor_ids.clear();
            sensor_module_map.clear();
            if (parseLineCommandInput(command_type)) serial->sendData(mcu_command);
            break;

        case CMD_SPNC:
            if (parseLineCommandInput(command_type)) serial->sendData(mcu_command);
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
            calibration_board->clearLogDirectoryPathsFile();
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
}

void CommandLine::commandLineProcess()
{
    QString command_cl = mainWindow->getLineEditText();
    mainWindow->Log2LinePlainText(command_cl);
    parseCommand(command_cl);
}
