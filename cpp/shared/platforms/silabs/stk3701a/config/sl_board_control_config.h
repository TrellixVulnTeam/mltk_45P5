#ifndef SL_BOARD_CONTROL_CONFIG_H
#define SL_BOARD_CONTROL_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <q SL_BOARD_ENABLE_VCOM> Enable Virtual COM UART
// <i> Default: 0
#define SL_BOARD_ENABLE_VCOM                    1

// <q SL_BOARD_ENABLE_DISPLAY> Enable Display
// <i> Default: 0
#define SL_BOARD_ENABLE_DISPLAY                 0

// <q SL_BOARD_ENABLE_SENSOR_RHT> Enable Relative Humidity and Temperature sensor
// <i> Default: 0
#define SL_BOARD_ENABLE_SENSOR_RHT              0

// <q SL_BOARD_ENABLE_SENSOR_HALL> Enable Hall Effect sensor
// <i> Default: 0
#define SL_BOARD_ENABLE_SENSOR_HALL             0

// <q SL_BOARD_ENABLE_SENSOR_MICROPHONE> Enable Microphone
// <i> Default: 0
#define SL_BOARD_ENABLE_SENSOR_MICROPHONE       0

// <q SL_BOARD_ENABLE_MEMORY_QSPI> Enable QSPI Flash
// <i> Default: 0
#define SL_BOARD_ENABLE_MEMORY_QSPI             0

// <q SL_BOARD_ENABLE_MEMORY_SDCARD> Enable SD Card
// <i> Default: 0
#define SL_BOARD_ENABLE_MEMORY_SDCARD           0

// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>

// <gpio> SL_BOARD_ENABLE_VCOM
// $[GPIO_SL_BOARD_ENABLE_VCOM]
#define SL_BOARD_ENABLE_VCOM_PORT                gpioPortE
#define SL_BOARD_ENABLE_VCOM_PIN                 1
// [GPIO_SL_BOARD_ENABLE_VCOM]$

// <gpio> SL_BOARD_ENABLE_DISPLAY
// $[GPIO_SL_BOARD_ENABLE_DISPLAY]
#define SL_BOARD_ENABLE_DISPLAY_PORT             gpioPortA
#define SL_BOARD_ENABLE_DISPLAY_PIN              9
// [GPIO_SL_BOARD_ENABLE_DISPLAY]$

// <gpio> SL_BOARD_ENABLE_SENSOR_RHT
// $[GPIO_SL_BOARD_ENABLE_SENSOR_RHT]
#define SL_BOARD_ENABLE_SENSOR_RHT_PORT          gpioPortB
#define SL_BOARD_ENABLE_SENSOR_RHT_PIN           3
// [GPIO_SL_BOARD_ENABLE_SENSOR_RHT]$

// <gpio> SL_BOARD_ENABLE_SENSOR_HALL
// $[GPIO_SL_BOARD_ENABLE_SENSOR_HALL]
#define SL_BOARD_ENABLE_SENSOR_HALL_PORT         gpioPortB
#define SL_BOARD_ENABLE_SENSOR_HALL_PIN          3
// [GPIO_SL_BOARD_ENABLE_SENSOR_HALL]$

// <gpio> SL_BOARD_ENABLE_SENSOR_MICROPHONE
// $[GPIO_SL_BOARD_ENABLE_SENSOR_MICROPHONE]
#define SL_BOARD_ENABLE_SENSOR_MICROPHONE_PORT   gpioPortD
#define SL_BOARD_ENABLE_SENSOR_MICROPHONE_PIN    0
// [GPIO_SL_BOARD_ENABLE_SENSOR_MICROPHONE]$

// <gpio> SL_BOARD_ENABLE_MEMORY_QSPI
// $[GPIO_SL_BOARD_ENABLE_MEMORY_QSPI]
#define SL_BOARD_ENABLE_MEMORY_QSPI_PORT         gpioPortG
#define SL_BOARD_ENABLE_MEMORY_QSPI_PIN          13
// [GPIO_SL_BOARD_ENABLE_MEMORY_QSPI]$

// <gpio> SL_BOARD_ENABLE_MEMORY_SDCARD
// $[GPIO_SL_BOARD_ENABLE_MEMORY_SDCARD]
#define SL_BOARD_ENABLE_MEMORY_SDCARD_PORT       gpioPortE
#define SL_BOARD_ENABLE_MEMORY_SDCARD_PIN        7
// [GPIO_SL_BOARD_ENABLE_MEMORY_SDCARD]$

// <<< sl:end pin_tool >>>

#endif // SL_BOARD_CONTROL_CONFIG_H
