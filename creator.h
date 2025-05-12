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

        QVector<QVector<QFile*>> sensorFiles;        // sensorFiles[tekrar][dosya]
        QVector<QVector<QTextStream*>> sensorStreams;

        QVector<QFile*> om106LogFiles;               // her tekrarda 1 tane
        QVector<QTextStream*> om106LogStreams;

        QVector<QVector<QFile*>> kalFiles;        // kalFiles[tekrar][dosya]
        QVector<QVector<QTextStream*>> kalStreams;

        QFile logFile;
        QTextStream logStream;

        /*QVector<QFile*> sensorFiles;
        QVector<QFile*> kalFiles;
        QVector<QFile*> om106LogFiles;
        //QFile om106LogFile;
        QFile logFile;

        QVector<QTextStream*> sensorStreams;
        QVector<QTextStream*> kalStreams;
        QVector<QTextStream*> om106LogStreams;
        QTextStream logStream;
        //QTextStream om106LogStream;*/

        uint8_t tekrar_sayisi;
        uint8_t sensor_sayisi;
        uint8_t kalibrasyon_noktasi_sayisi;

        QString timestamp;
        QString rootFolder;
        QString subFolder;
        QString kalFolder;
        QDir dir;

        QString logFilePath;
        QString om106LogPath;

        uint8_t calPointArraySize;

        int8_t getCalibrationPointsArraySize();
        int8_t create_files_folders();
};

#endif // CREATOR_H
