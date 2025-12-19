#ifndef PHYPHOXBLE
#define PHYPHOXBLE

#include "Arduino.h"

#define SENSOR_ACCELEROMETER "accelerometer"
#define SENSOR_ACCELEROMETER_WITHOUT_G "linear_acceleration"
#define SENSOR_GYROSCOPE "gyroscope"
#define SENSOR_MAGNETOMETER "magnetometer"
#define SENSOR_PRESSURE "pressure"
#define SENSOR_TEMPERATURE "temperature"
#define SENSOR_LIGHT "light"
#define SENSOR_HUMIDITY "humidity"
#define SENSOR_PROXIMITY "proximity"

#define STYLE_DOTS "dots"
#define STYLE_LINES "lines"
#define STYLE_VBARS "vbars"
#define STYLE_HBARS "hbars"
#define STYLE_MAP "map"

#define COLOR_RED "fe005d"
#define COLOR_BLUE "39a2ff"
#define COLOR_GREEN "2bfb4c"
#define COLOR_ORANGE "ff7e22"
#define COLOR_WHITE "ffffff"
#define COLOR_YELLOW "edf668"
#define COLOR_MAGENTA "eb46f4"

#define LAYOUT_AUTO "auto"
#define LAYOUT_EXTEND "extend"
#define LAYOUT_FIXED "fixed"

#if defined(ARDUINO_SAMD_MKR1000) || defined(ARDUINO_SENSEBOX_MCU_ESP32S2)
    #include "boards/phyphoxBLE_NINAB31.h"
#elif defined(ARDUINO_ARCH_MBED)
    #include "boards/phyphoxBLE_NRF52.h"
#elif defined(ESP32) && !defined(ARDUINO_SENSEBOX_MCU_ESP32S2)
    #include "boards/phyphoxBLE_ESP32.h"
#elif defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_UNOR4_WIFI) || defined(ARDUINO_SAMD_MKRWIFI1010)
    #include <ArduinoBLE.h>
    #include "boards/phyphoxBLE_NanoIOT.h"
#elif defined(ARDUINO_ARCH_STM32)
    #include "boards/phyphoxBLE_STM32.h"
#else
#error "Unsupported board selected!"
#endif

#endif
