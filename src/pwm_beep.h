#ifndef _PWM_BEEP_H_
#define _PWM_BEEP_H_

typedef enum {
    BEEP_OFF,   /*!< –Â‚Á‚Ä‚È‚¢ */
    BEEP_ON,    /*!< –Â‚Á‚Ä‚é */
}   BEEP_STATE;

/* 120bps‘Š“–‚Ì‰¹•„ (ms)*/
typedef enum {
    NOTE_WHOLE     = 2000,  /*!< ‘S‰¹•„ */
    NOTE_HALF      = 1000,  /*!< 2•ª‰¹•„ */
    NOTE_QUATER    = 500,   /*!< 4•ª‰¹•„ */
    NOTE_EIGHTH    = 250,   /*!< 8•ª‰¹•„ */
    NOTE_SIXTEENTH = 125    /*!< 16•ª‰¹•„ */
}   BEEP_NOTE;

/* ‰¹ŠK */
typedef enum {
    SCALE_B4  = 1027,
    SCALE_As4 = 953,
    SCALE_A4  = 880,
    SCALE_Gs4 = 843,
    SCALE_G4  = 807,
    SCALE_Fs4 = 770,
    SCALE_F4  = 733,
    SCALE_E4  = 697,
    SCALE_Ds4 = 660,
    SCALE_D4  = 623,
    SCALE_Cs4 = 587,
    SCALE_C4  = 550,
    SCALE_B3  = 513,
    SCALE_As3 = 477,
    SCALE_A3  = 440,
    SCALE_Gs3 = 422,
    SCALE_G3  = 403,
    SCALE_Fs3 = 385,
    SCALE_F3  = 367,
    SCALE_E3  = 348,
    SCALE_Ds3 = 330,
    SCALE_D3  = 312,
    SCALE_Cs3 = 293,
    SCALE_C3  = 275,
    SCALE_B2  = 257,
    SCALE_As2 = 238,
    SCALE_A2  = 220,
    SCALE_NON = 0
}   BEEP_SCALE;

typedef struct {
    BEEP_SCALE  scale;
    uint16_t    ms;
}   BEEP_INFO;

bool pwm_beep_init(void);
void pwm_beep(BEEP_INFO beep);
BEEP_STATE pwm_beep_run(void);
#endif

