

#include "TMR_TZ10xx.h"
#include "TZ01_system.h"

#include "pwm_beep.h"

extern TZ10XX_DRIVER_TMR Driver_ADVTMR0;

bool pwm_beep_init(void)
{
    /* AdvTMR */
    Driver_ADVTMR0.Initialize(NULL, 0);
    Driver_ADVTMR0.PowerControl(ARM_POWER_FULL);
    Driver_ADVTMR0.Configure(16, TMR_COUNT_MODE_PERIODIC, 4);
    
    Driver_ADVTMR0.ConfigureTFF(TMR_TFF_MODE_CMP_TERM_TOGGLE, false, false);
    Driver_ADVTMR0.EnableCompare(true);
    Driver_ADVTMR0.EnableTFF(true);
    
    return true;
    
}

void pwm_beep(BEEP_INFO beep)
{
    uint16_t tm_cnt;
    tm_cnt = (6000 /*(24000 / 4)*/ / beep.scale) * 1000;
    
    Driver_ADVTMR0.Stop();
    //�w�肵�����g����Duty 50%�̋�`�g���o��
    Driver_ADVTMR0.SetCompareValue(tm_cnt / 2, false);
    Driver_ADVTMR0.Start(tm_cnt);
    //��~�܂ł̃^�C���A�E�g��ݒ�
    TZ01_system_tick_start(USRTICK_NO_BEEP, beep.ms);
}

BEEP_STATE pwm_beep_run(void)
{
    if (TZ01_system_tick_check_timeout(USRTICK_NO_BEEP)) {
        //�^�C���A�E�g
        TZ01_system_tick_stop(USRTICK_NO_BEEP);
        Driver_ADVTMR0.Stop();
    }
    
    if (TZ01_system_tick_is_active(USRTICK_NO_BEEP)) {
        return BEEP_ON;
    } else {
        return BEEP_OFF;
    }
}

