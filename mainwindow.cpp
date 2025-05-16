#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "log_parser.h"

Creator file_folder_creator;
int calibration_repeat_count;
int kal_point;
int kal_point_val;
calibration_states calibration_state;

uint8_t sensor_module_status[NUM_OF_SENSOR_BOARD];
uint8_t active_sensor_count;

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
QMap<QString, uint8_t> sensor_folder_create_status;
QMap<QString, uint8_t> sensor_log_folder_create_status;
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
    connect(ui->btnSend,    &QPushButton::clicked,          this, &MainWindow::commandLineProcess);
    connect(ui->lineEdit,   &QLineEdit::returnPressed,      this, &MainWindow::commandLineProcess);
    connect(qApp,           &QCoreApplication::aboutToQuit, this, &MainWindow::onAppExit);

    selected_port_name   = "/dev/om106_1";
    selected_port_name_2 = "/dev/om106_2";

    connection_check_timer   = new QTimer(this);
    connection_check_timer_2 = new QTimer(this);
    time_check = new QTimer(this);
    connect(time_check, &QTimer::timeout, this, &MainWindow::checkTime);
    connect(connection_check_timer, &QTimer::timeout, this, &MainWindow::checkConnectionStatus);
    connect(connection_check_timer_2, &QTimer::timeout, this, &MainWindow::checkConnectionStatus_2);
    connection_check_timer->start(2000); // Her 2 saniyede bir kontrol et
    connection_check_timer_2->start(2000);
    //time_check->start(1000);

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

    memset(sensor_module_status, 0, NUM_OF_SENSOR_BOARD);
    cal_repeat_count_command = "?gpr";
    active_sensor_count_command = "?gpa";
    cal_points_request_command = "?gpd";

    is_oml_log_folder_created = 0;

    uart_buffer_index = 0;
    uart_log_parser = new LogParser();
    is_main_folder_created = file_folder_creator.createMainFolder();
}

MainWindow::~MainWindow()
{
    delete uart_log_parser;
    delete ui;
}

uint8_t MainWindow::uartLineProcess(char* input)
{
    //*(main_log_stream) << input;
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
                request_command = active_sensor_count_command;
            } else if (cal_repeat_count_data_received) {
                qDebug() << "Calibration repeat count data get received";
                current_request = ACTIVE_SENSOR_COUNT;
                data_received_time = 10;
                request_command = active_sensor_count_command;
            } else {
                sendData(request_command);
            }
            break;

        case ACTIVE_SENSOR_COUNT:
            if (data_received_time == 0) {
                qDebug() << "Active sensor count data couldn't get received";
                current_request = NONE;
                request_command = "";
            } else if (active_sensor_count_data_received) {
                qDebug() << "Active sensor count data get received";
                current_request = NONE;
                request_command = "";
                //data_received_time = 10;
            } else {
                sendData(request_command);
            }
            break;

        case CAL_POINTS:
            if (data_received_time == 0) {
                qDebug() << "Calibration points data couldn't get received";
                current_request = NONE;
                request_command = "";
            } else if (cal_points_data_received) {
                qDebug() << "Calibration points data get received";
                current_request = NONE;
                request_command = "";
            } else {
                sendData(request_command);
            }
            break;

        default:
            time_check->stop();
            break;
    }
}

void MainWindow::checkTime()
{
    if (data_received_time) {
        data_received_time--;
    } else if (data_received_time == 0) {
        data_received_time = 10;
    }
    if (current_request != NONE) {
        getDataFromMCU();
    }
}

void MainWindow::checkPortConnection(QSerialPort* port, const QString& portPath, int deviceIndex)
{
    QFileInfo portInfo(portPath);
    bool portExists = portInfo.exists() && portInfo.isSymLink();

    if (!portExists)
    {
        if (port->isOpen())
        {
            port->close();
            ui->plainTextEdit->appendPlainText(QString("Port-%1 bağlantı koptu: %2").arg(deviceIndex + 1).arg(portPath));
            om106l_device_status[deviceIndex] = 0;
        }
    }
    else
    {
        if (!port->isOpen())
        {
            port->setPortName(portPath);  // /dev/om106_1
            port->setBaudRate(ui->cmbBaudRate->currentText().toInt());

            if (port->open(QIODevice::ReadWrite))
            {
                ui->plainTextEdit->appendPlainText(QString("Port-%1 bağlantı yeniden kuruldu: %2").arg(deviceIndex + 1).arg(portPath));
                om106l_device_status[deviceIndex] = 1;
            }
            else
            {
                ui->plainTextEdit->appendPlainText(QString("Port-%1 var ama açılamıyor: %2").arg(deviceIndex + 1).arg(portPath));
                om106l_device_status[deviceIndex] = 0;
            }
        }
    }
}


void MainWindow::checkConnectionStatus_2()
{
    checkPortConnection(serial_2, selected_port_name_2, DEVICE_2);
}

void MainWindow::checkConnectionStatus()
{
    checkPortConnection(serial, selected_port_name, DEVICE_1);
}

void MainWindow::Log2LinePlainText(QString command)
{
    if (!command.isEmpty()) {
        ui->plainTextEdit->appendPlainText(command);  // Terminale yaz
        ui->lineEdit->clear();  // Girişi temizle
    }
}

uint8_t MainWindow::parseLineEditInput(const QStringList& inputList, QStringList& outputList)
{
    outputList.clear();

    // 15 değer var mı?
    if (inputList.size() != NUM_OF_SENSOR_BOARD) {
        ui->plainTextEdit->appendPlainText("Geçersiz giriş: 15 adet sXXXX formatında değer yok.");
        return 0;
    }

    // Her bir değeri kontrol et
    uint8_t counter = 0;
    QRegularExpression regex("^s\\d{4}$");
    for (const QString& part : inputList) {
        if ((sensor_module_status[counter] == 0 && part != "0") || (sensor_module_status[counter] == 1 && part == "0")) {
            ui->plainTextEdit->appendPlainText("Sensör kartındaki slotlara göre değerleri giriniz.");
            return 0;
        }
        if (!regex.match(part).hasMatch() && part != "0") {
            ui->plainTextEdit->appendPlainText("Geçersiz format:" + part);
            return 0;
        }
        counter++;
    }

    outputList = inputList;

    qDebug() << "Giriş geçerli.";
    return 1;
}

/*QString MainWindow::readBytes(QSerialPort* serial_port)
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
            ui->plainTextEdit->appendPlainText(line);
            uart_buffer_index = 0;
            memset(uart_rx_buffer, 0, RX_BUFFER_LEN);
        } else {
            if (uart_buffer_index < RX_BUFFER_LEN - 1)
                uart_rx_buffer[uart_buffer_index++] = ch;
        }
    }
    return response;
}*/

QString MainWindow::readFullResponse(QSerialPort &port, int timeoutMs = 1000)
{
    QByteArray buffer;
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < timeoutMs)
    {
        if (port.waitForReadyRead(100))  // 100 ms bekle ve parçaları topla
        {
            buffer += port.readAll();

            // Eğer '\n' varsa son satır geldi demektir (veya özel karakter belirleyebilirsin)
            if (buffer.contains('\n'))
                break;
        }
    }

    return QString::fromUtf8(buffer).trimmed();
}

void MainWindow::detectOm106Devices()
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
        ui->plainTextEdit->appendPlainText("om106l_1 bağlı: " + detectedPorts["om106l_1"]);
        om106l_device_status[DEVICE_1] = 1;
    } else {
        om106l_device_status[DEVICE_1] = 0;
    }

    if (detectedPorts.contains("om106l_2")) {
        serial_2->setPortName(detectedPorts["om106l_2"]);
        serial_2->setBaudRate(ui->cmbBaudRate->currentText().toInt());
        serial_2->open(QIODevice::ReadWrite);
        ui->plainTextEdit->appendPlainText("om106l_2 bağlı: " + detectedPorts["om106l_2"]);
        om106l_device_status[DEVICE_2] = 1;
    } else {
        om106l_device_status[DEVICE_2] = 0;
    }
}


void MainWindow::connectSerial()
{
    if (!serial->isOpen())
    {
        serial->setPortName(selected_port_name_2);
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
        serial_2->setPortName(selected_port_name);
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

QStringList MainWindow::getSensorFolderNames()
{
    QRegularExpression regex("^s\\d{4}$");
    QStringList folder_names;
    QStringList sensors_folders;

    folder_names = file_folder_creator.getFolderNames();
    for (const QString& folder_name: folder_names) {
        if (regex.match(folder_name).hasMatch()) {
            sensor_folder_create_status[folder_name] = 1;
            sensors_folders << folder_name;
        }
    }
    return sensors_folders;
}

uint8_t MainWindow::isArrayEmpty(const uint8_t* arr, size_t len)
{
    static const uint8_t zero_block[32] = {0}; // max karşılaştırma için buffer
    if (len <= sizeof(zero_block)) {
        return memcmp(arr, zero_block, len) == 0;
    }

    // uzun diziler için döngüsel karşılaştırma
    for (size_t i = 0; i < len; i++) {
        if (arr[i] != 0) {
            return 0; // en az bir eleman sıfır değil
        }
    }
    return 1; // hepsi sıfır
}

uint8_t MainWindow::createSensorFolders()
{
    uint8_t status;
    if (isArrayEmpty(sensor_module_status, NUM_OF_SENSOR_BOARD)) {
        ui->plainTextEdit->appendPlainText("Sensör modülleri dizisi boş, önce !gabc ile kalibrasyon kartından verileri alınız."); //get active board count -> gabc
        return 0;
    } else {
        parseLineEditInput(sensor_numbers, sensor_ids);
        if (!sensor_ids.isEmpty()) {
            for (const QString& sensor_id: sensor_ids) {
                status = file_folder_creator.createSensorFolder(sensor_id);
                if (status == 1) {
                    ui->plainTextEdit->appendPlainText(sensor_id + " klasörü oluşturuldu.");
                    sensor_folder_create_status.insert(sensor_id, 1);
                } else if (status == 2) {
                    ui->plainTextEdit->appendPlainText(sensor_id + " klasörü zaten var.");
                    sensor_folder_create_status.insert(sensor_id, 1);
                } else {
                    ui->plainTextEdit->appendPlainText(sensor_id + " klasörü oluşturulumadi.");
                    sensor_folder_create_status.insert(sensor_id, 0);
                    return 0;
                }
            }
        } else {
            ui->plainTextEdit->appendPlainText("?!csf komutu ile sensör numaralarını girmeniz gerekmektedir.");
            return 0;
        }
    }
    return 1;
}

uint8_t MainWindow::createCalibrationFolders()
{
    QStringList sensors_folders;
    uint8_t status;
    sensors_folders = getSensorFolderNames();
    for (const QString& sensor_id: sensors_folders) {
        if (!sensor_folder_create_status[sensor_id]) {
            ui->plainTextEdit->appendPlainText(sensor_id + " klasörü bulunamadi.");
        } else {
            if (!sensor_log_folder_create_status[sensor_id]) {
                status = file_folder_creator.createSensorLogFolder(sensor_id);
                if (status == 1) {
                    ui->plainTextEdit->appendPlainText(sensor_id + " log klasörü oluşturuldu.");
                    sensor_log_folder_create_status[sensor_id] = 1;
                } else if (status == 2) {
                    ui->plainTextEdit->appendPlainText(sensor_id + " log klasörü zaten var.");
                } else {
                    ui->plainTextEdit->appendPlainText(sensor_id + " log klasörü oluşturulamadi.");
                    sensor_log_folder_create_status[sensor_id] = 0;
                    return 0;
                }
            } else {
                ui->plainTextEdit->appendPlainText("Uygulama çalışırken yalnızca bir kez sensör log klasörü oluşturulabilir.");
            }
        }
    }


    if (!is_oml_log_folder_created) {
        status = file_folder_creator.createOm106LogFolder();
        if (status == 1) {
            ui->plainTextEdit->appendPlainText("om106 log klasörü oluşturuldu.");
            is_oml_log_folder_created = 1;
        } else if (status == 2) {
            ui->plainTextEdit->appendPlainText("om106 log klasörü zaten var.");
        } else {
            ui->plainTextEdit->appendPlainText("om106 log klasörü oluşturulamadi.");
            is_oml_log_folder_created = 0;
            return 0;
        }
    } else {
        ui->plainTextEdit->appendPlainText("Uygulama çalışırken yalnızca bir kez om106 log klasörü oluşturulabilir.");
    }

    if (is_oml_log_folder_created) return 0;
    return 1;
}

void MainWindow::startCalibrationProcess()
{
    if (createCalibrationFolders()) {
        ui->plainTextEdit->appendPlainText("log klasörleri ve dosyalari olusturuldu, kalibrasyon basladi");
        //sendData(mcu_command);
    }
}

void MainWindow::parseCommand(QString command)
{
    Command type;
    QString command_str;
    command = command.trimmed();

    if (command.startsWith("?")) mcu_command = command;

    if (command == "?r") type = CMD_R;
    else if (command == "?sc") type = CMD_SC;
    else type = CMD_SM;

    if (command.startsWith("!")) command_str = command.mid(1);

    if (command_str.startsWith("csf")) {
        sensor_numbers = command_str.split(" ", Qt::SkipEmptyParts);
        sensor_numbers.removeFirst();
        type = CMD_CSF;
    } else if (command_str == "gcd") type = CMD_GCD;
    else if (command_str == "gabc") type = CMD_GABC;

    processCommand(type);
}

uint8_t MainWindow::processCommand(Command command_type)
{
    switch (command_type) {
        case CMD_CSF:
            if (createSensorFolders()) {
                ui->plainTextEdit->appendPlainText("Sensör klasörleri başarıyla oluşturuldu.");
            }
            break;

        case CMD_GCD:
            current_request = CAL_REPEAT_COUNT;
            request_command = cal_repeat_count_command;
            cal_repeat_count_data_received = 0;
            active_sensor_count_data_received = 0;
            cal_points_data_received = 0;
            data_received_time = 10;
            time_check->start(1000);
            break;

        case CMD_GABC:
            current_request = ACTIVE_SENSOR_COUNT;
            request_command = active_sensor_count_command;
            active_sensor_count_data_received = 0;
            data_received_time = 10;
            time_check->start(1000);
            break;

        case CMD_SC:
            startCalibrationProcess();
            break;

        case CMD_R:
            break;

        case CMD_SM:
            sendData(mcu_command);
            break;

        default:
            break;
    }
    return 1;
}

void MainWindow::commandLineProcess()
{
    QString command_cl = ui->lineEdit->text();
    Log2LinePlainText(command_cl);
    parseCommand(command_cl);

    ui->lineEdit->clear();
}

void MainWindow::sendData(QString command)
{
    if (serial->isOpen()) {
        serial->write(command.toUtf8());
        serial->write("\r\n");
    }
}

void MainWindow::onBtnClearClicked()
{
    ui->plainTextEdit->clear();
}

void MainWindow::onAppExit()
{

}


