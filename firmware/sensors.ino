unsigned long vBatteryTimeUpdate = 0;
unsigned long lvlSignalTimeUpdate = 0;

/**
 * Настройка датчиков.
 */
void setupSensors() {
  //  Считываем показания датчиков из памяти.
  isRSwitchAlert = EEPROM.read(EEPROM_RSWITCH) == "1";
  isShockAlert = EEPROM.read(EEPROM_SENSOR_SHOCK) == "1";
}

/**
 * Обработка датчиков.
 */
void updateSensors() {
  if (digitalRead(PIN_SENSOR_SHOCK) != 1) {
    Serial.println("Сработал датчик удара");
    isShockAlert = true;
    EEPROM.write(EEPROM_SENSOR_SHOCK, 1);
    checkSendSOS();
  }

  if (digitalRead(PIN_RSWITCH) != 1) {
    Serial.println("Сработал датчик открытия двери");
    isRSwitchAlert = true;
    EEPROM.write(EEPROM_RSWITCH, 1);
    checkSendSOS();
  }
  
  updateVoltage();
  updateLvlSignal();
}

/**
 * Отправит сигнал тревоги, не дожидаясь след периода запроса.
 */
void checkSendSOS() {
  if (!isSendSOS) {
    lastCheckSimStatus = 0;
    modem.httpOrderReset();
    isSendSOS = true;
  }
}

/**
 * Вернет true, если сигнализация сработала.
 */
bool getAlarm() {
  return isOnGuard && (isShockAlert || isRSwitchAlert);
}

/**
 * Обновление значения напряжения сети.
 */
void updateVoltage() {
  if (vBatteryTimeUpdate < millis()) {
    float _val = analogRead(PIN_VOLTAGE);
    vBattery = _val * 0.011375;

    vBatteryTimeUpdate = millis() + 500;
  }
}

/**
 * Обновление значения уровня сигнала сети.
 */
void updateLvlSignal() {
  if (lvlSignalTimeUpdate < millis()) {
    uint8_t _level = modem.getLevelSignal();
    if (_level > -1) {
      lvlSignal = _level;
    }

    lvlSignalTimeUpdate = millis() + 5000;
  }
}
