#include "soc_cv_av/alt_clock_manager.h"

typedef enum
{
	MPU_CLOCK_HIGH = 1,
	MPU_CLOCK_LOW
} MPU_CLOCK;
/* Public Functions ----------------------------------------------------------- */
void set_main_pll(alt_freq_t new_freq);
void set_peri_pll(alt_freq_t new_freq);
void disable_unused_peripheral(void);
void change_mpu_clock(MPU_CLOCK clock);
void set_mpu_clock(alt_freq_t new_fre);
void set_main_pll_down(uint32_t div);
void get_pll_info(void);

/*----------------------------------------------------------------------*/
/* PLL Infomaiton 													   	*/
/*----------------------------------------------------------------------*/
extern uint8_t clock_enabled[ALT_CLK_H2F_USER2+1];
extern uint32_t clock_frequency[ALT_CLK_H2F_USER2+1];
extern uint8_t safe_enabled[2];
