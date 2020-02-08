#include <SoftwareSerial.h>
#include <Arduino.h>

#define SIM800_MAX_ERRORS   5       //  Максимальное количество ошибок, после чего модем будет перезагружен.

class Sim800
{
  public:
    Sim800(uint8_t RX, uint8_t TX, uint8_t PWR);
    void setup(char *apn, char *user, char *pwd);
    uint8_t getLevelSignal();
    String getProviderName();
    void checkSMS();
    String httpRequest(String url, int reqId);
    void httpOrderReset();
    void on();
    void off();

  private:
    SoftwareSerial *_serial;
    int8_t _reqBusyId = -1;
    uint8_t _reqSteps = 0;
    unsigned long _reqTimer = 0;
    uint8_t _numErrors = 0;
    uint8_t _powerPin;
    bool _isReady = false;
    char *_connectApn;
    char *_connectUser;
    char *_connectPwd;
    bool checkReqTimer(unsigned long delayTime);
    void checkErrors();
    void setupGPRS(String apn, String user, String password);
    bool checkGPRS();
    void sendAT(String cmd);
    String readAT();
    String reqAT(String cmd, long timeResponse);
    String checkAT(String cmd, long timeResponse);
    String parseAT(String cmd, String cmdBegin, String cmdEnd);
    void resetModem();
};
