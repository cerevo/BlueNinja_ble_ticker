/**
 * @file ble_msg.c
 * @breaf Cerevo CDP-TZ01B sample program.
 * BLE Message
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
#include "TZ10xx.h"
#include "RTC_TZ10xx.h"
#include "PMU_TZ10xx.h"
#include "RNG_TZ10xx.h"
#include "GPIO_TZ10xx.h"

#include "twic_interface.h"
#include "twic_led.h"
#include "blelib.h"

#include "TZ01_system.h"
#include "TZ01_console.h"

#include "i2c_lcd.h"
#include "pwm_beep.h"

#define APP_TZ1EM_WF_GPIO TZ1EM_WF_G2
#define BNMSG_MTU    (40)

extern TZ10XX_DRIVER_RNG Driver_RNG;

static uint64_t bnmsg_bdaddr  = 0xc0ce00000000;   //

/*--- GATT profile definition ---*/
const uint8_t bnmsg_gap_device_name[] = "CDP-TZ01B_MSG";
const uint8_t bnmsg_gap_appearance[] = {0x00, 0x00};

const uint8_t bnmsg_di_manufname[] = "Cerevo";
const uint8_t bnmsg_di_fw_version[] = "0.1";
const uint8_t bnmsg_di_sw_version[] = "0.1";
const uint8_t bnmsg_di_model_string[] = "CDP-TZ01B_MSG";

/* BLElib unique id. */
enum {
    BLE_GATT_UNIQUE_ID_GAP_SERVICE = 0,
    BLE_GATT_UNIQUE_ID_GAP_DEVICE_NAME,
    BLE_GATT_UNIQUE_ID_GAP_APPEARANCE,
    BLE_GATT_UNIQUE_ID_DI_SERVICE,
    BLE_GATT_UNIQUE_ID_DI_MANUF_NAME,
    BLE_GATT_UNIQUE_ID_DI_FW_VERSION,
    BLE_GATT_UNIQUE_ID_DI_SW_VERSION,
    BLE_GATT_UNIQUE_ID_DI_MODEL_STRING,
    BLE_GATT_UNIQUE_ID_BNMSG_SERVICE,
    BLE_GATT_UNIQUE_ID_BNMSG_MSG,
    BLE_GATT_UNIQUE_ID_BNMSG_BEEP,
};

/* GAP */
const BLELib_Characteristics gap_device_name = {
    BLE_GATT_UNIQUE_ID_GAP_DEVICE_NAME, 0x2a00, 0, BLELIB_UUID_16,
    BLELIB_PROPERTY_READ,
    BLELIB_PERMISSION_READ | BLELIB_PERMISSION_WRITE,
    bnmsg_gap_device_name, sizeof(bnmsg_gap_device_name),
    NULL, 0
};
const BLELib_Characteristics gap_appearance = {
    BLE_GATT_UNIQUE_ID_GAP_APPEARANCE, 0x2a01, 0, BLELIB_UUID_16,
    BLELIB_PROPERTY_READ,
    BLELIB_PERMISSION_READ,
    bnmsg_gap_appearance, sizeof(bnmsg_gap_appearance),
    NULL, 0
};
const BLELib_Characteristics *const gap_characteristics[] = { &gap_device_name, &gap_appearance };
    const BLELib_Service gap_service = {
    BLE_GATT_UNIQUE_ID_GAP_SERVICE, 0x1800, 0, BLELIB_UUID_16,
    true, NULL, 0,
    gap_characteristics, 2
};

/* DIS(Device Informatin Service) */
const BLELib_Characteristics di_manuf_name = {
    BLE_GATT_UNIQUE_ID_DI_MANUF_NAME, 0x2a29, 0, BLELIB_UUID_16,
    BLELIB_PROPERTY_READ,
    BLELIB_PERMISSION_READ,
    bnmsg_di_manufname, sizeof(bnmsg_di_manufname),
    NULL, 0
};
const BLELib_Characteristics di_fw_version = {
    BLE_GATT_UNIQUE_ID_DI_FW_VERSION, 0x2a26, 0, BLELIB_UUID_16,
    BLELIB_PROPERTY_READ,
    BLELIB_PERMISSION_READ,
    bnmsg_di_fw_version, sizeof(bnmsg_di_fw_version),
    NULL, 0
};
const BLELib_Characteristics di_sw_version = {
    BLE_GATT_UNIQUE_ID_DI_SW_VERSION, 0x2a28, 0, BLELIB_UUID_16,
    BLELIB_PROPERTY_READ,
    BLELIB_PERMISSION_READ,
    bnmsg_di_sw_version, sizeof(bnmsg_di_sw_version),
    NULL, 0
};
const BLELib_Characteristics di_model_string = {
    BLE_GATT_UNIQUE_ID_DI_MODEL_STRING, 0x2a24, 0, BLELIB_UUID_16,
    BLELIB_PROPERTY_READ,
    BLELIB_PERMISSION_READ,
    bnmsg_di_model_string, sizeof(bnmsg_di_model_string),
    NULL, 0
};
const BLELib_Characteristics *const di_characteristics[] = {
    &di_manuf_name, &di_fw_version, &di_sw_version, &di_model_string
};
const BLELib_Service di_service = {
    BLE_GATT_UNIQUE_ID_DI_SERVICE, 0x180a, 0, BLELIB_UUID_16,
    true, NULL, 0,
    di_characteristics, 4
};

static uint8_t bnmsg_msg_val[33];
static uint8_t bnmsg_beep_val[1];

/* BlueNinja Message Service Characteristics */
//Message
const BLELib_Characteristics bnmsg_msg = {
    BLE_GATT_UNIQUE_ID_BNMSG_MSG, 0x90bc080027cc79e0, 0x000100014ba211e5, BLELIB_UUID_128,
    BLELIB_PROPERTY_WRITE,
    BLELIB_PERMISSION_WRITE,
    bnmsg_msg_val, sizeof(bnmsg_msg_val),
    NULL, 0
};

//Beep
const BLELib_Characteristics bnmsg_beep = {
    BLE_GATT_UNIQUE_ID_BNMSG_BEEP, 0x90bc080027cc79e0, 0x000100024ba211e5, BLELIB_UUID_128,
    BLELIB_PROPERTY_WRITE,
    BLELIB_PERMISSION_WRITE,
    bnmsg_beep_val, sizeof(bnmsg_beep_val),
    NULL, 0
};

/* Characteristics list */
const BLELib_Characteristics *const bnmsg_characteristics[] = {
    &bnmsg_msg, &bnmsg_beep
};
/* BlueNinja Message Service */
const BLELib_Service bnmsg_service = {
    BLE_GATT_UNIQUE_ID_BNMSG_SERVICE, 0x90bc080027cc79e0, 0x000100004ba211e5, BLELIB_UUID_128,
    true, NULL, 0,
    bnmsg_characteristics, 2
};

/* Service list */
const BLELib_Service *const bnmsg_service_list[] = {
    &gap_service, &di_service, &bnmsg_service
};

/*- INDICATION data -*/
uint8_t bnmsg_advertising_data[] = {
    0x02, /* length of this data */
    0x01, /* AD type = Flags */
    0x06, /* LE General Discoverable Mode = 0x02 */
    /* BR/EDR Not Supported (i.e. bit 37
     * of LMP Extended Feature bits Page 0) = 0x04 */

    0x03, /* length of this data */
    0x08, /* AD type = Short local name */
    'T',  /* (T) */
    'Z',  /* (Z) */

    0x05, /* length of this data */
    0x03, /* AD type = Complete list of 16-bit UUIDs available */
    0x00, /* Generic Access Profile Service 1800 */
    0x18,
    0x0A, /* Device Information Service 180A */
    0x18,
    };

    uint8_t bnmsg_scan_resp_data[] = {
    0x02, /* length of this data */
    0x01, /* AD type = Flags */
    0x06, /* LE General Discoverable Mode = 0x02 */
    /* BR/EDR Not Supported (i.e. bit 37
     * of LMP Extended Feature bits Page 0) = 0x04 */

    0x02, /* length of this data */
    0x0A, /* AD type = TX Power Level (1 byte) */
    0x00, /* 0dB (-127...127 = 0x81...0x7F) */

    0x0e, /* length of this data */
    0x09, /* AD type = Complete local name */
    'C', 'D', 'P', '-', 'T', 'Z', '0', '1', 'B', '_', 'M', 'S', 'G' /* CDP-TZ01B_MSG */
};

/*=== BlueNinja messenger application ===*/
static uint64_t central_bdaddr;

/*= BLElib callback functions =*/
void connectionCompleteCb(const uint8_t status, const bool master, const uint64_t bdaddr, const uint16_t conn_interval)
{
    central_bdaddr = bdaddr;

    BLELib_requestMtuExchange(BNMSG_MTU);

    TZ01_system_tick_start(USRTICK_NO_BLE_MAIN, 100);
}

void connectionUpdateCb(const uint8_t status, const uint16_t conn_interval, const uint16_t conn_latency)
{
}

void disconnectCb(const uint8_t status, const uint8_t reason)
{    
    TZ01_system_tick_stop(USRTICK_NO_BLE_MAIN);
}

BLELib_RespForDemand mtuExchangeDemandCb(const uint16_t client_rx_mtu_size, uint16_t *resp_mtu_size)
{
    uint8_t msg[64];
	*resp_mtu_size = BNMSG_MTU;
    sprintf(msg, "client_rx_mtu_size=%d, resp_mtu_size=%d\r\n", client_rx_mtu_size, *resp_mtu_size);
    TZ01_console_puts(msg);
	return BLELIB_DEMAND_ACCEPT;
}

void mtuExchangeResultCb(const uint8_t status, const uint16_t negotiated_mtu_size)
{
    uint8_t msg[32];
    sprintf(msg, "negotiated_mtu_size=%d\r\n", negotiated_mtu_size);
    TZ01_console_puts(msg);
}

void notificationSentCb(const uint8_t unique_id)
{
}

void indicationConfirmCb(const uint8_t unique_id)
{
}

void updateCompleteCb(const uint8_t unique_id)
{
}

void queuedWriteCompleteCb(const uint8_t status)
{
}

BLELib_RespForDemand readoutDemandCb(const uint8_t *const unique_id_array, const uint8_t unique_id_num)
{
	return BLELIB_DEMAND_ACCEPT;
}

BLELib_RespForDemand writeinDemandCb(const uint8_t unique_id, const uint8_t *const value, const uint8_t value_len)
{
    RTC_TIME now;
    BEEP_INFO beep;
    int len;
    switch (unique_id) {
        case BLE_GATT_UNIQUE_ID_BNMSG_MSG:
            memset(bnmsg_msg_val, 0, sizeof(bnmsg_msg_val));
            len = sizeof(bnmsg_msg_val);
            if (value_len < len) {
                len = value_len;
            }
            strncpy(bnmsg_msg_val, value, len);
            
            i2c_lcd_clear();
            i2c_lcd_puts(bnmsg_msg_val, 0);
            //BEEP
            beep.scale = SCALE_G4;
            beep.ms    = NOTE_SIXTEENTH;
            pwm_beep(beep);
            break;
        case BLE_GATT_UNIQUE_ID_BNMSG_BEEP:
            //BEEP
            beep.scale = SCALE_C4;
            beep.ms    = NOTE_EIGHTH;
            pwm_beep(beep);
            break;
    }
    return BLELIB_DEMAND_ACCEPT;
}

void writeinPostCb(const uint8_t unique_id, const uint8_t *const value, const uint8_t value_len)
{
}

void isrNewEventCb(void)
{
    /* this sample always call BLELib_run() */
}

void isrWakeupCb(void)
{
    /* this callback is not used currently */
}

BLELib_CommonCallbacks tz01_common_callbacks = {
    connectionCompleteCb,
    connectionUpdateCb,
    mtuExchangeResultCb,
    disconnectCb,
    isrNewEventCb,
    isrWakeupCb
};

BLELib_ServerCallbacks tz01_server_callbacks = {
    mtuExchangeDemandCb,
    notificationSentCb,
    indicationConfirmCb,
    updateCompleteCb,
    queuedWriteCompleteCb,
    readoutDemandCb,
    writeinDemandCb,
    writeinPostCb,
};

int ble_msg_init(void)
{
    if (TZ1EM_STATUS_OK != tz1emInitializeSystem())
        return 1; /* Must not use UART for LOG before twicIfLeIoInitialize. */
    
    /* create random bdaddr */
    uint32_t randval;
    Driver_PMU.SetPowerDomainState(PMU_PD_ENCRYPT, PMU_PD_MODE_ON);
    Driver_RNG.Initialize();
    Driver_RNG.PowerControl(ARM_POWER_FULL);
    Driver_RNG.Read(&randval);
    Driver_RNG.Uninitialize();
    bnmsg_bdaddr |= (uint64_t)randval;

    /* initialize BLELib */
    int ret;
    BLELib_initialize(bnmsg_bdaddr, BLELIB_BAUDRATE_2304, &tz01_common_callbacks, &tz01_server_callbacks, NULL);
    ret = BLELib_registerService(bnmsg_service_list, 3);
    BLELib_setLowPowerMode(BLELIB_LOWPOWER_ON);

    twicLedInit();

    return ret;
}

static uint8_t msg[80];

int ble_msg_run(const int key_stat)
{
    int ret, res = 0;
    BLELib_State state;
    bool has_event;

    state = BLELib_getState();
    has_event = BLELib_hasEvent();

    switch (state) {
        case BLELIB_STATE_UNINITIALIZED:
        case BLELIB_STATE_INITIALIZED:
            break;

        case BLELIB_STATE_ADVERTISE_READY:
            twicSetLed(TWIC_LED_GPIO_LED3, false);
            if (key_stat == 1 /* Push */) {
                ret = BLELib_startAdvertising(bnmsg_advertising_data, sizeof(bnmsg_advertising_data), bnmsg_scan_resp_data, sizeof(bnmsg_scan_resp_data));
                sprintf(msg, "BLELib_startAdvertising(): %d state=%d\r\n", ret, state);
                TZ01_console_puts(msg);
            }
            break;
        case BLELIB_STATE_ADVERTISING:
            twicSetLed(TWIC_LED_GPIO_LED3, true);
            if (key_stat == 1 /* Push */) {
                ret = BLELib_stopAdvertising();
                sprintf(msg, "BLELib_stopAdvertising(): %d\r\n", ret);
                TZ01_console_puts(msg);
            }
            break;

        case BLELIB_STATE_ONLINE:
            if (key_stat == 1 /* Push */) {
                ret = BLELib_disconnect(central_bdaddr);
                sprintf(msg, "BLELib_disconnect(): %d\r\n", ret);
                TZ01_console_puts(msg);
                break;
            }
        default:
            break;
    }

    if (has_event) {
        ret = BLELib_run();
        if (ret != 0) {
            sprintf(msg, "BLELib_run(): %d state=%d\r\n", ret, state);
            TZ01_console_puts(msg);
        }
    }

    return res;
}

void ble_msg_stop(void)
{
    switch (BLELib_getState()) {
        case BLELIB_STATE_ADVERTISING:
            BLELib_stopAdvertising();
            twicSetLed(TWIC_LED_GPIO_LED3, false);
            break;

        case BLELIB_STATE_ONLINE:
            BLELib_disconnect(central_bdaddr);
            break;
        default:
            break;
    }
}
