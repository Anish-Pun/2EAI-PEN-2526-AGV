#include <PS4Controller.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *characteristic;

#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcdefab-1234-1234-1234-abcdefabcdef"

bool deviceConnected = false;

/* connection callbacks */
class MyServerCallbacks: public BLEServerCallbacks
{
  void onConnect(BLEServer* pServer)
  {
    deviceConnected = true;
    Serial.println("PSoC connected");
  }

  void onDisconnect(BLEServer* pServer)
  {
    deviceConnected = false;
    Serial.println("PSoC disconnected");
  }
};

void setup()
{
  Serial.begin(115200);

  /* start PS4 controller */
  PS4.begin();
  Serial.println("Waiting for PS4 controller...");

  /* start BLE */
  BLEDevice::init("ESP32_AGV");

  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new MyServerCallbacks());

  BLEService *service = server->createService(SERVICE_UUID);

  characteristic = service->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY
  );

  characteristic->addDescriptor(new BLE2902());

  service->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->start();

  Serial.println("BLE advertising started");
}

void loop()
{
  if(PS4.isConnected())
  {
    int steer = PS4.LStickX();
    int speed = -PS4.LStickY();

    char data[32];
    sprintf(data,"%d,%d",speed,steer);

    Serial.println(data);

    if(deviceConnected)
    {
      characteristic->setValue(data);
      characteristic->notify();
    }

    delay(50);
  }
}