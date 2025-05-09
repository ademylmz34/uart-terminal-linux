#include "mainwindow.h"
#include "log_parser.h"
#include <stdint.h>
#include <string.h>

LogParser::LogParser()
{
    //printf("sinif cagirildi\n");
}

LogParser::~LogParser()
{
    //printf("sinif yok edildi\n");
}

LogParser::ParsedCommand LogParser::parse_command_extended(const char* cmd) {
    ParsedCommand result = { .type = CMD_UNKNOWN, .th_no = -1, .kn_no = -1, .s_no = -1, .or_no = -1 };

    if (strcmp(cmd, "R1") == 0) result.type = CMD_R1;
    else if (strcmp(cmd, "R2") == 0) result.type = CMD_R2;
    else if (strcmp(cmd, "R3") == 0) result.type = CMD_R3;
    else if (strncmp(cmd, "TH", 2) == 0) {
        int no = atoi(cmd + 2);
        if (no >= 1 && no <= 15) {
            result.type = CMD_TH;
            result.s_no = no;
        }
    }
    else if (strcmp(cmd, "OM") == 0) result.type = CMD_OM;
    else if (strcmp(cmd, "L") == 0) result.type = CMD_L;
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
    return result;
}

int8_t LogParser::parse_th_data(const char* input, THData* th_data) {
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

int8_t LogParser::parse_pwm_data(const char* input, PWMData* pwm_data) {
    int pwm_duty, pwm_period;
    if (sscanf(input, "%d/%dmsec", &pwm_duty, &pwm_period) == 2) {
        pwm_data->duty = pwm_duty;
        pwm_data->period = pwm_period;
    } else {
        return -1;
    }

    return 0;
}

int8_t LogParser::parseCalibrationData(const char* input)
{
    char buff[256];
    if (input[0] != '>') {
        return 0;
    }
    const char* p = input + 21;
    qDebug() << p;
    switch(currentRequest) {
    case CAL_REPEAT_COUNT:
        if (sscanf(p, "L Yeni kalibrasyon sayisi %d", &calibrationRepeatCount) == 1) {
            qDebug() << "Calibration Repeat Count: " << QString::number(calibrationRepeatCount);
            //ui->plainTextEdit->appendPlainText("Calibration Repeat Count: " + QString::number(calibrationRepeatCount));
            return 1;
        }
        break;
    case ACTIVE_SENSOR_COUNT:
        if (sscanf(p, "L Aktif sensor sayisi %d", &activeSensorCount) == 1) {
            qDebug() << "Active Sensor Count: " << QString::number(activeSensorCount);
            //ui->plainTextEdit->appendPlainText("Active Sensor Count: " + QString::number(activeSensorCount));
            return 1;
        }
        break;
    case CAL_POINTS:
        if (sscanf(p, "L KN%d %d", &kalPoint, &kalPointVal) == 2) {
            //if (kalPointVal == 0) return;
            calibrationPoints[kalPoint] = kalPointVal;
            sprintf(buff, "L KN%d %d", kalPoint, kalPointVal);
            qDebug() << buff;
            return 1;
        }
        break;

    default:
        break;
    }
    return 0;
}

int8_t LogParser::parse_line(const char* input, Packet* packet) {
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

    packet->command = parse_command_extended(command);
    packet->command_str = strdup(command);

    if (*p == ' ') p++;

    packet->is_th_data = 0;
    packet->is_pwm_data = 0;
    packet->data = NULL;
    packet->data_count = 0;

    if (*p == '\0') {
        return 0;
    }

    if (packet->command.type == CMD_TH) {
        if (parse_th_data(p, &packet->th_data) == 0) {
            packet->is_th_data = 1;
            return 0;
        }
    }

    if (packet->command.type == CMD_L) {
        packet->data_str = strdup(p);
        if (!parseCalibrationData(p))
            //qDebug() << "Veri Alma işlemi başarısız";
        return 0;
    }

    if (packet->command.type == CMD_KN_PWM) {
        if (parse_pwm_data(p, &packet->pwm_data) == 0) {
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

void LogParser::print_command_info(const Packet* packet) {
    printf("Komut: %s\n", packet->command_str);
    printf("Komut Tipi: %d\n", packet->command.type);
    printf("KN No: %d\n", packet->command.kn_no);
    printf("S No: %d\n", packet->command.s_no);
    printf("OR No: %d\n", packet->command.or_no);
    printf("******************************\n");
}

int8_t LogParser::process_packet(const Packet* packet) {
    int sensor_no;
    char buff[256];
    char str[50];
    if (packet->command.type == CMD_L) return 0;

    switch (packet->command.type) {
    case CMD_R1:
    case CMD_R2:
    case CMD_R3:
        for (uint8_t i = 0; i < packet->data_count; i++) {
            if (sscanf(packet->data[i].key, "s%d", &sensor_no) == 1) {
                if (sensor_no == 0) continue;
                sprintf(buff, ">%s %s %s %f\n", packet->date, packet->time, packet->command_str, packet->data[i].value);
                *(file_folder_creator.sensorStreams[sensor_no - 1]) << buff;
                //fprintf(sensor_files[sensor_no - 1], ">%s %s %s %f\n", packet->date, packet->time, packet->command_str, packet->data[i].value);
            }
        }
        break;

    case CMD_TH:
        sprintf(buff, ">%s %s %s %.2f %s %.1f%%\n", packet->date, packet->time, packet->command_str, packet->th_data.temperature, packet->th_data.temp_unit, packet->th_data.humidity);
        *(file_folder_creator.sensorStreams[packet->command.s_no - 1]) << buff;
        //fprintf(sensor_files[packet->command.s_no - 1], ">%s %s %s %.2f %s %.1f%%\n", packet->date, packet->time, packet->command_str, packet->th_data.temperature, packet->th_data.temp_unit, packet->th_data.humidity);
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
            *(file_folder_creator.sensorStreams[packet->command.s_no - 1]) << buff << "\n";
            *(file_folder_creator.kalStreams[packet->command.kn_no]) << buff << "\n";
            //fprintf(sensor_files[packet->command.s_no - 1], "%s\n", buff);
            //fprintf(kal_files[packet->command.kn_no], "%s\n", buff);
        }
        else if (packet->command.type == CMD_OM) {
            file_folder_creator.om106LogStream << buff << "\n";
            //fprintf(om106log, "%s\n", buff);
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
            //fprintf(kal_files[packet->command.kn_no], ">%s %s %s %d/%dmsec\n", packet->date, packet->time, packet->command_str, packet->pwm_data.duty, packet->pwm_data.period);
        } else {
            if (sscanf(packet->data[0].key, "s%d", &sensor_no) == 1) {
                sprintf(buff, ">%s %s %s %.3f\n", packet->date, packet->time, packet->command_str, packet->data[0].value);
                //fprintf(kal_files[packet->command.kn_no], ">%s %s %s %.3f\n", packet->date, packet->time, packet->command_str, packet->data[0].value);
            }
        }
        *(file_folder_creator.kalStreams[packet->command.kn_no]) << buff;
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
        //printf("buff: %s\n", buff);
        *(file_folder_creator.sensorStreams[packet->command.s_no - 1]) << buff << "\n";
        //fprintf(sensor_files[packet->command.s_no - 1], "%s\n", buff);
        break;
    default:
        break;
    }
    return 1;
}

void LogParser::free_packet(Packet* packet) {
    if (packet->data) {
        free(packet->data);
        packet->data = NULL;
    }
}
