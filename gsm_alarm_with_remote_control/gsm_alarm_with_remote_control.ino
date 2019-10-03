                                                            // БИБЛИОТЕКИ
#include        "iarduino_GSM.h"                                                                                            // Подключаем библиотеку iarduino_GSM для работы с GSM/GPRS Shield.
#include        "SoftwareSerial.h"                                                                                          // Подключаем библиотеку SoftwareSerial для программной реализации шины UART.
#include        "OneWire.h"                                                                                                 // Подключаем библиотеку OneWire для работы с оборудованием фирмы Dallas
#include        "DallasTemperature.h"                                                                                       // Подключаем библиотеку DallasTemperature для работы с цифровым термометром DS18B20
                                            // ОБЪЕКТЫ ДЛЯ РАБОТЫ С ДАТЧИКАМИ ТЕМПЕРАТУРЫ
OneWire         oneWire_in(2);                                                                                                      // Указываем вывод, к которому подключен термометр внутри помещения
OneWire         oneWire_out(3);                                                                                                     // Указываем вывод, к которому подключен термометр снаружи помещения
DallasTemperature sensor_inhouse(&oneWire_in);                                                                              // Создаём объект sensor_inhouse для работы с термометром внутри помещения
DallasTemperature sensor_outhouse(&oneWire_out);                                                                            // Создаём объект sensor_outhouse для работы с термометром снаружи помещения
                                              // ОБЪЕКТЫ ДЛЯ РАБОТЫ С GSM/GPRS МОДУЛЕМ
iarduino_GSM    gsm;                                                                                                        // Создаём объект gsm для работы с функциями и методами библиотеки iarduino_GSM.
SoftwareSerial  softSerial(7, 8);                                                                                           // Создаём объект softSerial для работы по программной шине UATR, указывая выводы RX и TX платы Arduino (выводы 7 и 8)
                                                            // ПЕРЕМЕННЫЕ
String          TEXT;                                                                                                       // Переменная типа String для хранения текстовой информации и отправки её абоненту
String          strSMStxt;                                                                                                  // Переменная типа String для работы с поиском символа в строке
char            SMStxt[161];                                                                                                // Объявляем строку для хранения текста принятых SMS сообщений.
char            SMSnum[13];                                                                                                 // Объявляем строку для хранения номера отправителя SMS сообщений.
char            NUMBER[13]                            = "7_ВАШ_НОМЕР_ТЕЛЕФОНА";                                             // Зададим номер, с которым будет работать модуль
char*           REPORT_COMMAND                        = "ОТЧЁТ";                                                            // Объявляем массив для хранения указателя на строку
char            PINS[]                                = {A0, 4, 5};                                                         // Массив с номерами выводов устройств
uint8_t         MIN_TEMP_IN                           = 15;                                                                 // Минимальная температура срабатывания для термометра внутри помещения
uint8_t         MAX_TEMP_IN                           = 28;                                                                 // Максимальная температура срабатывания для термометра внутри помещения
uint8_t         MIN_TEMP_OUT                          = 10;                                                                 // Минимальная температура срабатывания для термометра снаружи помещения
uint8_t         MAX_TEMP_OUT                          = 35;                                                                 // Максимальная температура срабатывания для термометра снаружи помещения
uint16_t        MAX_GAS_VALUE                         = 450;                                                                // Максимальное значение концентрации газа
uint32_t        SMS_RESEND_TIME                       = 100000;                                                             // Время, по истечении которого производится повторная отправка СМС
uint32_t        WAITING_TIME_AFTER_SMS_FOR_TAKE_CALL  = 10000;                                                              // Время между отправкой СМС и совершением тревожного звонка
uint32_t        EMERGENCY_CALL_TALKING_TIME           = 30000;                                                              // Время разговора при тревожном звонке
uint32_t        TEMP_SENSOR_1_TIMER                   = 0;                                                                  // Таймер для датчика температуры внутри помещения на повторную отправку СМС
uint32_t        TEMP_SENSOR_2_TIMER                   = 0;                                                                  // Таймер для датчика температуры снаружи помещения на повторную отправку СМС
uint32_t        GAS_SENSOR_TIMER                      = 0;                                                                  // Таймер для датчика газа на повторную отправку СМС
uint32_t        MOTION_SENSOR_TIMER                   = 0;                                                                  // Таймер для датчика движения на повторную отправку СМС
uint32_t        GERKON_SENSOR_TIMER                   = 0;                                                                  // Таймер для датчика состояния на повторную отправку СМС
uint32_t        EMERGENCY_CALL_TIMER                  = 0;                                                                  // Таймер для совершения тревожного звонка
uint32_t        PHONE_TALKING_TIMER                   = 0;                                                                  // Таймер для времени дозванивания до абонента

void setup() {
  //Serial.begin(57600);
  sensor_inhouse.begin();                                                                                                   // Инициируем работу с термометром внутри помещения
  sensor_outhouse.begin();                                                                                                  // Инициируем работу с термометром снаружи помещения
  pinMode(PINS[0], INPUT);                                                                                                  // Настройка вывода на работу в режиме ВХОД
  pinMode(PINS[1], INPUT);                                                                                                  // Настройка вывода на работу в режиме ВХОД
  //Serial.print( F("Initialization, please wait ... ") );
  gsm.begin(softSerial);                                                                                                    // Инициируем работу GSM/GPRS Shield, указывая объект шины UART.
  while (gsm.status() != GSM_OK) {                                                                                          // Ждём завершения регистрации модема в сети оператора связи.
    //Serial.print(".");
    delay(1000);
  }
  //Serial.println(" OK!");
  // Установка кодировки для символов Кириллицы:
  gsm.TXTsendCodingDetect("п");                                                                                             // Выполняем автоопределение кодировки скетча для отправки текста на Русском языке.
  // Отправка сообщения об удачном запуске:
  gsm.SMSsend( F("Инициализация прошла успешно."), NUMBER);                                                                 // Данная строка будет отвечать отправителю указанным SMS сообщением.
  delay(3000);                                                                                                              // задержка
  //Serial.println(F("================AFTER SETUP================="));
}

void loop () {

  TEXT = "";                                                                                                                // Очищаем строку
  //========================================================================================================================//
                                                    // ДАТЧИК ГАЗА

  if (analogRead(PINS[0]) >= MAX_GAS_VALUE) {                                                                               // Проверяем, не превышена ли концентрация газа и
    //Serial.println(F("gas inside VALUE"));
    if ((GAS_SENSOR_TIMER + SMS_RESEND_TIME) < millis() || GAS_SENSOR_TIMER == 0) {                                         // если превышена, то проверяем, чтобы сообщения отправлялись не чаще, чем 1 раз в SMS_RESEND_TIME
      //Serial.println(F("gas inside TIMER"));
      GAS_SENSOR_TIMER = millis();                                                                                          // Обновляем счётчик
      TEXT = "Превышена концентрация газа.\r\nЗначение = " + analogRead(PINS[0]);                                           // Формируем текст сообщения
      gsm.SMSsend(TEXT, NUMBER);                                                                                            // Отправляем СМС с текстом
      delay(1000);                                                                                                          // задержка
    }
  }
  TEXT = TEXT + "Газ: " + analogRead(PINS[0]) + "ppm";                                                                      // добавляем в строку текст, значение уровня концентрации газа и
  delay(5);                                                                                                                 // ждём 5мс
  //Serial.println(F("===============AFTER GAS SENSOR=============="));
  //========================================================================================================================//
                                            // ДАТЧИК ТЕМПЕРАТУРЫ ВНУТРЕННИЙ

  sensor_inhouse.requestTemperatures();                                                                                     // Запрос значения термометра внутри помещения
  if (sensor_inhouse.getTempCByIndex(0) <= MIN_TEMP_IN || sensor_inhouse.getTempCByIndex(0) >= MAX_TEMP_IN) {               // Проверяем, не выходит ли полученное значение за установленные границы и
    //Serial.println(F("temp 1 inside VALUE"));
    if ((TEMP_SENSOR_1_TIMER + SMS_RESEND_TIME) < millis() || TEMP_SENSOR_1_TIMER == 0) {                                   // если выходит, то проверяем, чтобы сообщения отправлялись не чаще, чем 1 раз в SMS_RESEND_TIME
      //Serial.println(F("temp 1 inside TIMER"));
      TEMP_SENSOR_1_TIMER = millis();                                                                                       // Обновляем счётчик
      TEXT = "Пороговое изменение температуры внутри.\r\nЗначение = " + String (sensor_inhouse.getTempCByIndex(0));         // Формируем текст сообщения
      gsm.SMSsend(TEXT, NUMBER);                                                                                            // Отправляем СМС с текстом
      delay(1000);                                                                                                          // задержка
    }
  }
  TEXT = TEXT + "\r\nТвнут: " + sensor_inhouse.getTempCByIndex(0);                                                          // добавляем в строку текст, значение термометра и
  delay(5);                                                                                                                 // ждём 5мс
  //Serial.println(F("=============AFTER TEMP 1 SENSOR=================="));
  //========================================================================================================================//
                                               // ДАТЧИК ТЕМПЕРАТУРЫ ВНЕШНИЙ
  sensor_outhouse.requestTemperatures();                                                                                    // Запрос значения термометра снаружи помещения
  if (sensor_outhouse.getTempCByIndex(0) <= MIN_TEMP_OUT || sensor_outhouse.getTempCByIndex(0) >= MAX_TEMP_OUT) {           // Проверяем, не выходит ли полученное значение за установленные границы и
    //Serial.println(F("temp 2 inside VALUE"));
    if ((TEMP_SENSOR_2_TIMER + SMS_RESEND_TIME) < millis() || TEMP_SENSOR_2_TIMER == 0) {                                   // если выходит, то проверяем, чтобы сообщения отправлялись не чаще, чем 1 раз в SMS_RESEND_TIME
      //Serial.println(F("temp 2 inside TIMER"));
      TEMP_SENSOR_2_TIMER = millis();                                                                                       // Обновляем счётчик
      TEXT = "Пороговое изменение температуры снаружи.\r\nЗначение = " + String (sensor_outhouse.getTempCByIndex(0));       // Формируем текст сообщения
      gsm.SMSsend(TEXT, NUMBER);                                                                                            // Отправляем СМС с текстом
      delay(1000);                                                                                                          // задержка
    }
  }
  TEXT = TEXT + "\r\nТвнеш: " + sensor_outhouse.getTempCByIndex(0);                                                         // добавляем в строку текст, значение термометра и
  delay(5);                                                                                                                 // ждём 5мс
  //Serial.println(F("=================AFTER TEMP 2 SENSOR===================="));
  //========================================================================================================================//
                                                    // ДАТЧИК ДВИЖЕНИЯ

  if (digitalRead(PINS[1])) {                                                                                               // Проверяем, не было ли сработки датчика движения и
    //Serial.println(F("motion inside DIGITAL READ"));
    if ((MOTION_SENSOR_TIMER + SMS_RESEND_TIME) < millis() || MOTION_SENSOR_TIMER == 0) {                                   // если было, то проверяем, чтобы сообщения отправлялись не чаще, чем 1 раз в SMS_RESEND_TIME
      //Serial.println(F("motion inside TIMER"));
      MOTION_SENSOR_TIMER = millis();                                                                                       // Обновляем счётчик
      gsm.SMSsend(F("Замечено движение!"), NUMBER);                                                                         // Отправляем СМС с текстом
      if ((EMERGENCY_CALL_TIMER + WAITING_TIME_AFTER_SMS_FOR_TAKE_CALL) < millis() || EMERGENCY_CALL_TIMER == 0 ) {         // Ждём, пока не пройдёт время WAITING_TIME_AFTER_SMS_FOR_TAKE_CALL, прежде чем совершим тревожный звонок
        //Serial.println(F("motion inside EMERGENCY CALL TIMER"));
        EMERGENCY_CALL_TIMER = millis();
        gsm.SOUNDdevice(GSM_HEADSET);                                                                                       // Укажем использование модулем гарнитуры
        // Закомментируйте строки 125-149 и раскомментируйте строку 150, если вам не требуется ограничение тревожного звонка по времени
        if (gsm.CALLdial(NUMBER)) {                                                                                         // Если исходящий вызов на номер NUMBER инициирован, то...
          //Serial.println(F("motion inside CALL DIAL"));
          //Serial.println(F("motion BEFORE OUT DIAL"));
          while (gsm.CALLstatus() == GSM_CALL_OUT_DIAL) {                                                                   // Цикл выполняется пока набирается номер ...
            // Можно добавить код который будет выполняться в процессе набора номера
            //Serial.println(F("dial-ing..."));
          }
          delay(500);                                                                                                       // Даём немного времени для установки состояния - "дозвон" или "соединён".
          //Serial.println(F("motion BEFORE OUT BEEP"));
          if (gsm.CALLstatus() == GSM_CALL_OUT_BEEP) {                                                                      // Если начались гудки дозвона, то ...
            //Serial.println(F("motion inside CALL OUT BEEP"));
            while (gsm.CALLstatus() == GSM_CALL_OUT_BEEP) {                                                                 // Цикл выполняется пока идут гудки дозвона ...
              // Можно добавить код который будет выполняться в процессе ожидания ответа
              //Serial.println(F("beep-ing..."));
            }
            delay(500);                                                                                                     // Даём немного времени для установки состояния - "соединён".
          }
          //Serial.println(F("motion BEFORE ACTIVE"));
          if (gsm.CALLstatus() == GSM_CALL_ACTIVE) {                                                                        // Если соединение установлено (абонент ответил), то ...
            //Serial.println(F("motion inside CALL ACTIVE"));
            PHONE_TALKING_TIMER = millis();
            while (gsm.CALLstatus() == GSM_CALL_ACTIVE && (PHONE_TALKING_TIMER + EMERGENCY_CALL_TALKING_TIME) > millis()) { // Цикл выполняется пока установлено активное голосовое соединение и не закончилось время тревожного звонка
              // Можно добавить код который будет выполняться в процессе разговора
              //Serial.println(F("talk-ing..."));
            }
            gsm.CALLend();                                                                                                  // После окончания указанного времени разрываем соединение
          }
        }
        //gsm.CALLdial(NUMBER);                                                                                             // Выполняем набор номера
      }
    }
  }
  TEXT = TEXT + "\r\nДвижение: -";                                                                                          // добавляем в строку текст, значение датчика движения и
  delay(5);                                                                                                                 // ждём 5мс
  //Serial.println(F("===============AFTER MOTION SENSOR=================="));
  //========================================================================================================================//
                                                // ДАТЧИК СОСТОЯНИЯ (ГЕРКОН)

  if (!digitalRead(PINS[2])) {                                                                                              // Проверяем, не было ли сработки датчика состояния и
    //Serial.println(F("gerkon inside DIGITAL READ"));
    if ((GERKON_SENSOR_TIMER + SMS_RESEND_TIME) < millis() || GERKON_SENSOR_TIMER == 0) {                                   // если было, то проверяем, чтобы сообщения отправлялись не чаще, чем 1 раз в SMS_RESEND_TIME
      //Serial.println(F("gerkon inside TIMER"));
      GERKON_SENSOR_TIMER = millis();                                                                                       // Обновляем счётчик
      gsm.SMSsend(F("Окно открыто!"), NUMBER);                                                                              // Отправляем СМС с текстом
      if ((EMERGENCY_CALL_TIMER + WAITING_TIME_AFTER_SMS_FOR_TAKE_CALL) < millis() || EMERGENCY_CALL_TIMER == 0 ) {         // Ждём, пока не пройдёт время WAITING_TIME_AFTER_SMS_FOR_TAKE_CALL, прежде чем совершим тревожный звонок
        //Serial.println(F("gerkon inside EMERGENCY CALL TIMER"));
        EMERGENCY_CALL_TIMER = millis();
        gsm.SOUNDdevice(GSM_HEADSET);                                                                                       // Укажем использование модулем гарнитуры
        // Закомментируйте строки 174-201 и раскомментируйте строку 202, если вам не требуется ограничение тревожного звонка по времени
        if (gsm.CALLdial(NUMBER)) {                                                                                         // Если исходящий вызов на номер NUMBER инициирован, то...
          //Serial.println(F("gerkon inside CALL DIAL"));
          //Serial.println(F("gerkon BEFORE OUT DIAL"));
          while (gsm.CALLstatus() == GSM_CALL_OUT_DIAL) {                                                                   // Цикл выполняется пока набирается номер ...
            // Можно добавить код который будет выполняться в процессе набора номера
            //Serial.println(F("dial-ing..."));
          }
          delay(500);                                                                                                       // Даём немного времени для установки состояния - "дозвон" или "соединён".
          //Serial.println(F("gerkon BEFORE OUT BEEP"));
          if (gsm.CALLstatus() == GSM_CALL_OUT_BEEP) {                                                                      // Если начались гудки дозвона, то ...
            //Serial.println(F("gerkon inside CALL OUT BEEP"));
            while (gsm.CALLstatus() == GSM_CALL_OUT_BEEP) {                                                                 // Цикл выполняется пока идут гудки дозвона ...
              // Можно добавить код который будет выполняться в процессе ожидания ответа
              //Serial.println(F("beep-ing..."));
            }
            delay(500);                                                                                                     // Даём немного времени для установки состояния - "соединён".
          }
          //Serial.println(F("gerkon BEFORE ACTIVE"));
          if (gsm.CALLstatus() == GSM_CALL_ACTIVE) {                                                                        // Если соединение установлено (абонент ответил), то ...
            //Serial.println(F("gerkon inside CALL ACTIVE"));
            PHONE_TALKING_TIMER = millis();
            while (gsm.CALLstatus() == GSM_CALL_ACTIVE && (PHONE_TALKING_TIMER + EMERGENCY_CALL_TALKING_TIME) > millis()) { // Цикл выполняется пока установлено активное голосовое соединение и не закончилось время тревожного звонка
              // Можно добавить код который будет выполняться в процессе разговора
              //Serial.println(F("talk-ing..."));
            }
            gsm.CALLend();                                                                                                  // После окончания указанного времени разрываем соединение
          }
        }
        //gsm.CALLdial(NUMBER);                                                                                             // Выполняем набор номера
      }
    }
  }
  TEXT = TEXT + "\r\nОкно: +";                                                                                              // добавляем в строку текст и
  delay(5);                                                                                                                 // ждём 5мс
  //Serial.println(F("=============AFTER GERKON================="));
  //========================================================================================================================//
                                                  // ЗАПРОС ОТЧЁТА

  if (gsm.SMSavailable()) {                                                                                                 // Функция SMSavailable() возвращает количество входящих непрочитанных SMS сообщений.
    //Serial.println(F("sms inside if AVAILABLE"));
    gsm.SMSread(SMStxt, SMSnum);                                                                                            // Читаем SMS сообщение в ранее объявленные переменные (текст SMS сообщения, номер SMS).
    if (!strcmp(NUMBER, SMSnum)) {                                                                                          // Проверим, что все указанный в условии номер и номер, с которого пришло СМС совпадают
      //Serial.println(F("sms inside if right NUMBER"));
      strSMStxt = SMStxt;                                                                                                   // Присваиваем массиву strSMStxt значение массива SMStxt для дальнейшей проверки на совпадения
      if (strSMStxt.indexOf(REPORT_COMMAND) != -1 ) {                                                                       // Если совпадение текста из массива с командой в условии найдено, то
        //Serial.println(F("sms inside if RIGHT COMMAND"));
        gsm.SMSsend(TEXT, NUMBER);                                                                                          // отправляем сообщение с отчётом
        delay(1000);                                                                                                        // задержка
      } else {                                                                                                              // Если текст пришёл не верный, то
        //Serial.println(F("sms inside if WRONG COMMAND"));
        gsm.SMSsend(F("Ошибка в тексте!"), NUMBER);                                                                         // отправляем об этом сообщение
        delay(1000);                                                                                                        // задержка
      }
    }
  }
  //Serial.println(F("===============AFTER REPORT COMMAND==================="));
  //========================================================================================================================//
                                             // ОТВЕТ НА ВХОДЯЩИЙ ЗВОНОК
                                                  
  if (gsm.CALLavailable(SMSnum)) {                                                                                          // Функция CALLavailable() возвращает true если есть входящий дозванивающийся вызов. В качестве аргумента функции можно указать строку в которую будет записан адрес (номер) вызывающего абонента.
    //Serial.println(F("call inside if available"));
    if (!strcmp(NUMBER, SMSnum)) {                                                                                          // Проверим, что все символы в двух строках совпадают (при совпадении функция strcmp возвращает 0)
      //Serial.println(F("call inside if right NUMBER"));
      gsm.SOUNDdevice(GSM_HEADSET);                                                                                         // Использовать гарнитуру для ввода/вывода звука.
      gsm.CALLup();                                                                                                         // Отвечаем на вызов. Если нужно сбросить вызов, то обращаемся к функции CALLend().
      while (gsm.CALLavailable()) {}                                                                                        // Цикл выполняется пока входящий дозванивающийся вызов не сменит статус на "активный голосовой вызов" или любой другой.
    }
  }
  //Serial.println(F("==========AFTER INCOMING CALL=================="));
}
