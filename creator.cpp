#include "creator.h"
#include "mainwindow.h"

QMap<QString, SensorFiles> sensor_map;
QMap<Om106l_Devices, QFile*> om106_map;
Creator::Creator() {
    cal_point_array_size = 0;
    root_folder = "O3_Kalibrasyon_Loglari";
}

Creator::~Creator() {

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

    for (auto file : om106_map) {
        if (file->isOpen()) {
            file->close();
        }
        delete file; // new ile açtığımız için serbest bırakmamız gerekir
    }

    om106_map.clear();
    sensor_map.clear();
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

int8_t Creator::getCalibrationPointsArraySize() {
    uint8_t i;
    size_t number_of_elements = sizeof(calibration_points) / sizeof(calibration_points[0]);
    for (i = 0; i < number_of_elements; i++) {
        if (calibration_points[i] == 0) continue;
        cal_point_array_size++;
    }
    return cal_point_array_size;
}

uint8_t Creator::createMainFolder() {
    root_folder = QString("O3-Kalibrasyon-Karti-Verileri");
    if (dir.exists(root_folder)) {
        qDebug() << "Ana klasör zaten var: " << root_folder;
        return 2;
    }
    if (dir.mkpath(root_folder)) {
        qDebug() << "Ana klasör oluşturuldu:" << root_folder;
    } else {
        qDebug() << "Ana klasör oluşturulamadı:" << root_folder;
        return 0;
    }
    return 1;
}

uint8_t Creator::createSensorFolder(QString folder_name) {
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
    QDateTime now = QDateTime::currentDateTime();

    time_stamp = now.toString("yyyyMMdd_HHmm");
    sensor_log_folder = QString("%1/%2/%3_logs").arg(root_folder).arg(sensor_folder).arg(time_stamp);

    QString checkPrefix = now.toString("yyyyMMdd_HH");

    QDir checkDir(root_folder + "/" + sensor_folder);

    QStringList entries = checkDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString& entry : entries) {
        if (entry.startsWith(checkPrefix)) {
            qDebug() << "Aynı saat içerisinde klasör zaten var:" << entry;
            return 2;
        }
    }

    if (dir.mkpath(sensor_log_folder)) {
        qDebug() << "Sensör log klasörü oluşturuldu:" << sensor_log_folder;
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

    if (logFile->open(QIODevice::ReadWrite | QIODevice::Text) &&
        kalFile->open(QIODevice::ReadWrite | QIODevice::Text) &&
        kalEndFile->open(QIODevice::ReadWrite | QIODevice::Text)) {

        SensorFiles s;
        s.sensor_id = sensor_id;
        s.log_file = logFile;
        s.kal_log_file = kalFile;
        s.kal_end_log_file = kalEndFile;

        s.log_stream = new QTextStream(logFile);
        s.kal_stream = new QTextStream(kalFile);
        s.kal_end_stream = new QTextStream(kalEndFile);

        sensor_map.insert(sensor_id, s);
        qDebug() << sensor_id << ": tüm dosyalar başarıyla oluşturuldu.";

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
    om106l_folder = QString("%1/om106Logs").arg(root_folder);
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
    QDateTime now = QDateTime::currentDateTime();

    time_stamp = now.toString("yyyyMMdd_HHmm");
    om106_log_folder = QString("%1/%2_logs").arg(om106l_folder).arg(time_stamp);

    QString checkPrefix = now.toString("yyyyMMdd_HH");

    QDir checkDir(root_folder + "/om106Logs");

    QStringList entries = checkDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString& entry : entries) {
        qDebug() << entry;
        if (entry.startsWith(checkPrefix)) {
            qDebug() << "Aynı saat içerisinde klasör zaten var:" << entry;
            return 2;
        }
    }

    if (dir.mkpath(om106_log_folder)) {
        qDebug() << "Om106 log klasörü oluşturuldu:" << om106_log_folder;
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

    return 1;
}

uint8_t Creator::createOm106LogFiles(QString folder_name) {
    for (int i = 0; i < NUM_OF_OM106L_DEVICE; i++) {
        if (om106l_device_status[i]) {
            QString path = QString("%1/kabin-%2_omlogs.txt").arg(folder_name).arg(i + 1);
            QFile* file = new QFile(path);

            if (file->open(QIODevice::ReadWrite)) {
                if (i == DEVICE_1) {
                    om106_map.insert(DEVICE_1, file);
                } else if (i == DEVICE_2) {
                    om106_map.insert(DEVICE_2, file);
                }
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

int8_t Creator::createFilesFolders() {
    uint16_t kal_points[5] = {20, 50, 100, 200, 500};

    time_stamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");

    // Log dosyasını oluştur
    log_file_path = root_folder + "/log.txt";
    log_file.setFileName(log_file_path);
    if (!log_file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "Log dosyası oluşturulamadı.";
    }

    log_stream.setDevice(&log_file);

    for (int i = 0; i < calibration_repeat_count; ++i) {
        sub_folder = QString("%1/tekrar_%2").arg(root_folder).arg(i + 1);
        if (dir.mkpath(sub_folder)) {
            qDebug() << "Alt klasör oluşturuldu:" << sub_folder;
        } else {
            qDebug() << "Alt klasör oluşturulamadı:" << sub_folder;
        }
        QVector<QFile*> sensors;
        QVector<QTextStream*> sensor_streams;

        // Sensor dosyaları
        for (int j = 0; j < active_sensor_count; ++j) {
            QFile* sensor_file = new QFile(QString("%1/sensor-%2.txt").arg(sub_folder).arg(j + 1));
            if (sensor_file->open(QIODevice::ReadWrite | QIODevice::Text)) {
                QTextStream* sensor_stream = new QTextStream(sensor_file);

                sensors.append(sensor_file);
                sensor_streams.append(sensor_stream);
                qDebug() << QString("Sensor-%1 dosyası oluşturuldu.").arg(j + 1);
            } else {
                delete sensor_file;
                qDebug() << QString("Sensor-%1 dosyası oluşturulamadı.").arg(j + 1);
            }
        }
        sensor_files.append(sensors);
        sensor_streams.append(sensor_streams);

        // --- om106log dosyası ---
        QString om106Path = sub_folder + "/om106log.txt";
        QFile* om106 = new QFile(om106Path);
        if (om106->open(QIODevice::ReadWrite | QIODevice::Text)) {
            QTextStream* omStream = new QTextStream(om106);
            om106_log_files.append(om106);
            om106_log_streams.append(omStream);
        }

        QVector<QFile*> calpoints;
        QVector<QTextStream*> calpointstreams;
        getCalibrationPointsArraySize();
        // Kalibrasyon klasörü
        kal_folder = sub_folder + "/Kalibrasyon_Noktalari";
        if (!dir.mkpath(kal_folder)) {
            qDebug() << "Kalibrasyon klasörü oluşturulamadı:" << kal_folder;
        } else {
            for (int k = 0; k < cal_point_array_size; ++k) {
                QFile* kal_file = new QFile(QString("%1/%2ppb_kalibrasyonu.txt").arg(kal_folder).arg(kal_points[k]));
                if (kal_file->open(QIODevice::ReadWrite | QIODevice::Text)) {
                    QTextStream* kal_stream = new QTextStream(kal_file);

                    calpoints.append(kal_file);
                    calpointstreams.append(kal_stream);
                    qDebug() << QString("%1 ppb kalibrasyon dosyası oluşturuldu.").arg(kal_points[k]);
                } else {
                    delete kal_file;
                    qDebug() << QString("%1 ppb kalibrasyon dosyası oluşturulamadı.").arg(kal_points[k]);
                }
            }
            kal_files.append(calpoints);
            kal_streams.append(calpointstreams);
            cal_point_array_size = 0;
        }
    }
    return 1;
}
