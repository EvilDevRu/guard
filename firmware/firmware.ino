#include <EEPROM.h>

#define PIN_SENSOR_SHOCK 2    //  Датчик удара
#define PIN_RSWITCH 3         //  Геркон
#define PIN_VOLTAGE 0         //  Вольтметр

//  Настройки модема.
#define PIN_SIM800_RX   7
#define PIN_SIM800_TX   8
#define PIN_SIM800_PWR  4
#define SIM800_APN      "internet"
#define SIM800_USER     "gdata"
#define SIM800_PWD      "gdata"
#define SIM800_DEBUG    1

#define EEPROM_SENSOR_SHOCK 0
#define EEPROM_RSWITCH      1

//  Адрес сервера.
#define API_SERVER_URL      ""

bool isRSwitchAlert = false;          //  True, если сработал геркон.
bool isShockAlert = false;            //  True, если сработал датчик удара.
bool isOnGuard = false;               //  True, если устройсто на охране.
bool isSendSOS = false;               //  Флаг указывает на то, что сообщение о сработке надо отправлять сразу же.

float vBattery = 0.0;                   //  Напряжение батареи.
uint8_t lvlSignal = 0;                  //  Уровень сигнала связи.
unsigned long lastCheckSimStatus = 0;   //  Последнее время проверки статуса связи.

void setup() {
  Serial.begin(9600);

  pinMode(PIN_SENSOR_SHOCK, INPUT);
  pinMode(PIN_RSWITCH, INPUT);
  pinMode(PIN_VOLTAGE, OUTPUT);
  pinMode(PIN_SIM800_PWR, OUTPUT);
  
  setupApi();
  setupSensors();
}

void loop() {
  updateSensors();
  updateApi();
}
