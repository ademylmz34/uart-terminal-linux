#ifndef CREATOR_H
#define CREATOR_H

#include "enum_types.h"

#include <QString>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

#include <stdint.h>

#define NUM_OF_OM106L_DEVICE 2

struct SensorFiles {
    QString sensor_id;
    QFile* log_file;
    QFile* kal_log_file;
    QFile* kal_end_log_file;

    QTextStream* log_stream;
    QTextStream* kal_stream;
    QTextStream* kal_end_stream;
};

struct Om106Files {
    Om106l_Devices device_id;
    QFile* om106_log_file;
    QTextStream* om106_stream;
};

extern QMap<uint16_t, SensorFiles> sensor_map;
extern QMap<Om106l_Devices, Om106Files> om106_map;

extern QFile* main_log_file;
extern QTextStream* main_log_stream;

extern QFile* calibration_log_file;
extern QTextStream* calibration_stream;

extern uint8_t om106l_device_status[NUM_OF_OM106L_DEVICE];

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
