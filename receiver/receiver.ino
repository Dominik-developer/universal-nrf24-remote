#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// == PINS ==
const uint8_t PIN_CE = 9;
const uint8_t PIN_CSN = 10;

const uint8_t  PIN_LED_RIGHT = 4;   // GREEN_PIN
const uint8_t  PIN_LED_UP = 5;      // BLUE_PIN 5
const uint8_t  PIN_LED_DOWN = 6;    // YELLOW_PIN 6
const uint8_t  PIN_LED_LEFT = 7;    // RED_PIN 7

const uint8_t  PIN_LED_CONN_RED = 2;            // off LED
const uint8_t  PIN_LED_CONN_GREEN = 3;          // on LED
const uint8_t  PIN_LED_CONN_YELLOW_ERROR = 8;   // error LED

const uint8_t  PIN_ON_OFF_BTN = A0; 
const uint8_t  PIN_LASER = A1;
const uint8_t  PIN_BUZZER = A2;

// == RADIO ==
RF24 radio( PIN_CE,  PIN_CSN);
const byte address[6] = "ADR01";

// == DATA ==
struct ControlPacket {
    int16_t x;
    int16_t y;
    bool btn;
    bool extra_btn;
};

ControlPacket packet;

// == GLOBALS ==
unsigned long lastPacketTime = 0;
const unsigned long connectionTimeout = 100;    // in milliseconds (0.1 second) (300ms is better for real use)

const int JOYSTICK_THRESHOLD_HIGH = 700;
const int JOYSTICK_THRESHOLD_LOW = 300;

// == FUNCTIONS ==
void setupRadio() {
    radio.setAutoAck(true);
    radio.setChannel(76);
    radio.setPALevel(RF24_PA_LOW);  
    radio.setDataRate(RF24_250KBPS);
    radio.openReadingPipe(0, address);
    radio.startListening();
}

void debugPacket(const ControlPacket &packet) {
  Serial.print(" | X: "); 
  Serial.print(packet.x);

  Serial.print(" | Y: "); 
  Serial.print(packet.y);

  Serial.print(" | JOY BTN: "); 
  Serial.print(packet.btn);

  Serial.print(" | EXTRA BTN: "); 
  Serial.println(packet.extra_btn);
}

void updateHorizontalLEDs(const int x) {
    if(x > JOYSTICK_THRESHOLD_HIGH) {
        digitalWrite(PIN_LED_RIGHT, HIGH);
        digitalWrite(PIN_LED_LEFT, LOW);
    } else if(x < JOYSTICK_THRESHOLD_LOW) {
        digitalWrite(PIN_LED_LEFT, HIGH);
        digitalWrite(PIN_LED_RIGHT, LOW);
    } else {
        digitalWrite(PIN_LED_LEFT, LOW);
        digitalWrite(PIN_LED_RIGHT, LOW);
    }
}

void updateVerticalLEDs(const int y) {
    if(y > JOYSTICK_THRESHOLD_HIGH) {
        digitalWrite(PIN_LED_UP, HIGH);
        digitalWrite(PIN_LED_DOWN, LOW);
    } else if(y < JOYSTICK_THRESHOLD_LOW) {
        digitalWrite(PIN_LED_DOWN, HIGH);
        digitalWrite(PIN_LED_UP, LOW);
    } else {
        digitalWrite(PIN_LED_UP, LOW);
        digitalWrite(PIN_LED_DOWN, LOW);
    }
}

void extraBtnControl(const bool state) {
    if(state) {
            digitalWrite(PIN_LASER, HIGH);
    } else {
        digitalWrite(PIN_LASER, LOW);
    }
}

void joyBtnControl(const bool state) {
    if(state) {
        digitalWrite(PIN_BUZZER, HIGH);
    } else {
        digitalWrite(PIN_BUZZER, LOW);
    }
}


void setup() {
    Serial.begin(115200);

    pinMode(PIN_LED_RIGHT, OUTPUT);
    pinMode(PIN_LED_UP, OUTPUT);
    pinMode(PIN_LED_DOWN, OUTPUT);
    pinMode(PIN_LED_LEFT, OUTPUT);

    pinMode(PIN_LED_CONN_RED, OUTPUT);
    pinMode(PIN_LED_CONN_GREEN, OUTPUT);
    pinMode(PIN_LED_CONN_YELLOW_ERROR, OUTPUT);

    pinMode(PIN_ON_OFF_BTN, INPUT_PULLUP);

    pinMode(PIN_LASER, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);


    digitalWrite(PIN_LED_CONN_RED, HIGH);
    digitalWrite(PIN_LED_CONN_GREEN, LOW);
    digitalWrite(PIN_LED_CONN_YELLOW_ERROR, LOW);

    digitalWrite(PIN_LASER, LOW);
    digitalWrite(PIN_BUZZER, LOW);

    if (!radio.begin()) {
        Serial.println("Radio not responding!");
        while(true){
        }
    }

    setupRadio();

    Serial.println("Receiver ready...");
}

void loop() {
    bool receiverOn = !digitalRead(PIN_ON_OFF_BTN);

    if(receiverOn) { 
        if(radio.available()) { // on
            radio.read(&packet, sizeof(packet));
            lastPacketTime = millis();

            digitalWrite(PIN_LED_CONN_GREEN, HIGH);
            digitalWrite(PIN_LED_CONN_RED, LOW);

            updateHorizontalLEDs(packet.x);

            updateVerticalLEDs(packet.y);

            debugPacket(packet);

            // LASER CONTROL (extra button)
            extraBtnControl(packet.extra_btn); // LASER

            // BUZZER CONTROL (joystick button)
            joyBtnControl(packet.btn); // BUZZER
        }

        if(millis() - lastPacketTime > connectionTimeout){
            digitalWrite(PIN_LED_CONN_YELLOW_ERROR, HIGH);
            digitalWrite(PIN_LED_CONN_GREEN, LOW);

            digitalWrite(PIN_LED_LEFT, LOW);
            digitalWrite(PIN_LED_RIGHT, LOW);
            digitalWrite(PIN_LED_UP, LOW);
            digitalWrite(PIN_LED_DOWN, LOW);

            digitalWrite(PIN_LASER, LOW);
            digitalWrite(PIN_BUZZER, LOW);
        } else {
            digitalWrite(PIN_LED_CONN_YELLOW_ERROR, LOW);
        }
        digitalWrite(PIN_LED_CONN_RED, LOW);

    } else { // off
        digitalWrite(PIN_LED_CONN_RED, HIGH);
        digitalWrite(PIN_LED_CONN_GREEN, LOW);
        digitalWrite(PIN_LED_CONN_YELLOW_ERROR, LOW);

        digitalWrite(PIN_LED_LEFT, LOW);
        digitalWrite(PIN_LED_RIGHT, LOW);
        digitalWrite(PIN_LED_UP, LOW);
        digitalWrite(PIN_LED_DOWN, LOW);

        digitalWrite(PIN_LASER, LOW);
    }
    delay(20);
}
