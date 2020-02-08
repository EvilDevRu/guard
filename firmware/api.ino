#include "Sim800.h"

#define API_PATH_STATUS             "/v1/security/get-status"

#define API_STATUS_PERIOD           0

#define API_REQ_ID_STATUS           1

#define STATUS_UNLOCKED             1
#define STATUS_LOCKED               2

//  Переменные для последовательного опроса модема без блокировок.
unsigned long reqTimer = 0;
int reqSteps = 0;
int reqBusyId = 0;

Sim800 modem(PIN_SIM800_RX, PIN_SIM800_TX, PIN_SIM800_PWR);

/**
 * Установка апи.
 */
void setupApi() {
  modem.setup(SIM800_APN, SIM800_USER, SIM800_PWD);
}

/**
 * Обработка методов апи.
 */
void updateApi() {
  if (lastCheckSimStatus < millis()) {
    //  Получаем статус сигнализации, если STATUS_LOCKED, ставим на охрану и обратно.
    //  По мимо этого, отправляем сразу данные всех датчиков.
    String _addr = String(API_SERVER_URL) + API_PATH_STATUS + "?"
      + "rswitch=" + String(isRSwitchAlert)
      + "&shock=" + String(isShockAlert)
      + "&voltage=" + String(vBattery)
      + "&signal=" + String(lvlSignal);

    int state = modem.httpRequest(_addr, API_REQ_ID_STATUS).toInt();

    switch (state) {
      case STATUS_UNLOCKED:    
        if (isOnGuard) {
          Serial.println("Сигнализация отключена");

          //  Если не на охране, сбрасываем алерты на датчики
          isShockAlert = false;
          isRSwitchAlert = false;
          isSendSOS = false;
          
          EEPROM.write(EEPROM_SENSOR_SHOCK, 0);
          EEPROM.write(EEPROM_RSWITCH, 0);
        }
        isOnGuard = false;
        break;
        
      case STATUS_LOCKED:        
        if (!isOnGuard) {
          Serial.println("Сигнализация включена");
          isSendSOS = false;
        }
        isOnGuard = true;
        break;
        
      default:
        break;
    }
    
    lastCheckSimStatus = millis() + API_STATUS_PERIOD;    //  Т.к. функции не блокирующие, ждем API_STATUS_PERIOD секунд.
  }
}
