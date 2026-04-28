#include <bsec2.h>
#include <SPI.h>
#include "data.h"

#define SAMPLE_RATE BSEC_SAMPLE_RATE_ULP

extern SensorData measurement;
extern bool measurement_ready;

void setupSensor();
bool sense();
