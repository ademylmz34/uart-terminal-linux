#include "mainwindow.h"
#include "calibration_board.h"
#include "log_parser.h"

#include <QDebug>

QString mcu_command;
QStringList command_line_parameters;
QStringList sensor_ids;

QTimer *get_calibration_data_timer;

uint8_t data_received_timeout;
uint8_t is_calibration_folders_created;

QMap<QString, uint8_t> sensor_folder_create_status;
QMap<QString, uint8_t> sensor_log_folder_create_status;

QMap<CalibrationStates, QString> calibration_state_str = {
    { WAIT_STATE, "WAIT STATE" },
    { CLEAN_AIR_STATE, "CLEAN AIR STATE" },
    { SET_ENVIRONMENT_CONDITIONS_STATE, "SET ENVIRONMENT CONDITIONS STATE" },
    { ZERO_CALIBRATION_STATE, "ZERO CALIBRATION STATE"},
    { SPAN_CALIBRATION_START_STATE, "SPAN CALIBRATION START STATE" },
    { SPAN_CALIBRATION_MID_STATE, "SPAN CALIBRATION MID STATE" },
    { SPAN_CALIBRATION_END_STATE, "SPAN CALIBRATION END STATE" },
    { RETURN_TO_ZERO_STATE, "RETURN TO ZERO STATE"},
    { REPEAT_CALIBRATION_STATE, "REPEAT CALIBRATION STATE"},
    { END_STATE, "END STATE"}
};

QVector<Request_t> request_sequence = {
    { R_ACTIVE_SENSOR_COUNT, R_SENSOR_ID,         "?gpa", IDLE },
    { R_SENSOR_ID,           R_CABIN_INFO,        "?gpc", IDLE },
    { R_CABIN_INFO,          R_RESISTANCE_VALUES, "?gpi", IDLE },
    { R_RESISTANCE_VALUES,   R_SENSOR_VALUES,     "?gpb", IDLE },
    { R_SENSOR_VALUES,       R_CAL_STATUS,        "?gps", IDLE },
    { R_CAL_STATUS,          NONE,                "?gpk", IDLE }
};

CalibrationBoard::CalibrationBoard(QObject *parent): QObject(parent)  // üst sınıfa parametre gönderimi
{
    get_calibration_data_timer = new QTimer(this);

    connect(get_calibration_data_timer, &QTimer::timeout, this, &CalibrationBoard::getCalibrationData);

    is_oml_log_folder_created = 0;
    is_calibration_folders_created = 0;

    memset(sensor_module_status, 0, NUM_OF_SENSOR_BOARD);
    /*uint8_t temp_values[15] = {1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    memcpy(sensor_module_status, temp_values, sizeof(sensor_module_status));*/
}

CalibrationBoard::~CalibrationBoard()
{
    delete get_calibration_data_timer;
}

void CalibrationBoard::setMainWindow(MainWindow *mw)
{
    mainWindow = mw;
}

uint8_t CalibrationBoard::writeLogDirectoryPaths(const QMap<QString, QString>& log_folder_names)
{
    QTextStream out(log_directory_paths_file);
    for (auto folder = log_folder_names.begin(); folder != log_folder_names.end(); ++folder) {
        out << folder.key() << "=" << folder.value() << "\n";
    }
    return 0;
}

uint8_t CalibrationBoard::readLogDirectoryPaths()
{
    QTextStream in(log_directory_paths_file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.contains('=')) {
            QStringList parts = line.split('=');
            if (parts.size() == 2) {
                log_folder_names.insert(parts[0], parts[1]);
            }
        }
    }
    if (log_folder_names.isEmpty()) return 0;
    command_line->messageBox("Yarım kalmış kalibrasyon için log klasörleri bulundu, silmek için evet butonuna tıklayın.");
    clearLogDirectoryPathsFile();
    return 1;
}

void CalibrationBoard::clearLogDirectoryPathsFile()
{
    log_directory_paths_file->close();
    log_directory_paths_file->open(QIODevice::WriteOnly | QIODevice::Text);
}

void CalibrationBoard::startCalibrationProcess()
{
    if (is_calibration_folders_created) {
        mainWindow->setLineEditText("Kalibrasyon klasörleri zaten var.");
        return;
    }
    uint8_t status;
    if (sensors_serial_no.isEmpty())
    {
        mainWindow->setLineEditText("sensörler takılı değil veya seri numaralari girilmemiş.");
        return;
    }

    status = createCalibrationFolders();
    if (status == 1) {
        is_calibration_folders_created = 1;
        mainWindow->setLineEditText("log klasörleri ve dosyalari olusturuldu, kalibrasyon basladi");
        serial->sendData(mcu_command);
    } else if (status == 2) {
        mainWindow->setLineEditText("Kalibrasyon işlemi devam ederken log klasörleri oluşturamazsınız.");
    }
}

uint8_t CalibrationBoard::createCalibrationFolders()
{
    QString sensor_id;
    uint8_t status;
    if (cal_status_t.calibration_state != WAIT_STATE) return 2;
    if (log_directory_paths_file == NULL) file_folder_creator.createLogDirectoryPathsFile();
    status = createSensorFolders();
    if (status == 0)  {
        mainWindow->setLineEditText("Sensör klasörleri oluşturulamadi.");
        return 0;
    }

    for (auto element = sensors_serial_no.constBegin(); element != sensors_serial_no.constEnd(); ++element) {
        sensor_id = element.key();
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
            mainWindow->setLineEditText("Sensör Log klasörü zaten oluşturuldu.");
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
        mainWindow->setLineEditText("Om106 log klasörü zaten oluşturuldu.");
        status = 2;
    }

    if (status == 2) return 2;
    writeLogDirectoryPaths(log_folder_names);
    return 1;
}

void CalibrationBoard::getSensorFolderNames()
{
    QRegularExpression regex("^s\\d{1,9}$");
    QStringList folder_names;

    folder_names = file_folder_creator.getFolderNames();
    for (const QString& folder_name: folder_names) {
        if (regex.match(folder_name).hasMatch()) {
            if (!sensor_ids.contains(folder_name)) sensor_ids.append(folder_name);
        }
    }
}

QString CalibrationBoard::findSensorFolderNameByValue(int sensor_no)
{
    for (auto it = sensors_serial_no.constBegin(); it != sensors_serial_no.constEnd(); ++it) {
        if (it.value() == sensor_no)
            return it.key();
    }
    return "";
}

uint8_t CalibrationBoard::createSensorFolders()
{
    QString sensor_id;
    uint8_t status;
    if (isArrayEmpty(sensor_module_status, NUM_OF_SENSOR_BOARD)) {
        mainWindow->setLineEditText("Sensör modülleri dizisi boş, önce !gabc ile kalibrasyon kartından verileri alınız."); //get active board count -> gabc
        return 0;
    } else {
        if (!sensors_serial_no.isEmpty()) {
            for (auto element = sensors_serial_no.constBegin(); element != sensors_serial_no.constEnd(); ++element) {
                sensor_id = element.key();
                //mainWindow->setLineEditText("sensör id: " + sensor_id);
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
            mainWindow->setLineEditText("sensor ids bos");
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

RequestStatus CalibrationBoard::getRequestStatus(Request request) {
    for (const Request_t& r : request_sequence) {
        if (r.current_request == request)
            return r.request_status;
    }
    return IDLE; // ya da NONE varsa
}

void CalibrationBoard::updateRequestStatus(Request request, RequestStatus new_status) {
    for (Request_t& r : request_sequence) {
        if (r.current_request == request) {
            r.request_status = new_status;
            break;
        }
    }
}

void CalibrationBoard::getDataFromMCU()
{
    if (data_received_timeout == 0 || request.request_status == RECEIVED) {
        request.request_status = IDLE;

        if (request.next_request != NONE) {
            auto next_request = std::find_if(request_sequence.begin(), request_sequence.end(), [&](const Request_t& r) { return r.current_request == request.next_request; });
            request = *next_request;
            data_received_timeout = 10;

        } else {
            get_calibration_data_timer->stop();
        }
    } else {
        serial->sendData(request.request_command);
        if (request.request_status == IDLE) request.request_status = SENT;
    }
}

void CalibrationBoard::getCalibrationData()
{
    if (data_received_timeout) {
        data_received_timeout--;
    } else if (data_received_timeout == 0) {
        data_received_timeout = 10;
    }
    getDataFromMCU();
}
