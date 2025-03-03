/**
 * @file disp_spi_ili9341.c
 * @brief Implementation of ILI9341 TFT LCD driver with SPI interface. This file
 *        provides hardware-specific control functions for ILI9341 series TFT
 *        displays, including initialization sequence, pixel data transfer,
 *        and display control commands through SPI communication.
 *
 * @copyright Copyright (c) 2021-2024 Tuya Inc. All Rights Reserved.
 *
 */
#include <stdio.h>
#include "disp_spi_driver.h"

/* ILI9341 commands */
#define ILI9341_NOP     0x00
#define ILI9341_SWRESET 0x01
#define ILI9341_RDDID   0x04
#define ILI9341_RDDST   0x09

#define ILI9341_RDDPM      0x0A // Read display power mode
#define ILI9341_RDD_MADCTL 0x0B // Read display MADCTL
#define ILI9341_RDD_COLMOD 0x0C // Read display pixel format
#define ILI9341_RDDIM      0x0D // Read display image mode
#define ILI9341_RDDSM      0x0E // Read display signal mode
#define ILI9341_RDDSR      0x0F // Read display self-diagnostic result (ILI9341)

#define ILI9341_SLPIN  0x10 // Enter Sleep Mode
#define ILI9341_SLPOUT 0x11 // Sleep out
#define ILI9341_PTLON  0x12
#define ILI9341_NORON  0x13

#define ILI9341_INVOFF  0x20
#define ILI9341_INVON   0x21
#define ILI9341_GAMSET  0x26 // Gamma set
#define ILI9341_DISPOFF 0x28
#define ILI9341_DISPON  0x29
#define ILI9341_CASET   0x2A // Column Address Set
#define ILI9341_RASET   0x2B // Row Address Set
#define ILI9341_RAMWR   0x2C
#define ILI9341_RGBSET  0x2D // Color setting for 4096, 64K and 262K colors
#define ILI9341_RAMRD   0x2E

#define ILI9341_PTLAR   0x30
#define ILI9341_VSCRDEF 0x33 // Vertical scrolling definition (ILI9341)
#define ILI9341_TEOFF   0x34 // Tearing effect line off
#define ILI9341_TEON    0x35 // Tearing effect line on
#define ILI9341_MADCTL  0x36 // Memory lcd_data access control
#define ILI9341_IDMOFF  0x38 // Idle mode off
#define ILI9341_IDMON   0x39 // Idle mode on
#define ILI9341_COLMOD  0x3A
#define ILI9341_RAMWRC  0x3C // Memory write continue (ILI9341)
#define ILI9341_RAMRDC  0x3E // Memory read continue (ILI9341)

#define ILI9341_RAMCTRL   0xB0 // RAM control
#define ILI9341_RGBCTRL   0xB1 // RGB control
#define ILI9341_PORCTRL   0xB2 // Porch control
#define ILI9341_FRCTRL1   0xB3 // Frame rate control
#define ILI9341_PARCTRL   0xB5 // Partial mode control
#define ILI9341_DSIPCTRL  0xB6 // Display Function Control
#define ILI9341_GCTRL     0xB7 // Gate control
#define ILI9341_GTADJ     0xB8 // Gate on timing adjustment
#define ILI9341_DGMEN     0xBA // Digital gamma enable
#define ILI9341_VCOMS     0xBB // VCOMS setting
#define ILI9341_LCMCTRL   0xC0 // LCM control
#define ILI9341_IDSET     0xC1 // ID setting
#define ILI9341_VDVVRHEN  0xC2 // VDV and VRH command enable
#define ILI9341_VRHS      0xC3 // VRH set
#define ILI9341_VDVSET    0xC4 // VDV setting
#define ILI9341_VCMOFSET  0xC5 // VCOMS offset set
#define ILI9341_FRCTR2    0xC6 // FR Control 2
#define ILI9341_CABCCTRL  0xC7 // CABC control
#define ILI9341_REGSEL1   0xC8 // Register value section 1
#define ILI9341_REGSEL2   0xCA // Register value section 2
#define ILI9341_PWMFRSEL  0xCC // PWM frequency selection
#define ILI9341_PWCTRL1   0xD0 // Power control 1
#define ILI9341_VAPVANEN  0xD2 // Enable VAP/VAN signal output
#define ILI9341_CMD2EN    0xDF // Command 2 enable
#define ILI9341_PVGAMCTRL 0xE0 // Positive voltage gamma control
#define ILI9341_NVGAMCTRL 0xE1 // Negative voltage gamma control
#define ILI9341_DGMLUTR   0xE2 // Digital gamma look-up table for red
#define ILI9341_DGMLUTB   0xE3 // Digital gamma look-up table for blue
#define ILI9341_GATECTRL  0xE4 // Gate control
#define ILI9341_SPI2EN    0xE7 // SPI2 enable
#define ILI9341_PWCTRL2   0xE8 // Power control 2
#define ILI9341_EQCTRL    0xE9 // Equalize time control
#define ILI9341_PROMCTRL  0xEC // Program control
#define ILI9341_PROMEN    0xFA // Program mode enable
#define ILI9341_NVMSET    0xFC // NVM setting
#define ILI9341_PROMACT   0xFE // Program action

const uint8_t lcd_init_seq[] = {
    1, 100, ILI9341_SWRESET,              // Software reset
    1, 50,  ILI9341_SLPOUT,               // Exit sleep mode
    3, 0,   ILI9341_DSIPCTRL, 0x0a, 0xc2, // Display Function Control
    2, 0,   ILI9341_COLMOD,   0x55,       // Set colour mode to 16 bit
    2, 0,   ILI9341_MADCTL,   0x08,       // Set MADCTL: row then column, refresh is bottom to top
    1, 10,  ILI9341_DISPON,               // Main screen turn on, then wait 500 ms
    0                                     // Terminate list
};

/**
 * @brief Set the display window for the ILI9341 LCD driver.
 *
 * This function sets the visible window on the LCD display by specifying the
 * start and end coordinates for both the X and Y axes.
 *
 * @param x_start The starting X coordinate of the window.
 * @param y_start The starting Y coordinate of the window.
 * @param x_end The ending X coordinate of the window.
 * @param y_end The ending Y coordinate of the window.
 */
void disp_driver_set_window(uint32_t x_start, uint32_t y_start, uint32_t x_end, uint32_t y_end)
{
    uint8_t lcd_data[5];

    lcd_data[0] = ILI9341_CASET;
    lcd_data[1] = x_start >> 8;
    lcd_data[2] = x_start;
    lcd_data[3] = x_end >> 8;
    lcd_data[4] = x_end;
    drv_lcd_write_cmd(lcd_data, 4);

    lcd_data[0] = ILI9341_RASET;
    lcd_data[1] = y_start >> 8;
    lcd_data[2] = y_start;
    lcd_data[3] = y_end >> 8;
    lcd_data[4] = y_end;
    drv_lcd_write_cmd(lcd_data, 4);

    lcd_data[0] = ILI9341_RAMWR;
    drv_lcd_write_cmd(lcd_data, 0);
}
