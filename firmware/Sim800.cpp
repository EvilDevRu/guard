#include "Sim800.h"

Sim800::Sim800(uint8_t RX, uint8_t TX, uint8_t PWR) {
  _serial = new SoftwareSerial(RX, TX);
  _serial->begin(9600);
  this->_powerPin = PWR;
}

/**
 * //
 */
void Sim800::setup(char *apn, char *user, char *pwd) {
  this->_connectApn = apn;
  this->_connectUser = user;
  this->_connectPwd = pwd;
  
  this->off();
  delay(1000);
  this->on();
  
  Serial.println("Настройка модуля связи");
  Serial.println("Ожидание сети");
  
  //  Готовность модуля к работе
  this->checkAT("AT", 200);

  //  Проверяем, зарегистрировались или нет.
  while (this->getProviderName() == "") {
    delay(1000);
  }

  Serial.println("Провайдер " + this->getProviderName());
  
  Serial.println("Настройка GPRS");
  this->setupGPRS(apn, user, pwd);

  //  TODO: GPS
  //  TODO: Balance sms
  
  Serial.println("Модуль готов к работе");

  delay(1000);
}

/**
 * Вернет уровень сигнала.
 */
uint8_t Sim800::getLevelSignal() {
  if (!this->_isReady) {
    return 0;
  }

  //  Не можем посылать копанду модему, пока выполняется запрос к серверу.
  if (this->_reqBusyId > -1) {
    return -1;
  }
  
  String result = this->checkAT("AT+CSQ", 1000);
  return this->parseAT(result, "+CSQ: ", ",").toInt();
}

/**
 * Вернет название провайдера.
 */
String Sim800::getProviderName() {
  String result = this->checkAT("AT+COPS?", 200);
  result = this->parseAT(result, "+COPS: ", "\r");

  return result != "0"
    ? this->parseAT(result, "\"", "\"")
    : "";
}

/**
 * Отправит запрос на сервер и вернет результат.
 * Не блокирубщая функция.
 */
String Sim800::httpRequest(String url, int reqId) {
  String _url = "AT+HTTPPARA=\"URL\",\"" + url + "\"";

  if (!this->_isReady) {
    return "";
  }

  if (this->_reqBusyId > -1 && this->_reqBusyId != reqId) {
    return "";
  }

  switch (this->_reqSteps) {
    case 0:
      this->_reqBusyId = reqId;
      if (this->checkReqTimer(500)) {
        this->sendAT(_url);
        this->_reqSteps = 1;
      }
      break;

    case 1:
      if (this->checkReqTimer(500)) {
        this->readAT();
        this->sendAT("AT+HTTPACTION=0");
        this->_reqSteps = 2;
      }
      break;

    case 2:
      if (this->checkReqTimer(3500)) {
        this->readAT();
        //  TODO: Error check
        this->sendAT("AT+HTTPREAD");
        this->_reqSteps = 3;
      }
      break;

    case 3:
      if (this->checkReqTimer(500)) {
        this->httpOrderReset();
        
        String result = this->readAT();
        if (result.indexOf("OK\r") == -1) {
          #ifdef SIM800_DEBUG
            Serial.println("Ошибка ответа от сервера");
          #endif

          ++this->_numErrors;
          this->checkErrors();
          
          return "";
        }
        
        unsigned int bytes = this->parseAT(result, "+HTTPREAD: ", "\r").toInt();
        if (bytes == 0) {
          #ifdef SIM800_DEBUG
            Serial.println("Нулевой ответ от сервера");
          #endif

          ++this->_numErrors;
          this->checkErrors();
          
          return "";
        }

        result = result.substring(result.length() - 6 - bytes, result.length() - 6);
        result.trim();

        //  Если ответ есть, сбрасываем ошибки.
        this->_numErrors = 0;
        
        return result;
      }
      break;
  }

  return "";
}

/**
 * ##
 */
void Sim800::httpOrderReset() {
  this->_reqSteps = 0;
  this->_reqBusyId = -1;
}

/**
 * ##
 */
bool Sim800::checkReqTimer(unsigned long delayTime) {
  if (this->_reqTimer + delayTime < millis()) {
    this->_reqTimer = millis();
    return true;
  }

  return false;
}

/**
 * Настройка GPRS.
 */
void Sim800::setupGPRS(String apn, String user, String password) {
  String ATs[] = {
    "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"",  //  Установка настроек подключения
    "AT+SAPBR=3,1,\"APN\",\"" + apn + "\"",
    "AT+SAPBR=3,1,\"USER\",\"" + user + "\"",
    "AT+SAPBR=3,1,\"PWD\",\"" + password + "\"",
    
    "AT+SAPBR=1,1",                       //  Устанавливаем GPRS соединение
    "AT+HTTPINIT",                        //  Инициализация http сервиса
    "AT+HTTPPARA=\"CID\",1"               //  Установка CID параметра для http сессии
  };

  for (int i = 0; i < 4; i++) {
    this->checkAT(ATs[i], 500);
  }

  delay(1000);

  for (int i = 4; i < 7; i++) {
    this->checkAT(ATs[i], 500);
  }

  this->_isReady = true;
}

/**
 * Вернет true, если GPRS активен.
 */
bool Sim800::checkGPRS() {
  String result = this->checkAT("AT+SAPBR=2,1", 100);
  result = this->parseAT(result, "+SAPBR: 1,", ",");
  return result.toInt() == 1;
}

/**
 * Отправит AT комманду модулю и вернет ответ.
 * Если ответ с ошибкой, повторит отправку комманды через delayTime мс.
 * Блокирующая функция.
 * @param String command
 * @param long timeResponse
 * @return String
 */
String Sim800::checkAT(String cmd, long timeResponse) {
  String result = this->reqAT(cmd, timeResponse);
  if (result.indexOf("ERROR") > -1 || result.indexOf("OK\r") == -1) {
    delay(1000);
    return this->checkAT(cmd, timeResponse);
  }

  return result;
}

/**
 * //
 */
String Sim800::reqAT(String cmd, long timeResponse) {
  this->sendAT(cmd);
  delay(timeResponse);
  return this->readAT();
}

/**
 * Отправит команду в модем.
 * @param String cmd
 */
void Sim800::sendAT(String cmd) {
  if (!_serial) {
    return;
  }

  #ifdef SIM800_DEBUG
    Serial.println("Send AT: " + cmd);
  #endif

  _serial->println(cmd);
}

/**
 * Прочитает данные с можема.
 * @return String
 */
String Sim800::readAT() {
  int c;
  String v;

  while (_serial->available()) {
    c = _serial->read();
    v += char(c);
    delay(10);
  }

  #ifdef SIM800_DEBUG
    Serial.println("Read AT: " + v);
  #endif
  
  return v;
}

/**
 * Вернет данные из строки cmd начиная с cmdBegin и заканчивая cmdEnd.
 * @param String cmd
 * @param String cmdBegin
 * @param String cmdEnd
 * @return String
 */
String Sim800::parseAT(String cmd, String cmdBegin, String cmdEnd) {
  String result = cmd;
  result = result.substring(result.indexOf(cmdBegin) + cmdBegin.length());
  result = result.substring(0, result.indexOf(cmdEnd));
  return result;
}

/**
 * Перезагрузит модем через MOSFET транзистор.
 */
void Sim800::resetModem() {
  #ifdef SIM800_DEBUG
    Serial.println("Перезагрузка модема");
  #endif
  
  //  Тут проверка на то, включен модем или нет не нужна т.к. delay блокирует.
  this->off();
  delay(2000);
  this->on();
  this->setup(this->_connectApn, this->_connectUser, this->_connectPwd);
}

/**
 * Включит модем.
 */
void Sim800::on() {
  #ifdef SIM800_DEBUG
    Serial.println("Модем включен");
  #endif
  digitalWrite(this->_powerPin, HIGH);
}

/**
 * Выключит модем.
 */
void Sim800::off() {
  #ifdef SIM800_DEBUG
    Serial.println("Модем выключен");
  #endif
  
  digitalWrite(this->_powerPin, LOW);
  this->_isReady = false;
  this->_numErrors = 0;
}

/**
 * Если привысили число ошибок, перезагрузит модем.
 */
void Sim800::checkErrors() {
  if (this->_numErrors >= SIM800_MAX_ERRORS) {
    #ifdef SIM800_DEBUG
      Serial.println("Много ошибок");
    #endif
    this->resetModem();
  }
}
