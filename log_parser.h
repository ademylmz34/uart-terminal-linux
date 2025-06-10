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
    uint16_t repeat_calibration;
    uint16_t next_calibration_start_duration;
    uint16_t calibration_sensitivity;
    uint16_t zero_cal_conc;
    uint16_t stabilization_duration;
    uint16_t calibration_temp;
    uint16_t clean_air_duration;
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
        CMD_OM,
        CMD_L,
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
        CMD_KB_S_KN_B,

        CMD_D_SMS,
        CMD_D_RV,
        CMD_D_CI,
        CMD_D_CS,
        CMD_D_CP,
        CMD_D_SK,
        CMD_D_TH,
        CMD_D_SN
    } CommandType;

    // KOMUT ve PARAMETRELER
    typedef struct {
        CommandType type;
        int8_t th_no; // silinecek
        uint8_t kn_no;
        uint8_t s_no;
        uint8_t or_no;
    } ParsedCommand;

    // BİR SATIRLIK PAKET
    typedef struct {
        char date[11];
        char time[9];
        ParsedCommand command;
        char* command_str;
        char* data_str;
    } Packet;

    uint8_t received_serial_no_count;
    QMap<CommandType, std::function<void()>> cmd_funcs;

    Packet packet;
    void setMainWindow(MainWindow*);
    bool parseLine(const char*, Packet);

private:
    MainWindow* mainWindow;

    int8_t repeat_calibration_index;
    int8_t calibration_completed;

    ParsedCommand parseCommandExtended(const char*);
    CalibrationStates getCalibrationState(int);

    void initCommandFunctions();

    void parse_om_data();
    void parse_kn_s_data();
    void parse_kb_s_r_data();
    void parse_kb_s_or_data();
    void parse_kb_s_kn_data();
    void parse_kn_float_data();
    void parse_kn_decimal_data();
    void parse_pwm_data();
    void parse_sk_data();
    void parse_ks_data();

    void parse_sms_data();
    void parse_rv_data();
    void parse_ci_data();
    void parse_cs_data();
    void parse_cp_data();
    void parse_th_data();
    void parse_sn_data();

    void log_data();
};
#endif // LOG_PARSER_H
