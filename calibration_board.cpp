#include "mainwindow.h"
#include "calibration_board.h"

#include <QDebug>

QString request_command;
QStringList sensor_numbers;
QString mcu_command;

QTimer *get_calibration_data_timer;
uint8_t data_received_timeout;
uint8_t is_calibration_folders_created;

CalibrationBoard::CalibrationBoard(QObject *parent): QObject(parent)  // üst sınıfa parametre gönderimi
{
    get_calibration_data_timer = new QTimer(this);
    is_oml_log_folder_created = 0;
    is_calibration_folders_created = 0;
    connect(get_calibration_data_timer, &QTimer::timeout, this, &CalibrationBoard::checkTime);

    memset(sensor_module_status, 0, NUM_OF_SENSOR_BOARD);
    cal_repeat_count_command = "?gpr";
    active_sensor_count_command = "?gpa";
    cal_points_request_command = "?gpd";
    uint8_t temp_values[15] = {1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

    memcpy(sensor_module_status, temp_values, sizeof(sensor_module_status));

}

CalibrationBoard::~CalibrationBoard()
{

}

void CalibrationBoard::setMainWindow(MainWindow *mw) {
    mainWindow = mw;
}

void CalibrationBoard::startCalibrationProcess()
{
    uint8_t status = createCalibrationFolders();
    if (status == 1) {
        is_calibration_folders_created = 1;
        mainWindow->setLineEditText("log klasörleri ve dosyalari olusturuldu, kalibrasyon basladi");
        //serial->sendData(mcu_command);
    } else if (status == 2) {
        mainWindow->setLineEditText("log klasörleri ve dosyalari zaten oluşturulmuştu, Uygulama çalışırken yalnızca bir kez oluşturulabilir.");
    }
}

uint8_t CalibrationBoard::createCalibrationFolders()
{
    QStringList sensors_folders;
    uint8_t status;
    sensors_folders = getSensorFolderNames();
    for (const QString& sensor_id: sensors_folders) {
        if (!sensor_log_folder_create_status[sensor_id]) {
            status = file_folder_creator.createSensorLogFolder(sensor_id);
            if (status == 1) {
                mainWindow->setLineEditText(sensor_id + " log klasörü oluşturuldu.");
                sensor_log_folder_create_status[sensor_id] = 1;
            } else if (status == 2) {
                mainWindow->setLineEditText(sensor_id + " log klasörü zaten var.");
            } else {
                mainWindow->setLineEditText(sensor_id + " log klasörü oluşturulamadi.");
                sensor_log_folder_create_status[sensor_id] = 0;
            }
        } else {
            mainWindow->setLineEditText("Uygulama çalışırken yalnızca bir kez sensör log klasörü oluşturulabilir.");
        }

    }


    if (!is_oml_log_folder_created) {
        status = file_folder_creator.createOm106LogFolder();
        if (status == 1) {
            mainWindow->setLineEditText("om106 log klasörü oluşturuldu.");
            is_oml_log_folder_created = 1;
        } else if (status == 2) {
            mainWindow->setLineEditText("om106 log klasörü zaten var.");
        } else {
            mainWindow->setLineEditText("om106 log klasörü oluşturulamadi.");
            is_oml_log_folder_created = 0;
        }
    } else {
        mainWindow->setLineEditText("Uygulama çalışırken yalnızca bir kez om106 log klasörü oluşturulabilir.");
        status = 2;
    }

    if (status == 2) return 2;
    return 1;
}

QStringList CalibrationBoard::getSensorFolderNames()
{
    uint8_t  sensor_counter;
    QRegularExpression regex("^s\\d{4}$");
    QStringList folder_names;
    QStringList sensors_folders;

    folder_names = file_folder_creator.getFolderNames();
    for (const QString& folder_name: folder_names) {
        if (regex.match(folder_name).hasMatch()) {
            sensors_folders << folder_name;
            if (sensor_module_status[sensor_counter]) {
                sensor_module_map.insert(folder_name,  sensor_counter + 1);
            } else  {
                sensor_module_map.insert(folder_name,  sensor_counter + 2);
            }
        }
        sensor_counter++;
    }
    return sensors_folders;
}


uint8_t CalibrationBoard::createSensorFolders()
{
    uint8_t status;
    if (isArrayEmpty(sensor_module_status, NUM_OF_SENSOR_BOARD)) {
        mainWindow->setLineEditText("Sensör modülleri dizisi boş, önce !gabc ile kalibrasyon kartından verileri alınız."); //get active board count -> gabc
        return 0;
    } else {
        parseLineEditInput(sensor_numbers, sensor_ids);
        if (!sensor_ids.isEmpty()) {
            for (const QString& sensor_id: sensor_ids) {
                status = file_folder_creator.createSensorFolder(sensor_id);
                if (status == 1) {
                    mainWindow->setLineEditText(sensor_id + " klasörü oluşturuldu.");
                    sensor_folder_create_status.insert(sensor_id, 1);
                } else if (status == 2) {
                    mainWindow->setLineEditText(sensor_id + " klasörü zaten var.");
                    sensor_folder_create_status.insert(sensor_id, 1);
                } else {
                    mainWindow->setLineEditText(sensor_id + " klasörü oluşturulumadi.");
                    sensor_folder_create_status.insert(sensor_id, 0);
                }
            }
        } else {
            mainWindow->setLineEditText("?!csf komutu ile sensör numaralarını girmeniz gerekmektedir.");
            return 0;
        }
    }
    if (status == 2) return 2;
    return 1;
}


uint8_t CalibrationBoard::isArrayEmpty(const uint8_t* arr, size_t len)
{
    static const uint8_t zero_block[32] = {0}; // max karşılaştırma için buffer
    if (len <= sizeof(zero_block)) {
        return memcmp(arr, zero_block, len) == 0;
    }

    for (size_t i = 0; i < len; i++) {
        if (arr[i] != 0) {
            return 0;
        }
    }
    return 1;
}

uint8_t CalibrationBoard::parseLineEditInput(const QStringList& inputList, QStringList& outputList)
{
    outputList.clear();

    // 15 değer var mı?
    if (inputList.size() != NUM_OF_SENSOR_BOARD) {
        mainWindow->setLineEditText("Geçersiz giriş: 15 adet sXXXX formatında değer yok.");
        return 0;
    }

    // Her bir değeri kontrol et
    uint8_t counter = 0;
    QRegularExpression regex("^s\\d{4}$");
    for (const QString& part : inputList) {
        if ((sensor_module_status[counter] == 0 && part != "0") || (sensor_module_status[counter] == 1 && part == "0")) {
            mainWindow->setLineEditText("Sensör kartındaki slotlara göre değerleri giriniz.");
            return 0;
        }
        if (!regex.match(part).hasMatch() && part != "0") {
            mainWindow->setLineEditText("Geçersiz format:" + part);
            return 0;
        }
        if (sensor_module_status[counter]) {
            sensor_module_map.insert(part, counter + 1);
        }
        counter++;
    }

    outputList = inputList;

    qDebug() << "Giriş geçerli.";
    return 1;
}

void CalibrationBoard::getDataFromMCU()
{
    switch (current_request) {
    case CAL_REPEAT_COUNT:
        if (data_received_timeout == 0) {
            qDebug() << "Calibration repeat count data couldn't get received";
            current_request = ACTIVE_SENSOR_COUNT;
            request_command = active_sensor_count_command;
        } else if (cal_repeat_count_data_received) {
            qDebug() << "Calibration repeat count data get received";
            current_request = ACTIVE_SENSOR_COUNT;
            data_received_timeout = 10;
            request_command = active_sensor_count_command;
        } else {
            serial->sendData(request_command);
        }
        break;

    case ACTIVE_SENSOR_COUNT:
        if (data_received_timeout == 0) {
            qDebug() << "Active sensor count data couldn't get received";
            current_request = NONE;
            request_command = "";
        } else if (active_sensor_count_data_received) {
            qDebug() << "Active sensor count data get received";
            current_request = NONE;
            request_command = "";
            //data_received_time = 10;
        } else {
            serial->sendData(request_command);
        }
        break;

    case CAL_POINTS:
        if (data_received_timeout == 0) {
            qDebug() << "Calibration points data couldn't get received";
            current_request = NONE;
            request_command = "";
        } else if (cal_points_data_received) {
            qDebug() << "Calibration points data get received";
            current_request = NONE;
            request_command = "";
        } else {
            serial->sendData(request_command);
        }
        break;

    default:
        get_calibration_data_timer->stop();
        break;
    }
}

void CalibrationBoard::checkTime()
{
    if (data_received_timeout) {
        data_received_timeout--;
    } else if (data_received_timeout == 0) {
        data_received_timeout = 10;
    }
    if (current_request != NONE) {
        getDataFromMCU();
    }
}

