#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "log_parser.h"

Creator file_folder_creator;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow), serial(new QSerialPort(this))
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

    // Butonlara sinyal-slot bağla
    connect(ui->btnConnect, &QPushButton::clicked, this, &MainWindow::connectSerial);
    connect(ui->btnSend, &QPushButton::clicked, this, &MainWindow::sendData);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &MainWindow::sendData);
    connect(qApp, &QCoreApplication::aboutToQuit, this, &MainWindow::onAppExit);

    selectedPortName = ui->cmbPort->currentText();

    connectionCheckTimer = new QTimer(this);
    connect(connectionCheckTimer, &QTimer::timeout, this, &MainWindow::checkConnectionStatus);
    connectionCheckTimer->start(2000); // Her 2 saniyede bir kontrol et

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

    uart_buffer_index = 0;
    //create_files_folders();

    file_folder_creator.create_files_folders();
    uart_log_parser = new LogParser();
    //create_folders();
}

MainWindow::~MainWindow()
{
    delete uart_log_parser;
    delete ui;
}

int8_t MainWindow::uart_line_process(char* input)
{
    file_folder_creator.logStream << input << "\n";
    if (uart_log_parser->parse_line(input, &packet) == 0) {
        uart_log_parser->process_packet(&packet);
        uart_log_parser->free_packet(&packet);
    } else {
        printf("Hatali satir atlandi.\n");
    }
    return 0;
}

void MainWindow::checkConnectionStatus()
{
    bool portStillAvailable = false;
    foreach (const QSerialPortInfo & info, QSerialPortInfo::availablePorts())
    {
        if (info.portName() == selectedPortName)
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
            ui->plainTextEdit->appendPlainText("Bağlantı koptu: " + selectedPortName);
        }
    } else {
        if (!serial->isOpen())
        {
           // Otomatik yeniden bağlanma istenirse:
           serial->setPortName(selectedPortName);
           serial->setBaudRate(ui->cmbBaudRate->currentText().toInt());
           if (serial->open(QIODevice::ReadWrite))
           {
               ui->plainTextEdit->appendPlainText("Bağlantı yeniden kuruldu.");
           } else {
               ui->plainTextEdit->appendPlainText("Port var ama açılamıyor.");
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
            ui->plainTextEdit->appendPlainText("Port acildi.");
        } else {
            ui->plainTextEdit->appendPlainText("Port acilamadi.");
        }
    } else {
        serial->close();
        ui->plainTextEdit->appendPlainText("Port kapatildi.");
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
            //logStream << line << "\n";
            uart_line_process(uart_rx_buffer);
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
    Log2LinePlainText(command);
    serial->write(command.toUtf8());
    serial->write("\r\n");
    ui->lineEdit->clear();
}

void MainWindow::on_btnClear_clicked()
{
    ui->plainTextEdit->clear();
}

void MainWindow::onAppExit()
{

}


