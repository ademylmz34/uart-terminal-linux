#include "serial.h"
//#include "mainwindow.h"
#include <QMessageBox>

Serial::Serial(QObject *parent): QObject(parent)  // üst sınıfa parametre gönderimi
{
    serial = new QSerialPort();
    //serial_2 = new QSerialPort();

    selected_port_name   = "/dev/ttyUSB0";
    selected_port_name_2 = "/dev/om106_2";

    connection_check_timer   = new QTimer(this);
    connection_check_timer_2 = new QTimer(this);

    baud_rate = 115200;

    uart_buffer_index = 0;
    uart_log_parser = new LogParser();
}

Serial::~Serial()
{
    delete connection_check_timer;
    delete connection_check_timer_2;
    delete uart_log_parser;
    delete serial;
    //delete serial_2;
}

void Serial::setMainWindow(MainWindow *mw) {
    mainWindow = mw;
}

uint8_t Serial::uartLineProcess(char* input)
{
    if (main_log_stream != NULL) *(main_log_stream) << input;
    if (uart_log_parser->parseLine(input, &packet) == 0) {
        uart_log_parser->processPacket(&packet);
        uart_log_parser->freePacket(&packet);
    } else {
        printf("Hatali satir atlandi.\n");
    }
    return 0;
}

void Serial::checkPortConnection(QSerialPort* port, const QString& portPath, int deviceIndex)
{
    QFileInfo portInfo(portPath);
    //bool portExists = portInfo.exists() && portInfo.isSymLink();
    bool portExists = portInfo.exists();

    if (!portExists)
    {
        if (port->isOpen())
        {
            port->close();
            mainWindow->setLineEditText(QString("Port-%1 bağlantı koptu: %2").arg(deviceIndex).arg(portPath));
            om106l_device_status[deviceIndex - 1] = 0;
            if (cal_status_t.calibration_state == WAIT_STATE) whenConnectionLost();
        }
    }
    else
    {
        if (!port->isOpen())
        {
            port->setPortName(portPath);  // /dev/om106_1
            port->setBaudRate(mainWindow->getCmbBaudRateValue());

            if (port->open(QIODevice::ReadWrite))
            {
                mainWindow->setLineEditText(QString("Port-%1 bağlantı yeniden kuruldu: %2").arg(deviceIndex).arg(portPath));
                om106l_device_status[deviceIndex - 1] = 1;
                command_line->getActiveBoardCount();
            }
            else
            {
                mainWindow->setLineEditText(QString("Port-%1 var ama açılamıyor: %2").arg(deviceIndex).arg(portPath));
                om106l_device_status[deviceIndex - 1] = 0;
            }
        }
    }
}


void Serial::checkConnectionStatus_2()
{
    checkPortConnection(serial, selected_port_name, DEVICE_2);
}

void Serial::checkConnectionStatus()
{
    checkPortConnection(serial, selected_port_name, DEVICE_1);
}

QString Serial::readBytes(QSerialPort* serial_port)
{
    char ch;
    QString response;
    while (serial_port->bytesAvailable())
    {
        serial_port->read(&ch, 1);

        if (ch == '\n')
        {
            uart_rx_buffer[uart_buffer_index] = '\0';
            response = QString::fromUtf8(uart_rx_buffer);
            mainWindow->setLineEditText(line);
            uart_buffer_index = 0;
            memset(uart_rx_buffer, 0, RX_BUFFER_LEN);
        } else {
            if (uart_buffer_index < RX_BUFFER_LEN - 1)
                uart_rx_buffer[uart_buffer_index++] = ch;
        }
    }
    return response;
}

void Serial::detectOm106Devices()
{
    QMap<QString, QString> detectedPorts;

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort temp;
        temp.setPort(info);
        temp.setBaudRate(115200);  // Gerekirse kullanıcıdan al
        temp.setDataBits(QSerialPort::Data8);
        temp.setParity(QSerialPort::NoParity);
        temp.setStopBits(QSerialPort::OneStop);
        temp.setFlowControl(QSerialPort::NoFlowControl);

        if (temp.open(QIODevice::ReadWrite))
        {
            temp.write("?gid\r\n");
            if (temp.waitForBytesWritten(400))
            {
                qDebug() << "written: " << info.portName();
                if (temp.waitForReadyRead(800))
                {
                    QByteArray response = temp.readAll().trimmed();
                    QString answer = QString::fromUtf8(response);
                    qDebug() << answer;
                    if (answer.contains("om106l_1"))
                        detectedPorts["om106l_1"] = info.portName();
                    else if (answer.contains("om106l_2"))
                        detectedPorts["om106l_2"] = info.portName();
                }
            }
            temp.close();
        }
    }

    // Elde ettiğimiz portları şimdi kalıcı serial objelerine set edelim
    if (detectedPorts.contains("om106l_1")) {
        serial->setPortName(detectedPorts["om106l_1"]);
        serial->setBaudRate(115200);
        serial->open(QIODevice::ReadWrite);
        mainWindow->setLineEditText("om106l_1 bağlı: " + detectedPorts["om106l_1"]);
        om106l_device_status[DEVICE_1] = 1;
    } else {
        om106l_device_status[DEVICE_1] = 0;
    }

    if (detectedPorts.contains("om106l_2")) {
        serial_2->setPortName(detectedPorts["om106l_2"]);
        serial_2->setBaudRate(mainWindow->getCmbBaudRateValue());
        serial_2->open(QIODevice::ReadWrite);
        mainWindow->setLineEditText("om106l_2 bağlı: " + detectedPorts["om106l_2"]);
        om106l_device_status[DEVICE_2] = 1;
    } else {
        om106l_device_status[DEVICE_2] = 0;
    }
}


void Serial::connectSerial()
{
    if (!serial->isOpen())
    {
        serial->setPortName(selected_port_name);
        //serial->setBaudRate(ui->cmbBaudRate->itemText(3).toInt());
        serial->setBaudRate(mainWindow->getCmbBaudRateValue());
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);

        if (serial->open(QIODevice::ReadWrite))
        {
            mainWindow->setLineEditText("Port-1 acildi.");
            om106l_device_status[DEVICE_1] = 1;
        } else {
            mainWindow->setLineEditText("Port-1 acilamadi.");
            om106l_device_status[DEVICE_1] = 0;
        }
    } else {
        serial->close();
        mainWindow->setLineEditText("Port-1 kapatildi.");
        om106l_device_status[DEVICE_1] = 0;
    }
}

void Serial::readSerial()
{
    char ch;
    while (serial->bytesAvailable())
    {
        serial->read(&ch, 1);

        if (ch == '\n')
        {
            uart_rx_buffer[uart_buffer_index] = '\0';
            QString line = QString::fromUtf8(uart_rx_buffer);
            mainWindow->setLineEditText(line);
            uartLineProcess(uart_rx_buffer);
            uart_buffer_index = 0;
            memset(uart_rx_buffer, 0, RX_BUFFER_LEN);
        } else {
            if (uart_buffer_index < RX_BUFFER_LEN - 1)
                uart_rx_buffer[uart_buffer_index++] = ch;
        }
    }
}

void Serial::sendData(QString command)
{
    if (serial->isOpen()) {
        serial->write(command.toUtf8());
        serial->write("\r\n");
    }
}

void Serial::whenConnectionLost()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(mainWindow,
                                  "Onay",
                                  "Kalibrasyon sırasında bağlantı koptu, son log dosyalarının silinmesini istiyor musunuz?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        qDebug() << "Kullanıcı EVET dedi.";
        if (!log_folder_names.isEmpty()) {
            for (const QString &key : log_folder_names.keys()) {
                QString folderPath = log_folder_names.value(key);
                QDir dir(folderPath);

                if (dir.exists()) {
                    bool success = dir.removeRecursively();
                    qDebug() << key << (success ? "silindi" : "silinemedi") << folderPath;
                } else {
                    qDebug() << key << "zaten yok" << folderPath;
                }
            }
        } else {
            qDebug() << "Log klasörleri zaten oluşturulmamış";
        }
    } else {
        qDebug() << "Kullanıcı HAYIR dedi.";
        // işlemleri burada yap
    }
}

