#include <SPI.h>

#define LEDC_CHANNEL_0 0 //channel max 15
#define LEDC_TIMER_BIT 8 //max 16bit
//8bitの場合、最大周波数 312500Hz
#define LEDC_BASE_FREQ 80000.0
#define GPIO_PIN 33 //GPIO #36～#39 は設定不可

/*
 * HSPI
 * SCLK #14
 * MOSI #13: DATA
 * MISO #12
 * CS(SS) #15
 */
SPIClass hspi(HSPI);

/*
 * VSPI
 * SCLK #18
 * MOSI #23: DATA
 * MISO #19
 * CS(SS) #5
 */
SPIClass vspi(VSPI);

volatile int timeCounter1;
hw_timer_t *timer1 = NULL;    // For measurement
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

byte b=0b10100110;
byte b_array[8];
static bool is_gpio = LOW;

void IRAM_ATTR onTimer1(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  timeCounter1++;
  digitalWrite(32, is_gpio);
  is_gpio = !is_gpio;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
  Serial.begin(921600);

  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
  ledcAttachPin(GPIO_PIN, LEDC_CHANNEL_0);
  ledcWrite(LEDC_CHANNEL_0, 0x80); //0x80 = 128

  // pinMode(15, OUTPUT);
  // digitalWrite(15, HIGH);
  hspi.begin(14, 13, 12, 15);
  // hspi.beginTransaction(SPISettings(7000000, SPI_MSBFIRST, SPI_MODE0));
  hspi.setFrequency(7000000); //SSD1331 のSPI Clock Cycle Time 最低150ns
  hspi.setDataMode(SPI_MODE0);
  hspi.setHwCs(true);

  // pinMode(5, OUTPUT);
  // digitalWrite(5, HIGH);
  vspi.begin(18, 23, 19, 5);
  vspi.setFrequency(5000000); //SSD1331 のSPI Clock Cycle Time 最低150ns
  vspi.setDataMode(SPI_MODE0);
  vspi.setHwCs(true);

  for(int i=0; i<8; i++){
    b_array[i] = b;  
  }
  pinMode(32, OUTPUT);

  timer1 = timerBegin(0, 80, true);

  // Attach onTimer function.
  timerAttachInterrupt(timer1, &onTimer1, true);

  // Set alarm to call onTimer function every second (value in microseconds).
  timerAlarmWrite(timer1, 1000, true);

  // Start an alarm
  timerAlarmEnable(timer1);
}

void loop() {
  // digitalWrite(32, HIGH);
  // Serial.println("test");
  // digitalWrite(15, LOW);
  uint8_t hspi_ret = hspi.transfer(b);
  // hspi.transfer(b);
  // hspi.writeBytes(b_array, 8);
  // digitalWrite(15, HIGH);

  // digitalWrite(5, LOW);
  uint8_t vspi_ret = vspi.transfer(b);
  // vspi.writeBytes(b_array, 8);
  // digitalWrite(5, HIGH);
  delay(500);
  // digitalWrite(32, LOW);
  delay(500);
}
