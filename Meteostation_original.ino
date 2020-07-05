// библиотека для работы I²C
#include <Wire.h>
// библиотека для модуля приближения и освещённости VL6180X
#include <SparkFun_VL6180X.h>
// библиотека для дисплея
#include <QuadDisplay2.h>
// библиотека для датчика температуры и влажности
#include <TroykaMeteoSensor.h>
// библиотека для часов реального времени
#include <TroykaRTC.h>

void lightSwitchCondition(uint8_t tMonth, uint8_t tHour, uint8_t MDistance, const int MaxDistance);

// Блок переменных

  // размер массива для времени
  #define LEN_TIME 12
  // размер массива для даты
  #define LEN_DATE 12
  // размер массива для дня недели
  #define LEN_DOW 12
 
  // массив для хранения текущего времени
  char time[LEN_TIME];
  // массив для хранения текущей даты
  char date[LEN_DATE];
  // массив для хранения текущего дня недели
  char weekDay[LEN_DOW];

  #define LED_piranha 2
  // адрес датчика приближения по умолчанию
  #define VL6180X_ADDRESS 0x29

  // время вывода на дисплей показаний температуры и влажности, мс
  unsigned const delayTime = 2500;
  // время вывода на дисплей пиковых показаний
  unsigned const delayTimeMinMax = 2000;
  // время с начала работы Arduino
  unsigned long timer;

  // Пиковые показания температуры и влажности (первоначальная настройка)
  float tempMeasMax = -100.0;
  float humMeasMax = 0.0;
  float tempMeasMin = 100.0;
  float humMeasMin = 100.0;

// Объекты  - часы, датчик приближения и освещённости, метеодатчик и дисплей
  RTC clock;
  VL6180x sensor(VL6180X_ADDRESS);
  TroykaMeteoSensor meteoSensor;
  QuadDisplay qDisplay(9);
 
void setup() 
{
  pinMode(LED_piranha, OUTPUT);
  qDisplay.begin();
    
  // открываем последовательный порт
  Serial.begin(115200);
  // ждём открытия порта
  while(!Serial) {}
  // печатаем сообщение об успешной инициализации Serial-порта
  Serial.println("Serial init OK");
  // начало работы с датчиком температуры
  meteoSensor.begin();
  Serial.println("Meteo Sensor init OK");
  
  while (sensor.VL6180xInit()) 
  {
    Serial.println("Failed to initalize");
    delay(1000);
  }
  // загружаем настройки модуля по умолочнию
  sensor.VL6180xDefautSettings();

  // настройка часов
  clock.begin();
  // после первоначальной прошивки нужно закомментировать
  // clock.set(__TIMESTAMP__);

  // ждём одну секунду
  delay(1000);
}
 
void loop() 
{
  // считываем данные с метеодатчика
  int stateSensor = meteoSensor.read();
  
  // запрашиваем данные с часов
  clock.read();
  // сохраняем текущее время, дату и день недели в переменные
  clock.getTimeStamp(time, date, weekDay);
  // получаем показания времени для управления выводом данных на дисплей
  uint8_t month = clock.getMonth();
  uint8_t hour = clock.getHour();
  uint8_t minute = clock.getMinute();

  // получаем показания дальности и освещённости
  uint8_t distance = sensor.getDistance();
  float lightLevel = sensor.getAmbientLight(GAIN_1);
    
  const int DoorDistance = 110;
  lightSwitchCondition(month, hour, distance, DoorDistance);

  // проверяем состояние данных метеодатчика
  switch (stateSensor) 
  {
    case SHT_OK:
      // получаем и выводим показания температуры и влажности 
      float tempC    = meteoSensor.getTemperatureC();
      float humidity = meteoSensor.getHumidity();
      // округляем показания к ближайшему целому
      int8_t tempToDisplay     = round(tempC);
      uint8_t humidityToDisplay = round(humidity);
      
      // переменные для показаний температуры и влажности и время измерения
      uint8_t tempMaxMeasHour;
      uint8_t tempMaxMeasMinute;
      uint8_t tempMinMeasHour;
      uint8_t tempMinMeasMinute;
      uint8_t humMaxMeasHour;
      uint8_t humMaxMeasMinute;
      uint8_t humMinMeasHour;
      uint8_t humMinMeasMinute;
      // округлённые показания
      int8_t tempMeasMaxDisp;
      int8_t tempMeasMinDisp;
      uint8_t humMeasMaxDisp;
      uint8_t humMeasMinDisp;
      
      // Сброс показаний за текущие сутки
      if (hour == 0 && minute == 0)
      {
        tempMeasMax = -100.0;
        humMeasMax = 0.0;
        tempMeasMin = 100.0;
        humMeasMin = 100.0;
        //tempMeasMaxDisp = round(tempMeasMax);
        //tempMeasMinDisp = round(tempMeasMin);
        //humMeasMaxDisp = round(humMeasMax);
        //humMeasMinDisp = round(humMeasMin); 
      }
      // получаем пиковые показания температуры и влажности и время измерения
      if (tempC > tempMeasMax)
      {
        tempMeasMax = tempC;
        tempMeasMaxDisp = round(tempMeasMax);
        tempMaxMeasHour = hour;
        tempMaxMeasMinute = minute;
      }
     
      if (tempC < tempMeasMin)
      {
        tempMeasMin = tempC;
        tempMeasMinDisp = round(tempMeasMin);
        tempMinMeasHour = hour;
        tempMinMeasMinute = minute;
      }
     
      if (humidity > humMeasMax)
      {
        humMeasMax = humidity;
        humMeasMaxDisp = round(humMeasMax);
        humMaxMeasHour = hour;
        humMaxMeasMinute = minute;
      }
     
      if (humidity < humMeasMin)
      {
        humMeasMin = humidity;
        humMeasMinDisp = round(humMeasMin);
        humMinMeasHour = hour;
        humMinMeasMinute = minute;
      }
      
      // Отладка показаний
      /*
      Serial.println("Data sensor is OK");
      
      Serial.print("Temperature = ");
      Serial.print(meteoSensor.getTemperatureC());
      Serial.println(" C \t");

      Serial.print("Humidity = ");
      Serial.print(meteoSensor.getHumidity());
      Serial.println(" %\r\n");
      
      
      Serial.print("Light = ");
      Serial.print(sensor.getAmbientLight(GAIN_1));
      Serial.print(" Lx\t\t");
      Serial.print("Distance = ");
      Serial.print(sensor.getDistance() );
      Serial.println(" mm\n");
      
      // выводим в serial порт текущее время, дату и день недели
      Serial.print(time);
      Serial.print("\t");
      Serial.print(date);
      Serial.print("\t\t");
      Serial.print(weekDay);
      Serial.print("\n\n");
      
      // выводим пиковые показания
      Serial.print("Max Temperature = ");
      Serial.print(tempMeasMax);
      Serial.print(" C\t");
      Serial.print("Min Temperature = ");
      Serial.print(tempMeasMin);
      Serial.print(" C\t");

      Serial.print("Max humidity = ");
      Serial.print(humMeasMax);
      Serial.print(" %\t");
      Serial.print("Min humidity = ");
      Serial.print(humMeasMin);
      Serial.print(" %\t");
      Serial.print("\n");
      
      Serial.print("Measurement time = ");
      Serial.print(tempMaxMeasHour);
      Serial.print(":");
      Serial.print(tempMaxMeasMinute);
      Serial.print("\t");
      Serial.print("Measurement time = ");
      Serial.print(tempMinMeasHour);
      Serial.print(":");
      Serial.print(tempMinMeasMinute);
      Serial.print("\t");
      
      Serial.print("Measurement time = ");
      Serial.print(humMaxMeasHour);
      Serial.print(":");
      Serial.print(humMaxMeasMinute);
      Serial.print("\t");
      Serial.print("Measurement time = ");
      Serial.print(humMinMeasHour);
      Serial.print(":");
      Serial.print(humMinMeasMinute);
      Serial.print("\n\n\n");
           
      delay(delayTime);
      */
            
     // условие постоянной работы дисплея
     bool absoluteTaimCondition;
     if (hour >= 6 && hour < 8)
     {
       absoluteTaimCondition = 1;
     }
     else
     {
       absoluteTaimCondition = 0;
     }
      // выводим показания на дисплей, удерживая каждое на время timer
      timer = millis();
      // порог световой чувствительности при котором зажигается дисплей
      const int lightTreshold = 1;
      // порог расстояния для вывода на дисплей пиковых показаний
      const int minMaxDoorDistance = 80;
      // таймер, по которому переключаются показания на дисплее
      bool switchTime = timer / delayTime % 2;
      
      if (switchTime && distance > DoorDistance || switchTime && absoluteTaimCondition || switchTime && lightLevel > lightTreshold)
      {
        qDisplay.displayTemperatureC(tempToDisplay);
      }
      else if (!switchTime && distance > DoorDistance || !switchTime && absoluteTaimCondition || !switchTime && lightLevel > lightTreshold)
      {
        qDisplay.displayHumidity(humidityToDisplay);
      }
      else if (distance < minMaxDoorDistance)
      {
        qDisplay.displayDigits(QD_I, QD_n, QD_f, QD_O);
        delay(delayTimeMinMax);
        qDisplay.displayTemperatureC(tempMeasMaxDisp);
        delay(delayTimeMinMax);
        qDisplay.displayTemperatureC(tempMeasMinDisp);
        delay(delayTimeMinMax);
        qDisplay.displayHumidity(humMeasMaxDisp);
        delay(delayTimeMinMax);
        qDisplay.displayHumidity(humMeasMinDisp);
        delay(delayTimeMinMax);
      }
      else
      {
        qDisplay.displayClear();
      }
      break;
      
    // ошибка данных или сенсор не подключён
    case SHT_ERROR_DATA:
      Serial.println("Data error or sensor not connected");
      break; 
      
    // ошибка контрольной суммы
    case SHT_ERROR_CHECKSUM:
      Serial.println("Checksum error");
      break;
  }
}

//***********************************************************************************************//
// Функция проверки месяца, часа и измеряемой дистанции
// по условиям которых зажигается светодиод
void lightSwitchCondition(uint8_t tMonth, uint8_t tHour, uint8_t MDistance, const int MaxDistance)
{
    switch (tMonth)
  {
    case 1:
      if (MDistance > MaxDistance && tHour >= 16)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else if (MDistance > MaxDistance && tHour < 8)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else
      {
        digitalWrite(LED_piranha, LOW);
      }
      break;
    case 2:
      if (MDistance > MaxDistance && tHour >= 17)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else if (MDistance > MaxDistance && tHour < 8)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else
      {
        digitalWrite(LED_piranha, LOW);
      }
      break;
    case 3:
      if (MDistance > MaxDistance && tHour >= 18)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else if (MDistance > MaxDistance && tHour < 8)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else
      {
        digitalWrite(LED_piranha, LOW);
      }
      break;
    case 4:
      if (MDistance > MaxDistance && tHour >= 19)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else if (MDistance > MaxDistance && tHour < 6)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else
      {
        digitalWrite(LED_piranha, LOW);
      }
      break;
    case 5:
      if (MDistance > MaxDistance && tHour >= 19)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else if (MDistance > MaxDistance && tHour < 5)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else
      {
        digitalWrite(LED_piranha, LOW);
      }
      break;
    case 6:
      if (MDistance > MaxDistance && tHour >= 20)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else if (MDistance > MaxDistance && tHour < 5)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else
      {
        digitalWrite(LED_piranha, LOW);
      }
      break;
    case 7:
      if (MDistance > MaxDistance && tHour >= 20)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else if (MDistance > MaxDistance && tHour < 5)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else
      {
        digitalWrite(LED_piranha, LOW);
      }
      break;
    case 8:
      if (MDistance > MaxDistance && tHour >= 19)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else if (MDistance > MaxDistance && tHour < 6)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else
      {
        digitalWrite(LED_piranha, LOW);
      }
      break;
    case 9:
      if (MDistance > MaxDistance && tHour >= 19)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else if (MDistance > MaxDistance && tHour < 7)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else
      {
        digitalWrite(LED_piranha, LOW);
      }
      break;
    case 10:
      if (MDistance > MaxDistance && tHour >= 18)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else if (MDistance > MaxDistance && tHour < 7)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else
      {
        digitalWrite(LED_piranha, LOW);
      }
      break;
    case 11:
      if (MDistance > MaxDistance && tHour >= 17)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else if (MDistance > MaxDistance && tHour < 8)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else
      {
        digitalWrite(LED_piranha, LOW);
      }
      break;
    case 12:
      if (MDistance > MaxDistance && tHour >= 16)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else if (MDistance > MaxDistance && tHour < 8)
      {
        digitalWrite(LED_piranha, HIGH);
      }
      else
      {
        digitalWrite(LED_piranha, LOW);
      }
      break;
  }
}
