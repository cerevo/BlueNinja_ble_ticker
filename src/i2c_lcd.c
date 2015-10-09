#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "TZ10xx.h"
#include "Driver_I2C.h"

#include "TZ01_system.h"
#include "TZ01_console.h"

#include "utils.h"

#define LCD_ID  (0x3e)

extern ARM_DRIVER_I2C Driver_I2C0;

uint8_t msg[80];


static write_cmd(uint8_t cmd)
{
    int len;
    uint8_t dat[2];
    
    dat[0] = 0x00;
    dat[1] = cmd;
    
    len = Driver_I2C0.SendData(LCD_ID, dat, 2, false);
}

static write_char(uint8_t chr)
{
    uint8_t dat[2];
    
    dat[0] = 0x40;
    dat[1] = chr;
    
    Driver_I2C0.SendData(LCD_ID, dat, 2, false);
}

/*** ***/
bool i2c_lcd_init(void)
{
    //I2C0 initialize
    if (Driver_I2C0.Initialize(NULL) != ARM_I2C_OK) {
        return false;
    }
    if (Driver_I2C0.PowerControl(ARM_POWER_FULL) != ARM_I2C_OK) {
        return false;
    }
    if (Driver_I2C0.BusSpeed(ARM_I2C_BUS_SPEED_FAST) != ARM_I2C_OK) {
        return false;
    }
    //LCD initialize
    Usleep(50000);
    write_cmd(0x38);
    Usleep(30);
    write_cmd(0x02);
    Usleep(30);
    write_cmd(0x39);
    Usleep(30);
    write_cmd(0x14);
    Usleep(30);
    write_cmd(0x7f);
    Usleep(30);
    write_cmd(0x56);
    Usleep(30);
    write_cmd(0x6a);
    
    Usleep(200000);
    
    write_cmd(0x0c);
    Usleep(30);
    write_cmd(0x01);
    Usleep(30);
    write_cmd(0x04);
    Usleep(30);
    
    return true;
}

bool i2c_lcd_loc(uint8_t col, uint8_t row)
{
    uint8_t addr = 0x80;
    if ((col < 16) && (row < 2)) {
        addr |= col;
        if (row == 1) {
            addr |= 0x40;
        }
        write_cmd(addr);    //DDRAMのアドレスを設定
        return true;
    }

    return false;
}

void i2c_lcd_clear(void)
{
    write_cmd(0x01);
    Usleep(10000);
}

int i2c_lcd_puts(uint8_t *str, uint8_t start_index)
{
    int i;
    uint8_t addr = 0x80;
    
    if (start_index >= 32) {
        return -1;
    }
    
    //表示開始アドレス
    if (start_index < 16) {
        //1行目
        addr |= start_index;
    } else {
        //2行目
        addr |= (0x40 | (start_index - 16));
    }
    write_cmd(addr);
    Usleep(30);
    
    for (i = 0; i < 32; i++) {
        if (str[i] == '\0') {
            break;
        }
        
        if ((i + start_index) == 16) {
            write_cmd(0xc0);    //アドレスを2行目先頭に
            Usleep(30);
        } else if ((i + start_index) == 32) {
            write_cmd(0x80);    //アドレスを1行目先頭に
            Usleep(30);
        }
        write_char(str[i]);
        Usleep(30);
    }
    
    return i;
}

