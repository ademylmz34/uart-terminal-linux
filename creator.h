#ifndef CREATOR_H
#define CREATOR_H

#include <QString>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

#include <stdint.h>

class Creator {
    public:
        Creator();
        ~Creator();

        QString time_stamp;
        QString root_folder;
        QString sensor_folder;
        QString sensor_log_folder;
        QString om106l_folder;
        QString om106_log_folder;
        QDir dir;

        uint8_t cal_point_array_size;
        uint8_t sensor_id_counter;
        uint8_t is_main_log_file_created;
        uint8_t is_calibration_file_created;

        QStringList getFolderNames();

        int8_t getCalibrationPointsArraySize();

        uint8_t createMainFolder();
        uint8_t createSensorFolder(QString);
        uint8_t createSensorLogFolder(QString);
        uint8_t createOm106lFolder();
        uint8_t createOm106LogFolder();

    private:
        uint8_t createCalibrationLogFile(QString);
        uint8_t createMainLogFile(QString);
        uint8_t createSensorLogFiles(QString);
        uint8_t createOm106LogFiles(QString);

};

#endif // CREATOR_H
