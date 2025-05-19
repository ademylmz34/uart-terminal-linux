#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serial.h"
#include "command_line.h"
#include "calibration_board.h"

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
QMap<QString, uint8_t> sensor_module_map;

Serial *serial;
CommandLine *command_line;
CalibrationBoard *calibration_board;
uint8_t is_main_folder_created;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Seri portları listele
    uint8_t count = 0;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        if (count++ >= 5)
            break;
        ui->cmbPort->addItem(info.portName());
    }

    serial = new Serial(this);
    serial->setMainWindow(this);
    command_line = new CommandLine(this);
    command_line->setMainWindow(this);
    calibration_board = new CalibrationBoard(this);
    calibration_board->setMainWindow(this);

    // Baudrate seçenekleri
    ui->cmbBaudRate->addItems({"4800", "9600", "19200", "115200"});

    connect(ui->btnConnect, &QPushButton::clicked, serial, &Serial::connectSerial);
    connect(ui->btnSend, &QPushButton::clicked, command_line, &CommandLine::commandLineProcess);
    connect(ui->lineEdit, &QLineEdit::returnPressed, command_line, &CommandLine::commandLineProcess);

    connect(qApp,           &QCoreApplication::aboutToQuit, this, &MainWindow::onAppExit);

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

    is_main_folder_created = file_folder_creator.createMainFolder();
}

MainWindow::~MainWindow()
{
    delete serial;
    delete command_line;
    delete calibration_board;
    delete ui;
}

int MainWindow::getCmbBaudRateValue() {
    return ui->cmbBaudRate->currentText().toInt();
}

QString MainWindow::getLineEditText() const {
    return ui->lineEdit->text();
}

void MainWindow::setLineEditText(const QString &text) {
    ui->plainTextEdit->appendPlainText(text);
}

void MainWindow::Log2LinePlainText(const QString &command)
{
    if (!command.isEmpty()) {
        setLineEditText(command);
        ui->lineEdit->clear();
    }
}

void MainWindow::onBtnClearClicked()
{
    ui->plainTextEdit->clear();
}

void MainWindow::onAppExit()
{

}


