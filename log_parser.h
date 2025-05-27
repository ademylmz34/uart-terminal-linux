#ifndef LOG_PARSER_H
#define LOG_PARSER_H

#include "enum_types.h"

#include <stdint.h>
#include "mainwindow.h"

#define MAX_KEY_LEN 32

class MainWindow;

struct CalibrationStatus {
    float o3_average;
    uint16_t calibration_ppb;
    uint16_t calibration_state;
    uint16_t calibration_duration;
    uint16_t stabilization_timer;
    uint16_t repeat_calibration;
    uint16_t pwm_duty_cycle;
    uint16_t pwm_period;
};
extern CalibrationStatus cal_status_t;

extern QDateTime calibration_start_dt;
extern QDateTime calibration_end_dt;
extern QDateTime calibration_ppb_start_dt;
extern QDateTime calibration_ppb_end_dt;

extern int cal_ppb_cal_time;
extern int cabin_no;
extern int kal_point;
extern int kal_point_val;

extern int calibration_repeat_count;

class LogParser : public QObject
{
    Q_OBJECT
public:
    explicit LogParser(QObject *parent = nullptr);
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
        CMD_SMS,
        CMD_SK,
        CMD_KL,
        CMD_KS,
        CMD_RST,
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
        int8_t th_no; // silinecek
        int8_t kn_no;
        int8_t s_no;
        int8_t or_no;
    } ParsedCommand;

    // KEY-VALUE YAPISI
    typedef struct {
        char key[MAX_KEY_LEN];
        float value;
    } KeyValue;

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
        PWMData pwm_data;
    } Packet;

    Packet packet;
    void setMainWindow(MainWindow*);
    int8_t parseLine(const char*, Packet*);
    int8_t processPacket(Packet*);
    void freePacket(Packet*);

private:
    MainWindow* mainWindow;

    int8_t repeat_calibration_index;
    int8_t calibration_completed;
    ParsedCommand parseCommandExtended(const char*);
    CalibrationStates getCalibrationState(int);
    void parseLineData(KeyValue*, const char*, uint8_t &) ;
    void parseCalibrationData(const char*);
    void parseCalibrationTime(const char*);
    int8_t parsePwmData(const char*, PWMData*);
    void printCommandInfo(const Packet*);
};
#endif // LOG_PARSER_H
