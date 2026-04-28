#include "ble.h"

const char* SERVICE_UUID = "ee68355b-50d1-46fb-b12f-b3df085be30d";
const char* HISTORY_CHARACTERISTIC_UUID = "833a2d08-44c0-4ce9-a545-bd69b2561f60";
const char* SET_TIME_CHARACTERISTIC_UUID = "928035d4-5f3a-4d57-8168-ab8daf3951ea";

bool ble_device_connected = false;
bool time_initialized = false;
bool is_advertising = false;

void startAdvertising();
void stopAdvertising();
void sendHistory(BLECharacteristic* characteristic_ptr);
bool setTime(BLECharacteristic* characteristic_ptr);

BLECharacteristic* history_characteristic_ptr = nullptr;
BLECharacteristic* set_time_characteristic_ptr = nullptr;
// Descriptor assigned numbers: https://www.bluetooth.com/specifications/assigned-numbers/
BLEDescriptor history_descriptor(BLEUUID((uint16_t)0x2A52));   // "Record Access Control Point" descriptor
BLEDescriptor set_time_descriptor(BLEUUID((uint16_t)0x2A2B));  // "Current Time" descriptor


class ServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* server_ptr) {
    ble_device_connected = true;
    Serial.println("Device connected");
    BLEDevice::getAdvertising()->stop();
  }
  void onDisconnect(BLEServer* server_ptr) {
    Serial.println("Device disconnected");
    ble_device_connected = false;
  }
};

class HistoryCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic_ptr) {
    sendHistory(characteristic_ptr);
  }
};

class SetTimeCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic_ptr) {
    if (setTime(characteristic_ptr)) {
      characteristic_ptr->setValue("OK");
      time_initialized = true;
    }
  }
};


bool setupBle(const char* device_name) {
  time_initialized = false;
  ble_device_connected = false;

  BLEDevice::init(device_name);
  BLEServer *server_ptr = BLEDevice::createServer();
  server_ptr->setCallbacks(new ServerCallbacks());
  BLEService *service_ptr = server_ptr->createService(SERVICE_UUID);

  history_characteristic_ptr = service_ptr->createCharacteristic(HISTORY_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE);
  set_time_characteristic_ptr = service_ptr->createCharacteristic(SET_TIME_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ);

  history_characteristic_ptr->setCallbacks(new HistoryCallbacks());
  history_characteristic_ptr->addDescriptor(&history_descriptor);
  set_time_characteristic_ptr->setCallbacks(new SetTimeCallbacks());
  set_time_characteristic_ptr->addDescriptor(&set_time_descriptor);

  service_ptr->start();
  return true;
}

void startAdvertising() {
  if (!is_advertising) {
    BLEAdvertising* adv = BLEDevice::getAdvertising();
    adv->start();
  }
}

void stopAdvertising() {
  if (!is_advertising) {
    BLEAdvertising* adv = BLEDevice::getAdvertising();
    adv->start();
  }
}

void advertiseMeasurement(SensorData* data) {
  if (ble_device_connected) return;

  uint8_t payload[18];
  int16_t  temp    = (int16_t)(data->temp * 100);
  uint16_t hum     = (uint16_t)(data->humidity * 100);
  uint16_t pres    = (uint16_t)(data->pressure * 10);
  uint16_t iaq_    = (uint16_t)(data->iaq);
  uint16_t siaq    = (uint16_t)(data->static_iaq);
  uint16_t co2_    = (uint16_t)(data->co2);
  uint16_t voc_    = (uint16_t)(data->voc * 100);
  uint32_t ts      = time_initialized ? (uint32_t)time(nullptr) : 0;
  memcpy(payload + 0,  &temp,    2);
  memcpy(payload + 2,  &hum,     2);
  memcpy(payload + 4,  &pres,    2);
  memcpy(payload + 6,  &iaq_,    2);
  memcpy(payload + 8,  &siaq,    2);
  memcpy(payload + 10, &co2_,    2);
  memcpy(payload + 12, &voc_,    2);
  memcpy(payload + 14, &ts, 4);

  String mfr;
  mfr += (char)0xFF;
  mfr += (char)0xFF;
  mfr += String((char*)payload, 18);

  BLEAdvertising* adv = BLEDevice::getAdvertising();
  BLEAdvertisementData scan_response;
  scan_response.setManufacturerData(mfr);
  adv->setScanResponse(true);
  adv->setScanResponseData(scan_response);
  startAdvertising();
  delay(2000);
  stopAdvertising();
}

void sendHistory(BLECharacteristic* characteristic_ptr) {
  int count = getHistoryCount();
  characteristic_ptr->setValue((uint8_t*)&count, sizeof(count));
  characteristic_ptr->notify();
  delay(20);
  for (int i = 0; i < count; i++) {
    HistoricalDataPoint dp = getHistoricalDataPoint(i);
    if (dp.timestamp == 0) continue;
    characteristic_ptr->setValue((uint8_t*)&dp, sizeof(HistoricalDataPoint));
    characteristic_ptr->notify();
    delay(20);
  }
}

bool setTime(BLECharacteristic* characteristic_ptr) {
  String value = characteristic_ptr->getValue();

  time_t set_time = (time_t)atoll(value.c_str());
  if (set_time <= 0) {
    Serial.println("Set time rejected: invalid timestamp");
    return false;
  }

  struct timeval tv = { set_time, 0 };
  settimeofday(&tv, nullptr);
  Serial.print("System time set to: ");
  Serial.println((long long)set_time);

  return true;
}