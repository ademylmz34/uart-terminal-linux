#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serial.h"
#include "command_line.h"
#include "calibration_board.h"

#include <QCloseEvent>

Creator file_folder_creator;

QDateTime calibration_start_dt;
QDateTime calibration_end_dt;
QDateTime calibration_ppb_start_dt;
QDateTime calibration_ppb_end_dt;

QDateTime current_dt;

int cal_ppb_cal_time;

int calibration_repeat_count;
int kal_point;
int kal_point_val;
int cabin_no;

uint8_t sensor_module_status[NUM_OF_SENSOR_BOARD];
uint8_t active_sensor_count;

uint8_t cal_repeat_count_data_received;
uint8_t active_sensor_count_data_received;
uint8_t cal_points_data_received;
uint8_t om106l_device_status[NUM_OF_OM106L_DEVICE] = {0};

uint16_t calibration_points[NUM_OF_CAL_POINTS] = {20, 50, 100, 200, 500, 0, 0, 0, 0, 0};
Request current_request;

QStringList sensor_ids;
QMap<QString, uint8_t> sensor_folder_create_status;
QMap<QString, uint8_t> sensor_log_folder_create_status;
QMap<QString, uint8_t> sensor_module_map;
QMap<QString, QString> log_folder_names;

QTimer *get_calibration_status_timer;
QTimer *get_sensor_values_timer;

Serial *serial;
CommandLine *command_line;
CalibrationBoard *calibration_board;

uint8_t is_main_folder_created;
uint8_t is_oml_log_folder_created;

CalibrationStatus cal_status_t = { .o3_average = 0, .calibration_ppb = 0, .calibration_state = 0, .calibration_duration = 0,
                                   .stabilization_timer = 0, .repeat_calibration = 0, .pwm_duty_cycle = 0, .pwm_period = 0};

QMap<Request, QString> request_commands = {
    { R_ACTIVE_SENSOR_COUNT, "?gpa" },
    { R_CAL_POINTS, "?gpd" },
    { R_CAL_STATUS, "?gpk"},
    { R_CABIN_INFO, "?gpi"},
    { R_SENSOR_VALUES, "?gps"}
};

QMap<Request, uint8_t> request_data_status = {
    { R_ACTIVE_SENSOR_COUNT, 0 },
    { R_CAL_POINTS, 0 },
    { R_CAL_STATUS, 0 },
    { R_CABIN_INFO, 0},
    { R_SENSOR_VALUES, 0}
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

QMap<uint8_t, QLabel*> temp_labels;
QMap<uint8_t, QLabel*> hum_labels;
QMap<uint8_t, QLabel*> r1_labels;
QMap<uint8_t, QLabel*> r2_labels;
QMap<uint8_t, QLabel*> r3_labels;

CalibrationValLabels cal_val_labels;
MainWindowHeaderValLabels main_window_header_labels;

QMap<uint8_t, QFrame*> sensor_frames;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setLabels();

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
    current_date_time_timer = new QTimer(this);
    get_sensor_values_timer = new QTimer(this);

    // Baudrate seçenekleri
    ui->cmbBaudRate->addItems({"4800", "9600", "19200", "115200"});

    connect(ui->btnConnect, &QPushButton::clicked, serial, &Serial::connectSerial);
    //connect(ui->btnSend, &QPushButton::clicked, command_line, &CommandLine::commandLineProcess);
    connect(ui->lineEdit, &QLineEdit::returnPressed, command_line, &CommandLine::commandLineProcess);

    connect(serial->connection_check_timer, &QTimer::timeout, serial, &Serial::checkConnectionStatus);
    connect(serial->connection_check_timer_2, &QTimer::timeout, serial, &Serial::checkConnectionStatus_2);

    connect(serial->serial, &QSerialPort::readyRead, serial, &Serial::readSerial);

    connect(get_calibration_status_timer, &QTimer::timeout, command_line, &CommandLine::getCalStatus);
    connect(get_sensor_values_timer, &QTimer::timeout, command_line, &CommandLine::getSensorValues);

    QDateTime now = QDateTime::currentDateTime();
    QString zaman = now.toString("dd.MM.yyyy hh:mm");
    main_window_header_labels.current_date_time->setText(zaman);

    connect(current_date_time_timer, &QTimer::timeout, this, [=]() {
        QDateTime now = QDateTime::currentDateTime();
        QString zaman = now.toString("dd.MM.yyyy hh:mm");
        main_window_header_labels.current_date_time->setText(zaman);
    });

    cal_status_t.calibration_state = WAIT_STATE;
    is_main_folder_created = file_folder_creator.createMainFolder();
    serial->connection_check_timer->start(100); // Her 100 milisaniyede bir kontrol et
    //serial->connection_check_timer_2->start(2000);
    get_calibration_status_timer->start(18000);
    current_date_time_timer->start(60000);
    //get_sensor_values_timer->start(10000);
}

MainWindow::~MainWindow()
{
    delete get_calibration_status_timer;
    delete current_date_time_timer;
    delete get_sensor_values_timer;
    delete serial;
    delete command_line;
    delete calibration_board;
    delete ui;
}

void MainWindow::setLabels() {
    for (int i = 1; i <= 15; ++i)
    {
        QString frame_name = QString("sensor%1Frame").arg(i);
        QFrame *frame = findChild<QFrame*>(frame_name);

        QString temp_name = QString("sensor%1T").arg(i);
        QLabel *temp_label = findChild<QLabel*>(temp_name);

        QString hum_name = QString("sensor%1H").arg(i);
        QLabel *hum_label = findChild<QLabel*>(hum_name);

        QString r1_name = QString("sensor%1R1").arg(i);
        QLabel *r1_label = findChild<QLabel*>(r1_name);

        QString r2_name = QString("sensor%1R2").arg(i);
        QLabel *r2_label = findChild<QLabel*>(r2_name);

        QString r3_name = QString("sensor%1R3").arg(i);
        QLabel *r3_label = findChild<QLabel*>(r3_name);

        if (frame) {
            sensor_frames.insert(i, frame);
        }

        if (temp_label) {
            temp_labels.insert(i, temp_label);
        }

        if (hum_label) {
            hum_labels.insert(i, hum_label);
        }

        if (r1_label) {
            r1_labels.insert(i, r1_label);
        }

        if (r2_label) {
            r2_labels.insert(i, r2_label);
        }

        if (r3_label) {
            r3_labels.insert(i, r3_label);
        }
    }
    cal_val_labels.cal_point = ui->calPointlbl;
    cal_val_labels.cal_point_start_time = ui->calPointStartTimelbl;
    cal_val_labels.cal_duration = ui->calDurationlbl;
    cal_val_labels.cal_stabilization = ui->calStabilizationTimerlbl;
    cal_val_labels.cal_point_end_time = ui->calPointEndTimelbl;
    cal_val_labels.cal_status = ui->calStatuslbl;
    cal_val_labels.cal_o3_average = ui->o3Averagelbl;
    cal_val_labels.cal_pwm_cyle_period = ui->pwmCyclePeriodlbl;

    main_window_header_labels.cabin_info = ui->CabinInfolbl;
    main_window_header_labels.calibration_start_date = ui->calStartDatelbl;
    main_window_header_labels.calibration_repeat_val = ui->calRepeatVallbl;
    main_window_header_labels.cabin_temp_val = ui->cabinTemplbl;
    main_window_header_labels.cabin_o3_val = ui->cabinO3lbl;
    main_window_header_labels.current_date_time = ui->currentdateTimelbl;
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

void MainWindow::disableBaudCmb()
{
    ui->cmbBaudRate->setEnabled(false);
}

void MainWindow::disableConnectionButton()
{
    ui->btnConnect->setEnabled(false);
}

void MainWindow::onBtnClearClicked()
{
    ui->plainTextEdit->clear();
}

void MainWindow::aboutToExit()
{
    /*QFile log("crash_log.txt");
    log.open(QIODevice::Append | QIODevice::Text);
    QTextStream out(&log);
    out << "std::terminate çağrıldı. Muhtemelen exception yakalanmadı.\n"; */
    command_line->messageBox("Uygulamadan çıkış yapılıyor, son log dosyalarının silinmesini istiyor musunuz");
    //serial->sendData("?r");
    //serial->serial->waitForBytesWritten(2000);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "Uygulama kapanıyor, işlemler yapılıyor...";
    aboutToExit();
    event->accept();
}


