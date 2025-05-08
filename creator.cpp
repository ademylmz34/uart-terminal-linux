#include "creator.h"

Creator::Creator() {
    tekrar_sayisi = 1;
    sensor_sayisi = 15;
    kalibrasyon_noktasi_sayisi = 5;
}

Creator::~Creator() {
    uint8_t i;
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
    kalStreams.clear();
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

    for (int i = 0; i < tekrar_sayisi; ++i) {
        subFolder = QString("%1/tekrar_%2").arg(rootFolder).arg(i + 1);
        if (dir.mkpath(subFolder)) {
            qDebug() << "Alt klasör oluşturuldu:" << subFolder;
        } else {
            qDebug() << "Alt klasör oluşturulamadı:" << subFolder;
        }

        // om106log.txt dosyası
        om106LogPath = subFolder + "/om106log.txt";
        om106LogFile.setFileName(om106LogPath);
        if (!om106LogFile.open(QIODevice::ReadWrite | QIODevice::Text)) {
            qDebug() << "om106log dosyası oluşturulamadı.";
        }

        om106LogStream.setDevice(&om106LogFile);
        // Sensor dosyaları
        for (int j = 0; j < sensor_sayisi; ++j) {
            QFile* sensor_file = new QFile(QString("%1/sensor-%2.txt").arg(subFolder).arg(j + 1));
            if (sensor_file->open(QIODevice::ReadWrite | QIODevice::Text)) {
                QTextStream* sensor_stream = new QTextStream(sensor_file);

                sensorFiles.append(sensor_file);
                sensorStreams.append(sensor_stream);
                qDebug() << QString("Sensor-%1 dosyası oluşturuldu.").arg(j + 1);
            } else {
                delete sensor_file;
                qDebug() << QString("Sensor-%1 dosyası oluşturulamadı.").arg(j + 1);
            }
        }
    }

    // Kalibrasyon klasörü
    kalFolder = rootFolder + "/Kalibrasyon_Noktalari";
    if (!dir.mkpath(kalFolder)) {
        qDebug() << "Kalibrasyon klasörü oluşturulamadı:" << kalFolder;
    } else {
        for (int k = 0; k < kalibrasyon_noktasi_sayisi; ++k) {
            QFile* kal_file = new QFile(QString("%1/%2ppb_kalibrasyonu.txt").arg(kalFolder).arg(kal_points[k]));
            if (kal_file->open(QIODevice::ReadWrite | QIODevice::Text)) {
                QTextStream* kal_stream = new QTextStream(kal_file);

                kalFiles.append(kal_file);
                kalStreams.append(kal_stream);
                qDebug() << QString("%1 ppb kalibrasyon dosyası oluşturuldu.").arg(kal_points[k]);
            } else {
                delete kal_file;
                qDebug() << QString("%1 ppb kalibrasyon dosyası oluşturulamadı.").arg(kal_points[k]);
            }
        }
    }

    return 1;
}
