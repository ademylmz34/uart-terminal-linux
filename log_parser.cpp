#include "mainwindow.h"
#include "log_parser.h"
#include <stdint.h>
#include <string.h>

int kal_point;
int kal_point_val;
int cal_ppb_cal_time;
int cabin_no;

int calibration_repeat_count;

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
}

LogParser::~LogParser()
{
}

void LogParser::setMainWindow(MainWindow *mw) {
    mainWindow = mw;
}

LogParser::ParsedCommand LogParser::parseCommandExtended(const char* cmd) {
    ParsedCommand result = { .type = CMD_UNKNOWN, .th_no = -1, .kn_no = -1, .s_no = -1, .or_no = -1 };

    if (strcmp(cmd, "R1") == 0) result.type = CMD_R1;
    else if (strcmp(cmd, "R2") == 0) result.type = CMD_R2;
    else if (strcmp(cmd, "R3") == 0) result.type = CMD_R3;
    else if (strcmp(cmd, "SMS") == 0) result.type = CMD_SMS;
    else if (strcmp(cmd, "RST") == 0) result.type = CMD_RST;
    else if (strncmp(cmd, "SN", 2) == 0) result.type = CMD_SN;
    else if (strncmp(cmd, "SK", 2) == 0) {
        int s_no;
        if (sscanf(cmd, "SK-%d", &s_no) == 1) {
            result.type = CMD_SK;
            result.s_no = s_no;
        }
    }
    else if (strncmp(cmd, "TH", 2) == 0) {
        int no = atoi(cmd + 2);
        if (no >= 1 && no <= 15) {
            result.type = CMD_TH;
            result.s_no = no;
        }
    }
    else if (strcmp(cmd, "OM") == 0) result.type = CMD_OM;
    else if (strcmp(cmd, "L") == 0) result.type = CMD_L;
    else if (strcmp(cmd, "D") == 0) result.type = CMD_D;
    else if (strcmp(cmd, "KL") == 0) result.type = CMD_KL;
    else if (strcmp(cmd, "KS") == 0) result.type = CMD_KS;
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

int8_t LogParser::parsePwmData(const char* input, PWMData* pwm_data) {
    int pwm_duty, pwm_period;
    if (sscanf(input, "%d/%dmsec", &pwm_duty, &pwm_period) == 2) {
        pwm_data->duty = pwm_duty;
        pwm_data->period = pwm_period;
    } else {
        return -1;
    }

    return 0;
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

void LogParser::parseCalibrationData(const char* input)
{
    char buff[256];
    char bitStr[32];
    char valStr[8];

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

    int sensor_no;
    int serial_no;

    size_t len;

    switch(current_request) {
        case R_ACTIVE_SENSOR_COUNT:
            sscanf(input, "%s %s", bitStr, valStr);  // boşlukla ayır

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
                request_data_status[current_request] = 1;
            }
            break;
        case R_SENSOR_ID:
            if (sscanf(input, "s%d: %d", &sensor_no, &serial_no) == 2) {
                if (received_serial_no_count++ == active_sensor_count) {
                    request_data_status[current_request] = 1;
                    received_serial_no_count = 1;
                }
                header_labels[sensor_no]->setText(QString("s%1").arg(QString::number(serial_no)));
                sensors_serial_no.insert(sensor_no, serial_no);
                sensors_eeprom_is_data_exist.insert(sensor_no, 1);
            } else if (sscanf(input, "s%d bilgileri bos", &sensor_no) == 1) {
                if (received_serial_no_count++ == active_sensor_count) {
                    request_data_status[current_request] = 1;
                    received_serial_no_count = 1;
                }
                sensors_eeprom_is_data_exist.insert(sensor_no, 0);
                header_labels[sensor_no]->setText("Veri Yok");
            }
            break;
        case R_CAL_POINTS:
            if (sscanf(input, "KN%d %d", &kal_point, &kal_point_val) == 2)
            {
                //if (kal_point_val == 0) return;
                calibration_points[kal_point] = kal_point_val;
                sprintf(buff, "L KN%d %d", kal_point, kal_point_val);
                qDebug() << buff;
                request_data_status[current_request] = 1;
            }
            break;

        case R_CABIN_INFO:
            if (sscanf(input, "Kabin-%d", &cabin_no) == 1)
            {
                if (cabin_no == 1) main_window_header_labels.cabin_info->setText("Kabin-1 Kalibrasyon");
                else if (cabin_no == 2) main_window_header_labels.cabin_info->setText("Kabin-2 Kalibrasyon");
                request_data_status[current_request] = 1;
            }
            break;

        case R_CAL_STATUS:
            if (sscanf(input, "%d %d %d %f %d %d %d %d %d %d", &calibration_ppb, &calibration_state, &repeat_calibration, &o3_average,
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

                request_data_status[current_request] = 1;
            }
            break;

        case R_SENSOR_VALUES:
            break;
        default:
            break;
    }
}

void LogParser::parseSerialNoData(const char* input) {
    int sensor_no, serial_no;

    if (sscanf(input, "s%d: %d", &sensor_no, &serial_no) == 2) {
        if (!sensors_serial_no.contains(sensor_no)) sensors_serial_no.insert(sensor_no, serial_no);
        if (!sensors_eeprom_is_data_exist.contains(sensor_no)) sensors_eeprom_is_data_exist.insert(sensor_no, 1);

        QString serial_no_str = QString("s%1").arg(QString::number(serial_no));
        if (received_serial_no_count++ == active_sensor_count) {
            request_data_status[serial_no_request] = 1;
            received_serial_no_count = 1;
        }
        if (serial_no_request == NONE)
        {
            if (serial_no != 0)
            {
                if (!sensor_module_map.contains(serial_no_str)) sensor_module_map.insert(serial_no_str,  sensor_no);
                if (!sensor_ids.contains(serial_no_str)) sensor_ids.append(serial_no_str);
            }
        }
        header_labels[sensor_no]->setText(QString("s%1").arg(QString::number(serial_no)));

    } else if (sscanf(input, "s%d bilgileri bos", &sensor_no) == 1) {
        if (received_serial_no_count++ == active_sensor_count) {
            request_data_status[serial_no_request] = 1;
            received_serial_no_count = 1;
        }//?spn s3104 0 s3105 0 s3106 s3107 s3108 s3109 s3110 s3111 s3112 s3113 s3114 s3115 s3116
        sensors_eeprom_is_data_exist.insert(sensor_no, 0);
        header_labels[sensor_no]->setText("Veri Yok");

    } else if (sscanf(input, "Sensor-%d seri numarasi EEPROM'a yazilamadi: %d", &sensor_no, &serial_no) == 2) {
        mainWindow->setLineEditText(QString("Sensor-%1 seri numarasi %2 EEPROM'a yazilamadi.").arg(QString::number(sensor_no)).arg(QString::number(serial_no)));

    } else if (sscanf(input, "Sensor-%d seri numarasi EEPROM'a yazildi: %d", &sensor_no, &serial_no) == 2) {
        mainWindow->setLineEditText(QString("Sensor-%1 seri numarasi %2 EEPROM'a yazildi.").arg(QString::number(sensor_no)).arg(QString::number(serial_no)));
        QString serial_no_str = QString("s%1").arg(QString::number(serial_no));

        if (!sensor_folder_create_status.contains(serial_no_str)) {
            if (file_folder_creator.createSensorFolder(serial_no_str) == 1) {
                mainWindow->setLineEditText(QString("Sensor-%1 klasörü olusturuldu: %2").arg(QString::number(sensor_no)).arg(QString::number(serial_no)));
                sensor_folder_create_status.insert(serial_no_str, 1);
            }
        }
    } else if (sscanf(input, "Sensor-%d seri numarasi degistirilemedi: %d", &sensor_no, &serial_no) == 2) {
        mainWindow->setLineEditText(QString("Sensor-%1 seri numarasi %2 degistirilemedi.").arg(QString::number(sensor_no)).arg(QString::number(serial_no)));
        serial_no_changed = false;

    } else if (sscanf(input, "Sensor-%d seri numarasi degistirildi: %d", &sensor_no, &serial_no) == 2) {
        mainWindow->setLineEditText(QString("Sensor-%1 seri numarasi %2 degistirildi.").arg(QString::number(sensor_no)).arg(QString::number(serial_no)));
        QString new_folder_name = QString("s%1").arg(QString::number(serial_no));
        if (!sensors_serial_no.contains(sensor_no)) {
            if (file_folder_creator.createSensorFolder(new_folder_name) == 1) {
                mainWindow->setLineEditText(QString("Sensor-%1 klasörü olusturuldu: %2").arg(QString::number(sensor_no)).arg(QString::number(serial_no)));
                sensor_folder_create_status.insert(new_folder_name, 1);
            }
        } else {
            QString older_folder_name = QString("s%1").arg(sensors_serial_no[sensor_no]);
            //QString older_folder_name = calibration_board->findSensorFolderNameByValue(sensor_no);
            sensor_module_map.remove(older_folder_name);
            sensor_ids.removeOne(older_folder_name);
            sensor_folder_create_status.remove(older_folder_name);
            sensors_serial_no.remove(sensors_serial_no[sensor_no]);
            sensor_folder_create_status.insert(new_folder_name, 1);
            if (file_folder_creator.changeFolderName(older_folder_name, new_folder_name)) mainWindow->setLineEditText(QString("Sensor-%1 klasör ismi degistirildi: %2").arg(QString::number(sensor_no)).arg(QString::number(serial_no)));
            else mainWindow->setLineEditText(QString("Sensor-%1 klasör ismi degistirilemedi: %2").arg(QString::number(sensor_no)).arg(QString::number(serial_no)));
        }
        serial_no_changed = true;
    }
}

void LogParser::parseCalibrationTime(const char* input) {
    int year, month, day, hour, minute, second, cal_ppb;
    QDate date;
    QTime time;

    if (sscanf(input, "KalBasSaati %d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6)
    {
        date.setDate(year, month, day);
        time.setHMS(hour, minute, second);
        calibration_start_dt = QDateTime(date, time);
        main_window_header_labels.calibration_start_date->setText(calibration_start_dt.toString("hh:mm"));
    } else if(sscanf(input, "KalBitisSaati %d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6)
    {
        date.setDate(year, month, day);
        time.setHMS(hour, minute, second);
        calibration_end_dt = QDateTime(date, time);
    } else if(sscanf(input, "Kal%d-BasSaati %d-%d-%d %d:%d:%d", &cal_ppb, &year, &month, &day, &hour, &minute, &second) == 7)
    {
        date.setDate(year, month, day);
        time.setHMS(hour, minute, second);
        calibration_ppb_start_dt = QDateTime(date, time);
        cal_ppb_cal_time = cal_ppb;
        cal_val_labels.cal_point_start_time->setText(calibration_ppb_start_dt.toString("hh:mm"));
    } else if(sscanf(input, "Kal%d-BitisSaati %d-%d-%d %d:%d:%d", &cal_ppb, &year, &month, &day, &hour, &minute, &second) == 7)
    {
        date.setDate(year, month, day);
        time.setHMS(hour, minute, second);
        calibration_ppb_end_dt = QDateTime(date, time);
        cal_ppb_cal_time = cal_ppb;
        cal_val_labels.cal_point_end_time->setText(calibration_ppb_end_dt.toString("hh:mm"));
    } else
    {
        qDebug() << "Tarih ve saat ayrıştırılamadı!";
    }
}

void LogParser::parseLineData(KeyValue* data, const char* line, uint8_t &data_count) {
    while (*line) {
        char key[32];
        int n = 0;
        while (*line && !isspace(*line) && n < (int)(sizeof(key) - 1)) {
            key[n++] = *line++;
        }
        key[n] = '\0';

        while (isspace(*line)) line++;

        if (strchr(key, ':')) {
            char* colon = strchr(key, ':');
            *colon = '\0';
            strncpy(data[data_count].key, key, sizeof(data[data_count].key) - 1);

            char* endptr;
            float val = strtof(line, &endptr);
            data[data_count].value = (line == endptr) ? 0.0f : val;
            data_count++;

            line = endptr;
            while (isspace(*line)) line++;
        }
        else {
            char* endptr;
            float val = strtof(key, &endptr);
            if (key != endptr) {
                snprintf(data[data_count].key, sizeof(data[data_count].key), "s%d", data_count);
                data[data_count].value = val;
                data_count++;
            }
        }
    }
}

int8_t LogParser::parseLine(const char* input, Packet* packet) {
    if (input[0] != '>') {
        return -1;
    }

    line = QString::fromUtf8(input);

    sscanf(input, ">%10s %8s", packet->date, packet->time);
    const char* p = input + 21;

    char command[16];
    int i = 0;
    while (*p != '\0' && *p != ' ' && i < (int)(sizeof(command) - 1)) {
        command[i++] = *p++;
    }
    command[i] = '\0';
    packet->command = parseCommandExtended(command);
    packet->command_str = strdup(command);
    //mainWindow->setLineEditText(line);
    if (packet->command.type != CMD_D && packet->command.type != CMD_SN && packet->command.type != CMD_SK && packet->command.type != CMD_OM && packet->command.type != CMD_SMS) mainWindow->setLineEditText(line);

    if (*p == ' ') p++;

    packet->data = NULL;
    packet->data_count = 0;

    if (*p == '\0') {
        return 0;
    }

    if (packet->command.type == CMD_RST) {
        return 0;
    }

    if (packet->command.type == CMD_D || packet->command.type == CMD_KL || packet->command.type == CMD_KN_PWM
        || packet->command.type == CMD_L || packet->command.type == CMD_KS || packet->command.type == CMD_SMS || packet->command.type == CMD_SN)
    {
        packet->data_str = strdup(p);
        return 0;
    }

    packet->data = (KeyValue*)malloc(20 * sizeof(KeyValue));
    if (!packet->data) {
        return -3;
    } 
    packet->data_count = 0;

    parseLineData(packet->data, p, packet->data_count);
    return 0;
}

void LogParser::printCommandInfo(const Packet* packet) {
    printf("Komut: %s\n", packet->command_str);
    printf("Komut Tipi: %d\n", packet->command.type);
    printf("KN No: %d\n", packet->command.kn_no); // kalibrasyon noktası no
    printf("S No: %d\n", packet->command.s_no); // sensör no
    printf("OR No: %d\n", packet->command.or_no); // kalibrasyon bitiminde o3 değerlerinin numarası
    printf("******************************\n");
}

int8_t LogParser::processPacket(Packet* packet) {
    int sensor_no;
    char buff[256];
    char str[50];

    //if (packet->command.type == CMD_L) return 0;
    //if (packet->command.type == CMD_D) return 0;
    if (calibration_completed) return 0;

    switch (packet->command.type) {
    /*case CMD_R1:
    case CMD_R2:
    case CMD_R3:
        for (uint8_t i = 0; i < packet->data_count; i++) {
            if (sscanf(packet->data[i].key, "s%d", &sensor_no) == 1) {
                if (sensor_no == 0) continue;
                sprintf(buff, ">%s %s %s %f\n", packet->date, packet->time, packet->command_str, packet->data[i].value);
                *(file_folder_creator.sensor_streams[repeat_calibration_index][sensor_no - 1]) << buff;
            }
        }
        break;
    */
    case CMD_RST:
        command_line->messageBox("MCU yeniden başlatıldı, son log dosyalarının silinmesini istiyor musunuz?");
        break;
    case CMD_SN:
        sprintf(buff, ">%s %s %s\n", packet->date, packet->time, packet->data_str);
        parseSerialNoData(packet->data_str);
        break;
    case CMD_SK:
        sprintf(buff, ">%s %s ", packet->date, packet->time);
        for (uint8_t i = 0; i < packet->data_count; i++) {
            if (
                strcmp(packet->data[i].key, "OM") == 0 ||
                strcmp(packet->data[i].key, "T") == 0  ||
                strcmp(packet->data[i].key, "H") == 0
            )
            {
                if (strcmp(packet->data[i].key, "T") == 0) temp_labels[packet->command.s_no]->setText(QString("%1 °C").arg(QString::number(packet->data[i].value, 'f', 2)));
                else if (strcmp(packet->data[i].key, "H") == 0) hum_labels[packet->command.s_no]->setText(QString("%1 %").arg(QString::number(packet->data[i].value, 'f', 2)));

                sprintf(str, "%.2f ", packet->data[i].value);
            } else
            {
                if (strcmp(packet->data[i].key, "R1") == 0) r1_labels[packet->command.s_no]->setText(QString("%1").arg(QString::number(packet->data[i].value, 'f', 0)));
                else if (strcmp(packet->data[i].key, "R2") == 0) r2_labels[packet->command.s_no]->setText(QString("%1").arg(QString::number(packet->data[i].value, 'f', 0)));
                else if (strcmp(packet->data[i].key, "R3") == 0) r3_labels[packet->command.s_no]->setText(QString("%1").arg(QString::number(packet->data[i].value, 'f', 0)));

                sprintf(str, "%.0f ", packet->data[i].value);
            }
            strcat(buff, str);
        }
        request_data_status[current_request] = 1;
        //*(sensor_map[packet->command.s_no].log_stream) << buff << "\n";
        break;

    case CMD_KS:
    case CMD_KL:
        sprintf(buff, ">%s %s %s\n", packet->date, packet->time, packet->data_str);
        *(calibration_stream) << buff;
        if (packet->command.type == CMD_KS) {
            parseCalibrationTime(packet->data_str);
            //cal_ppb_cal_time ui->labelCalPpb->setText(QString::number(cal_ppb_cal_time));
            //QString formatted = calibration_dt.toString("dd.MM.yyyy hh:mm:ss");
            //calibration_dt ui->labelDateTime->setText(formatted);
        }
        break;

    case CMD_L:
        break;
    case CMD_D:
    case CMD_SMS:
        sprintf(buff, ">%s %s %s %s\n", packet->date, packet->time, packet->command_str, packet->data_str);
        parseCalibrationData(packet->data_str);
        //*(main_log_stream) << buff;
        break;
    case CMD_TH:
        //sprintf(buff, ">%s %s %s %.2f %s %.1f%%\n", packet->date, packet->time, packet->command_str, packet->th_data.temperature, packet->th_data.temp_unit, packet->th_data.humidity);
        //*(file_folder_creator.sensor_streams[repeat_calibration_index][packet->command.s_no - 1]) << buff;
        break;

    case CMD_OM:
    case CMD_KN_S:
        sprintf(buff, ">%s %s %s ", packet->date, packet->time, packet->command_str);
        for (uint8_t i = 0; i < packet->data_count; i++) {
            if (sscanf(packet->data[i].key, "s%d", &sensor_no) == 1) {
                if (packet->command.type == CMD_OM) {
                    sprintf(str, "%.3f ", packet->data[i].value);
                } else {
                    sprintf(str, "%.0f ", packet->data[i].value);
                }
                strcat(buff, str);
            }
        }
        if (packet->command.type == CMD_KN_S) {
            //*(sensor_map[packet->command.s_no].kal_stream) << buff << "\n";
        }
        else if (packet->command.type == CMD_OM) {
            //*(om106_map[DEVICE_1].om106_stream) << buff << "\n"; // log'un hangi om106l cihazından geldiğini bilmem lazım
        }
        break;

    case CMD_KN_O3:
    case CMD_KN_T:
    case CMD_KN_H:
    case CMD_KN_PWM:
    case CMD_KN_CT:
    case CMD_KN_CP:
    case CMD_KN_FR:
    case CMD_KN_PV:
        if (packet->command.type == CMD_KN_PWM) {
            parsePwmData(packet->data_str, &packet->pwm_data);
            sprintf(buff, ">%s %s %s %d/%dmsec\n", packet->date, packet->time, packet->command_str, packet->pwm_data.duty, packet->pwm_data.period);
        } else {
            if (sscanf(packet->data[0].key, "s%d", &sensor_no) == 1) {
                sprintf(buff, ">%s %s %s %.3f\n", packet->date, packet->time, packet->command_str, packet->data[0].value);
            }
        }
        *(calibration_stream) << buff;
        break;

    case CMD_KB_S_R:
    case CMD_KB_S_OR:
    case CMD_KB_S_KN:
    case CMD_KB_S_KN_B:
        sprintf(buff, ">%s %s %s ", packet->date, packet->time, packet->command_str);
        for (uint8_t i = 0; i < packet->data_count; i++) {
            if (
                strcmp(packet->data[i].key, "alpha") == 0 ||
                strcmp(packet->data[i].key, "beta") == 0  ||
                strcmp(packet->data[i].key, "logRR") == 0 ||
                strcmp(packet->data[i].key, "logO3") == 0
                ) {
                sprintf(str, "%s: %.5f ", packet->data[i].key, packet->data[i].value);
            } else {
                sprintf(str, "%s: %.0f ", packet->data[i].key, packet->data[i].value);
            }
            strcat(buff, str);
        }
        *(sensor_map[packet->command.s_no].kal_end_stream) << buff << "\n";
        break;
    default:
        break;
    }
    return 1;
}

void LogParser::freePacket(Packet* packet) {
    if (packet->data) {
        free(packet->data);
        packet->data = NULL;
    }
}
