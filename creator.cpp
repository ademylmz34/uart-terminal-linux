#include "creator.h"
#include "mainwindow.h"

Creator::Creator() {
    cal_point_array_size = 0;
}

Creator::~Creator() {

    for (auto& dosyalar : sensor_files)
        for (auto* f : dosyalar) { f->close(); delete f; }

    for (auto& streamler : sensor_streams)
        for (auto* s : streamler) { delete s; }

    for (auto& dosyalar : kal_files)
        for (auto* f : dosyalar) { f->close(); delete f; }

    for (auto& streamler : kal_streams)
        for (auto* s : streamler) { delete s; }

    for (auto* f : om106_log_files) { f->close(); delete f; }
    for (auto* s : om106_log_streams) { delete s; }

    log_file.close();

    sensor_files.clear();
    sensor_streams.clear();
    kal_files.clear();
    kal_streams.clear();
    om106_log_files.clear();
    om106_log_streams.clear();
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

int8_t Creator::createFilesFolders() {
    uint16_t kal_points[5] = {20, 50, 100, 200, 500};

    time_stamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    root_folder = QString("logs_%1").arg(time_stamp);

    if (dir.mkpath(root_folder)) {
        qDebug() << "Ana klasör oluşturuldu:" << root_folder;
    } else {
        qDebug() << "Ana klasör oluşturulamadı:" << root_folder;
        return -1;
    }

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
