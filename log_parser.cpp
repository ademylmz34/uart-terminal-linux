#include "mainwindow.h"
#include "log_parser.h"
#include <stdint.h>
#include <string.h>

LogParser::LogParser()
{
    repeat_calibration_index = 0;
    calibration_completed = 0;
}

LogParser::~LogParser()
{
}

LogParser::ParsedCommand LogParser::parseCommandExtended(const char* cmd) {
    ParsedCommand result = { .type = CMD_UNKNOWN, .th_no = -1, .kn_no = -1, .s_no = -1, .or_no = -1 };

    if (strcmp(cmd, "R1") == 0) result.type = CMD_R1;
    else if (strcmp(cmd, "R2") == 0) result.type = CMD_R2;
    else if (strcmp(cmd, "R3") == 0) result.type = CMD_R3;
    else if (strcmp(cmd, "SMS") == 0) result.type = CMD_SMS;
    else if (strcmp(cmd, "RST") == 0) result.type = CMD_RST;
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

int8_t LogParser::parseThData(const char* input, THData* th_data) {
    float temp_value = 0.0f;
    char unit[8];
    char humidity_str[16];

    int parsed = sscanf(input, "%f %7s %15s", &temp_value, unit, humidity_str);
    if (parsed != 3) {
        return -1;
    }

    th_data->temperature = temp_value;
    strncpy(th_data->temp_unit, unit, sizeof(th_data->temp_unit) - 1);
    th_data->temp_unit[sizeof(th_data->temp_unit) - 1] = '\0';

    size_t len = strlen(humidity_str);
    if (humidity_str[len - 1] == '%') {
        humidity_str[len - 1] = '\0';
    }

    th_data->humidity = atof(humidity_str);

    return 0;
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
}

void LogParser::parseCalibrationData(const char* input)
{
    char buff[256];
    char bitStr[32];
    char valStr[8];

    float o3_average;
    int calibration_ppb;
    int calibration_state;
    int calibration_duration;
    int stabilization_timer;
    int repeat_calibration;
    int pwm_duty_cycle;
    int pwm_period;

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
                qDebug() << cabin_no;
                if (cabin_no == 1) main_window_header_labels.cabin_info->setText("Kabin-1 Kalibrasyon");
                else if (cabin_no == 2) main_window_header_labels.cabin_info->setText("Kabin-2 Kalibrasyon");
                request_data_status[current_request] = 1;
            }
            break;

        case R_CAL_STATUS:
            if (sscanf(input, "%d %d %d %d %d %f %d %d", &calibration_ppb, &calibration_state, &calibration_duration, &stabilization_timer,
                       &repeat_calibration, &o3_average, &pwm_duty_cycle, &pwm_period) == 8) {
                cal_status_t.calibration_ppb = calibration_ppb;
                cal_status_t.calibration_state = calibration_state;
                cal_status_t.calibration_duration = calibration_duration;
                cal_status_t.stabilization_timer = stabilization_timer;
                cal_status_t.repeat_calibration = repeat_calibration;
                cal_status_t.o3_average = o3_average;
                cal_status_t.pwm_duty_cycle = pwm_duty_cycle;
                cal_status_t.pwm_period = pwm_period;

                cal_val_labels.cal_point->setText(QString("%1 PPB").arg(QString::number(calibration_points[calibration_ppb])));
                cal_val_labels.cal_duration->setText(QString("%1 dakika").arg(QString::number(calibration_duration)));
                cal_val_labels.cal_stabilization->setText(QString::number(stabilization_timer));
                cal_val_labels.cal_status->setText(calibration_state_str[getCalibrationState(calibration_state)]);
                cal_val_labels.cal_o3_average->setText(QString("%1 PPB").arg(QString::number(o3_average)));
                cal_val_labels.cal_pwm_cyle_period->setText(QString("%1/%2 msec").arg(QString::number(pwm_duty_cycle)).arg(QString::number(pwm_period)));

                main_window_header_labels.calibration_repeat_val->setText(QString::number(repeat_calibration));

                request_data_status[current_request] = 1;
            }
            break;

        case R_SENSOR_VALUES:
            break;
        default:
            break;
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

int8_t LogParser::parseLine(const char* input, Packet* packet) {
    if (input[0] != '>') {
        return -1;
    }

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

    if (*p == ' ') p++;

    packet->is_th_data = 0;
    packet->is_pwm_data = 0;
    packet->data = NULL;
    packet->data_count = 0;

    if (*p == '\0') {
        return 0;
    }

    if (packet->command.type == CMD_RST) {
        return 0;
    }

    if (packet->command.type == CMD_TH) {
        if (parseThData(p, &packet->th_data) == 0) {
            packet->is_th_data = 1;
            return 0;
        }
    }

    if (packet->command.type == CMD_D || packet->command.type == CMD_KL
        || packet->command.type == CMD_L || packet->command.type == CMD_KS || packet->command.type == CMD_SMS)
    {
        //qDebug() << p;
        packet->data_str = strdup(p);
        if (packet->command.type == CMD_D || packet->command.type == CMD_SMS) parseCalibrationData(p);
        else if(packet->command.type == CMD_KS) parseCalibrationTime(p);
        return 0;
    }

    if (packet->command.type == CMD_KN_PWM) {
        if (parsePwmData(p, &packet->pwm_data) == 0) {
            packet->is_pwm_data = 1;
            return 0;
        } else {
            return -1;
        }
    }

    packet->data = (KeyValue*)malloc(20 * sizeof(KeyValue));
    if (!packet->data) {
        return -3;
    }

    packet->data_count = 0;
    while (*p) {
        char key[32];
        int n = 0;
        while (*p && !isspace(*p) && n < (int)(sizeof(key) - 1)) {
            key[n++] = *p++;
        }
        key[n] = '\0';

        while (isspace(*p)) p++;

        if (strchr(key, ':')) {
            char* colon = strchr(key, ':');
            *colon = '\0';
            strncpy(packet->data[packet->data_count].key, key, sizeof(packet->data[packet->data_count].key) - 1);

            char* endptr;
            float val = strtof(p, &endptr);
            packet->data[packet->data_count].value = (p == endptr) ? 0.0f : val;
            packet->data_count++;

            p = endptr;
            while (isspace(*p)) p++;
        }
        else {
            char* endptr;
            float val = strtof(key, &endptr);
            if (key != endptr) {
                snprintf(packet->data[packet->data_count].key, sizeof(packet->data[packet->data_count].key), "s%d", packet->data_count);
                packet->data[packet->data_count].value = val;
                packet->data_count++;
            }
        }
    }

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

int8_t LogParser::processPacket(const Packet* packet) {
    int sensor_no;
    char buff[256];
    char str[50];

    if (packet->command.type == CMD_L) return 0;
    if (packet->command.type == CMD_D) return 0;
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
        //*(sensor_map[packet->command.s_no].log_stream) << buff << "\n";
        break;

    case CMD_KS:
    case CMD_KL:
        sprintf(buff, ">%s %s %s\n", packet->date, packet->time, packet->data_str);
        *(calibration_stream) << buff;
        if (packet->command.type == CMD_KS) {
            //cal_ppb_cal_time ui->labelCalPpb->setText(QString::number(cal_ppb_cal_time));
            //QString formatted = calibration_dt.toString("dd.MM.yyyy hh:mm:ss");
            //calibration_dt ui->labelDateTime->setText(formatted);
        }
        break;

    case CMD_L:
    case CMD_D:
        sprintf(buff, ">%s %s %s %s\n", packet->date, packet->time, packet->command_str, packet->data_str);
        *(main_log_stream) << buff;
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
        if (packet->is_pwm_data) {
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
