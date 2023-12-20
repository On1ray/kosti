#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>


#define PIN 6  // Пин для управления WS2812B-63
#define NUM_LEDS 63  // Количество светодиодов в матрице

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);;


const uint64_t IMAGES[] = {
  0x0002020202020200,
  0x3c04043c20203c00,
  0x3c20203c20203c00,
  0x2020203c24242400,
  0x3c20203c04043c00,
  0x3c24243c04043c00,
  0x2020202020203c00,
  0x3c24243c24243c00,
  0x3c20203c24243c00,
  0xf29292929292f200,
  0x1212121212121200,
  0xf21212f28282f200,
  0xf28282f28282f200,
  0x828282f292929200,
  0xf28282f21212f200,
  0xf29292f21212f200,
  0x828282828282f200,
  0xf29292f29292f200,
  0xf28282f29292f200,
  0xf79191979494f700
};
const int IMAGES_LEN = sizeof(IMAGES)/8;


//
// MPU6050 defines
//
Adafruit_MPU6050 mpu;
float shakeThreshold = 1; // Порог встряски
float AccelOffsetX = 0;
float AccelOffsetY = 0;
float AccelOffsetZ = 0;
sensors_event_t event;



const int buttonPin1 = 2;  // Первая кнопка подключена к пину 2
const int buttonPin2 = 3;  // Вторая кнопка подключена к пину 3
const int ledPin = 13;  // Встроенный светодиод на плате Arduino

int lastButtonState1 = HIGH;  // Переменная для хранения предыдущего состояния первой кнопки
int lastButtonState2 = HIGH;  // Переменная для хранения предыдущего состояния второй кнопки
int currentButtonState1;  // Переменная для хранения текущего состояния первой кнопки
int currentButtonState2;  // Переменная для хранения текущего состояния второй кнопки

int selectedDice = 0;  // Выбранный кубик (значение от 4 до 20)

void setup() {
  strip.begin();
  strip.show();  // Инициализация светодиодной матрицы
  Serial.begin(9600);
  pinMode(buttonPin1, INPUT_PULLUP);  // Установка пина первой кнопки как вход
  pinMode(buttonPin2, INPUT_PULLUP);  // Установка пина второй кнопки как вход

  // MPU6050 Init
  while (!mpu.begin()) {
    Serial.println("MPU6050 not connected!");
    delay(1000);
  }
  Serial.println("MPU6050 ready!");

  mpu.getAccelerometerSensor()->getEvent(&event);
  AccelOffsetX = event.acceleration.x;
  AccelOffsetY = event.acceleration.y;
  AccelOffsetZ = event.acceleration.z;
  pinMode(ledPin, OUTPUT);  // Установка встроенного светодиода на плате Arduino как выход
  char str[128] = {0};
  sprintf(str, "MPU6050 offsets: x = %.6f; y = %.6f; z = %.6f;", AccelOffsetX, AccelOffsetY, AccelOffsetZ);
  Serial.println(str);

}

void loop() {
  currentButtonState1 = digitalRead(buttonPin1);  // Считывание состояния первой кнопки
  currentButtonState2 = digitalRead(buttonPin2);  // Считывание состояния второй кнопки

  if (currentButtonState1 != lastButtonState1) {
    if (currentButtonState1 == LOW) {
      selectDice();  // Вызов функции для выбора кубика
    }
  }

  if (currentButtonState2 != lastButtonState2 || isShakeDetected()) {
    if (currentButtonState2 == LOW  || isShakeDetected()) {
      confirmSelection();  // Вызов функции для подтверждения выбора
    }
  }

  lastButtonState1 = currentButtonState1;  // Сохранение текущего состояния первой кнопки
  lastButtonState2 = currentButtonState2;  // Сохранение текущего состояния второй кнопки
}

void selectDice() {
  if (selectedDice < 20) {
    selectedDice += 2;  // Увеличение выбранного значения кубика
  } else {
    selectedDice = 4;  // Сброс выбранного значения кубика до начального
  }
  displayDiceValue(selectedDice);  // Отображение значения кубика на светодиодной матрице
}

void confirmSelection() {
  rollDice(selectedDice);  // Вызов функции для эмуляции броска выбранного кубика
  delay(1000);  // Задержка для эмуляции броска и отображения результата
}

void rollDice(int max) {
  int diceValue = random(1, max + 1);  // Генерация случайного значения от 1 до max
  displayDiceValue(diceValue);  // Отображение значения на светодиодной матрице
  return;
}

void displayDiceValue(int data) {
  uint64_t image = IMAGES[data - 1];
  for (int i = 0; i < 8; i++) {
    byte row = (image >> i * 8) & 0xFF;
    for (int j = 0; j < 8; j++) {
      if (bitRead(row, j)) {
        strip.setPixelColor(i*8 + j - 1, strip.Color(255,0,0,0));
      } else {
        strip.setPixelColor(i*8 + j - 1, strip.Color(0,0,0,0));

      }
    }
  }
  strip.show();
}



bool isShakeDetected() {
  mpu.getAccelerometerSensor()->getEvent(&event);
  float x = event.acceleration.x;
  float y = event.acceleration.y;
  float z = event.acceleration.z;

  char str[128] = {0};
  sprintf(str, "MPU6050: x = %.6f; y = %.6f; z = %.6f;", x-AccelOffsetX, y-AccelOffsetY, z-AccelOffsetZ);
  Serial.println(str);
  
  int totalAcceleration = abs(x-AccelOffsetX) + abs(y-AccelOffsetY) + abs(z-AccelOffsetZ);
  
  return totalAcceleration > shakeThreshold;
}