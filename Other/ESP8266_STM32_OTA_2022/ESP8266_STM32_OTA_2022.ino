/*
Установка Arduino IDE
https://alexgyver.ru/arduino-first/
Работа с delay
https://alexgyver.ru/lessons/time/
ESP8266 в Arduino IDE
https://habr.com/ru/post/371853/
ESP8266 работа с экраном oled 128x64 i2c
https://test-version.ru/nodemcu-esp8266-dlya-otobrazheniya-na-oled-128x64-i2c-ispolzuya-biblioteku-adadruit-ssd1306/
https://ru.stackoverflow.com/questions/1075619/oled-дисплей-модуль-Перевернуть-изображение-на-180-градусов
STM32 в Arduino IDE
http://digitrode.ru/computing-devices/mcu_cpu/2237-kak-zaprogrammirovat-stm32-s-pomoschyu-arduino-ide.html
https://2150692.ru/faq/112-quick-start-stm32f103c8t6
STM32 OTA через ESP
https://istarik.ru/blog/stm32/122.html
USART протокол используемый в STM32 загрузчике
https://www.st.com/resource/en/application_note/cd00264342-usart-protocol-used-in-the-stm32-bootloader-stmicroelectronics.pdf
SIMM800l команды
http://arduino-kid.ru/blog/gsm-sim800-komandy-i-podklyuchenie-k-arduino
SIMM800l форум о проблеме перезагрузки
http://arduino.ru/forum/apparatnye-voprosy/gsm-modem-sim800l
SIMM800l отправка SMS
https://portal-pk.ru/news/319-gsm-modul-sim800l-at-komandy-i-otpravka-sms.html
SIMM800l GPRS
https://2150692.ru/faq/62-gprs-svyaz-cherez-sim800%7C-i-arduino
https://alex-exe.ru/radio/wireless/gsm-sim900-gprs/
https://beelinex.ru/nastrojki-gprs/
Мой сервер
http://cc33953.tmweb.ru
*/

#include <SoftwareSerial.h>     // Работа с программным UART
#include <SD.h>                 // Работа с SD картой
#include <SPI.h>                // Работа с интерфейсом SPI (Для подключения SD карты)
#include <ESP8266WiFi.h>        // Работа с WiFi
#include <ESP8266WiFiMulti.h>   // Работа с WiFi
#include <ESP8266HTTPClient.h>  // Работа с WiFi
#include <Wire.h>               // Работа с WiFi
#include <Adafruit_GFX.h>       // Работа с экраном дебагера
#include <Adafruit_SSD1306.h>   // Работа с экраном дебагера
#include <Ticker.h>             // Работа с таймером

#define BOOT_PIN    D3          // GPIO0
#define RESET_PIN   D4          // GPIO2
#define USART_STM32 Serial      //
#define WRITE_ADDR  0x08000000  // Адрес для заливки прошивки
#define SIZE_WRITE  256         //
#define DELAY       1000        // Время задержки OLED сообщений
#define WAIT        5           // Время ожидания ответа 

const int CHIPSELECT    = D8;                                             // GPIO15
const char* ssid        = "Redmi_Note_7";                                 // RT-GPON-3B59 // Redmi_Note_7 // Beeline_2G_452
const char* password    = "69911201";                                     // SUyDy87W     // 69911201     // 0854912445
String FILEURL          = "http://cc33953.tmweb.ru/other/bin/STM32.bin";  //
String SERVERURL        = "http://cc33953.tmweb.ru/php/frmwr.php";        //
String FIRMWARE_WiFi    = "STM32_WiFi.bin";                               //
String FIRMWARE_GPRS    = "STM32_GPRS.bin";                               //
String APN              = "internet.beeline.ru";                          // Настройка GPRS
String USER             = "beeline";                                      // Настройка GPRS
String PWD              = "beeline";                                      // Настройка GPRS
unsigned long TIMER1000 = 0;                                              //
unsigned long TIMER100  = 0;                                              //

Ticker timer1000;
Ticker timer100;
HTTPClient http;
File uploadFileWiFi;
File uploadFileGPRS;

// Декларация для дисплея SSD1306, подключенного к I2C (контакты SDA, SCL)
Adafruit_SSD1306 display(128, 64, &Wire, LED_BUILTIN);

// bitmap изображения (Выводим при инициализации дисплея в setup)
static const unsigned char PROGMEM logo_bmp[] =
{ B00000000, B11000000, B00000001, B11000000, B00000001, B11000000, B00000011, B11100000,
  B11110011, B11100000, B11111110, B11111000, B01111110, B11111111, B00110011, B10011111,
  B00011111, B11111100, B00001101, B01110000, B00011011, B10100000, B00111111, B11100000,
  B00111111, B11110000, B01111100, B11110000, B01110000, B01110000, B00000000, B00110000 };





void setup() {
  timer1000.attach(1, funcTimer1000); // Привязываем таймер к функции (1 раз в секунду)
  timer100.attach(0.1, funcTimer100); // Привязываем таймер к функции (10 раз в секунду)
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Работа с 128x64 OLED дисплеем по адресу 0x3C
  // Переворачиваем изображение
//  oledCommand(0xA0); // 0xA1 отражение по горизонтали, для отображения справа налево необходимо использовать команду 0xA0
//  oledCommand(0xC0); // 0xC8 отражение по вертикали, 0xC0 для переворота изображения по вертикали.
  display.display(); delay(DELAY); // Устанавливаем изображение
  
  drawImg("Старт"); delay(DELAY);
  
  USART_STM32.begin(115200, SERIAL_8E1);

  pinMode(CHIPSELECT,OUTPUT);
  pinMode(BOOT_PIN, OUTPUT);
  digitalWrite(BOOT_PIN, LOW);
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW);
  delay(250);

  on_reset(); // Приводим STM32 в режим исполнения прошивки

  // Соединение с сетью
  drawImg("Подключение к сети..."); delay(DELAY);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) { delay(100); } 
  drawImg("Подключение к сети\nуспешно"); delay(DELAY);
  
  // Соединение с SD картой
  drawImg("Подключение\nSD карты..."); delay(DELAY);
  if (!SD.begin(CHIPSELECT)) { drawImg("SD карта\nошибка nподключения"); delay(DELAY); } 
  drawImg("SD карта\nподключена"); delay(DELAY);

  // Функция загрузки прошивки по WiFi
  drawImg("Функция загрузки\nпрошивки"); delay(DELAY);
  if(!downloadFileWiFi(FILEURL, FIRMWARE_WiFi)){ return; }
  drawImg("Функция загрузки\nпрошивки завершена"); delay(DELAY);

  // Функция установки прошивки
  drawImg("Функция установки\nпрошивки"); delay(DELAY);
  program_stm32(FIRMWARE_WiFi);
  drawImg("Функция установки\nпрошивки завершена"); delay(DELAY);

  // Соединение с GSM модулем
  drawImg("Подключение\nGSM модуля..."); delay(DELAY);
  if (!SIM800lATOK()) { drawImg("GSM модуль\nне подключен"); delay(DELAY); }
  else { drawImg("GSM модуль\nподключен"); delay(DELAY); }

  // Процедура начальной инициализации GSM модуля
  drawImg("GPRS инициализация"); delay(DELAY);
  if(!gprs_init(APN, USER, PWD)) { drawImg("GPRS\nошибка инициализации"); delay(DELAY); }
  else { drawImg("GPRS инициализация\nуспешна"); delay(DELAY); } 

  drawImg("Функция загрузки\nпрошивки\nGPRS"); delay(DELAY);
  downloadFileGPRS(SERVERURL, FIRMWARE_GPRS);
  drawImg("Функция загрузки\nпрошивки завершена\nGPRS"); delay(DELAY);

  // Функция установки прошивки
  drawImg("Функция установки\nпрошивки\nGPRS"); delay(DELAY);
  program_stm32(FIRMWARE_GPRS);
  drawImg("Функция установки\nпрошивки завершена\nGPRS"); delay(DELAY);

//  drawImg("Отправка SMS..."); delay(DELAY);
//  sendSms("+79610699024", "Hello World!!!");
//  drawImg("Отправлено"); delay(DELAY);
}





void loop() {
//  delay(10000);

//  // Соединение с GSM модулем
//  drawImg("Подключение\nGSM модуля..."); delay(DELAY);
//  if (!SIM800lATOK()) { drawImg("GSM модуль\nне подключен"); delay(DELAY); }
//  else { drawImg("GSM модуль\nподключен"); delay(DELAY); }
}





// Проверка работы модуля SIM800l
int SIM800lATOK(){
  String answer = "";                   // Хранит весь ответ SIM800l
  int err = 1;                          // Проверка на ошибки где err = 1 - всё в норме, err = 0 - всё плохо
  for(int i = 0; i < 5; i++){           // Проведём 5 попыток соединиться с SIM800l каждые 2 секунды с ещё 2-мя дополнительными секундами на ожидание ответа
    delay(2000);
    err = 1;
    USART_STM32.println("AT");          // Посылаем в GSM модуль
    answer = waitAnswerUSART(2, "OK");  // Ждём ответ и проверяем на корректность (если ответ не пришёл или ответ не соответствует ожиданиям вернётся "ERROR")
    if(answer == "ERROR") err = 0;
    else i = 5;                         // Если ответ пришёл и всё замечательно прерываем цикл
  }
  return err;
}


// Загрузка файла по GPRS
void downloadFileGPRS(String serverURL, String firmware) 
{  
  TIMER100 = 0;
  String answer = "";       // Ответ функции выполняющей соединение по GPRS
  String sizeFileStr = "";  // Размер файла в виде строки
  String checksumStr = "";  // Полученная контрольная сумма в виде строки
  double checksum = 0;      // Полученная контрольная сумма
  double newChecksum = 0;   // Вычисленная контрольная сумма
  int err = 1;              // Проверка на ошибки где err = 1 - всё в норме, err = 0 - всё плохо
  int sizeFile = 0;         // Размер файла
  int indexSymbolA = 0;     // Индекс первого A символа (необходимо для извлечения размера файла)
  int indexSymbolF = 0;     // Индекс первого F символа (необходимо для извлечения кол-ва итераций)
  int count = 0;            // Количество итераций
  uint8_t num = 0;          // Получаем десятичное число
  String hexStr = "";       // Хранит шестнадцатеричное число в виде строки (пример "0x3f")
  
  // Определяем размер файла
  for(int i = 0; i < 1; i){
    newChecksum = 0;
    answer = gprs_send(serverURL, "0");                                // Размер файла приходит с сервера в виде "=245.34FFFFFFFFFFFFF12780A128FFF..." (220 символов). Параметр "0" говорит что мы хотим именно размер файла прошивки 
    if(answer != "ERROR"){                                             // Если успешно
      for(int k = 1; k < 20; k++) checksumStr += answer[k];            // Полученную контрольную сумму переписываем в отдельную строку
      
      // Удаляем лишние символы 'F' в контрольной сумме
      while(checksumStr.charAt(checksumStr.length()-1) == 'F' && checksumStr.length() > 0) { 
        checksumStr.remove(checksumStr.length()-1, 1);
      }
      checksum = checksumStr.toDouble(); // Полученная контрольная сумма
      
      answer.remove(0, 20);                                            // Удаляем из полезной нагрузки стартовый символ "=" и контрольную сумму, получаем 100 байт в hex формате (два символа на каждый байт)

      // Вычисление контрольной суммы из answer
      for(int j = 0; j < answer.length(); j+=2){
        hexStr = "0x" + String(answer[j]) + String(answer[j+1]); 
        num = strtol(hexStr.c_str(),NULL,0);
        newChecksum+=num;
      }
      newChecksum /= 100;

      // Если контрольные суммы совпадают
      if(checksum == newChecksum){
        indexSymbolA = substrInStr(answer, "A", 1);                      // Находим первое вхождение символа A (на данном этапе строка имеет вид "12780A128FFF...")
        for(int k = 0; k < indexSymbolA; k++) sizeFileStr += answer[k];  // Вытаскиваем размер файла прошивки в отдельную строку
        sizeFile = sizeFileStr.toInt();                                  // Размер файла в байтах
        count = ceil(sizeFile / 100); count++;                           // Кол-во итераций

        drawImg("Функция загрузки\nпрошивки\nGPRS\nразмер  " + String(sizeFile) + "\nскачано 0\nвремя   " + String(ceil(TIMER100 / 10)));
        i++;
      }
    }
  }

  
  SD.remove(firmware);                                 // Удалить файл:
  uploadFileGPRS = SD.open(firmware, FILE_WRITE);      // Откроем файл в режиме записи


  // Получаем полезную нагрузку
  for(int i = 1; i <= count; i){
    newChecksum = 0;
    answer = gprs_send(serverURL, String(i));                        // Ответ должен иметь вид "=35.26FFFFFFFFFFFFFF0028002071010008..."(220 символов)
    if(answer != "ERROR"){
      for(int k = 1; k < 20; k++) checksumStr += answer[k];          // Контрольную сумму переписываем в отдельную строку

      // Удаляем лишние символы 'F' в контрольной сумме
      while(checksumStr.charAt(checksumStr.length()-1) == 'F' && checksumStr.length() > 0) { 
        checksumStr.remove(checksumStr.length()-1, 1);
      }
      checksum = checksumStr.toDouble(); // Контрольная сумма
      
      answer.remove(0, 20);                                          // Удаляем из полезной нагрузки стартовый символ "=" и контрольную сумму, получаем 100 байт в hex формате (два символа на каждый байт)

      // Вычисление контрольной суммы из answer
      newChecksum = 0;
      for(int j = 0; j < answer.length(); j+=2){
        hexStr = "0x" + String(answer[j]) + String(answer[j+1]); 
        num = strtol(hexStr.c_str(),NULL,0);
        newChecksum+=num;
      }
      newChecksum /= 100;

      drawImg(String(checksum) + "\n" + String(newChecksum));
      delay(5000);
      USART_STM32.println(answer);
      delay(5000);
      
      // Если контрольные суммы совпадают
      if(checksum == newChecksum){
        if(i == count){                                                // В последней итерации не все байты полезные. Т.к. размер прошивки редко бывает кратным 100, сервер дописывает нули в конце, вырезаем все последние нули
          while(answer.charAt(answer.length()-1) == '0' && answer.length() > 0) {
            answer.remove(answer.length()-1, 1);
          }
        } 
        for(int j = 0; j < answer.length(); j+=2){
          // hexStr = String(answer[j]) + String(answer[j+1]);       // Вырежем новый байт из ответа (строка с числом в hex формате)
          // num = converter(hexStr, 16, 10).toInt();                // Получим значение байта в десятичной системе счисления
          hexStr = "0x" + String(answer[j]) + String(answer[j+1]); 
          num = strtol(hexStr.c_str(),NULL,0);
          uploadFileGPRS.write(char(num));                        // Запишем в файл новый байт
        }
        i++;

        drawImg("Функция загрузки\nпрошивки\nGPRS\nразмер  " + String(sizeFile) + "\nскачано " + String((i-1) * 100) + "\nвремя   " + String(ceil(TIMER100 / 10)));
      }
    }
  }
  
  uploadFileGPRS.close(); // Закроем файл
}


// Процедура начальной инициализации GSM модуля
int gprs_init(String APN, String USER, String PWD) 
{  
  String answer = "";                               // Хранит весь ответ SIM800l
  int err = 1;                                      // Проверка на ошибки где err = 1 - всё в норме, err = 0 - всё плохо
  // Массив АТ команд
  String ATs[7] = {  
    "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"",            // Установка настроек подключения
    "AT+SAPBR=3,1,\"APN\",\"" + APN + "\"", // APN
    "AT+SAPBR=3,1,\"USER\",\"" + USER + "\"",       // Имя пользователя
    "AT+SAPBR=3,1,\"PWD\",\"" + PWD + "\"",         // Пароль
    "AT+SAPBR=1,1",                                 // Устанавливаем GPRS соединение
    "AT+HTTPINIT",                                  // Инициализация http сервиса
    "AT+HTTPPARA=\"CID\",1"                         // Установка CID параметра для http сессии
  };
  for (int i = 0; i < 7; i++) {
    USART_STM32.println(ATs[i]);                          // Посылаем в GSM модуль
    if(i!=4) {
      if(waitAnswerUSART(WAIT, "OK") == "ERROR") err = 0;   // Ждём ответ и проверяем на корректность (если ответ не пришёл или ответ не соответствует ожиданиям вернётся "ERROR")
    }
    if(i==4) {                                            // Наэтом этапе ожидается два последовательных ответа от SIM800l
      if(waitAnswerUSART(WAIT, ATs[4]) == "ERROR") err = 0; // Здесь SIM800l вернёт строку запроса (сам хз почему ответы отдельно, вообще это костыль решение которого для меня загадка)
      if(waitAnswerUSART(WAIT, "OK") == "ERROR") err = 0;
    }
  }
  return err;
}


// Отправка данных на сайт
String gprs_send(String serverURL, String data) 
{
  String result = "ERROR";    // Результирующая строка (после обработки) (в идеале вернёт 220 байт)
  String answer = "";         // Хранит весь ответ SIM800l
  String sizePayloadStr = ""; // Размер полезной нагрузки в байтах (220 байт, первый байт - знак "=", контрольная сумма "FFF", дальше полезная нагрузка). В виде строки
  int sizePayload = 0;        // Всё тоже самое только int
  int sizeAnswer = 0;         // Размер всего ответа
  int err = 1;                // Проверка на ошибки где err = 1 - всё в норме, err = 0 - всё плохо
  int indexNum = 0;           // Индекс необходимых символов в строке ответа (answer). Поможет найти размер полезной нагрузки
  // Массив АТ команд
  String ATs[] = {  
    "AT+HTTPPARA=\"URL\",\"" + serverURL + "?d=" + data + "\"",
    "AT+HTTPACTION=0",
    "AT+HTTPREAD"
  };
  for(int i = 0; i < 3; i++){
    if(err){
      USART_STM32.println(ATs[i]);                          // Посылаем в GSM модуль
      answer = waitAnswerUSART(WAIT, "OK");                   // Ждём ответ и проверяем на корректность (если ответ не пришёл или ответ не соответствует ожиданиям вернётся "ERROR")
      if(answer == "ERROR") err = 0;
      if(i==1){                                             // На этом этапе ожидается два последовательных ответа от SIM800l
        answer = waitAnswerUSART(WAIT, "200");                // Проверим код ответа (должен быть "200") иначе вернётся "ERROR"
        if(answer == "ERROR") err = 0;
        else {                                              // Определим размер полезной нагрузки (третий параметр ответа)
          indexNum = substrInStr(answer, ",", 1);           // Достанем третий параметр ответа (пример: "0,200,220"). Важно размер ответа должен быть трёх значным (за это отвечает сервер) а в идеале вообще 220 иначе ошибка
          for(int i = 0; i < 3; i++) {
            sizePayloadStr += answer[indexNum + 5 + i];
          }
          sizePayload = sizePayloadStr.toInt();             // Вытащим размер в байтах (в идеале 220 байт)
        }
      }
    }
  }
  sizeAnswer = answer.length();   // Размер ответа 
  if(sizeAnswer != 255) err = 0;  // Должен быть 255 (220 байт полезной нагрузки от сервера + 35 байт мусора от SIM800l)
  if(err){
    result = "";
    indexNum = substrInStr(answer, "=", 1);
    for(int i = indexNum; i < indexNum + sizePayload; i++) result += answer[i];
  }
  else result = "ERROR";
  return result;
}


// Ожидание ответа USART
// wait - сколько секунд ждать на каждом запросе
int waitAnswerUSART(int wait)
{
  TIMER1000 = 0;                                                        // Таймер, инкрементируется каждую секунду
  while(USART_STM32.available() == 0 && TIMER1000 < wait) delay(100);   // Ждём ответа до тех пор пока он не придёи или пока не закончится время (wait)
  if(USART_STM32.available()) return 1;                                 // Если ответ пришёл вернём 1
  return 0;                                                             // Если ответ не пришёл за указанное время вернём 0
}
// answer - предполагаемый ответ
String waitAnswerUSART(int wait, String substr)
{
  TIMER1000 = 0;                                                        // Таймер, инкрементируется каждую секунду
  String answer = "ERROR";                                              // Хранит весь ответ SIM800l
  while(USART_STM32.available() == 0 && TIMER1000 < wait) delay(100);   // Ждём ответа до тех пор пока он не придёт или пока не закончится время (wait)
  if(USART_STM32.available()) {                                         // Если ответ пришёл
    answer = Read_USART_STM32();                                        // Ответ прочтём его в answer
    if(substrInStr(answer, substr, 0)) return answer;                   // Проверка на наличие необходимой подстроки в строке ответа. Если всё успешно функция вернёт строку ответа
    else answer = "ERROR";                                              
  }
  return answer;                                                        // Если ответ не пришёл или он не соответствует ожиданиям функция вернёт "ERROR"
}


// Функция чтения данных от USART_STM32
String Read_USART_STM32() 
{
  int tmp;
  String str;
  while (USART_STM32.available()) {  // Сохраняем входную строку в переменную str
    tmp = USART_STM32.read();
    str += char(tmp);
    delayMicroseconds(100);
  }
  return str;
}


// Отправка SMS (без проверок)
void sendSms(String number, String text)
{
  USART_STM32.print("AT+CMGF=1\r");                   // Формат SMS
  delay(1000);
  USART_STM32.print("AT+CMGS=\"" + number + "\"\r");  // Отправка СМС на указанный номер number"
  delay(2000);
  USART_STM32.print(text);                            // Текст сообщения
  delay(1000);
  USART_STM32.print((char)26);                        // Требуется в соответствии с таблицей данных
  delay(3000);
}



// Таймеры
void funcTimer1000()
{ 
  TIMER1000++;
}
void funcTimer100()
{ 
  TIMER100++;
}



int downloadFileWiFi(String fileURL, String firmware) 
{  
  int err = 0;
  
  // Открываем соединение
  http.begin(fileURL);
  drawImg("Открываем соединение"); delay(100);
  
  // Код ответа
  int httpCode = http.GET();
  drawImg("Код ответа: " + String(httpCode)); delay(100);
  
  if (httpCode > 0) {
  
    // Файл найден на сервере
    if (httpCode == HTTP_CODE_OK) {
      
      drawImg("Файл найден на\nсервере"); delay(100);

      // Удалить файл:
      SD.remove(firmware);
      drawImg("Удаляем " + firmware + "\nс SD карты"); delay(100);

       // Открываем в режиме записи
      uploadFileWiFi = SD.open(firmware, FILE_WRITE);
      drawImg("Подготовка к загрузке\nновой прошивки"); delay(100);

       // Получить длину документа
      int len = http.getSize();
      drawImg("Размер документа:\n" + String(len) + " byte"); delay(100);

       // Создать буфер для чтения
      uint8_t buff[2048] = {0};
      
      // Получить TCP-поток
      WiFiClient *stream = http.getStreamPtr();
    
      // Считывание всех данных с сервера
      while (http.connected() && (len > 0 || len == -1)) {

        // Получить доступный размер данных
        size_t size = stream -> available();
        
        // Чтение до 128 байт
        if (size) { 
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

           // Запишем в файл
          uploadFileWiFi.write(buff, c);
          
          if (len > 0) len -= c;
        }
        delayMicroseconds(1);
      }
      
      drawImg("Загрузка завершена"); delay(100);
      err = 1;
    }
    
    else {
      drawImg("Ошибка соединения\nКод: " + String(httpCode)); delay(100);
      err = 0;
    }
  }
  
  http.end();
  drawImg("Закрываем соединение: "); delay(100);
  
  uploadFileWiFi.close();
  drawImg("Закрываем Файл: "); delay(100);

  if(err == 0){
    drawImg("Ошибка соединения\nКод: " + String(httpCode)); delay(100);
  }
  
  return err;
}



//////////////////////////// program_stm32 /////////////////////////////////////
void program_stm32(String firmware) {
  entr_bootloader();
  write_memory(firmware);
}

//////////////////////////// entr_bootloader /////////////////////////////////////
void entr_bootloader()
{
  on_off_boot(HIGH); // Подтягиваем BOOT_0 к плюсу
  delay(500);
  drawImg("Reset");
  on_reset(); // Нажимаем ресет
  delay(200);

  while(USART_STM32.available()) { USART_STM32.read(); } // В приёмном буфере может быть мусор

  USART_STM32.write(0x7F); // Первый запрос (для определения скорости) 
  
  if(ack_byte() == 0) {
    drawImg ("Bootloader\n - OK"); delay(100);
  }
  else {
    drawImg ("Bootloader\n- ERROR"); delay(100);
  }
}

//////////////////////////// write_memory /////////////////////////////////////
void write_memory(String firmware) {
  if(erase_memory() == 0) {
     File df = SD.open(firmware);
     
     if(df) {
       uint32_t size_file = df.size();

       drawImg ("Size file " + String(size_file)); delay(100);
     
       uint8_t cmd_array[2] = {0x31, 0xCE}; // Код Write Memory 
       uint32_t count_addr = 0;
       uint16_t len = 0;
       uint32_t seek_len = 0;

       while(true) {      
         if(send_cmd(cmd_array) == 0) {
           uint8_t ret_adr = send_adress(WRITE_ADDR + count_addr);
           count_addr = count_addr + SIZE_WRITE;
           
           if(ret_adr == 0) {       
             uint8_t write_buff[SIZE_WRITE] = {0,};
             len = df.read(write_buff, SIZE_WRITE);
             seek_len++;
             df.seek(SIZE_WRITE * seek_len);  
             //write_memory(write_buff, len);file.position()
              
             uint8_t cs, buf[SIZE_WRITE + 2];
             uint16_t i, aligned_len;
              
             aligned_len = (len + 3) & ~3;
             cs = aligned_len - 1;
             buf[0] = aligned_len - 1;
              
             for(i = 0; i < len; i++) {
               cs ^= write_buff[i];
               buf[i + 1] = write_buff[i];
             }
              
             for(i = len; i < aligned_len; i++) {
               cs ^= 0xFF;
               buf[i + 1] = 0xFF;
             }
              
             buf[aligned_len + 1] = cs;
             USART_STM32.write(buf, aligned_len + 2);
             uint8_t ab = ack_byte();
          
             if(ab != 0) {
               drawImg ("Block not\nWrite Memory\n- ERROR"); delay(100);
               break;
             }
             
             if(size_file == df.position()) {
               drawImg ("End Write Memory2\n- OK"); delay(100);
               boot_off_and_reset();
               break;
             }                 
           }
           else {
               drawImg ("Address\nWrite Memory\n- ERROR"); delay(100);
             break;
           }
         }
         else {
           drawImg ("Cmd cod\nWrite Memory\n- ERROR"); delay(100);
           break;
         }             
       } // end while
       
       df.close();
     }
     else {
       drawImg ("Not file\n- ERROR"); delay(100);       
     }
  } 
  else {
    drawImg ("Not erase\nWrite Memory\n- ERROR"); delay(100);    
  }
}

////////////////////////////// erase_memory ////////////////////////////////////
uint8_t erase_memory() {
  uint8_t cmd_array[2] = {0x43, 0xBC}; // Команда на стирание 

  if(send_cmd(cmd_array) == 0) {
    uint8_t cmd_array[2] = {0xFF, 0x00}; // Код стирания (полное) 

    if(send_cmd(cmd_array) == 0) {
      drawImg ("Erase Memory\n - OK"); delay(100);
      return 0;
    } 
    else {
      drawImg ("Cmd cod\nErase Memory\n- ERROR"); delay(100);         
    }
  }
  else {
      drawImg ("Cmd start\nErase Memory\n- ERROR"); delay(100); 
  }

  return 1;
}

////////////////////////////// send_cmd ////////////////////////////////////
uint8_t send_cmd(uint8_t *cmd_array) {
  USART_STM32.write(cmd_array, 2); 
  if(ack_byte() == 0) {
    return 0;
  }
  else {
    return 1;
  }
}

/////////////////////////////// ack_byte ////////////////////////////////////
uint8_t ack_byte() { 
  for(uint16_t i = 0; i < 500; i++) {
    if(USART_STM32.available()) {
      uint8_t res = USART_STM32.read();
      if(res == 0x79) {
        return 0;         
      }
    } 
    delay(1);  
  }
  return 1;
}

///////////////////////////// send_adress ////////////////////////////////////
uint8_t send_adress(uint32_t addr) {
  uint8_t buf[5] = {0,};  
  buf[0] = addr >> 24;
  buf[1] = (addr >> 16) & 0xFF;
  buf[2] = (addr >> 8) & 0xFF;
  buf[3] = addr & 0xFF;
  buf[4] = buf[0] ^ buf[1] ^ buf[2] ^ buf[3];
  
  USART_STM32.write(buf, 5);
  if(ack_byte() == 0) {
    return 0;
  }
  else {
    return 1;  
  }
}

////////////////////////////// on_reset //////////////////////////////////////
void on_reset() {
  digitalWrite(RESET_PIN, HIGH); // reset
  delay(500);    
  digitalWrite(RESET_PIN, LOW); 
}

////////////////////////////// on_off_boot ///////////////////////////////////
void on_off_boot(uint8_t i) {
  digitalWrite(BOOT_PIN, i); 
}

//////////////////////////// boot_off_and_reset //////////////////////////////
void boot_off_and_reset() {
  on_off_boot(LOW);
  delay(500);
  on_reset(); 
  drawImg ("Boot off and reset"); delay(100);
}

////////////////////////////// on_boot ///////////////////////////////////
void on_boot() {
  digitalWrite(BOOT_PIN, HIGH); 
}

////////////////////////////// off_boot ///////////////////////////////////
void off_boot() {
  digitalWrite(BOOT_PIN, LOW); 
}



// Функция для принудительных команд напрямую дисплею
void oledCommand(int comm) {
 Wire.beginTransmission(0x3C);  // Начинаем передачу команд устройству с адресом 0x3C.
 Wire.write(0x00);              // Сообщаем дисплею, что следующее передаваемое значение - команда.
 Wire.write(comm);              // Передаем команду.
 Wire.endTransmission();        // Завершаем передачу данных.
}



// OLED работа с русским текстом 
String utf8rus(String source) 
{
  int i,k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };
  k = source.length(); i = 0;
  while (i < k) {
    n = source[i]; i++;
    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
          if (n == 0x81) { n = 0xA8; break; }
          if (n >= 0x90 && n <= 0xBF) n = n + 0x2F;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB7; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x6F;
          break;
        }
      }
    }
    m[0] = n; target = target + String(m);
  }
  return target;
}


// Вывод сообщения на OLED дисплей
void drawImg(String str) 
{
  display.clearDisplay();         // Чистим дисплей
  display.setTextSize(1);         // Устанавливаем размер шрифта (размер пикселей)
  display.setTextColor(WHITE);    // Устанавливаем цвет текста
  display.setCursor(0, 0);        // Устанавливаем курсор  
  display.println(utf8rus(str));  // Вводим русский текст
  display.display();              // Устанавливаем изображение
}

// Вывод сообщения на OLED дисплей
void drawImg(String str, int x, int y, int textSize, int dl, String language) 
{
  if(x > 128 || x < 0) x = 0;    // Дабы не вылезти за пределы экрана
  if(y > 64 || y < 0) y = 0;     // Дабы не вылезти за пределы экрана
  
  display.clearDisplay();        // Чистим дисплей
  display.setTextSize(textSize); // Устанавливаем размер шрифта (размер пикселей)
  display.setTextColor(WHITE);   // Устанавливаем цвет текста
  display.setCursor(x, y);       // Устанавливаем курсор  
  
  if(language == "rus") display.println(utf8rus(str));  // Вводим русский текст
  if(language == "eng") display.println(str);           // Вводим английский текст

  display.display(); // Устанавливаем изображение
  delay(dl); // Задержка
}



// Функция для поиска подстроки в строке
// num = 0 - возвращает кол-во вхождений подстроки в строку
// num = 1 - возвращает индекс первого символа первого вхождения подстроки в строку
int substrInStr(String str, String substr, int num)
{
  int lenStr = str.length();
  int lenSubstr = substr.length();
  
  int symbol = 0; // Кол-во совпавших символов
  int count = 0;  // Кол-во вхождений подстроки в строку
  int index = -1; // Индекс начала подстроки в строке
  int tmp = 0;

  // Пока есть возможность поиска
  for (int i = 0; i <= lenStr - lenSubstr; i++) {
    if (str[i] == substr[0]) {
      if (tmp == 0) { index = i; tmp++; }
      for (int j = 0; j < lenSubstr; j++) {
        if (str[i + j] == substr[j]) symbol++;
        else symbol = 0;
      }
      if (lenSubstr == symbol) {
        count++;
        symbol = 0;
      }
      else index = -1;
    }
  }
  if(num == 0) return count;
  else return index;
}



// Конвертирует число в виде строки из in разрядности в число в виде строки out разрядности
// В функции нет проверки на корректность введённой строки
String converter(String str, int in, int out)
{
  String outStr = "";
  int decInt = 0;
  double number = 0;
  int sizeStr = str.length(); // Кол-во цифр

  if(in < 1 || in > 16) return "ERROR"; 
  if(out < 1 || in > out) return "ERROR"; 

  if(sizeStr > 0) {
    // Удалим все лишние нули (пример: из "00203" в "203")
    while(str.charAt(0) == '0' && sizeStr > 1) {
      str.remove(0, 1);
      sizeStr = str.length(); // Кол-во цифр
    }
    if(str == "0") return "0"; 
    
    // Кусок кода, который конвертирует число in-разрядности в десятичное число
    for(int j = 0; j < sizeStr; j++) {
      if(str.charAt(j) == '0')                         number = 0  * pow(in, (sizeStr - j) - 1);
      if(str.charAt(j) == '1')                         number = 1  * pow(in, (sizeStr - j) - 1);
      if(str.charAt(j) == '2')                         number = 2  * pow(in, (sizeStr - j) - 1);
      if(str.charAt(j) == '3')                         number = 3  * pow(in, (sizeStr - j) - 1);
      if(str.charAt(j) == '4')                         number = 4  * pow(in, (sizeStr - j) - 1);
      if(str.charAt(j) == '5')                         number = 5  * pow(in, (sizeStr - j) - 1);
      if(str.charAt(j) == '6')                         number = 6  * pow(in, (sizeStr - j) - 1);
      if(str.charAt(j) == '7')                         number = 7  * pow(in, (sizeStr - j) - 1);
      if(str.charAt(j) == '8')                         number = 8  * pow(in, (sizeStr - j) - 1);
      if(str.charAt(j) == '9')                         number = 9  * pow(in, (sizeStr - j) - 1);
      if(str.charAt(j) == 'A' || str.charAt(j) == 'a') number = 10 * pow(in, (sizeStr - j) - 1);
      if(str.charAt(j) == 'B' || str.charAt(j) == 'b') number = 11 * pow(in, (sizeStr - j) - 1);
      if(str.charAt(j) == 'C' || str.charAt(j) == 'c') number = 12 * pow(in, (sizeStr - j) - 1);
      if(str.charAt(j) == 'D' || str.charAt(j) == 'd') number = 13 * pow(in, (sizeStr - j) - 1);
      if(str.charAt(j) == 'E' || str.charAt(j) == 'e') number = 14 * pow(in, (sizeStr - j) - 1);
      if(str.charAt(j) == 'F' || str.charAt(j) == 'f') number = 15 * pow(in, (sizeStr - j) - 1);
      decInt = decInt + (int) number;
    }
    // Кусок кода, который конвертирует десятичное чесло в число out-разрядности
    while (decInt != 0) {
      if (decInt % out == 0)  outStr = "0" + outStr;
      if (decInt % out == 1)  outStr = "1" + outStr;
      if (decInt % out == 2)  outStr = "2" + outStr;
      if (decInt % out == 3)  outStr = "3" + outStr;
      if (decInt % out == 4)  outStr = "4" + outStr;
      if (decInt % out == 5)  outStr = "5" + outStr;
      if (decInt % out == 6)  outStr = "6" + outStr;
      if (decInt % out == 7)  outStr = "7" + outStr;
      if (decInt % out == 8)  outStr = "8" + outStr;
      if (decInt % out == 9)  outStr = "9" + outStr;
      if (decInt % out == 10) outStr = "A" + outStr;
      if (decInt % out == 11) outStr = "B" + outStr;
      if (decInt % out == 12) outStr = "C" + outStr;
      if (decInt % out == 13) outStr = "D" + outStr;
      if (decInt % out == 14) outStr = "E" + outStr;
      if (decInt % out == 15) outStr = "F" + outStr;
      decInt = decInt / out;
    }
    return outStr;
  }
  else return "Error";
}
                
