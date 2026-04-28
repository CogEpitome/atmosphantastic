#include "sensor.h"

const int8_t BME_SCK  = 0;
const int8_t BME_MISO = 2;
const int8_t BME_MOSI = 1;
const int8_t BME_CS   = 3;

Bsec2 envSensor;
SensorData measurement;
bool measurement_ready = false;

void checkBsecStatus(Bsec2 bsec);
void newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec);

void setupSensor() {
    measurement = {0};
    SPI.begin(BME_SCK, BME_MISO, BME_MOSI, BME_CS);

    bsecSensor sensorList[] = {
        BSEC_OUTPUT_IAQ,
        BSEC_OUTPUT_STATIC_IAQ,
        BSEC_OUTPUT_CO2_EQUIVALENT,
        BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
        BSEC_OUTPUT_RAW_TEMPERATURE,
        BSEC_OUTPUT_RAW_PRESSURE,
        BSEC_OUTPUT_RAW_HUMIDITY,
        BSEC_OUTPUT_RAW_GAS,
        BSEC_OUTPUT_STABILIZATION_STATUS,
        BSEC_OUTPUT_RUN_IN_STATUS,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
        BSEC_OUTPUT_GAS_PERCENTAGE,
        BSEC_OUTPUT_COMPENSATED_GAS
    };

    if (!envSensor.begin(BME_CS, SPI))
        checkBsecStatus(envSensor);

    if (SAMPLE_RATE == BSEC_SAMPLE_RATE_ULP)
        envSensor.setTemperatureOffset(TEMP_OFFSET_ULP);
    else if (SAMPLE_RATE == BSEC_SAMPLE_RATE_LP)
        envSensor.setTemperatureOffset(TEMP_OFFSET_LP);

    if (!envSensor.updateSubscription(sensorList, ARRAY_LEN(sensorList), SAMPLE_RATE))
        checkBsecStatus(envSensor);

    envSensor.attachCallback(newDataCallback);

    Serial.println("BSEC library version " +
        String(envSensor.version.major) + "." +
        String(envSensor.version.minor) + "." +
        String(envSensor.version.major_bugfix) + "." +
        String(envSensor.version.minor_bugfix));
}

bool sense() {
  bool ran = envSensor.run();
  if (!ran) { checkBsecStatus(envSensor); }
  return ran;
}

void newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec) {
    if (!outputs.nOutputs) return;

    for (uint8_t i = 0; i < outputs.nOutputs; i++) {
        const bsecData output = outputs.output[i];
        switch (output.sensor_id) {
            case BSEC_OUTPUT_IAQ:
                measurement.iaq = output.signal;
                Serial.println("\tIAQ = " + String(output.signal));
                Serial.println("\tIAQ accuracy = " + String((int)output.accuracy));
                break;
            case BSEC_OUTPUT_STATIC_IAQ:
                measurement.static_iaq = output.signal;
                Serial.println("\tStatic IAQ = " + String(output.signal));
                break;
            case BSEC_OUTPUT_CO2_EQUIVALENT:
                measurement.co2 = output.signal;
                Serial.println("\tCO2 equivalent = " + String(output.signal));
                break;
            case BSEC_OUTPUT_BREATH_VOC_EQUIVALENT:
                measurement.voc = output.signal;
                Serial.println("\tbVOC equivalent = " + String(output.signal));
                break;
            case BSEC_OUTPUT_RAW_PRESSURE:
                measurement.pressure = output.signal / 10.0f;
                Serial.println("\tPressure = " + String(measurement.pressure));
                break;
            case BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE:
                measurement.temp = output.signal;
                Serial.println("\tCompensated temperature = " + String(output.signal));
                break;
            case BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY:
                measurement.humidity = output.signal;
                Serial.println("\tCompensated humidity = " + String(output.signal));
                break;
            default:
                break;
        }
    }

    time_t t = time(nullptr);
    measurement.timestamp = (uint32_t)t;
    measurement_ready = true;
}

void checkBsecStatus(Bsec2 bsec) {
    if (bsec.status < BSEC_OK) {
      Serial.println("BSEC error code : " + String(bsec.status));
      delay(1000);
      esp_restart();
    } else if (bsec.status > BSEC_OK && bsec.status != 14)
      Serial.println("BSEC warning code : " + String(bsec.status));
    if (bsec.sensor.status < BME68X_OK) {
      Serial.println("BME68X error code : " + String(bsec.sensor.status));
      delay(1000);
      esp_restart();
    } else if (bsec.sensor.status > BME68X_OK)
      Serial.println("BME68X warning code : " + String(bsec.sensor.status));
}