#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "log_parser.h"

Creator file_folder_creator;
int calibration_repeat_count;
int active_sensor_count;
int kal_point;
int kal_point_val;

uint8_t cal_repeat_count_data_received;
uint8_t active_sensor_count_data_received;
uint8_t cal_points_data_received;
uint8_t om106l_device_status[NUM_OF_OM106L_DEVICE] = {0};

uint16_t calibration_points[NUM_OF_CAL_POINTS] = {0};
QString cal_repeat_count_command;
QString active_sensor_count_command;
QString cal_points_request_command;
Request current_request;

QStringList sensor_ids;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow), serial(new QSerialPort(this)), serial_2(new QSerialPort(this))
{
    ui->setupUi(this);

    // Seri portları listele
    uint8_t count = 0;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        if (count++ >= 5)
            break;
        ui->cmbPort->addItem(info.portName());
    }

    // Baudrate seçenekleri
    ui->cmbBaudRate->addItems({"4800", "9600", "19200", "115200"});

    // Port bağlanınca gelen veriyi dinle
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readSerial);
    //connect(serial_2, &QSerialPort::readyRead, this, &MainWindow::readSerial_2);

    // Butonlara sinyal-slot bağla
    connect(ui->btnConnect, &QPushButton::clicked,          this, &MainWindow::connectSerial);
    connect(ui->btnSend,    &QPushButton::clicked,          this, &MainWindow::sendData);
    connect(ui->lineEdit,   &QLineEdit::returnPressed,      this, &MainWindow::sendData);
    connect(qApp,           &QCoreApplication::aboutToQuit, this, &MainWindow::onAppExit);

    selected_port_name   = ui->cmbPort->currentText();
    selected_port_name_2 = ui->cmbPort->itemText(1); //ui->cmbPort->itemText(1); //ttyUSB1

    connection_check_timer   = new QTimer(this);
    connection_check_timer_2 = new QTimer(this);
    time_check = new QTimer(this);
    connect(time_check, &QTimer::timeout, this, &MainWindow::checkTime);
    connect(connection_check_timer, &QTimer::timeout, this, &MainWindow::checkConnectionStatus);
    connect(connection_check_timer_2, &QTimer::timeout, this, &MainWindow::checkConnectionStatus_2);
    connection_check_timer->start(2000); // Her 2 saniyede bir kontrol et
    connection_check_timer_2->start(2000);

    ui->btnSend->setStyleSheet(
        R"(
            QPushButton {
                background-color: #007ACC;
                color: white;
                border: none;
                padding: 6px 12px;
                font-weight: bold;
            }
            QPushButton:hover {
                color: yellow;
            }
        )"
    );

    cal_repeat_count_command = "?gpr";
    active_sensor_count_command = "?gpa";
    cal_points_request_command = "?gpd";

    cal_repeat_count_data_received = 0;
    active_sensor_count_data_received = 0;
    cal_points_data_received = 0;

    data_received_time = 10;

    uart_buffer_index = 0;
    current_request = CAL_REPEAT_COUNT;
    uart_log_parser = new LogParser();
}

MainWindow::~MainWindow()
{
    delete uart_log_parser;
    delete ui;
}

uint8_t MainWindow::uartLineProcess(char* input)
{
    if (uart_log_parser->parseLine(input, &packet) == 0) {
        uart_log_parser->processPacket(&packet);
        uart_log_parser->freePacket(&packet);
    } else {
        printf("Hatali satir atlandi.\n");
    }
    return 0;
}

void MainWindow::getDataFromMCU()
{
    switch (current_request) {
        case CAL_REPEAT_COUNT:
            if (data_received_time == 0) {
                qDebug() << "Calibration repeat count data couldn't get received";
                current_request = ACTIVE_SENSOR_COUNT;
            } else if (cal_repeat_count_data_received) {
                qDebug() << "Calibration repeat count data get received";
                current_request = ACTIVE_SENSOR_COUNT;
                data_received_time = 10;
            } else {
                sendData();
            }
            break;

        case ACTIVE_SENSOR_COUNT:
            if (data_received_time == 0) {
                qDebug() << "Active sensor count data couldn't get received";
                current_request = CAL_POINTS;
            } else if (active_sensor_count_data_received) {
                qDebug() << "Active sensor count data get received";
                current_request = CAL_POINTS;
                data_received_time = 10;
            } else {
                sendData();
            }
            break;

        case CAL_POINTS:
            if (data_received_time == 0) {
                qDebug() << "Calibration points data couldn't get received";
                current_request = NONE;
            } else if (cal_points_data_received) {
                qDebug() << "Calibration points data get received";
                current_request = NONE;
            } else {
                sendData();
            }
            break;

        default:
            break;
    }
}

void MainWindow::checkTime()
{
    if (data_received_time && current_request != NONE) {
        data_received_time--;
    } else if (data_received_time == 0) {
        data_received_time = 10;
    }
    if (current_request != NONE) {
        getDataFromMCU();
    }
}

void MainWindow::checkConnectionStatus_2()
{
    bool portStillAvailable = false;
    foreach (const QSerialPortInfo & info, QSerialPortInfo::availablePorts())
    {
        if (info.portName() == selected_port_name_2)
        {
            portStillAvailable = true;
            break;
        }
    }
    if (!portStillAvailable)
    {
        if (serial_2->isOpen())
        {
            serial_2->close();
            ui->plainTextEdit->appendPlainText("Port-2 Bağlantı koptu: " + selected_port_name_2);
            om106l_device_status[DEVICE_2] = 0;
        }
    } else {
        if (!serial_2->isOpen())
        {
            // Otomatik yeniden bağlanma istenirse:
            serial_2->setPortName(selected_port_name_2);
            serial_2->setBaudRate(ui->cmbBaudRate->currentText().toInt());
            if (serial_2->open(QIODevice::ReadWrite))
            {
                ui->plainTextEdit->appendPlainText("Port-2 Bağlantı yeniden kuruldu.");
                om106l_device_status[DEVICE_2] = 1;
            } else {
                ui->plainTextEdit->appendPlainText("Port-2 var ama açılamıyor.");
                om106l_device_status[DEVICE_2] = 0;
            }
        }
    }
}

void MainWindow::checkConnectionStatus()
{
    bool portStillAvailable = false;
    foreach (const QSerialPortInfo & info, QSerialPortInfo::availablePorts())
    {
        if (info.portName() == selected_port_name)
        {
            portStillAvailable = true;
            break;
        }
    }
    if (!portStillAvailable)
    {
        if (serial->isOpen())
        {
            serial->close();
            ui->plainTextEdit->appendPlainText("Port-1 Bağlantı koptu: " + selected_port_name);
            om106l_device_status[DEVICE_1] = 0;
        }
    } else {
        if (!serial->isOpen())
        {
           // Otomatik yeniden bağlanma istenirse:
           serial->setPortName(selected_port_name);
           serial->setBaudRate(ui->cmbBaudRate->currentText().toInt());
           if (serial->open(QIODevice::ReadWrite))
           {
               ui->plainTextEdit->appendPlainText("Port-1 Bağlantı yeniden kuruldu.");
               om106l_device_status[DEVICE_1] = 1;
           } else {
               ui->plainTextEdit->appendPlainText("Port-1 var ama açılamıyor.");
               om106l_device_status[DEVICE_1] = 0;
           }
        }
    }
}

void MainWindow::Log2LinePlainText(QString command)
{
    if (!command.isEmpty()) {
        ui->plainTextEdit->appendPlainText(command);  // Terminale yaz
        ui->lineEdit->clear();  // Girişi temizle
    }
}

uint8_t MainWindow::parseLineEditInput(const QString& input, QStringList& outputList)
{
    outputList.clear();

    // Boşlukla ayırıp elemanlara ayır
    QStringList parts = input.split(" ", Qt::SkipEmptyParts);

    // İlk eleman ?spn olduğundan onu çıkar
    parts.removeFirst();

    // 15 değer var mı?
    if (parts.size() != active_sensor_count) {
        qDebug() << "Geçersiz giriş: aktif sensör kader adet sXXXX formatında değer yok.";
        return 0;
    }

    // Her bir değeri kontrol et
    QRegularExpression regex("^s\\d{4}$");
    for (const QString& part : parts) {
        if (!regex.match(part).hasMatch()) {
            qDebug() << "Geçersiz format:" << part;
            return 0;
        }
    }

    outputList = parts;

    qDebug() << "Giriş geçerli.";
    return 1;
}


void MainWindow::connectSerial()
{
    if (!serial->isOpen())
    {
        serial->setPortName(ui->cmbPort->currentText());
        //serial->setBaudRate(ui->cmbBaudRate->itemText(3).toInt());
        serial->setBaudRate(ui->cmbBaudRate->currentText().toInt());
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);

        if (serial->open(QIODevice::ReadWrite))
        {
            ui->plainTextEdit->appendPlainText("Port-1 acildi.");
            om106l_device_status[DEVICE_1] = 1;
        } else {
            ui->plainTextEdit->appendPlainText("Port-1 acilamadi.");
            om106l_device_status[DEVICE_1] = 0;
        }
    } else {
        serial->close();
        ui->plainTextEdit->appendPlainText("Port-1 kapatildi.");
        om106l_device_status[DEVICE_1] = 0;
    }

    if (!serial_2->isOpen())
    {
        serial_2->setPortName(ui->cmbPort->itemText(1));
        //serial->setBaudRate(ui->cmbBaudRate->itemText(3).toInt());
        serial_2->setBaudRate(ui->cmbBaudRate->currentText().toInt());
        serial_2->setDataBits(QSerialPort::Data8);
        serial_2->setParity(QSerialPort::NoParity);
        serial_2->setStopBits(QSerialPort::OneStop);
        serial_2->setFlowControl(QSerialPort::NoFlowControl);

        if (serial_2->open(QIODevice::ReadWrite))
        {
            ui->plainTextEdit->appendPlainText("Port-2 acildi.");
            om106l_device_status[DEVICE_2] = 1;

        } else {
            ui->plainTextEdit->appendPlainText("Port-2 acilamadi.");
            om106l_device_status[DEVICE_2] = 0;
        }
    } else {
        serial_2->close();
        ui->plainTextEdit->appendPlainText("Port-2 kapatildi.");
        om106l_device_status[DEVICE_2] = 0;
    }
}

void MainWindow::readSerial()
{
    char ch;
    while (serial->bytesAvailable())
    {
        serial->read(&ch, 1);

        if (ch == '\n')
        {
            uart_rx_buffer[uart_buffer_index] = '\0';
            QString line = QString::fromUtf8(uart_rx_buffer);
            ui->plainTextEdit->appendPlainText(line);
            uartLineProcess(uart_rx_buffer);
            uart_buffer_index = 0;
            memset(uart_rx_buffer, 0, RX_BUFFER_LEN);
        } else {
            if (uart_buffer_index < RX_BUFFER_LEN - 1)
                uart_rx_buffer[uart_buffer_index++] = ch;
        }
    }
}

void MainWindow::sendData()
{
    QString command = ui->lineEdit->text();
    if (command == "create-f") {
        Log2LinePlainText(command);
        file_folder_creator.createFilesFolders();
    } else if (command == "get-data") {
        Log2LinePlainText(command);
        time_check->start(1000);
    } else if (command.startsWith("?spn")) {
        Log2LinePlainText(command);
        if (!parseLineEditInput(command, sensor_ids)) {
            ui->plainTextEdit->appendPlainText("Geçersiz format: " + command);
            return;
        }
    } else {
        if (command.isEmpty())
        {
            switch (current_request) {
                case CAL_REPEAT_COUNT:
                    command = cal_repeat_count_command;
                    break;
                case ACTIVE_SENSOR_COUNT:
                    command = active_sensor_count_command;
                    break;
                case CAL_POINTS:
                    command = cal_points_request_command;
                    break;
                default:
                    break;
            }
        }
        Log2LinePlainText(command);
        serial->write(command.toUtf8());
        serial->write("\r\n");
    }
    ui->lineEdit->clear();
}

void MainWindow::onBtnClearClicked()
{
    ui->plainTextEdit->clear();
}

void MainWindow::onAppExit()
{

}


