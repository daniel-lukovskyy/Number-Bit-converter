#include <IRremote.hpp>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <map>

// remote pins
#define IR_PIN 14
// display pins
#define OLED_SDA 21
#define OLED_SCL 22
// 74HC595 pins
#define SER 23 // data
#define RCLK 18 // latch
#define SRCLK 19 // clock
// display prop
#define W 128
#define H 64
Adafruit_SSD1306 display(W, H, &Wire, -1);

enum class ButtonTyp{
  Digit,
  Ok,
  Clear,
  Mode,
  OnOf,
};

struct Button{
  ButtonTyp type;
  uint8_t but_val;
};

std::map<uint8_t, Button> irMap;

void write_module(uint8_t val) {
  digitalWrite(RCLK, LOW);
  shiftOut(SER, SRCLK, LSBFIRST, val);
  digitalWrite(RCLK, HIGH);
}

uint16_t val_show = 0;

void txt_show(const char* line1, const char* line2){
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(line1);

  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print(line2);

  display.display();
}

void setup() {
  // 74HC595
  pinMode(RCLK, OUTPUT);
  pinMode(SRCLK, OUTPUT);
  pinMode(SER, OUTPUT);

  // remote
  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);

  irMap[22] = {ButtonTyp::Digit, 0};
  irMap[12] = {ButtonTyp::Digit, 1};
  irMap[24] = {ButtonTyp::Digit, 2};
  irMap[94] = {ButtonTyp::Digit, 3};
  irMap[8] = {ButtonTyp::Digit, 4};
  irMap[28] = {ButtonTyp::Digit, 5};
  irMap[90] = {ButtonTyp::Digit, 6};
  irMap[66] = {ButtonTyp::Digit, 7};
  irMap[82] = {ButtonTyp::Digit, 8};
  irMap[74] = {ButtonTyp::Digit, 9};

  irMap[13] = {ButtonTyp::Clear, 0};
  irMap[25] = {ButtonTyp::Ok, 0};
  irMap[71] = {ButtonTyp::Mode, 0};
  irMap[69] = {ButtonTyp::OnOf, 0};

  // display
  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    while(true){
      Serial.begin(115200);
      Serial.println("Display bug");
    }
  }
  txt_show("Ready", "Press OK");
  write_module(0b00000001); // check to delete after
}

void loop() {
  if (IrReceiver.decode()){
    uint8_t command = IrReceiver.decodedIRData.command;

    auto it = irMap.find(command);
    if (it == irMap.end()){
      IrReceiver.resume();
      return;
    }

    Button btn = it->second;

    switch(btn.type){

      case ButtonTyp::Digit:
        // display text
        
        val_show = val_show*10 + btn.but_val;
        if (val_show <= 255) {
          char buf[32];
          snprintf(buf, sizeof(buf), "%u", val_show);
          txt_show("Number: ", buf);
        }
        else {
          txt_show("Error: ", "Input must be smaller 255");
        }

        break;

      case ButtonTyp::Clear:
        // clear
        val_show = 0;
        txt_show("Digit", "None");
        break;

      case ButtonTyp::Ok:
        //  enter number
        write_module((uint8_t)val_show);
        break;

      case ButtonTyp::Mode:
        // mode 
        break;

      case ButtonTyp::OnOf:
        // on of
        break;  
    }

    IrReceiver.resume();
  }
}
