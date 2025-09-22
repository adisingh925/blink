#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLESecurity.h> /** Include this for BLE security **/

#define LED_PIN 2 /** Onboard LED **/
#define BLE_NAME "ESP32_BLE_LED"
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define SERVER_PASSKEY 123456 /** 6-digit BLE PIN **/

BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr; /** Make global to send data in loop **/

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    Serial.println("BLE Device connected");
  }
  void onDisconnect(BLEServer *pServer) {
    Serial.println("BLE Device disconnected");
  }
};

/** Function to send a float value over BLE **/
void sendFloat(float value) {
  uint8_t floatBytes[4];
  memcpy(floatBytes, &value, sizeof(float));
  pCharacteristic->setValue(floatBytes, sizeof(floatBytes));
  pCharacteristic->notify();
  Serial.print("Sent float: ");
  Serial.println(value);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);

  /** BLE setup **/
  BLEDevice::init(BLE_NAME);

  /** Security setup **/
  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND); /** Require pairing **/
  pSecurity->setCapability(ESP_IO_CAP_OUT); /** Device displays PIN **/
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK); /** Encryption **/
  pSecurity->setStaticPIN(SERVER_PASSKEY); /** Set your PIN **/

  /** Create server **/
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );

  /** Initial value (0.0) **/
  float initValue = 0.0;
  uint8_t initBytes[4];
  memcpy(initBytes, &initValue, sizeof(initValue));
  pCharacteristic->setValue(initBytes, sizeof(initBytes));

  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("BLE Server is running...");
}

void loop() {
  /** Blink LED **/
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);

  /** Example: send a float value **/
  /** Option 1: Incrementing test value **/
  static float testValue = 0.0;
  testValue += 0.1; /** Increment each loop **/
  if (testValue > 100.0) testValue = 0.0; /** Reset after 100 **/
  sendFloat(testValue);

  /** Option 2: Read from a sensor (uncomment if using an actual sensor) **/
  // float sensorValue = analogRead(34) * (3.3 / 4095.0); /** Example: voltage on pin 34 **/
  // sendFloat(sensorValue);

  delay(500); /** Adjust update rate as needed **/
}

