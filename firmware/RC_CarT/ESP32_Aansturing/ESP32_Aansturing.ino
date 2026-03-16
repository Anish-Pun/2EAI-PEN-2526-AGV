



#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// PWM pins
const int motor1Pin = 25;
const int motor2Pin = 26;

// PWM waarden
const int stopValue = 128;
const int forwardSpeed = stopValue + 63;
const int reverseSpeed = stopValue - 63;

BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;

#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID "abcdef01-1234-5678-1234-56789abcdef0"

volatile int motor1Value = stopValue;
volatile int motor2Value = stopValue;

// Callback: wanneer client iets schrijft naar characteristic
class MotorCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pChar) override {
    String value = pChar->getValue().c_str();
    int spaceIndex = value.indexOf(' ');
    if (spaceIndex > 0) {
      int m1 = value.substring(0, spaceIndex).toInt();
      int m2 = value.substring(spaceIndex + 1).toInt();
      motor1Value = constrain(m1, 0, 255);
      motor2Value = constrain(m2, 0, 255);
    }
  }
};

// Callback: wanneer client connect / disconnect
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    Serial.println("Client verbonden! Stuur welkom bericht...");
    pCharacteristic->setValue("Hallo, ik ben een ESP");
    pCharacteristic->notify();
  }
  void onDisconnect(BLEServer* pServer) override {
    Serial.println("Client losgekoppeld");
  }
};

void setup() {
  Serial.begin(115200);

  // PWM setup
  pinMode(motor1Pin, OUTPUT);
  pinMode(motor2Pin, OUTPUT);
  analogWrite(motor1Pin, stopValue);
  analogWrite(motor2Pin, stopValue);

  Serial.println("ESP32 BLE MD22 ready to receive");

  // BLE server opzetten
  BLEDevice::init("ESP32_RC_CarT");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->setCallbacks(new MotorCallbacks());
  pCharacteristic->addDescriptor(new BLE2902()); // nodig voor notify

  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
}

void loop() {
  // Realtime PWM
  analogWrite(motor1Pin, motor1Value);
  analogWrite(motor2Pin, motor2Value);
  delay(10);
}