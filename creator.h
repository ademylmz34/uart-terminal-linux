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

        QVector<QFile*> sensorFiles;
        QVector<QFile*> kalFiles;
        QFile om106LogFile;
        QFile logFile;

        QVector<QTextStream*> sensorStreams;
        QVector<QTextStream*> kalStreams;
        QTextStream logStream;
        QTextStream om106LogStream;

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

        int8_t create_files_folders();
};

#endif // CREATOR_H
