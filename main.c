/*
Titel: main
Beschreibung:
Autor: Patrick Wintner
GitHub: https://github.com/EternalNothingness/https://github.com/EternalNothingness/DIC-5BHEL-Projekt_1-Serielle-Kommunikation-mit-uC.git.git
Datum der letzten Bearbeitung: 25.01.2021
*/

#include <stdlib.h>
#include <stdint.h>

volatile int adc_interrupt_flag = 0; // dient der Ueberpruefung, ob ein neues Wandelergebnis vorliegt
volatile int usart_interrupt_flag = 0; // dient der Ueberpruefung, ob Daten empfangen worden sind

// ADC-Interrupt-Service-Routine
// ADC boundary start address: 0x40012400
void ADC1_IRQHandler(void) __attribute__((interrupt));
void ADC1_IRQHandler(void){
	uint32_t *adc_isr; // Interrupt And Status Register
	adc_isr = 0x40012400 + 0x00;
	*adc_isr |= 0x00000004; // Clear EOC-flag
	adc_interupt_flag = 1; // Wandelergebnis liegt vor
}

// USART1-Interrupt-Service-Routine
// USART1 boundary start address: 0x40013800
void USART1_IRQHandler(void __attribute__((interrupt));
void USART1_IRQHandler(void){
	uint32_t *usart_rqr; // Request Register
	usart_rqr = 0x40013800 + 0x18;
	*usart_rqr |= 0x00000008; // Clear RXNE-flag
	usart_interrupt_flag = 1; // Daten wurden empfangen
}

int main(){
	// -- boot configuration --
	// Default; Boot von Main Flash Memory (Boot0 = 0)
	// ----------------------------------------

	// -- alternate function configuration --
	// Port A boundary start address: 0x48000000

	uint32_t *gpioA_afrh; // GPIO alternate function high register
	gpioA_afrh = 0x48000000 + 0x24;
	*gpioA_afrh |= 0x00010110; // Auswahl der AF1 (USART1) auf PA9 (TX), PA10 (RX) und PA12 (DE)

	uint32_t *gpioA_ospeedr; // GPIO port output speed register
	gpioA_ospeedr = 0x48000000 + 0x08;
	*gpioA_ospeedr |= 0x030C0000; // Auswahl High Speed auf PA9 (TX) und PA12 (DE)

	uint32_t *gpioA_moder; // GPIO port mode register
	gpioA_moder = 0x48000000 + 0x00;
	*gpioA_moder |= 0x2A280003; // Enable der alternate functions USART1 auf PA9 (TX), PA10 (RX) und PA12(DE) sowie Aktivierung analog function auf PA0 fuer den ADC
	// ----------------------------------------

	// -- clock configuration --
	// RCC boundary start address: 0x4002100

	uint32_t *rcc_cfgr; // Clock configuration register
	rcc_cfgr = 0x4002100 + 0x04;
	*rcc_cfgr |= 0x00280000; // Festlegung des Multiplikationsfaktors der PLL (12 => f=12*8MHz/2=48MHz)

	uint32_t *rcc_cr; // Clock control register
	rcc_cr = 0x4002100 + 0x00;
	*rcc_cr |= 0x01000000; // Enable PLL
	while((*rcc_cr) & 0x02000000) != 0x02000000){ // Warten auf PLLRDY = 1
	}

	*rcc_cfgr |= 0x00000002; // Festlegung der PLL als SYSCLK

	uint32_t *rcc_cfgr3; // Clock configuration register 3
	rcc_cfgr3 = 0x40021000 + 0x30;
	*rcc_cfgr3 |= 0x00000001; // SYSCLK (=PLL) als USART1-CLK ausgewaehlt

	uint32_t *rcc_cr2; // Clock control register 2
	rcc_cr2 = 0x40021000 + 0x34;
	*rcc_cr2 |= 0x00000001; // Enable HSI14
	
	// Konfiguration ADC-Clock
	// Verwendung der Default-Einstellung (async. clock mode -> HSI14-Clock)
	
	uint32_t *rcc_apb2enr; // APB peripheral clock enable register 2
	rcc_apb2enr = 0x40021000 + 0x18;
	*rcc_apb2enr |= 0x00004200 // Enable CLK der USART1 und des ADC
	// ----------------------------------------

	// -- ADC configuration --
	// ADC boundary start adress: 0x40012400

	// Calibration software procedure
	// Ensure that ADEN=0 and DMAEN=0 -> automatisch nach Reset der Fall
	uint32_t *adc_cr; // ADC control register
	adc_cr = 0x40012400 + 0x08;
	*adc_cr |= 0x80000000; // Set ADCAL=1
	while((*adc_cr) & 0x80000000 == 0x80000000){ // Wait until ADCAL=0
	}

	// Enable the ADC
	uint32_t *adc_isr; // ADC interrupt and status register
	adc_isr = 0x040012400 + 0x00;
	*adc_isr |= 0x00000001; // Clear ADRDY bit
	*adc_cr |= 0x00000001; // Set ADEN=1
	while(*adc_isr == 0x00000001){// Wait until ADRDY=1
	}

	// miscellaneous
	uint32_t *adc_cfgr1; // ADC configuration register 1
	adc_cfgr1 = 0x40012400 + 0x0C;
	*adc_cfgr1 |= 0x00003010; // continuous mode, overrun mode (overwrite), data alignment (right), resolution (8 bits)

	// Sampling time selection
	Defaulteinstellungen: sampling time = 1,5 clock cycles

	// Channel Selection
	uint32_t *adc_chselr; // ADC channel selection register
	adc_chselr = 0x40012400 + 0x28;
	*adc_chselr |= 0x00000001; // Channel 0 ausgewaehlt

	// Enable Interrupts
	uint32_t *adc_ier; // ADC interrupt enable register
	adc_ier = 0x40012400 + 0x04;
	*adc_ier |= 0x00000004; // Interrupt bei Vorliegen des Wandelergebnisses

	// Set ADSTART
	*adc_cr |= 0x00000004;
	// ----------------------------------------

	// -- NVIC/Interrupt configuration --
	NVIC_EnableIRQ(12); // Enable ADC-Interrupt
	NVIC_SetPriority(12, 3); // Setzen der Prioritaet

	NVIC_EnableIRQ(27); // Enable USART1-Interrupt
	NVIC_SetPriority(27, 2); // Setzen der Prioritaet
	// ----------------------------------------

	// -- USART1 configuration --
	// USART1 boundary start address: 0x40013800

	// reception procedure
	uint32_t *usart_cr1; // USART control register 1
	usart_cr1 = 0x40013800 + 0x00;
	*usart_cr1 |= 0x1021 0620 // Word length = 9 bits (8 data bits + 1 parity bit), Driver Enable assertion time = 00001, Driver Enable de-assertion time = 00001, oversampling rate = 16, enable parity, ODD parity gewaehlt, Interrupt-Enable fuer RXNE -> Interrupt bei Empfang von Daten

	// miscellaneous
	uint32_t *usart_cr3; // USART control register 3
	usart_cr3 = 0x40013800 + 0x08;
	*usart_cr3 |= 0x00005800 // Disable Overrun-Error, Enable DE-function, one sample bit method

	// Configuration baud rate
	uint32_t *usart_brr; // Baud rate register
	usart_brr = 0x40013800 + 0x0C;
	*usart_brr |= 0x000004E2; // Baud rate = 38400 fuer f=48MHz

	// configuration of the amount of stop bits
	// Default: 1 stop bit

	usart_cr1 |= 0x00000001; // Enable USART1

	usart_cr1 |= 0x00000004; // Set the RE bit for enabling reception
	// ----------------------------------------

	// -- Other Setup Code
	// adc boundary start address: 0x40012400
	// usart boundary start address: 0x40013800

	uint32_t *adc_dr; // ADC data register
	adc_dr = 0x40012400 + 0x40;
	
	uint32 *usart_isr; // USART interrupt and status register
	usart_isr = 0x40013800 + 0x1C;
	uint32 *usart_tdr; // USART transmit data register
	usart_tdr = 0x40013800 + 0x28;

	uint32_t *buf=malloc(10*sizeof(uint32_t)); // 10 Byte (nur die jeweils letzten 8 Bit werden verwendet) Buffer fuer ADC-Wandel-Ergebnisse

	for(int i=0; i<=9; i++){
		*(buf+i) = 0x00; // Reset buffer contents
	}
	// ----------------------------------------
	
	// -- main loop --
	for(;;){
		if(adc_interrupt_flag == 1){
			for(int i=9; i>0; i--){
				*(buf+i) = *(buf+i-1); // shift right
			}
			*buf = *adc_dr; // latest data written at the beginning of the buffer
			*adc_interrupt_flag = 0; // Reset Interrupt-Flag
		}
		if(usart_interrupt_flag == 1){
			*adc_crr |= 0x00000010// Stop ADC to avoid interruptions during transmission
			*usart_cr1 &= 0xFFFFFFFB // Disable Reception
			*usart_cr1 |= 0x00000008 // Enable Transmission
			for(int i=9; i>=0; i--){
				*usart_tdr = *(buf+i); // Older data is sent first
				while((usart_isr & 0x00000080) != 0x00000080){ // Wait until data is transferred to the shift register
				}
			}
			while((usart_isr & 0x00000040) != 0x00000040){ // Wait until transmission complete
			}
			*usart_cr1 &= 0xFFFFFFF7 // Disable Transmission
			*usart_cr1 |= 0x00000004; // Enable Reception
			*adc_cr |= 0x00000004; // Start ADC
			*usart_interrupt_flag = 0; // Reset Interrupt-Flag
		}
	}
}
