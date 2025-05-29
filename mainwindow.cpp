#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serial.h"
#include "command_line.h"
#include "calibration_board.h"

#include <QCloseEvent>

Creator file_folder_creator;

QDateTime current_dt;

uint8_t sensor_module_status[NUM_OF_SENSOR_BOARD];
uint8_t active_sensor_count;

uint16_t calibration_points[NUM_OF_CAL_POINTS] = {20, 50, 100, 200, 500, 0, 0, 0, 0, 0};
Request current_request;
Request serial_no_request;

QMap<QString, uint8_t> sensor_module_map;
QMap<QString, QString> log_folder_names;

QTimer *get_calibration_status_timer;

Serial *serial;
CommandLine *command_line;
CalibrationBoard *calibration_board;
LogParser *uart_log_parser;

QString line;

uint8_t is_main_folder_created;
uint8_t is_oml_log_folder_created;

QMap<Request, QString> request_commands = {
    { R_ACTIVE_SENSOR_COUNT, "?gpa" },
    { R_CAL_POINTS, "?gpd" },
    { R_CAL_STATUS, "?gpk"},
    { R_CABIN_INFO, "?gpi"},
    { R_SENSOR_VALUES, "?gps"},
    { R_SENSOR_ID, "?gpc"}
};

QMap<Request, uint8_t> request_data_status = {
    { R_ACTIVE_SENSOR_COUNT, 0 },
    { R_CAL_POINTS, 0 },
    { R_CAL_STATUS, 0 },
    { R_CABIN_INFO, 0},
    { R_SENSOR_VALUES, 0},
    { R_SENSOR_ID, 0}
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
    { REPEAT_CALIBRATION_STATE, "REPEAT CALIBRATION STATE"},
    { END_STATE, "END STATE"}
};

QMap<uint8_t, QLabel*> header_labels;
QMap<uint8_t, QLabel*> temp_labels;
QMap<uint8_t, QLabel*> hum_labels;
QMap<uint8_t, QLabel*> r1_labels;
QMap<uint8_t, QLabel*> r2_labels;
QMap<uint8_t, QLabel*> r3_labels;

CalibrationValLabels cal_val_labels;
MainWindowHeaderValLabels main_window_header_labels;

QMap<uint8_t, QFrame*> sensor_frames;
QMap<uint16_t, uint32_t> sensors_serial_no;
QMap<uint8_t, uint8_t> sensors_eeprom_is_data_exist;

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
    uart_log_parser = new LogParser(this);
    uart_log_parser->setMainWindow(this);

    get_calibration_status_timer = new QTimer(this);
    current_date_time_timer = new QTimer(this);

    // Baudrate seçenekleri
    ui->cmbBaudRate->addItems({"4800", "9600", "19200", "115200"});

    connect(ui->btnConnect, &QPushButton::clicked, serial, &Serial::connectSerial);
    connect(ui->btnClear, &QPushButton::clicked, this, &MainWindow::onBtnClearClicked);
    //connect(ui->btnSend, &QPushButton::clicked, command_line, &CommandLine::commandLineProcess);
    connect(ui->lineEdit, &QLineEdit::returnPressed, command_line, &CommandLine::commandLineProcess);

    connect(serial->connection_check_timer, &QTimer::timeout, serial, &Serial::checkConnectionStatus);

    connect(serial->serial, &QSerialPort::readyRead, serial, &Serial::readSerial);

    connect(get_calibration_status_timer, &QTimer::timeout, command_line, &CommandLine::getPeriodicData);

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
    calibration_board->readLogDirectoryPaths();
    serial->connection_check_timer->start(100); // Her 100 milisaniyede bir kontrol et

    get_calibration_status_timer->start(18000);
    current_date_time_timer->start(60000);
}

MainWindow::~MainWindow()
{
    delete get_calibration_status_timer;
    delete current_date_time_timer;
    delete serial;
    delete command_line;
    delete calibration_board;
    delete uart_log_parser;
    delete ui;
}

void MainWindow::setLabels() {
    for (int i = 1; i <= 15; ++i)
    {
        QString frame_name = QString("sensor%1Frame").arg(i);
        QFrame *frame = findChild<QFrame*>(frame_name);

        QString header_name = QString("sensor%1Headerlbl").arg(i);
        QLabel *header_label = findChild<QLabel*>(header_name);

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

        if (header_label) {
            header_labels.insert(i, header_label);
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
    cal_val_labels.cal_next_cal_start_duration = ui->nextCalStartDurationlbl;
    cal_val_labels.cal_clean_air_duration = ui->cleanAirDurationlbl;
    cal_val_labels.cal_const_cal_temp = ui->calTemplbl;
    cal_val_labels.cal_sensitivity = ui->calSensitivitylbl;
    cal_val_labels.cal_stabilization_duration = ui->stabilizationDurationlbl;
    cal_val_labels.cal_zero_cal_conc = ui->zeroCalConclbl;
    cal_val_labels.cal_point_end_time = ui->calPointEndTimelbl;
    cal_val_labels.cal_status = ui->calStatuslbl;
    cal_val_labels.cal_o3_average = ui->o3Averagelbl;

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


