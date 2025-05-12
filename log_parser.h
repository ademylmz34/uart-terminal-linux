#ifndef LOG_PARSER_H
#define LOG_PARSER_H

#include <stdint.h>
#include "mainwindow.h"

#define MAX_KEY_LEN 32

class LogParser
{
    public:
        LogParser();
        ~LogParser();

        typedef enum {
            CMD_UNKNOWN = 0,
            CMD_R1,
            CMD_R2,
            CMD_R3,
            CMD_TH,
            CMD_OM,
            CMD_L,
            CMD_D,
            CMD_KN_S,
            CMD_KN_O3,
            CMD_KN_T,
            CMD_KN_H,
            CMD_KN_PWM,
            CMD_KN_CT,
            CMD_KN_CP,
            CMD_KN_FR,
            CMD_KN_PV,
            CMD_KB_S_OR,
            CMD_KB_S_R,
            CMD_KB_S_KN,
            CMD_KB_S_KN_B
        } CommandType;

        // KOMUT ve PARAMETRELER
        typedef struct {
            CommandType type;
            int8_t th_no;
            int8_t kn_no;
            int8_t s_no;
            int8_t or_no;
        } ParsedCommand;

        // KEY-VALUE YAPISI
        typedef struct {
            char key[MAX_KEY_LEN];
            float value;
        } KeyValue;

        // TH VERİSİ (Sıcaklık, Nem)
        typedef struct {
            float temperature;
            char temp_unit[8];
            float humidity;
        } THData;

        // PWM Verisi
        typedef struct {
            uint16_t duty;
            uint16_t period;
        } PWMData;

        // BİR SATIRLIK PAKET
        typedef struct {
            char date[11];
            char time[9];
            ParsedCommand command;
            char* command_str;
            KeyValue* data;
            char* data_str;
            uint8_t data_count;
            uint8_t is_th_data;
            uint8_t is_pwm_data;
            THData th_data;
            PWMData pwm_data;
        } Packet;

        int8_t parseLine(const char*, Packet*);
        int8_t processPacket(const Packet*);
        void freePacket(Packet*);

    private:
        int8_t repeat_calibration_index;
        int8_t calibration_completed;
        ParsedCommand parseCommandExtended(const char*);
        int8_t parseCalibrationData(const char*);
        int8_t parseThData(const char*, THData*);
        int8_t parsePwmData(const char*, PWMData*);
        void printCommandInfo(const Packet*);
};
#endif // LOG_PARSER_H
