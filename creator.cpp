#include "creator.h"
#include "mainwindow.h"

QMap<uint16_t, SensorFiles> sensor_map;
QMap<Om106l_Devices, Om106Files> om106_map;

QFile* main_log_file = NULL;
QTextStream* main_log_stream = NULL;

QFile* calibration_log_file = NULL;
QTextStream* calibration_stream = NULL;

QFile* log_directory_paths_file = NULL;

uint8_t om106l_device_status[NUM_OF_OM106L_DEVICE];

Creator::Creator() {
    cal_point_array_size = 0;
    is_main_log_file_created = 0;
    is_calibration_file_created = 0;
    root_folder = "O3_Kalibrasyon_Loglari-5";
    om106l_folder = QString("%1/om106Logs").arg(root_folder);
    log_directory_file_path = QString("%1/log_directory_paths.txt").arg(root_folder);
    log_directory_paths_file = new QFile(log_directory_file_path);
    if (QFile::exists(log_directory_file_path)) log_directory_paths_file->open(QIODevice::ReadWrite | QIODevice::Text);
}

Creator::~Creator() {
    freeFiles();
}

void Creator::freeFiles() {
    if (is_calibration_folders_created) {
        is_oml_log_folder_created = 0;
        is_calibration_folders_created = 0;
        is_main_log_file_created = 0;
        is_calibration_file_created = 0;
        for (auto& s : sensor_map) {
            if (s.log_file) {
                if (s.log_file->isOpen()) s.log_file->close();
                delete s.log_file;
            }
            if (s.kal_log_file) {
                if (s.kal_log_file->isOpen()) s.kal_log_file->close();
                delete s.kal_log_file;
            }
            if (s.kal_end_log_file) {
                if (s.kal_end_log_file->isOpen()) s.kal_end_log_file->close();
                delete s.kal_end_log_file;
            }

            delete s.log_stream;
            delete s.kal_stream;
            delete s.kal_end_stream;
        }

        for (auto&om : om106_map) {
            if (om.om106_log_file) {
                if (om.om106_log_file->isOpen()) om.om106_log_file->close();
                delete om.om106_log_file;
            }
            delete om.om106_stream;
        }

        if (main_log_file->isOpen()) {
            main_log_file->close();
        }

        if (main_log_file != NULL) delete main_log_file;
        if (main_log_stream != NULL) {
            delete main_log_stream;
            main_log_stream = nullptr;
        }

        if (calibration_log_file->isOpen()) {
            calibration_log_file->close();
        }

        if (calibration_log_file != NULL) delete calibration_log_file;
        if (calibration_stream != NULL) delete calibration_stream;

        if (log_directory_paths_file->isOpen()) {
            log_directory_paths_file->close();
        }

        if (log_directory_paths_file != NULL) delete log_directory_paths_file;

        om106_map.clear();
        sensor_map.clear();
        log_folder_names.clear();
        sensor_log_folder_create_status.clear();
    }
}

QStringList Creator::getFolderNames() {
    QDir dizin(root_folder);
    // Sadece klasörleri al (.) ve (..) hariç
    QStringList klasorListesi = dizin.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString& folder: klasorListesi) {
        qDebug() << folder;
    }

    return klasorListesi;
}

uint8_t Creator::changeFolderName(QString older_folder_name, QString new_folder_name) {
    QDir dir;
    QString older_folder_path = QString("%1/%2").arg(root_folder).arg(older_folder_name);
    QString new_folder_path = QString("%1/%2").arg(root_folder).arg(new_folder_name);
    qDebug() << older_folder_path << ", " << new_folder_path;
    if (!dir.rename(older_folder_path, new_folder_path)) return false;
    return true;
}

int8_t Creator::getCalibrationPointsArraySize() {
    uint8_t i;
    size_t number_of_elements = sizeof(calibration_points) / sizeof(calibration_points[0]);
    for (i = 0; i < number_of_elements; i++) {
        if (calibration_points[i] == 0) continue;
        cal_point_array_size++;
    }
    return cal_point_array_size;
}

uint8_t Creator::createLogDirectoryPathsFile() {
    if (log_directory_paths_file->open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "Log dizinleri dosyasi başarıyla oluşturuldu.";
    } else {
        qDebug() << "Log dizinleri dosyasi oluşturulamadi.";
        return 0;
    }
    return 1;
}

uint8_t Creator::createMainFolder() {
    if (dir.exists(root_folder)) {
        qDebug() << "Ana klasör zaten var: " << root_folder;
        return 2;
    }
    if (dir.mkpath(root_folder)) {
        qDebug() << "Ana klasör oluşturuldu:" << root_folder;
        createOm106lFolder();
        createLogDirectoryPathsFile();
    } else {
        qDebug() << "Ana klasör oluşturulamadı:" << root_folder;
        return 0;
    }
    return 1;
}

uint8_t Creator::createMainLogFile(QString folder_name) {
    time_stamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmm");
    QString file_path = QString("%1/%2_log.txt").arg(folder_name).arg(time_stamp);
    if (main_log_file != NULL) {
        if (main_log_file->isOpen()) {
            main_log_file->close();
            delete main_log_file;
            delete main_log_stream;
            qDebug() << "eski log dosyası silindi";
        }
    }
    main_log_file = new QFile(file_path);
    if (QFile::exists(file_path)) {
        qDebug() << "Log dosyası zaten var.";
        return 2;
    } else {
        if (main_log_file->open(QIODevice::ReadWrite | QIODevice::Text)) {
            qDebug() << "Log dosyasi başarıyla oluşturuldu.";
            main_log_stream = new QTextStream(main_log_file);
        } else {
            qDebug() << "Log dosyasi oluşturulamadi.";
            return 0;
        }
    }
    return 1;
}

uint8_t Creator::createCalibrationLogFile(QString folder_name) {
    QString file_path = QString("%1/calibration_log.txt").arg(folder_name);
    if (calibration_log_file != NULL) {
        if (calibration_log_file->isOpen()) {
            calibration_log_file->close();
            delete calibration_log_file;
            delete calibration_stream;
            qDebug() << "eski kalibrasyon dosyası silindi";
        }
    }

    calibration_log_file = new QFile(file_path);
    if (QFile::exists(file_path)) {
        qDebug() << "Kalibrasyon log dosyası zaten var.";
        return 2;
    } else {
        if (calibration_log_file->open(QIODevice::ReadWrite | QIODevice::Text)) {
            qDebug() << "Kalibrasyon log dosyasi başarıyla oluşturuldu.";
            calibration_stream = new QTextStream(calibration_log_file);
        } else {
            qDebug() << "Kalibrasyon log dosyasi oluşturulamadi.";
            return 0;
        }
    }
    return 1;
}

uint8_t Creator::createSensorFolder(QString folder_name) {
    if (folder_name == "0") return 5;
    sensor_folder = QString("%1/" + folder_name).arg(root_folder);
    if (dir.exists(sensor_folder)) {
        qDebug() << "Sensör klasörü zaten var: " << sensor_folder;
        return 2;
    } else {
        if (dir.mkpath(sensor_folder)) {
            qDebug() << "Sensör klasörü oluşturuldu:" << sensor_folder;
        } else {
            qDebug() << "Sensör klasörü oluşturulamadı:" << sensor_folder;
            return 0;
        }
    }
    return 1;
}

uint8_t Creator::createSensorLogFolder(QString sensor_folder) {
    time_stamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmm");
    sensor_log_folder = QString("%1/%2/%3_logs").arg(root_folder).arg(sensor_folder).arg(time_stamp);

    if (dir.exists(sensor_log_folder)) {
        qDebug() << "Sensör log klasörü zaten var: " << sensor_log_folder;
        return 2;
    } else {
        if (dir.mkpath(sensor_log_folder)) {
            qDebug() << "Sensör log klasörü oluşturuldu:" << sensor_log_folder;
            log_folder_names.insert(sensor_folder, sensor_log_folder); // used for deleting log files when uart connection lost
            if (createSensorLogFiles(sensor_log_folder)) {
                qDebug() << "Sensör log klasörü dosyaları oluşturuldu:" << sensor_log_folder;
            } else {
                qDebug() << "Sensör log klasörü dosyaları oluşturulamadi:" << sensor_log_folder;
                return 0;
            }
        } else {
            qDebug() << "Sensör log klasörü oluşturulamadı:" << sensor_log_folder;
            return 0;
        }
    }
    return 1;
}

uint8_t Creator::createSensorLogFiles(QString folder_name) {
    QString sensor_id    = folder_name.split("/", Qt::SkipEmptyParts).value(1); // s3104 gibi bir değer döndürecek

    QString log_path     = folder_name + "/log.txt";
    QString kal_path     = folder_name + "/kalibrasyon.txt";
    QString kal_end_path = folder_name + "/kalibrasyon_sonu.txt";

    QFile* logFile     = new QFile(log_path);
    QFile* kalFile     = new QFile(kal_path);
    QFile* kalEndFile  = new QFile(kal_end_path);

    if (logFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate) &&
        kalFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate) &&
        kalEndFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {

        if (sensor_map[sensors_serial_no[sensor_id]].log_file != NULL) {
            if(sensor_map[sensors_serial_no[sensor_id]].log_file->isOpen()) {
                sensor_map[sensors_serial_no[sensor_id]].log_file->close();
                delete sensor_map[sensors_serial_no[sensor_id]].log_file;
                delete sensor_map[sensors_serial_no[sensor_id]].log_stream;
                qDebug() << "eski sensör " << sensors_serial_no[sensor_id] << " " << " log file dosyası silindi";
            }
        }

        if (sensor_map[sensors_serial_no[sensor_id]].kal_log_file != NULL) {
            if(sensor_map[sensors_serial_no[sensor_id]].kal_log_file->isOpen()) {
                sensor_map[sensors_serial_no[sensor_id]].kal_log_file->close();
                delete sensor_map[sensors_serial_no[sensor_id]].kal_log_file;
                sensor_map[sensors_serial_no[sensor_id]].kal_stream;
                qDebug() << "eski sensör " << sensors_serial_no[sensor_id] << " " << " kal_log_file dosyası silindi";
            }
        }

        if (sensor_map[sensors_serial_no[sensor_id]].kal_end_log_file != NULL) {
            if(sensor_map[sensors_serial_no[sensor_id]].kal_end_log_file->isOpen()) {
                sensor_map[sensors_serial_no[sensor_id]].kal_end_log_file->close();
                delete sensor_map[sensors_serial_no[sensor_id]].kal_end_log_file;
                sensor_map[sensors_serial_no[sensor_id]].kal_end_stream;
                qDebug() << "eski sensör " << sensors_serial_no[sensor_id] << " " << " kal_end_stream dosyası silindi";
            }
        }

        SensorFiles s;
        s.sensor_id = sensor_id;
        s.log_file = logFile;
        s.kal_log_file = kalFile;
        s.kal_end_log_file = kalEndFile;

        s.log_stream = new QTextStream(logFile);
        s.kal_stream = new QTextStream(kalFile);
        s.kal_end_stream = new QTextStream(kalEndFile);

        sensor_map.insert(sensors_serial_no[sensor_id], s);
        qDebug() << sensor_id << ": tüm dosyalar başarıyla oluşturuldu.: " << sensors_serial_no[sensor_id];

    } else {
        qDebug() << sensor_id << ": dosya(lar) açılamadı.";
        delete logFile;
        delete kalFile;
        delete kalEndFile;

        return 0;
    }
    return 1;

}

uint8_t Creator::createOm106lFolder() {
    if (dir.exists(om106l_folder)) {
        qDebug() << "Om106Logs klasörü zaten var: " << om106l_folder;
        return 2;
    } else {
        if (dir.mkpath(om106l_folder)) {
            qDebug() << "Om106Logs klasörü oluşturuldu:" << om106l_folder;
        } else {
            qDebug() << "Om106Logs klasörü oluşturulamadı:" << om106l_folder;
            return 0;
        }
    }
    return 1;
}

uint8_t Creator::createOm106LogFolder() {
    time_stamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmm");
    om106_log_folder = QString("%1/%2_logs").arg(om106l_folder).arg(time_stamp);
    if (dir.exists(om106_log_folder)) {
        qDebug() << "Om106 log klasörü zaten var: " << om106_log_folder;
        return 2;
    } else {
        if (dir.mkpath(om106_log_folder)) {
            qDebug() << "Om106 log klasörü oluşturuldu:" << om106_log_folder;
            log_folder_names.insert("om106Logs", om106_log_folder); // used for deleting log files when uart connection lost
            if (createOm106LogFiles(om106_log_folder)) {
                qDebug() << "Om106 log klasörü dosyaları oluşturuldu:" << om106_log_folder;
            } else {
                qDebug() << "Om106 log klasörü dosyaları oluşturulamadi:" << om106_log_folder;
                return 0;
            }
        } else {
            qDebug() << "Om106 log klasörü oluşturulamadı:" << om106_log_folder;
            return 0;
        }
    }
    return 1;
}

uint8_t Creator::createOm106LogFiles(QString folder_name) {
    uint8_t status;
    if (!is_main_log_file_created) {
        status = createMainLogFile(folder_name);
        if (status == 1) is_main_log_file_created = 1;
        else if (status == 0) is_main_log_file_created = 0;
    }

    if (!is_calibration_file_created) {
        status = createCalibrationLogFile(folder_name);
        if (status == 1) is_calibration_file_created = 1;
        else if (status == 0) is_calibration_file_created = 0;
    }
    om106l_device_status[DEVICE_1] = 1;
    om106l_device_status[DEVICE_2] = 1;
    for (int i = 1; i <= NUM_OF_OM106L_DEVICE; i++) {
        if (om106l_device_status[i]) {
            QString path = QString("%1/kabin-%2_omlogs.txt").arg(folder_name).arg(i);
            if (om106_map[DEVICE_1].om106_log_file != NULL) {
                if (om106_map[DEVICE_1].om106_log_file->isOpen()) {
                    om106_map[DEVICE_1].om106_log_file->close();
                    delete om106_map[DEVICE_1].om106_log_file;
                    delete om106_map[DEVICE_1].om106_stream;
                    qDebug() << "eski " << i << " " << "om106log dosyası silindi";
                }
            }

            QFile* file = new QFile(path);
            Om106Files om;
            if (file->open(QIODevice::ReadWrite)) {
                if (i == DEVICE_1) {
                    om.device_id = DEVICE_1;
                } else if (i == DEVICE_2) {
                    om.device_id = DEVICE_2;
                }
                om.om106_log_file = file;
                om.om106_stream = new QTextStream(file);
                om106_map.insert(om.device_id, om);
                qDebug() << "om106L cihaz-" << i + 1 << " dosyası oluşturuldu";
            } else {
                qDebug() << "om106L cihaz-" << i + 1 << " dosyası oluşturulamadi";
                delete file;
                return 0;
            }
        } else {
            qDebug() << "om106L cihaz-" << i + 1 << " bulunamadi";
            return 0;
        }
    }
    return 1;
}
