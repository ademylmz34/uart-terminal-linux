#include "serial.h"
#include "mainwindow.h"


Serial::Serial(QObject *parent): QObject(parent)  // üst sınıfa parametre gönderimi
{
    serial_port = new QSerialPort();
    selected_port_name   = "/dev/ttyUSB0";
    connection_check_timer   = new QTimer(this);
    baud_rate = 115200;
    uart_buffer_index = 0;
}

Serial::~Serial()
{
    delete connection_check_timer;
    delete serial_port;
}

void Serial::setMainWindow(MainWindow *mw) {
    mainWindow = mw;
}

uint8_t Serial::uartLineProcess(char* input)
{
    if (!uart_log_parser->parseLine(input, &uart_log_parser->packet)) mainWindow->setLineEditText("Hatali satir atlandi.\n");
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
            whenConnectionLost();
        }
    }
    else
    {
        if (!port->isOpen())
        {
            port->setPortName(portPath);  // /dev/om106_1
            port->setBaudRate(BR_115200);

            if (port->open(QIODevice::ReadWrite))
            {
                mainWindow->setLineEditText(QString("Port-%1 bağlantı yeniden kuruldu: %2").arg(deviceIndex).arg(portPath));
                om106l_device_status[deviceIndex - 1] = 1;
                command_line->getFirstData();
                mainWindow->disableConnectionButton();
                mainWindow->disableBaudCmb();
            }
            else
            {
                mainWindow->setLineEditText(QString("Port-%1 var ama açılamıyor: %2").arg(deviceIndex).arg(portPath));
                om106l_device_status[deviceIndex - 1] = 0;
            }
        }
    }
}

void Serial::checkConnectionStatus()
{
    checkPortConnection(serial_port, selected_port_name, DEVICE_1);
}

void Serial::connectSerial()
{
    if (!serial_port->isOpen())
    {
        serial_port->setPortName(selected_port_name);
        //serial->setBaudRate(ui->cmbBaudRate->itemText(3).toInt());
        serial_port->setBaudRate(mainWindow->getCmbBaudRateValue());
        serial_port->setDataBits(QSerialPort::Data8);
        serial_port->setParity(QSerialPort::NoParity);
        serial_port->setStopBits(QSerialPort::OneStop);
        serial_port->setFlowControl(QSerialPort::NoFlowControl);

        if (serial_port->open(QIODevice::ReadWrite))
        {
            mainWindow->setLineEditText("Port-1 acildi.");
            om106l_device_status[DEVICE_1] = 1;
        } else {
            mainWindow->setLineEditText("Port-1 acilamadi.");
            om106l_device_status[DEVICE_1] = 0;
        }
    } else {
        serial_port->close();
        mainWindow->setLineEditText("Port-1 kapatildi.");
        om106l_device_status[DEVICE_1] = 0;
    }
}

void Serial::readSerial()
{
    char ch;
    while (serial_port->bytesAvailable())
    {
        serial_port->read(&ch, 1);

        if (ch == '\n')
        {
            uart_rx_buffer[uart_buffer_index] = '\0';
            //line = QString::fromUtf8(uart_rx_buffer);
            //if (log_status) mainWindow->setLineEditText(line);
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
    if (serial_port->isOpen()) {
        serial_port->write(command.toUtf8());
        serial_port->write("\r\n");
        serial_port->waitForBytesWritten(2000);
    }
}

void Serial::whenConnectionLost()
{
    command_line->messageBox("Kalibrasyon sırasında bağlantı koptu, son log dosyalarının silinmesini istiyor musunuz?");
}

