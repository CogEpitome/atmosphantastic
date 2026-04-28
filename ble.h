#pragma once

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEAdvertising.h>
#include "data.h"
#include "history.h"

extern bool ble_device_connected;

bool setupBle(const char* device_name);
void startAdvertising();
void stopAdvertising();
void advertiseMeasurement(SensorData* data);