#include "creator.h"
#include "mainwindow.h"

Creator::Creator() {
    calPointArraySize = 0;
}

Creator::~Creator() {

    for (auto& dosyalar : sensorFiles)
        for (auto* f : dosyalar) { f->close(); delete f; }

    for (auto& streamler : sensorStreams)
        for (auto* s : streamler) { delete s; }

    for (auto& dosyalar : kalFiles)
        for (auto* f : dosyalar) { f->close(); delete f; }

    for (auto& streamler : kalStreams)
        for (auto* s : streamler) { delete s; }

    for (auto* f : om106LogFiles) { f->close(); delete f; }
    for (auto* s : om106LogStreams) { delete s; }

    logFile.close();

    sensorFiles.clear();
    sensorStreams.clear();
    kalFiles.clear();
    kalStreams.clear();
    om106LogFiles.clear();
    om106LogStreams.clear();

    /*uint8_t i;
    for (i = 0; i < sensorFiles.size(); ++i) {
        if (sensorStreams[i]) {
            delete sensorStreams[i];
        }
        if (sensorFiles[i]) {
            sensorFiles[i]->close();
            delete sensorFiles[i];
        }
    }
    for (i = 0; i < kalFiles.size(); i++) {
        if (kalStreams[i]) {
            delete kalStreams[i];
        }
        if (kalFiles[i]) {
            kalFiles[i]->close();
            delete kalFiles[i];
        }
    }
    logFile.close();
    om106LogFile.close();

    sensorFiles.clear();
    sensorStreams.clear();
    kalFiles.clear();
    kalStreams.clear();*/
}

int8_t Creator::getCalibrationPointsArraySize() {
    uint8_t i;
    size_t number_of_elements = sizeof(calibrationPoints) / sizeof(calibrationPoints[0]);
    for (i = 0; i < number_of_elements; i++) {
        if (calibrationPoints[i] == 0) continue;
        calPointArraySize++;
    }
    return calPointArraySize;
}

int8_t Creator::create_files_folders() {
    uint16_t kal_points[5] = {20, 50, 100, 200, 500};

    timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    rootFolder = QString("logs_%1").arg(timestamp);

    if (dir.mkpath(rootFolder)) {
        qDebug() << "Ana klasör oluşturuldu:" << rootFolder;
    } else {
        qDebug() << "Ana klasör oluşturulamadı:" << rootFolder;
        return -1;
    }

    // Log dosyasını oluştur
    logFilePath = rootFolder + "/log.txt";
    logFile.setFileName(logFilePath);
    if (!logFile.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "Log dosyası oluşturulamadı.";
    }

    logStream.setDevice(&logFile);

    for (int i = 0; i < calibrationRepeatCount; ++i) {
        subFolder = QString("%1/tekrar_%2").arg(rootFolder).arg(i + 1);
        if (dir.mkpath(subFolder)) {
            qDebug() << "Alt klasör oluşturuldu:" << subFolder;
        } else {
            qDebug() << "Alt klasör oluşturulamadı:" << subFolder;
        }
        QVector<QFile*> sensors;
        QVector<QTextStream*> sensorstreams;

        // Sensor dosyaları
        for (int j = 0; j < activeSensorCount; ++j) {
            QFile* sensor_file = new QFile(QString("%1/sensor-%2.txt").arg(subFolder).arg(j + 1));
            if (sensor_file->open(QIODevice::ReadWrite | QIODevice::Text)) {
                QTextStream* sensor_stream = new QTextStream(sensor_file);

                sensors.append(sensor_file);
                sensorstreams.append(sensor_stream);
                qDebug() << QString("Sensor-%1 dosyası oluşturuldu.").arg(j + 1);
            } else {
                delete sensor_file;
                qDebug() << QString("Sensor-%1 dosyası oluşturulamadı.").arg(j + 1);
            }
        }
        sensorFiles.append(sensors);
        sensorStreams.append(sensorstreams);

        // --- om106log dosyası ---
        QString om106Path = subFolder + "/om106log.txt";
        QFile* om106 = new QFile(om106Path);
        if (om106->open(QIODevice::ReadWrite | QIODevice::Text)) {
            QTextStream* omStream = new QTextStream(om106);
            om106LogFiles.append(om106);
            om106LogStreams.append(omStream);
        }

        QVector<QFile*> calpoints;
        QVector<QTextStream*> calpointstreams;
        getCalibrationPointsArraySize();
        // Kalibrasyon klasörü
        kalFolder = subFolder + "/Kalibrasyon_Noktalari";
        if (!dir.mkpath(kalFolder)) {
            qDebug() << "Kalibrasyon klasörü oluşturulamadı:" << kalFolder;
        } else {
            for (int k = 0; k < calPointArraySize; ++k) {
                QFile* kal_file = new QFile(QString("%1/%2ppb_kalibrasyonu.txt").arg(kalFolder).arg(kal_points[k]));
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
            kalFiles.append(calpoints);
            kalStreams.append(calpointstreams);
            calPointArraySize = 0;
        }
    }
    return 1;
}
