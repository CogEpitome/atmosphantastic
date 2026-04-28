#pragma once

#include <stdint.h>

typedef struct {
  uint32_t timestamp = 0;
  float temp = 0;
  float humidity = 0;
  float pressure = 0;
  float iaq = 0;
  float static_iaq = 0;
  float co2 = 0;
  float voc = 0;
} SensorData;

// 20 bytes
// 168 data points -> 3360 bytes for one week history.
typedef struct {
  uint32_t timestamp = 0;
  float temp = 0;               
  float pressure = 0;           
  float static_iaq = 0;         
} HistoricalDataPoint;