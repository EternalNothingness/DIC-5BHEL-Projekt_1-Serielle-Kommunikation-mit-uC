/*
Titel: main
Beschreibung:
Autor: Patrick Wintner
GitHub: https://github.com/EternalNothingness/https://github.com/EternalNothingness/DIC-5BHEL-Projekt_1-Serielle-Kommunikation-mit-uC.git.git
Datum der letzten Bearbeitung: 02.01.2021
*/

#include <stdlib.h>

// -- verwendete Abkuerzungen --
// STMR ... STM32F030F4 Reference Manual

int main(){
	// -- boot configuration --
	// Default; Boot entweder 
	// * von Main Flash Memory (Boot0 = 0) oder
	// * von System Memory (Boot0 = 1)
	// --------------------------------------------------

	// -- alternate function configuration --
	uint32_t *gpioA_afrh; // GPIO alternate function high register (STMR S. 141)
	gpioA_afrh = 0x48000000 + 0x24;
	*gpioA_afrh |= 0x00010110; // Auswahl der AF1 (USART1) auf 
	// * PA9 (TX),
	// * PA10 (RX) und
	// * PA12 (DE)
	// falls half-duplex:
	// *gpioA_afrh |= 0x00010010; // Auswahl der AF1 (USART1) auf
	// * PA9 (TX/RX) (half-duplex)
	// * PA12 (DE)

	uint32_t *gpioA_moder; // GPIO port mode register (STMR S. 136)
	gpioA_moder = 0x48000000 + 0x00;
	*gpioA_moder |= 0x2A280003; // Enable der alternate functions...
	// * USART1 auf PA9(TX),
	// * PA10(RX) und 
	// * PA12(DE) sowie
	// * Aktivierung analog function auf PA0 fuer den ADC
	// falls half-duplex:
	// *gpioA_moder |= 0x2A080003; // Enable der alternate functions...
	// * USART1 auf PA9(TX/RX) (half-duplex) und
	// * PA12(DE) sowie
	// * Aktivierung analog function auf PA0 fuer den ADC

	// falls half-duplex:
	// uint32 *gpioA_otyper; // GPIO port output type register (STMR S. 136)
	// gpioA_otyper = 0x48000000 + 0x04;
	// *gpioA_otyper |= 0x00000200; // open-drain auf PA9 (TX/RX)
	// uint32 *gpioA_pupdr; // GPIO port pull-up/down register (STM S. 137)
	// gpioA_pupdr = 0x48000000 + 0x0C;
	// *gpioA_pupdr = 0x00040000; // PA9 (TX/RX) auf Pull-up (STMR S. 618)
	// --------------------------------------------------

	// -- clock configuration --
	uint32_t *rcc_cfgr; // Clock configuration register (STMR S. 101)
	rcc_cfgr = 0x4002100 + 0x04;
	*rcc_cfgr |= 0x00280000; // Festlegung des Multiplikationsfaktors der PLL (12 => f=12* 8MHz/2)

	uint32_t *rcc_cr; // Clock control register (STMR S. 99)
	rcc_cr = 0x4002100 + 0x00;
	*rcc_cr |= 0x01000083; // Enable PLL
	while((*rcc_cr) && 0x02000000) != 0x02000000){ // Warten auf PLLRDY = 1
	}

	*rcc_cfgr |= 0x00000002; // Festlegung der PLL als SYSCLK

	uint32_t *rcc_cfgr3; // Clock configuration register 3 (S. 123)
	rcc_cfgr3 = 0x40021000 + 0x30;
	*rcc_cfgr3 |= 0x00000001; // SYSCLK (PLL) als USART1-CLK ausgewaehlt

	uint32_t *rcc_cr2; // Clock control register 2 (STMR S. 123)
	rcc_cr2 = 0x40021000 + 0x34;
	*rcc_cr2 |= 0x00000001; // Enable HSI14
	
	uint32 *adc_cfgr2; // ADC configuration register 2 (STMR S. 216)
	adc_cfgr2 = 0x40012400 + 0x10;
	*adc_cfgr2 |= 0x00000000; // Default; HSI14-CLK als ADC-CLK festgelegt
	
	uint32_t *rcc_apb2enr; // APB peripheral clock enable register 2 (STMR S. 112)
	rcc_apb2enr = 0x40021000 + 0x18;
	*rcc_apb2enr |= 0x00004200 // Enable CLK der 
	// * USART1 und des
	// * ADC
	// --------------------------------------------------

	// -- ADC configuration --
	// Calibration software procedure (STMR S. 185)
	// Ensure that ADEN=0 and DMAEN=0 -> automatisch nach Reset der Fall
	uint32_t *adc_cr; // ADC control register STMR S.210
	adc_cr = 0x40012400 + 0x08;
	*adc_cr |= 0x80000000; // Set ADCAL=1
	while((*adc_cr) && 0x80000000 == 0x80000000){ // Wait until ADCAL=0
	}

	// Enable the ADC (STMR S. 185)
	uint32_t *adc_isr; // STMR S. 207
	adc_isr = 0x040012400 + 0x00;
	*adc_isr |= 0x00000001; // Clear ADRDY bit
	*adc_cr |= 0x00000001; // Set ADEN=1
	while(*adc_isr == 0x00000001){// Wait until ADRDY=1
	}

	// miscellaneous
	uint32_t *adc_cfgr1; // ADC configuration register 1 (STMR S.212)
	adc_cfgr1 = 0x40012400 + 0x0C;
	//*adc_cfgr1 |= 0x00003010;
	*adc_cfgr1 |= 0x00002000; // continuous mode
	*adc_cfgr1 |= 0x00001000; // overrun mode (overwrite)
	*adc_cfgr1 |= 0x00000000; // data alignment (right)
	*adc_cfgr1 |= 0x00000010; // resolution (8 bits)

	// Sampling time selection
	uint32_t *adc_smpr; // ADC sampling time register (STMR S. 216)
	adc_smpr = 0x0x40012400 + 0x14;
	*adc_smpr |= 0x00000000; // Default; sampling time = 1,5 clock cycles

	// Channel Selection
	uint32_t *adc_chselr; // ADC channel selection register (STMR S. 218)
	adc_chselr = 0x40012400 + 0x28;
	*adc_chselr |= 0x00000001; // Channel 0 ausgewaehlt

	// Enable Interrupts
	uint32_t *adc_ier; // ADC interrupt enable register (STMR S. 208)
	adc_ier = 0x40012400 + 0x04;
	*adc_ier |= 0x00000004; // Interrupt bei Vorliegen des Wandelergebnisses

	// Setting ADSTART
	*adc_cr |= 0x00000004;
	// --------------------------------------------------

	// -- NVIC/Interrupt configuration --
	uint32_t *usart1_interrupt; // Vector table USART1 entry (STMR S. 171)
	usart1_interrupt = 0x000000AC; // location of USART1 entry in vector table
	*usart1_interrupt = 0x???; // SETZEN der Startadresse der ISR; !!!UNGERADE!!!
	NVIC_EnableIRQ(27); // Enable USART1-Interrupt
	NVIC_SetPriority(27, 2); // Setzen der Prioritaet

	uint32_t *adc_interrupt; // Vector table ADC entry (STMR S. 171)
	adc_interrupt = 0x00000070; // location of ADC entry in vector table
	*adc_interrupt = 0x???; // SETZEN der Stardadresse der ISR !!!UNGERADE!!!
	NVIC_EnableIRQ(19); // Enable USART1-Interrupt
	NVIC_SetPriority(19, 3); // Setzen der Prioritaet
	// --------------------------------------------------

	// -- USART1 configuration --
	// reception procedure (STMR S. 605)
	uint32_t *usart_cr1; // USART control register 1 (STMR S. 625)
	usart_cr1 = 0x40013800 + 0x00;
	*usart_cr1 |= 0x00000000; // Word length = 8 Bits (Default)
	*usart_cr1 |= 0x00??0000; // Drive Enable assertion time = ???
	*usart_cr1 |= 0x00??0000; // Drive Enable de-assertion time = ???
	*usart_cr1 |= 0x00000000; // oversampling rate 16
	*usart_cr1 |= 0x00000400; // enable parity
	*usart_cr1 |= 0x00000200; // ODD parity gewaehlt
	*usart_cr1 |= 0x00000020; // Interrupt bei Empfang von Daten

	uint32_t *usart_cr3; // USART control register 3 (STMR S. 630)
	usart_cr3 = 0x40013800 + 0x08;
	*usart_cr3 |= 0x00005800 // Disable Overrun-Error, Enable DE-function, one sample bit method
	// falls half-duplex:
	// *usart_cr3 |= 0x00005808
	// * Disable Overrun-Error, 
	// * Enable DE-function,
	// * one sample bit method,
	// * half-duplex-mode

	uint32_t *usart_brr; // Baud rate register (STMR S. 633)
	usart_brr = 0x40013800 + 0x0C;
	*usart_brr |= 0x000004E2; // Baud rate = 38400 fuer f=48MHz

	uint32_t *usart_cr2; // USART control register 2 (STMR S. 628)
	usart_cr2 = 0x40013800 + 0x04;
	*usart_cr2 |= 0x00000000; // Default; 1 STOP bit

	usart_cr1 |= 0x00000001; // Enable USART1

	usart_cr1 |= 0x00000004; // Set the RE bit for enabling reception
	// --------------------------------------------------

	uint32_t *adc_isr; // ADC interrupt and status register (STMR S. 207)
	adc_isr = 0x40012400 + 0x00;
	uint32_t *adc_dr; // ADC data register (STMR S. 218)
	adc_dr = 0x40012400 + 0x40;
	
	uint32 *usart_isr; // USART interrupt and status register (STM S.635)
	usart_isr = 0x40013800 + 0x1C;
	uint32 *usart_tdr; // USART transmit data register (STMR S. 640)
	usart_tdr = 0x40013800 + 0x28;

	uint32_t *buf=malloc(10*sizeof(uint32_t)); // 10 Byte (nur die jeweils letzten 8 Bit werden verwendet) Buffer fuer ADC

	for(int i=0; i<=9; i++){
		*(buf+i) = 0x00; // Reset Buffer
	}

	// Deklaration der Interrupt-Flags
	volatile uint8_t *adc_interrupt_flag;
	adc_interrupt_flag = 0x???; // Adresse im SRAM
	*adc_interrupt_flag = 0x00;
	volatile uint8_t *usart_interrupt_flag;
	usart_interrupt_flag = 0x???; // Adresse im SRAM
	*usart_interrupt_flag = 0x00;
	
	// main loop
	for(;;){
		if(*adc_interrupt_flag == 0x01){
			for(int i=9; i>0; i--){
				*(buf+i) = *(buf+i-1); // shift right
			}
			*buf = *adc_dr;
			*adc_interrupt_flag = 0x00; // Reset Interrupt-Flag
		}
		if(*usart_interrupt_flag == 0x01){
			*adc_crr |= 0x00000010// Stop ADC to avoid interruptions during transmission
			*usart_cr1 &= 0xFFFFFFFB // Disable Reception
			*usart_cr1 |= 0x00000008 // Enable Transmission
			for(int i=9; i>=0; i--){
				*usart_tdr = *(buf+i); // Aeltere Daten werden zuerst gesendet
				while((usart_isr && 0x00000040) != 0x00000040){ // Wait until transmission complete
				}
			}
			*usart_cr1 &= 0xFFFFFFF7 // Disable Transmission
			*usart_cr1 |= 0x00000004; // Enable Reception
			*adc_cr |= 0x00000004; // Start ADC
			*usart_interrupt_flag = 0x00; // Reset Interrupt-Flag
		}
	}
}
