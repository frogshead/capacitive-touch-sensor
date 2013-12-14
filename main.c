/*
*  Capacitive code is copied from:
* 	http://tuomasnylund.fi/drupal6/content/capacitive-touch-sensing-avr-and-single-adc-pin
*/
#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>


#include <util/delay.h>

#define MAX 5
#define MIN 1
#define true 1
#define false 0

/** Holds information related to a single touch channel */
typedef struct {
    volatile uint8_t *port;    //PORTx register for pin
    volatile uint8_t portmask; //mask for the bit in port
    volatile uint8_t mux;      //ADMUX value for the channel
} touch_channel_t;
 
/**Initializing the touch sensing */
void touch_init(void);
 
/**Doing a measurement */
uint16_t touch_measure(touch_channel_t *channel);


void touch_init(void){
    ADMUX  |= (1<<REFS0); //reference AVCC (5v)
 
    ADCSRA |= (1<<ADPS2)|(1<<ADPS1); //clockiv 64
    //final clock 8MHz/64 = 125kHz
 
    ADCSRA |= (1<<ADEN); //enable ADC
}

static inline void adc_channel(uint8_t channel){
    ADMUX &= ~(0b11111);
    ADMUX |=   0b11111 & channel;
}
 
static inline uint16_t adc_get(void){
    ADCSRA |= (1<<ADSC); //start conversion
    while(!(ADCSRA & (1<<ADIF))); //wait for conversion to finish
    ADCSRA |= (1<<ADIF); //reset the flag
    return ADC; //return value
}
uint16_t touch_measure(touch_channel_t *channel){
    uint8_t i;
    uint16_t retval;
 
    retval = 0;
 
    //Do four measurements and average, just to smooth things out
    for (i=0 ; i<4 ; i++){
        *(channel->port) |= channel->portmask;    // set pullup on
        _delay_ms(1);                             // wait (could probably be shorter)
        *(channel->port) &= ~(channel->portmask); // set pullup off
 
        adc_channel(0b11111); //set ADC mux to ground;
        adc_get();            //do a measurement (to discharge the sampling cap)
 
        adc_channel(channel->mux); //set mux to right channel
        retval +=  adc_get(); //do a conversion
    }
    return retval /4;
    
}

static touch_channel_t btn1 = {
    .mux = 0,
    .port = &PORTB,
    .portmask = (1<<PB5),
};
/* Ports PB0 (pin5) and PB1 Can be used as pwm outputs because no ADC channels */
void pwm_init(void){
	OCR0A = 51; // set pulse width to 20% pin 5
	OCR1A = 3*51; // 80% pin 6 --> let this be fixed value and can be measured from pin
}
/*Set pulse width percentage 0-100%  */
void pwm_width(uint8_t p){
    if( p >= 0 && p < 100)
        OCR0A = (uint8_t)((p/100) * 255);
}
 
int main(void){
    //uint16_t sample;
    uint8_t step = MIN;
    uint8_t going_up = true;
    touch_init();
    pwm_init();
    for(;;){
        if( touch_measure(&btn1) < (1023 /2) )
        {
		
			if(step == MAX && going_up == true)
			{
			  step--;
			  going_up = false;
			  pwm_width(20 * step);
                        }			}
			else if(step == MIN && going_up == false)
			{
			  step++;
			  going_up = true;
                          pwm_width(20 * step);
			 }
			else if(going_up == true)
			{
			  step++;
                          pwm_width(20 * step);
			 }
			else if(going_up == false)
			{
			  step--;
                          pwm_width(20 * step);
			 }
			
			_delay_ms(500);
	  }
 
    }
    return 0;
}
