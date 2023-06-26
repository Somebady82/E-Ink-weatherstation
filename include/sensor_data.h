#ifndef SENSORDATA_H
#define SENSORDATA_H

/**
 * A structure of all the sensor data.
 */
struct SensorData {
  public:
    /**
     * The temperature in degrees celsius.
     */
    float room_temp = 0.0f;

    /**
     * The humidity in percentage.
     */
    float room_humidity = 0.0f;

    /**
     * The pressure in Pascal.
     */
    float room_pressure = 0.0f;

    /**
     * The temperature in degrees celsius.
     */
    float outside_temp = 0.0f;

    /**
     * The humidity in percentage.
     */
    float outside_humidity = 0.0f;

};

#endif