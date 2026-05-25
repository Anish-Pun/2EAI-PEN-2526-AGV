#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;

#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID "abcdef01-1234-5678-1234-56789abcdef0"

// Callback: alles printen wat client stuurt
class EchoCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pChar) override {
    String value = pChar->getValue().c_str();
    value.trim(); // whitespace verwijderen

    Serial.print("Ontvangen: ");
    for (unsigned int i = 0; i < value.length(); i++) {
      Serial.print(value[i]);
    }
    Serial.println();
  }
};

// Server connect / disconnect callbacks
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    Serial.println("Client verbonden (MAC niet beschikbaar in deze library)");
  }

  void onDisconnect(BLEServer* pServer) override {
    Serial.println("Client losgekoppeld");
    BLEDevice::startAdvertising(); // opnieuw adverteren
  }
};

void setup() {
  Serial.begin(115200);

  BLEDevice::init("ESP32_BLE_ECHO_SERVER");
  Serial.print("ESP32 BLE MAC Address: ");
  Serial.println(BLEDevice::getAddress().toString().c_str());

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
  );

  pCharacteristic->setCallbacks(new EchoCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  Serial.println("BLE advertising gestart");
}

void loop() {
  delay(1000);
}