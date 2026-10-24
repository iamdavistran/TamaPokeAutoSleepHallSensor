#pragma once

// Pines oficiales de la Waveshare ESP32-S3-Touch-AMOLED-1.75
// Fuente: github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75 (libraries/Mylibrary/pin_config.h)

#define XPOWERS_CHIP_AXP2101

// Pantalla AMOLED 466x466, driver CO5300 por QSPI
#define LCD_SDIO0 4
#define LCD_SDIO1 5
#define LCD_SDIO2 6
#define LCD_SDIO3 7
#define LCD_SCLK 38
#define LCD_CS 12
#define LCD_RESET 39
#define LCD_WIDTH 466
#define LCD_HEIGHT 466

// Táctil capacitivo CST9217 por I2C
#define IIC_SDA 15
#define IIC_SCL 14
#define TP_INT 11
#define TP_RESET 40

// Audio ES8311. NOTA: el MCLK real es GPIO42 (verificado en placa con el
// proyecto PlaneRadar2.0); el 16 que figuraba era erroneo. Aun asi el codec se
// configura con reloj derivado del BCLK, asi que el MCLK apenas importa.
#define I2S_MCK_IO 42
#define I2S_BCK_IO 9
#define I2S_DI_IO 10
#define I2S_WS_IO 45
#define I2S_DO_IO 8
#define PA 46

// Ranura TF (no usada todavía)
#define SDMMC_CLK 2
#define SDMMC_CMD 1
#define SDMMC_DATA 3
#define SDMMC_CS 41

// Lid sensor (DRV5032 Hall breakout, see lid.h): the expansion-header (H2) pin marked GPIO16.
// It has to be an RTC GPIO (0-21) to wake the chip from deep sleep. Header pins
// 6/7 (GPIO17/18) qualify too but may be the GNSS UART on the -G board.
#define LID_PIN 16
// 0 for the push-pull Hall sensor (the default); 1 only for a bare reed switch,
// which has nothing else to pull the pin high while the chip sleeps.
#define LID_SLEEP_PULLUP 0
