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

        QVector<QVector<QFile*>> sensor_files;        // sensor_files[tekrar][dosya]
        QVector<QVector<QTextStream*>> sensor_streams;

        QVector<QFile*> om106_log_files;               // her tekrarda 1 tane
        QVector<QTextStream*> om106_log_streams;

        QVector<QVector<QFile*>> kal_files;        // kal_files[tekrar][dosya]
        QVector<QVector<QTextStream*>> kal_streams;

        QFile log_file;
        QTextStream log_stream;

        QString time_stamp;
        QString root_folder;
        QString sub_folder;
        QString kal_folder;
        QDir dir;

        QString log_file_path;
        QString om106_log_path;

        uint8_t cal_point_array_size;

        int8_t getCalibrationPointsArraySize();
        int8_t createFilesFolders();
};

#endif // CREATOR_H
