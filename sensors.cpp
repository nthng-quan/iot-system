#include "sensors.h"

DHT dht(DHTPIN, DHTTYPE);
MQ135 mq135_sensor(MQ135_PIN);

bool compare(const SensorData& data1, const SensorData& data2, const String& scope) {
    if (scope == "dht") {
        return (
            data1.temperature != data2.temperature ||
            data1.humidity != data2.humidity
        );
    } else if (scope == "mq135") {
        return (
            data1.mq135_rzero != data2.mq135_rzero ||
            data1.corrected_rzero != data2.corrected_rzero ||
            data1.resistance != data2.resistance ||
            data1.ppm != data2.ppm ||
            data1.corrected_ppm != data2.corrected_ppm
        );
    }
    return (
        data1.temperature != data2.temperature ||
        data1.humidity != data2.humidity ||
        data1.IR_value != data2.IR_value ||
        data1.mq135_rzero != data2.mq135_rzero ||
        data1.corrected_rzero != data2.corrected_rzero ||
        data1.resistance != data2.resistance ||
        data1.ppm != data2.ppm ||
        data1.corrected_ppm != data2.corrected_ppm
    );
}

void copy(SensorData& dest, const SensorData& src) {
    dest.temperature = src.temperature;
    dest.humidity = src.humidity;
    dest.IR_value = src.IR_value;
    dest.mq135_rzero = src.mq135_rzero;
    dest.corrected_rzero = src.corrected_rzero;
    dest.resistance = src.resistance;
    dest.ppm = src.ppm;
    dest.corrected_ppm = src.corrected_ppm;
}

bool compare(Config& config1, Config& config2, const String& scope) {
    if (scope == "servo") {
        return (
            config1.servo_base != config2.servo_base ||
            config1.servo_neck != config2.servo_neck
        );
    }
    return (
        config1.servo_base != config2.servo_base ||
        config1.servo_neck != config2.servo_neck ||
        config1.ppm_thrsh != config2.ppm_thrsh ||
        config1.rzero_thrsh != config2.rzero_thrsh ||
        config1.temp_thrsh != config2.temp_thrsh ||
        config1.hum_thrsh != config2.hum_thrsh
    );
}

void copy(Config& dest, const Config& src) {
    dest.servo_base = src.servo_base;
    dest.servo_neck = src.servo_neck;
    dest.ppm_thrsh = src.ppm_thrsh;
    dest.rzero_thrsh = src.rzero_thrsh;
    dest.temp_thrsh = src.temp_thrsh;
    dest.hum_thrsh = src.hum_thrsh;
}

SensorData readSensorData() {
    SensorData data;

    // DHT11
    data.humidity = dht.readHumidity();
    data.temperature = dht.readTemperature();
    if (isnan(data.humidity) || isnan(data.temperature) || data.humidity > 110 || data.temperature > 110) {
        Serial.println("Failed to read from DHT!");
        return { -1, -1, -1, -1, -1, -1, -1, -1};
    }

    // IR sensor
    data.IR_value = digitalRead(IR_PIN);

    // MQ-135
    data.mq135_rzero = mq135_sensor.getRZero();
    data.corrected_rzero = mq135_sensor.getCorrectedRZero(data.temperature, data.humidity);
    data.resistance = mq135_sensor.getResistance();
    data.ppm = mq135_sensor.getPPM();
    data.corrected_ppm = mq135_sensor.getCorrectedPPM(data.temperature, data.humidity);

    return data;
}

void initSensors() {
    pinMode(IR_PIN, INPUT);
    dht.begin();
}


bool check_anomaly(const SensorData& data, const Config& config) { // more verbose
    if (data.IR_value == 0) {
        Serial.println("*** IR sensor value is 0");
        return true;
    }
    if (data.temperature > config.temp_thrsh) {
        Serial.println("*** Temperature is higher than threshold");
        return true;
    }
    if (data.humidity < config.hum_thrsh) {
        Serial.println("*** Humidity is lower than threshold");
        return true;
    }
    if (data.corrected_rzero < config.rzero_thrsh) {
        Serial.println("*** Corrected RZero is lower than threshold");
        return true;
    }
    if (data.corrected_ppm > config.ppm_thrsh) {
        Serial.println("*** Corrected PPM is higher than threshold");
        return true;
    }

    return false;
}

void printSensorData(const SensorData& data) {
    Serial.print("* IR sensor value: ");
    Serial.println(data.IR_value);

    Serial.println("--- DHT11 ---");
    Serial.print("- Temperature: ");
    Serial.println(data.temperature);
    Serial.print("- Humidity: ");
    Serial.println(data.humidity);

    Serial.println("--- MQ135 ---");
    Serial.print("- MQ135 RZero: ");
    Serial.println(data.mq135_rzero);
    Serial.print("- Corrected RZero: ");
    Serial.println(data.corrected_rzero);
    Serial.print("- Resistance: ");
    Serial.println(data.resistance);
    Serial.print("- PPM: ");
    Serial.print(data.ppm);
    Serial.println("ppm");
    Serial.print("- Corrected PPM: ");
    Serial.print(data.corrected_ppm);
    Serial.println("ppm");
    Serial.println("-------------");
}