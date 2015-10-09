
#ifndef _I2C_LCD_H_
#define _I2C_LCD_H_

bool i2c_lcd_init(void);
bool i2c_lcd_loc(uint8_t col, uint8_t row);
void i2c_lcd_clear(void);
int  i2c_lcd_puts(uint8_t *str, uint8_t start_index);

#endif
