#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// == PINS ==
const uint8_t PIN_CE = 9;
const uint8_t PIN_CSN = 10;

const uint8_t PIN_JOY_X = A0;
const uint8_t PIN_JOY_Y = A1;
const uint8_t PIN_JOY_BTN = 2;
const uint8_t PIN_EXTRA_BTN = 5;

const uint8_t PIN_LED_CONN_GREEN = 3;         // on LED
const uint8_t PIN_LED_CONN_RED = 4;           // off LED
const uint8_t PIN_LED_CONN_YELLOW_ERROR = 6;  // error LED
const uint8_t PIN_ON_OFF_BTN = 7; 

// == RADIO ==
RF24 radio(PIN_CE, PIN_CSN);
const byte address[6] = "ADR01";

// == DATA ==
struct ControlPacket {
  int16_t x;
  int16_t y;
  bool joy_btn;
  bool extra_btn;
};

ControlPacket packet;

// == FUNCTIONS ==
void setupRadio() {
  radio.setAutoAck(true);           // tries again if reciver not confirm
  radio.setRetries(5, 15);          // 1.25ms break, max 15 tries
  radio.setChannel(76); 
  radio.setPALevel(RF24_PA_LOW);    // Power Amplifier
  radio.setDataRate(RF24_250KBPS);  // Speed of transmision
  radio.openWritingPipe(address);   // adress of reciver
  radio.stopListening();            // deactivate listening option
}

void readInputs(ControlPacket &packet) {
  packet.x = analogRead(PIN_JOY_X);
  packet.y = analogRead(PIN_JOY_Y);
  packet.joy_btn = !digitalRead(PIN_JOY_BTN);
  packet.extra_btn = !digitalRead(PIN_EXTRA_BTN);
}

void debugPacket(const ControlPacket &packet) {
  Serial.print(" | X: "); 
  Serial.print(packet.x);

  Serial.print(" | Y: "); 
  Serial.print(packet.y);

  Serial.print(" | JOY BTN: "); 
  Serial.print(packet.joy_btn);
  
  Serial.print(" | EXTRA BTN: "); 
  Serial.println(packet.extra_btn);
}


void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED_CONN_RED, OUTPUT);
  pinMode(PIN_LED_CONN_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW_ERROR, OUTPUT);

  pinMode(PIN_JOY_BTN, INPUT_PULLUP);
  pinMode(PIN_EXTRA_BTN, INPUT_PULLUP);
  pinMode(PIN_ON_OFF_BTN, INPUT_PULLUP);

  digitalWrite(PIN_LED_CONN_RED, HIGH);
  digitalWrite(PIN_LED_CONN_GREEN, LOW);
  digitalWrite(PIN_LED_YELLOW_ERROR, LOW);

  if(!radio.begin()) {
    Serial.println("Radio not responding!");
    while(true){
    }
  }
  setupRadio();

  delay(1500);
  digitalWrite(PIN_LED_CONN_RED, LOW);
  digitalWrite(PIN_LED_CONN_GREEN, HIGH);

  Serial.println("Sender ready...");
}

void loop() {

  bool transmitEnabled = (digitalRead(PIN_ON_OFF_BTN) == LOW);

  if(transmitEnabled){
    readInputs(packet);
    debugPacket(packet); 

    bool success = radio.write(&packet, sizeof(packet));

    if(success) {
      digitalWrite(PIN_LED_CONN_GREEN, HIGH);
      digitalWrite(PIN_LED_CONN_RED, LOW);
      digitalWrite(PIN_LED_YELLOW_ERROR, LOW);

    } else {
      digitalWrite(PIN_LED_YELLOW_ERROR, HIGH);
      digitalWrite(PIN_LED_CONN_RED, LOW);
      digitalWrite(PIN_LED_CONN_GREEN, LOW);
    }
  } else {
    digitalWrite(PIN_LED_CONN_RED, HIGH);
    digitalWrite(PIN_LED_CONN_GREEN, LOW);
    digitalWrite(PIN_LED_YELLOW_ERROR, LOW);
  }
  delay(20);
} 