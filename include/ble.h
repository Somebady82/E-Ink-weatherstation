#ifndef BLE_H_
#define BLE_H_
 
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <sstream>

    typedef struct {
    float temperature;
    float humidity;
    float battery_voltage;
    uint8_t battery_level;
    bool found;
    } ble_sens_struct;

    extern ble_sens_struct ble_sens[];

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
{
public:
    uint8_t* findServiceData(uint8_t* data, size_t length, uint8_t* foundBlockLength);
	void onResult(BLEAdvertisedDevice advertisedDevice);

private:    
};
 
#endif /* BLE_H_ */