#include <Arduino.h>
#include "ble.h"

#define NUM_SENS 1

    ble_sens_struct ble_sens[NUM_SENS];

typedef struct __attribute__((__packed__))
{
    int16_t   temperature;
    int16_t   humidity;
    uint16_t  battery_voltage;
    uint8_t   battery_level;
    uint32_t  uptime;
    uint8_t   spare[5];
} ble_sens_payload_t;

    // float ble_sens_temp[NUM_SENS];
    // float ble_sens_humidity[NUM_SENS];
    // float ble_sens_battery[NUM_SENS];
    // bool ble_sens_found[NUM_SENS] = {false};

String knownBLEAddresses[NUM_SENS] = {"84:2e:14:31:bb:5c"};

    uint8_t* MyAdvertisedDeviceCallbacks::findServiceData(uint8_t* data, size_t length, uint8_t* foundBlockLength) {
        // пэйлоад у состоит из блоков [байт длины][тип блока][данные]
        // нам нужен блок с типом 0x16, следом за которым будет 0x95 0xfe
        // поэтому считываем длину, проверяем следующий байт и если что пропускаем
        // вернуть надо указатель на нужный блок и длину блока
        uint8_t* rightBorder = data + length;
        while (data < rightBorder) {
            uint8_t blockLength = *data;
            if (blockLength < 5) { // нам точно такие блоки не нужны
                data += (blockLength+1);
                continue;
            }
            uint8_t blockType = *(data+1);
            uint16_t serviceType = *(uint16_t*)(data + 2);
            log_i("\nblockType: %u   serviceType: %u \n", blockType, serviceType);
            if (blockType == 0xff && serviceType == 0x55aa) { // мы нашли что искали
                *foundBlockLength = blockLength-3; // вычитаем длину типа сервиса
                return data+4; // пропускаем длину и тип сервиса
            }
            data += (blockLength+1);
        }   
        return nullptr;
    }

    void MyAdvertisedDeviceCallbacks::onResult(BLEAdvertisedDevice advertisedDevice) {
        int8_t sens_num = -1;
        // Тут нужно по хорошему проверить mac-адрес устройства    
//        for (int i = 0; i < (sizeof(knownBLEAddresses) / sizeof(knownBLEAddresses[0])); i++) {
        for (int i = 0; i < NUM_SENS; i++) {
            if (strcmp(advertisedDevice.getAddress().toString().c_str(), knownBLEAddresses[i].c_str()) != 0) {
                return; // Не совпадает mac-адрес
            } else {
                sens_num = i;
            }
        }
//        if (!advertisedDevice.haveName() || advertisedDevice.getName().compare("MJ_HT_V1"))
//            return; // нас интересуют только устройства, которые транслируют нужное нам имя

        uint8_t* payload = advertisedDevice.getPayload();
        size_t payloadLength = advertisedDevice.getPayloadLength();
        log_i("\nSENS Advertised Device: %s\n", advertisedDevice.toString().c_str());
        uint8_t serviceDataLength=0;
        uint8_t* serviceData = findServiceData(payload, payloadLength, &serviceDataLength);

        if (serviceData == nullptr) {
            return; // нам этот пакет больше не интересен
        }

        ble_sens_payload_t* ble_sens_payload;
        ble_sens_payload = (ble_sens_payload_t*)serviceData;

        log_i("Found service data len: %d\n", serviceDataLength);

/* Format:
 *  packetId = AA55
 *  payload =
 *        2-bytes temperature
 *        2-bytes relativeHumidity
 *        2-bytes voltage
 *        4-bytes uptime
 *        2-bytes spare
 *        4-bytes magic field AA55FF00 (must decrypt correctly = MAC)
 */     
        if (sens_num >= 0) {
            // ble_sens[sens_num].temperature = ((*(uint8_t*)(serviceData) << 8) + (*(uint8_t*)(serviceData + 1))) / 100.0;
            // ble_sens[sens_num].humidity = ((*(uint8_t*)(serviceData + 2) << 8) + (*(uint8_t*)(serviceData + 3))) / 100.0;
            // ble_sens[sens_num].battery =  ((*(uint8_t*)(serviceData + 4) << 8) + (*(uint8_t*)(serviceData + 5))) / 100.0;
            // ble_sens[sens_num].battery_level =  *(uint8_t*)(serviceData + 6) ;
            ble_sens[sens_num].temperature = ble_sens_payload->temperature / 100.0;
            ble_sens[sens_num].humidity = ble_sens_payload->humidity / 100.0;
            ble_sens[sens_num].battery_voltage =  ble_sens_payload->battery_voltage / 100.0;
            ble_sens[sens_num].battery_level =  ble_sens_payload->battery_level;
            log_i("\nTemp: %.1f   Humidity: %.1f   Battery: %.1f   Level: %d \n", ble_sens[sens_num].temperature, ble_sens[sens_num].humidity, ble_sens[sens_num].battery_voltage, ble_sens[sens_num].battery_level);
            ble_sens[sens_num].found = true;
        }
        BLEDevice::getScan()->stop();
    }
