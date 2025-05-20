#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serial.h"
#include "command_line.h"
#include "calibration_board.h"

Creator file_folder_creator;
int calibration_repeat_count;
int kal_point;
int kal_point_val;

uint8_t sensor_module_status[NUM_OF_SENSOR_BOARD];
uint8_t active_sensor_count;

uint8_t cal_repeat_count_data_received;
uint8_t active_sensor_count_data_received;
uint8_t cal_points_data_received;
uint8_t om106l_device_status[NUM_OF_OM106L_DEVICE] = {0};

uint16_t calibration_points[NUM_OF_CAL_POINTS] = {0};
Request current_request;

QStringList sensor_ids;
QMap<QString, uint8_t> sensor_folder_create_status;
QMap<QString, uint8_t> sensor_log_folder_create_status;
QMap<QString, uint8_t> sensor_module_map;
QTimer *get_calibration_status_timer;

Serial *serial;
CommandLine *command_line;
CalibrationBoard *calibration_board;
uint8_t is_main_folder_created;

CalibrationStatus cal_status_t = { .o3_average = 0, .calibration_ppb = 0, .calibration_state = 0, .calibration_duration = 0,
                                   .stabilization_timer = 0, .repeat_calibration = 0, .pwm_duty_cycle = 0, .pwm_period = 0};

QMap<Request, QString> request_commands = {
    { ACTIVE_SENSOR_COUNT, "?gpa" },
    { CAL_POINTS, "?gpd" },
    { CAL_STATUS, "?gk"}
};

QMap<Request, uint8_t> request_data_status = {
    { ACTIVE_SENSOR_COUNT, 0 },
    { CAL_POINTS, 0 },
    { CAL_STATUS, 0 }
};

QMap<CalibrationStates, QString> calibration_state_str = {
    { WAIT_STATE, "WAIT STATE" },
    { CLEAN_AIR_STATE, "CLEAN AIR STATE" },
    { SET_ENVIRONMENT_CONDITIONS_STATE, "SET ENVIRONMENT CONDITIONS STATE" },
    { ZERO_CALIBRATION_STATE, "ZERO CALIBRATION STATE"},
    { SPAN_CALIBRATION_START_STATE, "SPAN CALIBRATION START STATE" },
    { SPAN_CALIBRATION_MID_STATE, "SPAN CALIBRATION MID STATE" },
    { SPAN_CALIBRATION_END_STATE, "SPAN CALIBRATION END STATE" },
    { RETURN_TO_ZERO_STATE, "RETURN TO ZERO STATE"},
    { REPEAT_CALIBRATION_STATE, "REPEAT CALIBRATION STATE"}
};

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

    get_calibration_status_timer = new QTimer(this);

    // Baudrate seçenekleri
    ui->cmbBaudRate->addItems({"4800", "9600", "19200", "115200"});

    connect(ui->btnConnect, &QPushButton::clicked, serial, &Serial::connectSerial);
    connect(ui->btnSend, &QPushButton::clicked, command_line, &CommandLine::commandLineProcess);
    connect(ui->lineEdit, &QLineEdit::returnPressed, command_line, &CommandLine::commandLineProcess);

    connect(serial->connection_check_timer, &QTimer::timeout, serial, &Serial::checkConnectionStatus);
    connect(serial->connection_check_timer_2, &QTimer::timeout, serial, &Serial::checkConnectionStatus_2);

    connect(serial->serial, &QSerialPort::readyRead, serial, &Serial::readSerial);
    connect(qApp, &QCoreApplication::aboutToQuit, this, &MainWindow::onAppExit);

    connect(get_calibration_status_timer, &QTimer::timeout, command_line, &CommandLine::getCalStatus);

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

    cal_status_t.calibration_state = SPAN_CALIBRATION_START_STATE;
    is_main_folder_created = file_folder_creator.createMainFolder();
    serial->connection_check_timer->start(2000); // Her 2 saniyede bir kontrol et
    //serial->connection_check_timer_2->start(2000);
    get_calibration_status_timer->start(20000);
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


