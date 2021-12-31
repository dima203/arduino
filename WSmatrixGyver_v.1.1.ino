// **************** НАСТРОЙКИ МАТРИЦЫ ****************
#define LED_PIN 6           // пин ленты
#define BRIGHTNESS 20       // стандартная маскимальная яркость (0-255)
#define CURRENT_LIMIT 2000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define WIDTH 32            // ширина матрицы
#define HEIGHT 16           // высота матрицы
#define SEGMENTS 1            // диодов в одном "пикселе" (для создания матрицы из кусков ленты)

#define COLOR_ORDER GRB       // порядок цветов на ленте. Если цвет отображается некорректно - меняйте. Начать можно с RGB

#define MATRIX_TYPE 0       // тип матрицы: 0 - зигзаг, 1 - последовательная
#define CONNECTION_ANGLE 3  // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION 1   // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
// при неправильном наборе настроек вы увидите надпись "Wrong matrix parameters!"


// ***************** НАСТРОЙКИ Blynk *****************
#define BLYNK_PRINT Serial
#define EspSerial Serial3
#define ESP8266_BAUD 115200


// ***************** НАСТРОЙКИ RFID ******************
#define RST_PIN         5
#define SS_PIN          53 


// **************** ДЛЯ РАЗРАБОТЧИКОВ ****************
#include "bitmap2.h"
#define DEBUG 0
#define NUM_LEDS WIDTH * HEIGHT
#include "FastLED.h"
CRGB leds[NUM_LEDS];

#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>

#include "IRremote.h"
#include <Wire.h>
#include "RTClib.h"

#include <SPI.h>
#include <MFRC522.h>

char auth[] = "2JrFGGWs5_c9KSqT50zR3mTGj6seY2Mz";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "29/1";
char pass[] = "88888888";


// функция загрузки картинки в матрицу. должна быть здесь, иначе не работает =)
void loadImage(uint16_t (*frame)[WIDTH]) {
  for (byte i = 0; i < WIDTH; i++)
    for (byte j = 0; j < HEIGHT; j++)
      drawPixelXY(i, j, gammaCorrection(expandColor((pgm_read_word(&(frame[HEIGHT - j - 1][i]))))));
  // да, тут происходит лютенький п@здец, а именно:
  // 1) pgm_read_word - восстанавливаем из PROGMEM (флэш памяти) цвет пикселя в 16 битном формате по его координатам
  // 2) expandColor - расширяем цвет до 24 бит (спасибо adafruit)
  // 3) gammaCorrection - проводим коррекцию цвета для более корректного отображения
}


IRrecv irrecv(11);

ESP8266 wifi(&EspSerial);
BlynkTimer timer;

RTC_DS3231 rtc;

MFRC522 mfrc522(SS_PIN, RST_PIN);

int test_mode = 0; 
int brightness = BRIGHTNESS;

byte red = 255;
byte green = 255;
byte blue = 255;

int snake_move = 0;

bool fire_loading_flag = true;

bool is_access = false;

uint16_t noise_x_offset = 0;
uint16_t noise_y_offset = 0;

CRGB color = CRGB(red, green, blue);


BLYNK_WRITE(V1)
{
  int sliderValue = param.asInt();
  brightness = sliderValue;
  FastLED.setBrightness(brightness);
}

BLYNK_WRITE(V2)
{
  int pinValue = param.asInt();
  test_mode = pinValue;
}

BLYNK_WRITE(V3)
{
  red = param[0].asInt();
  green = param[1].asInt();
  blue = param[2].asInt();
  set_hue(int(red / 255. * 230));
  color = CRGB(red, green, blue);
}

BLYNK_WRITE(V4)
{
  int pinValue = param.asInt();
  if (pinValue != 0) {
    snake_move = pinValue;
  }
}


void matrix_update() {
  if(test_mode != 7) {
    digitalWrite(7, 0);
  }
  if (test_mode != 5) {
    fire_loading_flag = true;
  }
  if (test_mode != 6) {
    noise_x_offset = random(0, 65535);
    noise_y_offset = random(0, 65535);
  }
  if (test_mode != 7) {
    is_access = false;
  }
  switch (test_mode) {
    case 1:
      animation1();
      break;
    case 2:
      text_line();
      break;
    case 3:
      clock_draw();
      break;
    case 4:
      snake_update();
      break;
    case 5:
      fire_effect();
      break;
    case 6:
      noise_update();
      break;
    case 7:
      flag_check();
      break;
    case 8:
      analogWrite(7, brightness);
      FastLED.clear();
      FastLED.show();
      break;
  }
}


byte frameNum;
byte animation_frames = 0;

void animation1() {
  animation_frames++;
  if (animation_frames == 6) {
    animation_frames = 0;

    frameNum++;
    if (frameNum >= 3) frameNum = 0;
    FastLED.clear();
    loadImage(sharingan_array[frameNum]);
    FastLED.show();
  }
}


int text_x = 32;
byte text_y = 5;

byte text_frames = 0;

void text_line() {
  text_frames++;
  if (text_frames == 6) {
    text_frames = 0;

    FastLED.clear();
    drawdima203(text_x, text_y, color);
    FastLED.show();
    text_x--;
    if (text_x < -35) {
      text_x = 32;
    }
  }
}


byte clock_frames = 0;

void clock_draw() {
  clock_frames++;
  if (clock_frames == 6) {
    clock_frames = 0;
    
    DateTime now = rtc.now();
    
    byte nowHour = now.hour();
    byte nowMinute = now.minute();
    byte nowSecond = now.second();
    
    FastLED.clear();
    drawDigit5x7(nowHour / 10, 4, 9, color);
    drawDigit5x7(nowHour % 10, 10, 9, color);
    drawDigit5x7(nowMinute / 10, 17, 9, color);
    drawDigit5x7(nowMinute % 10, 23, 9, color);
    drawDigit5x7(nowSecond / 10, 10, 0, color);
    drawDigit5x7(nowSecond % 10, 17, 0, color);
    FastLED.show();
  }
}


byte snake_x = 16;
byte snake_y = 8;

byte snake_tail[512][2] = {{255, 255}};

byte food_x = 255;
byte food_y = 255;

uint16_t score = 0;

byte snake_frames = 0;

void snake_update() 
{
  snake_frames++;
  if (snake_frames == 12) {
    snake_frames = 0;

    if (snake_x == food_x && snake_y == food_y) {
      if (score > 0) {
        snake_tail[score][0] = snake_tail[score - 1][0];
        snake_tail[score][1] = snake_tail[score - 1][1];
      }
      else {
        snake_tail[score][0] = snake_x;
        snake_tail[score][1] = snake_y;
      }
      food_x = 255;
      food_y = 255;
      score++;
    }

    if (food_x == 255) {
      while (true) {
        food_x = random(0, 32);
        food_y = random(0, 16);
        bool in_tail = false;
        for (int i = 0; i < 512; i++) {
          if (food_x == snake_tail[i][0] && food_y == snake_tail[i][1]) {
            in_tail = true;
            break;
          }
        }
        if (!in_tail) {
          break;
        }
      }
    }

    int end_tail;
    for (end_tail = 0; end_tail < 512; end_tail++) {
      if (snake_tail[end_tail][0] == 255) {
        break;
      }
    }
    
    for (int i = end_tail - 1; i > 0; i--) {
      snake_tail[i][0] = snake_tail[i - 1][0];
      snake_tail[i][1] = snake_tail[i - 1][1];
    }
    if (snake_tail[0][0] != 255) {
      snake_tail[0][0] = snake_x;
      snake_tail[0][1] = snake_y;
    }

    switch (snake_move) {
      case 1:
        snake_y++;
        break;
      case -1:
        snake_y--;
        break;
      case 2:
        snake_x++;
        break;
      case -2:
        snake_x--;
        break;
    }

    bool is_death = false;

    if (snake_x > 31 || snake_x < 0 || snake_y > 15 || snake_y < 0) {
      is_death = true;
    }

    for (int i = 0; i < 512; i++) {
      if (snake_x == snake_tail[i][0] && snake_y == snake_tail[i][1]) {
        is_death = true;
        break;
      }
    }

    if (is_death) {
      snake_x = 16;
      snake_y = 8;
      snake_move = 0;
      score = 0;
      food_x = 255;
      food_y = 255;
      for (int i = 0; i < 512; i++) {
        snake_tail[i][0] = 255;
        snake_tail[i][1] = 255;
      }
    }

    Blynk.virtualWrite(V5, score);
  }
  FastLED.clear();
  drawPixelXY(snake_x, snake_y, CRGB(0, 255, 0));
  drawPixelXY(food_x, food_y, CRGB(255, 0, 0));
  for (int i = 0; i < 512; i++) {
    if (snake_tail[i][0] == 255) {
      break;
    }
    drawPixelXY(snake_tail[i][0], snake_tail[i][1], CRGB(0, 255, 0));
  }
  FastLED.show();
}


void fire_effect() 
{
  fireRoutine();
}


byte uidCard[4] = {0xCC, 0xF7, 0x0C, 0x31};

void flag_check() 
{
  if (! is_access) {
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
      return;
    }
    
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) {
      return;
    }

    bool is_true_id = true;

    for (byte i = 0; i < 4; i++) {
      mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
      if (uidCard[i] != mfrc522.uid.uidByte[i]) {
        is_true_id = false;
        break;
      }
    }

    if (is_true_id) {
      is_access = true;
    }
  }
  else {
    flag_update();
  }
}


byte width_multiply = 5;
byte height_multiply = 3;
byte x_offset = 0;

byte flag_frames = 0;

void flag_update() 
{
  flag_frames++;
  if (flag_frames == 6) {
    flag_frames = 0;

    FastLED.clear();
    for (byte x = 0; x < 32; x++) {
      int y = height_multiply * sin(double(x + x_offset) / double (width_multiply));
      byte flag_height = 0;
      for (byte i = 15 - height_multiply + y; i >= 0 + height_multiply + y; i--) {
        if (flag_height < 3 || flag_height > 6) {
          drawPixelXY(x, i, CRGB(255, 255, 255));
        }
        else {
          drawPixelXY(x, i, CRGB(255, 0, 0));
        }
        flag_height++;
      }
    }
    x_offset++;
    FastLED.show();
  }
}


uint16_t noise_z = 0;

void noise_update()
{
  FastLED.clear();
  for (byte x = 0; x < 32; x++) {
    for (byte y = 0; y < 16; y++) {
      byte data = inoise8((x * 30) + noise_x_offset, (y * 30) + noise_y_offset, noise_z);
      drawPixelXY(x, y, CRGB(data * (red / 255.), data * (green / 255.), data * (blue / 255.)));
    }
  }
  FastLED.show();
  noise_z += 20;
}


void setup() 
{
  irrecv.enableIRIn();
  Serial.begin(9600);
  randomSeed(analogRead(0) + analogRead(1));    // пинаем генератор случайных чисел

  // настройки ленты
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.clear();
  FastLED.show();

  SPI.begin();
  mfrc522.PCD_Init();
  
  drawwait(6, 5, color);
  FastLED.show();

  EspSerial.begin(ESP8266_BAUD);
  delay(50);
  Blynk.begin(auth, wifi, ssid, pass);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  for (int i = 0; i < 512; i++) {
    snake_tail[i][0] = 255;
    snake_tail[i][1] = 255;
  } 
  
  timer.setInterval(16L, matrix_update);

  pinMode(7, OUTPUT);

  FastLED.clear();
  drawready(3, 5, color);
  FastLED.show();
}


void loop() 
{
  Blynk.run();
  timer.run();
}
