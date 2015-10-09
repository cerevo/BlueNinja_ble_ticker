/**
 * @file   main.c
 * @brief  Application main.
 *
 * @author Cerevo Inc.
 */

/*
Copyright 2015 Cerevo Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "TZ10xx.h"
#include "PMU_TZ10xx.h"
#include "GPIO_TZ10xx.h"
#include "TMR_TZ10xx.h"
#include "Driver_I2C.h"

#include "TZ01_system.h"
#include "TZ01_console.h"

#include "utils.h"

#include "i2c_lcd.h"
#include "pwm_beep.h"
#include "ble_msg.h"

extern TZ10XX_DRIVER_PMU  Driver_PMU;
extern TZ10XX_DRIVER_GPIO Driver_GPIO;

uint8_t msg[80];

/** Button **/
typedef struct {
    uint8_t     on      :1;
    uint8_t     hist    :3;
    uint16_t    on_cnt;
}   BtnStat;

static void sw_dev_init(void)
{
    Driver_PMU.StandbyInputBuffer(PMU_IO_FUNC_GPIO_6, 0);
    Driver_GPIO.Configure(6, GPIO_DIRECTION_INPUT_HI_Z, GPIO_EVENT_DISABLE, NULL);
}

static void sw_init(BtnStat *btn)
{
    btn->on = 0;
    btn->hist = 0x07;
    btn->on_cnt = 0;
}

static void sw_update(BtnStat *btn, int gpio)
{
    uint32_t val;
    
    btn->hist = (btn->hist << 1) & 0x07;    //履歴を更新
    if (Driver_GPIO.ReadPin(gpio, &val) == GPIO_ERROR) {
        return;
    }
    btn->hist |= (val & 0x01);
    
    if (btn->hist == 0x07) {
        //OFF
        btn->on = 0;
        btn->on_cnt = 0;
    }
    if (btn->hist == 0x00) {
        //ON
        btn->on = 1;
        btn->on_cnt++;
    }
}

static uint8_t sw_state(BtnStat *btn)
{
    if (btn->on == 1) {
        if (btn->on_cnt < 200) {
            return 1;   //ON
        } else {
            return 2;   //HOLD
        }
    } else {
        return 0;       //OFF
    }
}

int main(void)
{
    /* Initialize */
    if (ble_msg_init() != 0) {
        return 1;
    }
    
    TZ01_system_init();
    TZ01_console_init();

    if (i2c_lcd_init() == false) {
        goto term;
    }
    
    if (pwm_beep_init() == false) {
        goto term;
    }
    
    sw_dev_init();

    Usleep(100000);
    i2c_lcd_puts("Cerevo BlueNinja", 0);
    
    i2c_lcd_puts("I2C LCD DEMO", 18);
    
    //スイッチ監視タイマ開始
    TZ01_system_tick_start(USRTICK_NO_GPIO_INTERVAL, 10);
    
    int cnt = 50000;
    BtnStat sw_info;
    int curr_sw_stat = 0, prev_sw_stat = 0, sw_event = 0;
    
    BEEP_INFO beep = {SCALE_G3, NOTE_EIGHTH};
    pwm_beep(beep);
    
    sw_init(&sw_info);
    for (;;) {
        if (TZ01_system_run() == RUNEVT_POWOFF) {
            /* Power off operation detected */
            break;
        }
        
        /* スイッチ監視 */
        sw_event = 0;
        if (TZ01_system_tick_check_timeout(USRTICK_NO_GPIO_INTERVAL)) {
            /* 10ms event*/
            TZ01_system_tick_start(USRTICK_NO_GPIO_INTERVAL, 10);
            
            sw_update(&sw_info, 6);
            curr_sw_stat = sw_state(&sw_info);
            if (curr_sw_stat != prev_sw_stat) {
                switch (curr_sw_stat) {
                case 0:
                    sw_event = 0;
                    break;
                case 1:
                    sw_event = 1;
                    break;
                case 2:
                    sw_event = 2;
                    break;
                }
            } else {
                sw_event = -1;
            }
        }
        prev_sw_stat = curr_sw_stat;
        
        /* BLE */
        ble_msg_run(sw_event);

/*        
        if ((cnt % 100) == 0) {
            Driver_ADVTMR0.SetCompareValue(cnt / 100, false);
        }
        if (cnt-- == 0) {
            cnt = 50000;
        }
*/
        pwm_beep_run();
    }
term:
    i2c_lcd_clear();
    i2c_lcd_puts("Program terminated.", 0);
    return 0;
}
