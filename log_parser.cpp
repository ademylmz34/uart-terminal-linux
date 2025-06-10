#include "mainwindow.h"
#include "log_parser.h"
#include <stdint.h>
#include <string.h>

int kal_point;
int kal_point_val;
int cal_ppb_cal_time;
int cabin_no;

int calibration_repeat_count;

char buff[256];
char str[50];

QDateTime calibration_start_dt;
QDateTime calibration_end_dt;
QDateTime calibration_ppb_start_dt;
QDateTime calibration_ppb_end_dt;

CalibrationStatus cal_status_t = { .o3_average = 0, .calibration_ppb = 0, .calibration_state = 0, .repeat_calibration = 0,
                                  .next_calibration_start_duration = 0, .calibration_sensitivity = 0, .zero_cal_conc = 0,
                                  .stabilization_duration = 0, .calibration_temp = 0, .clean_air_duration = 0};

LogParser::LogParser(QObject *parent): QObject(parent)
{
    repeat_calibration_index = 0;
    calibration_completed = 0;
    received_serial_no_count = 1;
    initCommandFunctions();
}

LogParser::~LogParser()
{
}

void LogParser::initCommandFunctions()
{
    cmd_funcs.insert(CMD_OM,        [this]() { this->parse_om_data(); });
    cmd_funcs.insert(CMD_KN_S,      [this]() { this->parse_kn_s_data(); });
    cmd_funcs.insert(CMD_KB_S_R,    [this]() { this->parse_kb_s_r_data(); });
    cmd_funcs.insert(CMD_KB_S_OR,   [this]() { this->parse_kb_s_or_data(); });
    cmd_funcs.insert(CMD_KB_S_KN,   [this]() { this->parse_kb_s_kn_data(); });
    cmd_funcs.insert(CMD_KB_S_KN_B, [this]() { this->parse_kb_s_kn_data(); });
    cmd_funcs.insert(CMD_KN_O3,     [this]() { this->parse_kn_float_data(); });
    cmd_funcs.insert(CMD_KN_T,      [this]() { this->parse_kn_float_data(); });
    cmd_funcs.insert(CMD_KN_H,      [this]() { this->parse_kn_float_data(); });
    cmd_funcs.insert(CMD_KN_CT,     [this]() { this->parse_kn_float_data(); });
    cmd_funcs.insert(CMD_KN_CP,     [this]() { this->parse_kn_float_data(); });
    cmd_funcs.insert(CMD_KN_FR,     [this]() { this->parse_kn_decimal_data(); });
    cmd_funcs.insert(CMD_KN_PV,     [this]() { this->parse_kn_float_data(); });
    cmd_funcs.insert(CMD_KN_PWM,    [this]() { this->parse_pwm_data(); });
    cmd_funcs.insert(CMD_KS,        [this]() { this->parse_ks_data(); });

    cmd_funcs.insert(CMD_D_SMS,     [this]() { this->parse_sms_data(); });
    cmd_funcs.insert(CMD_D_RV,      [this]() { this->parse_rv_data(); });
    cmd_funcs.insert(CMD_D_CI,      [this]() { this->parse_ci_data(); });
    cmd_funcs.insert(CMD_D_CS,      [this]() { this->parse_cs_data(); });
    cmd_funcs.insert(CMD_D_CP,      [this]() { this->parse_cp_data(); });
    cmd_funcs.insert(CMD_D_SK,      [this]() { this->parse_sk_data(); });
    cmd_funcs.insert(CMD_D_TH,      [this]() { this->parse_th_data(); });
    cmd_funcs.insert(CMD_D_SN,      [this]() { this->parse_sn_data(); });

    cmd_funcs.insert(CMD_KL,        [this]() { this->log_data(); });
    cmd_funcs.insert(CMD_L,         [this]() { this->log_data(); });
}

LogParser::ParsedCommand LogParser::parseCommandExtended(const char* cmd) {
    ParsedCommand result = { .type = CMD_UNKNOWN, .th_no = 0, .kn_no = 0, .s_no = 0, .or_no = 0 };

    if (strcmp(cmd, "R1") == 0) result.type = CMD_R1;
    else if (strcmp(cmd, "R2") == 0) result.type = CMD_R2;
    else if (strcmp(cmd, "R3") == 0) result.type = CMD_R3;

    else if (strncmp(cmd, "D_SMS", 5) == 0) result.type = CMD_D_SMS;
    else if (strncmp(cmd, "D_TH" , 4) == 0) result.type = CMD_D_TH;
    else if (strncmp(cmd, "D_SN" , 4) == 0) result.type = CMD_D_SN;
    else if (strncmp(cmd, "D_RV" , 4) == 0) result.type = CMD_D_RV;
    else if (strncmp(cmd, "D_CI" , 4) == 0) result.type = CMD_D_CI;
    else if (strncmp(cmd, "D_CS" , 4) == 0) result.type = CMD_D_CS;
    else if (strncmp(cmd, "D_CP" , 4) == 0) result.type = CMD_D_CP;
    else if (strncmp(cmd, "D_SK" , 4) == 0) {
        int s_no;
        if (sscanf(cmd, "D_SK-%d", &s_no) == 1) {
            result.type = CMD_D_SK;
            result.s_no = s_no;
        }
    }

    else if (strcmp(cmd, "RST") == 0) result.type = CMD_RST;
    else if (strcmp(cmd, "OM" ) == 0) result.type = CMD_OM;
    else if (strcmp(cmd, "L"  ) == 0) result.type = CMD_L;
    else if (strcmp(cmd, "KL" ) == 0) result.type = CMD_KL;
    else if (strcmp(cmd, "KS" ) == 0) result.type = CMD_KS;
    else if (strncmp(cmd, "KN", 2) == 0) {
        int kn, s;
        if (strstr(cmd, "-S")) {
            if (sscanf(cmd, "KN%d-S%d", &kn, &s) == 2) {
                if (kn >= 0 && kn <= 4 && s >= 1 && s <= 15) {
                    result.type = CMD_KN_S;
                    result.kn_no = kn;
                    result.s_no = s;
                }
            }
        }
        else if (strstr(cmd, "-O3")) {
            if (sscanf(cmd, "KN%d-O3", &kn) == 1 && kn >= 0 && kn <= 4) {
                result.type = CMD_KN_O3;
                result.kn_no = kn;
            }
        }
        else if (strstr(cmd, "-T")) {
            if (sscanf(cmd, "KN%d-T", &kn) == 1 && kn >= 0 && kn <= 4) {
                result.type = CMD_KN_T;
                result.kn_no = kn;
            }
        }
        else if (strstr(cmd, "-H")) {
            if (sscanf(cmd, "KN%d-H", &kn) == 1 && kn >= 0 && kn <= 4) {
                result.type = CMD_KN_H;
                result.kn_no = kn;
            }
        }
        else if (strstr(cmd, "-PWM")) {
            if (sscanf(cmd, "KN%d-PWM", &kn) == 1 && kn >= 0 && kn <= 4) {
                result.type = CMD_KN_PWM;
                result.kn_no = kn;
            }
        }
        else if (strstr(cmd, "-CT")) {
            if (sscanf(cmd, "KN%d-CT", &kn) == 1 && kn >= 0 && kn <= 4) {
                result.type = CMD_KN_CT;
                result.kn_no = kn;
            }
        }
        else if (strstr(cmd, "-CP")) {
            if (sscanf(cmd, "KN%d-CP", &kn) == 1 && kn >= 0 && kn <= 4) {
                result.type = CMD_KN_CP;
                result.kn_no = kn;
            }
        }
        else if (strstr(cmd, "-FR")) {
            if (sscanf(cmd, "KN%d-FR", &kn) == 1 && kn >= 0 && kn <= 4) {
                result.type = CMD_KN_FR;
                result.kn_no = kn;
            }
        }
        else if (strstr(cmd, "-PV")) {
            if (sscanf(cmd, "KN%d-PV", &kn) == 1 && kn >= 0 && kn <= 4) {
                result.type = CMD_KN_PV;
                result.kn_no = kn;
            }
        }
    }
    else if (strncmp(cmd, "KB-S", 4) == 0) {
        int s, or_no, kn;

        if (strstr(cmd, "-KN") && strstr(cmd, "-B")) {
            // Eğer hem KN hem de B varsa -> KB-S%d-KN%d-B
            if (sscanf(cmd, "KB-S%d-KN%d-B", &s, &kn) == 2) {
                result.type = CMD_KB_S_KN_B;
                result.s_no = s;
                result.kn_no = kn;
            }
        }
        else if (strstr(cmd, "-KN")) {
            // Sadece KN varsa (ama B yok) -> KB-S%d-KN%d
            if (sscanf(cmd, "KB-S%d-KN%d", &s, &kn) == 2) {
                result.type = CMD_KB_S_KN;
                result.s_no = s;
                result.kn_no = kn;
            }
        }
        else if (strstr(cmd, "-OR-")) {
            if (sscanf(cmd, "KB-S%d-OR-%d", &s, &or_no) == 2) {
                result.type = CMD_KB_S_OR;
                result.s_no = s;
                result.or_no = or_no;
            }
        }
        else if (strstr(cmd, "-R")) {
            if (sscanf(cmd, "KB-S%d-R", &s) == 1) {
                result.type = CMD_KB_S_R;
                result.s_no = s;
            }
        }
    }
    else if (strncmp(cmd, "KB", 2) == 0) {
        int kt_no;
        if (sscanf(cmd, "KB%d", &kt_no) == 1) {
            repeat_calibration_index = (calibration_repeat_count - kt_no) + 1;
            if (repeat_calibration_index < calibration_repeat_count)
                calibration_completed = 0;
            else
                calibration_completed = 1;
        }
    }
    return result;
}

CalibrationStates LogParser::getCalibrationState(int calibration_state_int)
{
    switch (calibration_state_int)
    {
    case 0: return WAIT_STATE; break;
    case 1: return CLEAN_AIR_STATE; break;
    case 2: return SET_ENVIRONMENT_CONDITIONS_STATE; break;
    case 3: return ZERO_CALIBRATION_STATE; break;
    case 4: return SPAN_CALIBRATION_START_STATE; break;
    case 5: return SPAN_CALIBRATION_MID_STATE; break;
    case 6: return SPAN_CALIBRATION_END_STATE; break;
    case 7: return RETURN_TO_ZERO_STATE; break;
    case 8: return REPEAT_CALIBRATION_STATE; break;
    }
    return WAIT_STATE;
}

void LogParser::setMainWindow(MainWindow *mw)
{
    mainWindow = mw;
}


void LogParser::parse_om_data() {
    float OM106L_o3_level, OM106L_temp, OM106L_pressure, OM106L_voltage;
    int OM106L_flow;
    if (sscanf(packet.data_str, "%f %f %f %d %f", &OM106L_o3_level, &OM106L_temp, &OM106L_pressure, &OM106L_flow, &OM106L_voltage) == 5)
    {
        sprintf(buff, ">%s %s %s %.3f %.3f %.3f %d %.3f", packet.date, packet.time, packet.command_str, OM106L_o3_level, OM106L_temp, OM106L_pressure,
                OM106L_flow, OM106L_voltage );
        if (om106_map[DEVICE_1].om106_stream != NULL) {
            *(om106_map[DEVICE_1].om106_stream) << buff << "\n"; // log'un hangi om106l cihazından geldiğini bilmem lazım
            om106_map[DEVICE_1].om106_stream->flush();
        }
    }
}

void LogParser::parse_kn_s_data() {
    int r1_val, r2_val, r3_val;
    if (sscanf(packet.data_str, "%d %d %d", &r1_val, &r2_val, &r3_val) == 3)
    {
        sprintf(buff, ">%s %s %s %d %d %d", packet.date, packet.time, packet.command_str, r1_val, r2_val, r3_val);
        if (sensor_map[packet.command.s_no].kal_stream != NULL) {
            *(sensor_map[packet.command.s_no].kal_stream) << buff << "\n";
            sensor_map[packet.command.s_no].kal_stream->flush();
        }
    }
}

void LogParser::parse_kb_s_r_data() {
    float alpha, beta;
    int rs0, rs100, rs1000;
    if (sscanf(packet.data_str, "Rs0: %d Rs100: %d Rs1000: %d alpha: %f beta: %f", &rs0, &rs100, &rs1000, &alpha, &beta) == 5)
    {
        sprintf(buff, ">%s %s %s Rs0: %d Rs100: %d Rs1000: %d alpha: %.5f beta: %.5f", packet.date, packet.time, packet.command_str, rs0, rs100, rs1000, alpha, beta);
        if (sensor_map[packet.command.s_no].kal_stream != NULL) {
            *(sensor_map[packet.command.s_no].kal_stream) << buff << "\n";
            sensor_map[packet.command.s_no].kal_stream->flush();
        }
    }
}

void LogParser::parse_kb_s_or_data() {
    float o3, logO3, rs, logRR;
    if (sscanf(packet.data_str, "O3: %f logO3: %f Rs: %f logRR: %f", &o3, &logO3, &rs, &logRR) == 4)
    {
        sprintf(buff, ">%s %s %s O3: %f logO3: %.5f Rs: %f logRR: %.5f", packet.date, packet.time, packet.command_str, o3, logO3, rs, logRR);
        if (sensor_map[packet.command.s_no].kal_end_stream != NULL)
        {
            *(sensor_map[packet.command.s_no].kal_end_stream) << buff << "\n";
            sensor_map[packet.command.s_no].kal_end_stream->flush();
        }
    }
}

void LogParser::parse_kb_s_kn_data() {
    int o3, rs;
    if (sscanf(packet.data_str, "O3: %d Rs: %d ", &o3, &rs) == 2)
    {
        sprintf(buff, ">%s %s %s O3: %d Rs: %d", packet.date, packet.time, packet.command_str, o3, rs);
        if (sensor_map[packet.command.s_no].kal_end_stream != NULL)
        {
            *(sensor_map[packet.command.s_no].kal_end_stream) << buff << "\n";
            sensor_map[packet.command.s_no].kal_end_stream->flush();
        }
    }
}

void LogParser::parse_kn_float_data() {
    float value;
    if (sscanf(packet.data_str, "%f", &value) == 1)
    {
        sprintf(buff, ">%s %s %s %.3f\n", packet.date, packet.time, packet.command_str, value);
        if (calibration_stream != NULL) {
            *(calibration_stream) << buff;
            calibration_stream->flush();
        }
    }
}

void LogParser::parse_kn_decimal_data() {
    int value;
    if (sscanf(packet.data_str, "%d", &value) == 1)
    {
        sprintf(buff, ">%s %s %s %d\n", packet.date, packet.time, packet.command_str, value);
        if (calibration_stream != NULL) {
            *(calibration_stream) << buff;
            calibration_stream->flush();
        }
    }
}

void LogParser::parse_pwm_data() {
    int pwm_cycle, pwm_period;
    if (sscanf(packet.data_str, "%d/%dmsec", &pwm_cycle, &pwm_period) == 2)
    {
        sprintf(buff, ">%s %s %s %d/%dmsec\n", packet.date, packet.time, packet.command_str, pwm_cycle, pwm_period);
        if (calibration_stream != NULL) {
            *(calibration_stream) << buff;
            calibration_stream->flush();
        }
    }
}

void LogParser::parse_sk_data() {
    float om, t, h;
    int r1, r2, r3;
    if (sscanf(packet.data_str, "OM: %f R1: %d R2: %d R3: %d T: %f H: %f", &om, &r1, &r2, &r3, &t, &h) == 6)
    {
        sprintf(buff, ">%s %s %.2f %d %d %d %.2f %.2f", packet.date, packet.time, om, r1, r2, r3, t, h);
        temp_labels[packet.command.s_no]->setText(QString("%1 °C").arg(QString::number(t, 'f', 2)));
        hum_labels[packet.command.s_no]->setText(QString("%1 %").arg(QString::number(h, 'f', 2)));
        r1_labels[packet.command.s_no]->setText(QString("%1").arg(QString::number(r1, 'f', 0)));
        r2_labels[packet.command.s_no]->setText(QString("%1").arg(QString::number(r2, 'f', 0)));
        r3_labels[packet.command.s_no]->setText(QString("%1").arg(QString::number(r3, 'f', 0)));

        if (calibration_board->getRequestStatus(R_SENSOR_VALUES) == SENT) calibration_board->updateRequestStatus(R_SENSOR_VALUES, RECEIVED);

        if (sensor_map[packet.command.s_no].log_stream != NULL) {
            *(sensor_map[packet.command.s_no].log_stream) << buff << "\n";
            sensor_map[packet.command.s_no].log_stream->flush();
        }
    }
}

void LogParser::parse_ks_data() {
    int year, month, day, hour, minute, second, cal_ppb;
    QDate date;
    QTime time;

    sprintf(buff, ">%s %s %s\n", packet.date, packet.time, packet.data_str);
    if (calibration_stream != NULL) {
        *(calibration_stream) << buff;
        calibration_stream->flush();
    }

    if (sscanf(packet.data_str, "KalBasSaati %d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6)
    {
        date.setDate(year, month, day);
        time.setHMS(hour, minute, second);
        calibration_start_dt = QDateTime(date, time);
        main_window_header_labels.calibration_start_date->setText(calibration_start_dt.toString("hh:mm"));
    } else if (sscanf(packet.data_str, "KalBitisSaati %d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6)
    {
        date.setDate(year, month, day);
        time.setHMS(hour, minute, second);
        calibration_end_dt = QDateTime(date, time);
    } else if (sscanf(packet.data_str, "Kal%d-BasSaati %d-%d-%d %d:%d:%d", &cal_ppb, &year, &month, &day, &hour, &minute, &second) == 7)
    {
        date.setDate(year, month, day);
        time.setHMS(hour, minute, second);
        calibration_ppb_start_dt = QDateTime(date, time);
        cal_ppb_cal_time = cal_ppb;
        cal_val_labels.cal_point_start_time->setText(calibration_ppb_start_dt.toString("hh:mm"));
    } else if (sscanf(packet.data_str, "Kal%d-BitisSaati %d-%d-%d %d:%d:%d", &cal_ppb, &year, &month, &day, &hour, &minute, &second) == 7)
    {
        date.setDate(year, month, day);
        time.setHMS(hour, minute, second);
        calibration_ppb_end_dt = QDateTime(date, time);
        cal_ppb_cal_time = cal_ppb;
        cal_val_labels.cal_point_end_time->setText(calibration_ppb_end_dt.toString("hh:mm"));
        cal_val_labels.cal_ppb_for_end_time->setText(QString("%1 ppb").arg(cal_ppb));
    } else
    {
        qDebug() << "Tarih ve saat ayrıştırılamadı!";
    }
}

void LogParser::parse_sms_data() {
    char bitStr[32];
    char valStr[8];
    size_t len;

    sscanf(packet.data_str, "%s %s", bitStr, valStr);  // boşlukla ayır

    len = strlen(bitStr);
    for (uint8_t i = 0; i < len; i++) {
        if (bitStr[i] == '1') {
            sensor_module_status[i] = 1;
            sensor_frames[i + 1]->setStyleSheet("background-color: rgb(38, 162, 105);");
        } else if (bitStr[i] == '0') {
            sensor_module_status[i] = 0;
            sensor_frames[i + 1]->setStyleSheet("background-color: rgb(165, 29, 45);");
        } else {
            printf("Geçersiz karakter: %c\n", bitStr[i]);
        }
    }

    active_sensor_count = (uint8_t)atoi(valStr);
    if (active_sensor_count) {
        if (calibration_board->getRequestStatus(R_ACTIVE_SENSOR_COUNT) == SENT) calibration_board->updateRequestStatus(R_ACTIVE_SENSOR_COUNT, RECEIVED);
    }
}

void LogParser::parse_rv_data() {
    int r1, r2, r3, resistance_count;
    if (sscanf(packet.data_str, "%d %d %d %d", &r1, &r2, &r3, &resistance_count) == 4) {
        cal_val_labels.cal_r1_value->setText(QString("%1 Ω").arg(r1));
        cal_val_labels.cal_r2_value->setText(QString("%1 Ω").arg(r2));
        cal_val_labels.cal_r3_value->setText(QString("%1 Ω").arg(r3));
        number_of_resistors2calibrate = resistance_count;

        if (calibration_board->getRequestStatus(R_RESISTANCE_VALUES) == SENT) calibration_board->updateRequestStatus(R_RESISTANCE_VALUES, RECEIVED);
    }
}

void LogParser::parse_ci_data() {
    if (sscanf(packet.data_str, "Kabin-%d", &cabin_no) == 1)
    {
        if (cabin_no == 1) main_window_header_labels.cabin_info->setText("Kabin-1 Kalibrasyon");
        else if (cabin_no == 2) main_window_header_labels.cabin_info->setText("Kabin-2 Kalibrasyon");

        if (calibration_board->getRequestStatus(R_CABIN_INFO) == SENT) calibration_board->updateRequestStatus(R_CABIN_INFO, RECEIVED);
    }
}

void LogParser::parse_cs_data() {
    float o3_average;
    int calibration_ppb;
    int calibration_state;
    int repeat_calibration;
    int next_calibration_start_duration;
    int calibration_sensitivity;
    int zero_cal_conc;
    int stabilization_duration;
    int calibration_temp;
    int clean_air_duration;

    if (sscanf(packet.data_str, "%d %d %d %f %d %d %d %d %d %d", &calibration_ppb, &calibration_state, &repeat_calibration, &o3_average,
               &next_calibration_start_duration, &calibration_sensitivity, &zero_cal_conc, &stabilization_duration,
               &calibration_temp, &clean_air_duration) == 10
        ) {
        if (cal_status_t.calibration_ppb != calibration_ppb) {
            cal_status_t.calibration_ppb = calibration_ppb;
            cal_val_labels.cal_point->setText(QString("%1 PPB").arg(QString::number(calibration_ppb)));
        }
        if (cal_status_t.calibration_state != calibration_state) {
            cal_status_t.calibration_state = calibration_state;
            cal_val_labels.cal_status->setText(calibration_state_str[getCalibrationState(calibration_state)]);
            if (calibration_state == END_STATE) {
                calibration_board->clearLogDirectoryPathsFile();
                file_folder_creator.freeFiles();
                cal_status_t.calibration_state = WAIT_STATE;
                mainWindow->setLineEditText("KALİBRASYON TAMAMLANDI!!!");
            }
        }
        if (cal_status_t.repeat_calibration != repeat_calibration) {
            cal_status_t.repeat_calibration = repeat_calibration;
            main_window_header_labels.calibration_repeat_val->setText(QString::number(repeat_calibration));
        }
        if (cal_status_t.o3_average != o3_average) {
            cal_status_t.o3_average = o3_average;
            cal_val_labels.cal_o3_average->setText(QString("%1 PPB").arg(QString::number(o3_average)));
        }
        if (cal_status_t.next_calibration_start_duration != next_calibration_start_duration) {
            cal_status_t.next_calibration_start_duration = next_calibration_start_duration;
            cal_val_labels.cal_next_cal_start_duration->setText(QString::number(next_calibration_start_duration));
        }
        if (cal_status_t.calibration_sensitivity != calibration_sensitivity) {
            cal_status_t.calibration_sensitivity = calibration_sensitivity;
            cal_val_labels.cal_sensitivity->setText(QString::number(calibration_sensitivity));
        }
        if (cal_status_t.zero_cal_conc != zero_cal_conc) {
            cal_status_t.zero_cal_conc = zero_cal_conc;
            cal_val_labels.cal_zero_cal_conc->setText(QString::number(zero_cal_conc));
        }
        if (cal_status_t.stabilization_duration != stabilization_duration) {
            cal_status_t.stabilization_duration = stabilization_duration;
            cal_val_labels.cal_stabilization_duration->setText(QString::number(stabilization_duration));
        }
        if (cal_status_t.calibration_temp != calibration_temp) {
            cal_status_t.calibration_temp = calibration_temp;
            cal_val_labels.cal_const_cal_temp->setText(QString::number(calibration_temp));
        }
        if (cal_status_t.clean_air_duration != clean_air_duration) {
            cal_status_t.clean_air_duration = clean_air_duration;
            cal_val_labels.cal_clean_air_duration->setText(QString::number(clean_air_duration));
        }

        if (calibration_board->getRequestStatus(R_CAL_STATUS) == SENT) calibration_board->updateRequestStatus(R_CAL_STATUS, RECEIVED);
    }
}

void LogParser::parse_cp_data() {
    if (sscanf(packet.data_str, "KN%d %d", &kal_point, &kal_point_val) == 2)
    {
        //if (kal_point_val == 0) return;
        calibration_points[kal_point] = kal_point_val;
        //sprintf(buff, "L KN%d %d", kal_point, kal_point_val);
        //qDebug() << buff;
        if (calibration_board->getRequestStatus(R_CAL_POINTS) == SENT) calibration_board->updateRequestStatus(R_CAL_POINTS, RECEIVED);
    }
}

void LogParser::parse_th_data() {
    float temp, hum;
    if (sscanf(packet.data_str, "%f C %f% %", &temp, &hum) == 2) main_window_header_labels.cabin_temp_val->setText(QString("%1 C").arg(temp));
}

void LogParser::parse_sn_data() {
    int sensor_no, serial_no;

    if (sscanf(packet.data_str, "s%d: %d", &sensor_no, &serial_no) == 2) {
        QString serial_no_str = QString("s%1").arg(QString::number(serial_no));
        if (!sensors_serial_no.contains(serial_no_str)) sensors_serial_no.insert(serial_no_str, sensor_no); // example: s3105 -> 1
        if (!sensors_eeprom_is_data_exist.contains(sensor_no)) sensors_eeprom_is_data_exist.insert(sensor_no, 1);

        if (received_serial_no_count++ == active_sensor_count) {
            if (calibration_board->getRequestStatus(R_SENSOR_ID) == SENT) calibration_board->updateRequestStatus(R_SENSOR_ID, RECEIVED);
            received_serial_no_count = 1;
        }
        header_labels[sensor_no]->setText(QString("s%1").arg(QString::number(serial_no)));

    } else if (sscanf(packet.data_str, "s%d bilgileri bos", &sensor_no) == 1) {
        if (received_serial_no_count++ == active_sensor_count) {
            if (calibration_board->getRequestStatus(R_SENSOR_ID) == SENT) calibration_board->updateRequestStatus(R_SENSOR_ID, RECEIVED);
            received_serial_no_count = 1;
        }
        // ?spn s3104 0 s3105 0 s3106 s3107 s3108 s3109 s3110 s3111 s3112 s3113 s3114 s3115 s3116
        if (!sensors_eeprom_is_data_exist.contains(sensor_no)) sensors_eeprom_is_data_exist.insert(sensor_no, 0);
        header_labels[sensor_no]->setText("Veri Yok");

    } else if (sscanf(packet.data_str, "Sensor-%d seri numarasi EEPROM'a yazilamadi: %d", &sensor_no, &serial_no) == 2) {
        mainWindow->setLineEditText(QString("Sensor-%1 seri numarasi %2 EEPROM'a yazilamadi.").arg(QString::number(sensor_no)).arg(QString::number(serial_no)));

    } else if (sscanf(packet.data_str, "Sensor-%d seri numarasi EEPROM'a yazildi: %d", &sensor_no, &serial_no) == 2) {
        mainWindow->setLineEditText(QString("Sensor-%1 seri numarasi %2 EEPROM'a yazildi.").arg(QString::number(sensor_no)).arg(QString::number(serial_no)));

    } else if (sscanf(packet.data_str, "Sensor-%d seri numarasi degistirilemedi: %d", &sensor_no, &serial_no) == 2) {
        mainWindow->setLineEditText(QString("Sensor-%1 seri numarasi %2 degistirilemedi.").arg(QString::number(sensor_no)).arg(QString::number(serial_no)));
        serial_no_changed = false;

    } else if (sscanf(packet.data_str, "Sensor-%d seri numarasi degistirildi: %d", &sensor_no, &serial_no) == 2) {
        mainWindow->setLineEditText(QString("Sensor-%1 seri numarasi %2 degistirildi.").arg(QString::number(sensor_no)).arg(QString::number(serial_no)));
        QString older_folder_name = calibration_board->findSensorFolderNameByValue(sensor_no);
        QString new_folder_name = QString("s%1").arg(QString::number(serial_no));
        sensors_serial_no.remove(older_folder_name);
        sensors_serial_no.insert(new_folder_name, sensor_no);
        serial_no_changed = true;
    }
}

void LogParser::log_data() {
    sprintf(buff, ">%s %s %s\n", packet.date, packet.time, packet.data_str);
    if (packet.command.type != CMD_KL) {
        if (calibration_stream != NULL) {
            *(calibration_stream) << buff;
            calibration_stream->flush();
        }
    } else if (packet.command.type != CMD_L) {
        if (main_log_stream != NULL) {
            *(main_log_stream) << buff;
            main_log_stream->flush();
        }
    }
}

bool LogParser::parseLine(const char* input, Packet packet) {
    char command[16];
    uint8_t i = 0;

    if (input[0] != '>') return false;

    line = QString::fromUtf8(input);

    sscanf(input, ">%10s %8s", packet.date, packet.time); // tarih ve saat erişim

    const char* p = input + 21;

    while (*p != '\0' && *p != ' ' && i < (int)(sizeof(command) - 1)) command[i++] = *p++; // Komut'a erişim
    command[i] = '\0';

    packet.command = parseCommandExtended(command);
    packet.command_str = strdup(command);

    if (packet.command.type != CMD_D_TH && packet.command.type != CMD_D_SN && packet.command.type != CMD_D_SK &&
        packet.command.type != CMD_OM && packet.command.type != CMD_D_SMS )
        mainWindow->setLineEditText(line);

    if (*p == ' ') p++; // Komut sonrası veriye erişim
    if (*p == '\0') return false;

    packet.data_str = strdup(p);
    cmd_funcs[packet.command.type]();
    return true;
}
