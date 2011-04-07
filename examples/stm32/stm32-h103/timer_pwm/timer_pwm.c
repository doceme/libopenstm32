/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2011 Piotr Esden-Tempski <piotr@esden.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/exti.h>

int frequency_sel = 0;

u16 compare_time;
u16 new_time;
u16 frequency;
int debug = 0;

void clock_setup(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

}

void gpio_setup(void)
{
	/* Enable GPIOC clock. */
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);

	/*
	 * Set GPIO12 (PORTC) (led) to
	 * 'output alternate function push-pull'.
	 */
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);

	gpio_set(GPIOC, GPIO12);

}

void tim_setup(void)
{

	/* Enable TIM3 clock. */
	rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_TIM3EN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN |
						  RCC_APB2ENR_AFIOEN);

	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
			GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
			GPIO_TIM3_FR_CH1 |
			GPIO_TIM3_FR_CH2 |
			GPIO_TIM3_FR_CH3 |
			GPIO_TIM3_FR_CH4);

	AFIO_MAPR |= AFIO_MAPR_TIM3_REMAP_FULL_REMAP;

	/* Reset TIM3 peripheral */
	timer_reset(TIM3);

	/* Timer global mode:
	 * - No divider
	 * - alignment edge
	 * - direction up
	 */
	timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT,
		       TIM_CR1_CMS_EDGE,
		       TIM_CR1_DIR_UP);

	/* Reset prescaler value. */
	timer_set_prescaler(TIM3, 72000000 / 1000000);

	/* Enable preload. */
	timer_disable_preload(TIM3);

	/* Continous mode. */
	timer_continuous_mode(TIM3);

	/* Period (36kHz) */
	timer_set_period(TIM3, 1000000 / 50);

	/* Disable outputs. */
	timer_disable_oc_output(TIM3, TIM_OC1);
	timer_disable_oc_output(TIM3, TIM_OC2);
	timer_disable_oc_output(TIM3, TIM_OC3);
	timer_disable_oc_output(TIM3, TIM_OC4);

	/* -- OC1 configuration -- */

	/* Configure global mode of line 1. */
	timer_disable_oc_clear(TIM3, TIM_OC1);
	timer_enable_oc_preload(TIM3, TIM_OC1);
	timer_set_oc_slow_mode(TIM3, TIM_OC1);
	timer_set_oc_mode(TIM3, TIM_OC1, TIM_OCM_PWM1);

	timer_set_oc_polarity_high(TIM3, TIM_OC1);
	timer_set_oc_idle_state_set(TIM3, TIM_OC1);

	/* Set the capture compare value for OC1. */
	timer_set_oc_value(TIM3, TIM_OC1, 1000);
	
	/* Configure global mode of line 1. */
	timer_disable_oc_clear(TIM3, TIM_OC2);
	timer_enable_oc_preload(TIM3, TIM_OC2);
	timer_set_oc_slow_mode(TIM3, TIM_OC2);
	timer_set_oc_mode(TIM3, TIM_OC2, TIM_OCM_PWM1);
	
	timer_set_oc_polarity_high(TIM3, TIM_OC2);
	timer_set_oc_idle_state_set(TIM3, TIM_OC2);

	/* Set the capture compare value for OC2. */
	timer_set_oc_value(TIM3, TIM_OC2, 1000);
	
	/* Configure global mode of line 1. */
	timer_disable_oc_clear(TIM3, TIM_OC3);
	timer_disable_oc_preload(TIM3, TIM_OC3);
	timer_set_oc_slow_mode(TIM3, TIM_OC3);
	timer_set_oc_mode(TIM3, TIM_OC3, TIM_OCM_PWM1);

	timer_set_oc_polarity_high(TIM3, TIM_OC3);
	timer_set_oc_idle_state_set(TIM3, TIM_OC3);

	/* Set the capture compare value for OC3. */
	timer_set_oc_value(TIM3, TIM_OC3, 1000);
	
	/* Configure global mode of line 1. */
	timer_disable_oc_clear(TIM3, TIM_OC4);
	timer_disable_oc_preload(TIM3, TIM_OC4);
	timer_set_oc_slow_mode(TIM3, TIM_OC4);
	timer_set_oc_mode(TIM3, TIM_OC4, TIM_OCM_PWM1);

	timer_set_oc_polarity_high(TIM3, TIM_OC4);
	timer_set_oc_idle_state_set(TIM3, TIM_OC4);

	/* Set the capture compare value for OC4. */
	timer_set_oc_value(TIM3, TIM_OC4, 1000);
	
	/* Enable outputs. */
	timer_enable_oc_output(TIM3, TIM_OC1);
	timer_enable_oc_output(TIM3, TIM_OC2);
	timer_enable_oc_output(TIM3, TIM_OC3);
	timer_enable_oc_output(TIM3, TIM_OC4);

	/* ---- */
	/* ARR reload enable */
	timer_enable_preload(TIM3);

	/* Counter enable */
	timer_enable_counter(TIM3);

}

int main(void)
{
	int i;
	u16 oc1, oc2, oc3, oc4;

	clock_setup();
	//gpio_setup();
	tim_setup();

	oc1 = 1000;
	oc2 = 1000;
	oc3 = 1000;
	oc4 = 1000;

	while (1) {
		timer_set_oc_value(TIM3, TIM_OC1, oc1);
		oc1+=1;
		if(oc1 > 2000) oc1 = 1000;

		timer_set_oc_value(TIM3, TIM_OC2, oc2);
		oc2+=2;
		if(oc2 > 2000) oc2 = 1000;

		timer_set_oc_value(TIM3, TIM_OC3, oc3);
		oc3+=3;
		if(oc3 > 2000) oc3 = 1000;
		
		timer_set_oc_value(TIM3, TIM_OC4, oc4);
		oc4+=4;
		if(oc4 > 2000) oc4 = 1000;

		for (i = 0; i < 10000; i++)    /* Wait a bit. */
			__asm__("nop");
	}

	return 0;
}
