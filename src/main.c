#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

#define GREEN 0b00100000
#define RED   0b01000000
#define BUZZER 0b10000000

volatile uint32_t millis = 0;

ISR(TIMER1_COMPA_vect)
{
    millis++;
}


void timer1_init()
{
    // WGM12 = bit 3 of TCCR1B
    // TCCR1B = 00001000
    TCCR1B |= 0b00001000;

    OCR1A = 249;

    // OCIE1A = bit 1 of TIMSK1
    // TIMSK1 = 00000010
    TIMSK1 |= 0b00000010;

    // CS11 and CS10 = bits 1 and 0
    // Prescaler = 64
    // TCCR1B = 00000011
    TCCR1B |= 0b00000011;

    sei();
}


uint32_t getMillis()
{
    uint32_t t;

    cli();

    t = millis;

    sei();

    return t;
}


void gpio_init()
{
    // PD5, PD6, PD7 outputs
    // DDRD = 11100000
    DDRD |= 0b11100000;


    // PB0, PB1, PB2, PB3 inputs
    // Clear lower 4 bits
    DDRB &= 0b11110000;


    // Enable internal pull-ups on PB0-PB3
    PORTB |= 0b00001111;
}


uint8_t readButton()
{
    // PB0
    if(!(PINB & 0b00000001))
        return 1;

    // PB1
    if(!(PINB & 0b00000010))
        return 2;

    // PB2
    if(!(PINB & 0b00000100))
        return 3;

    // PB3
    if(!(PINB & 0b00001000))
        return 4;


    return 0;
}


int main()
{
    gpio_init();
    timer1_init();


    uint8_t password[4]={2,4,1,3};

    uint8_t entered[4];

    uint8_t index=0;

    uint8_t wrongAttempts=0;

    uint8_t locked=0;

    uint32_t lockStart=0;


    while(1)
    {

        if(locked)
        {
            if(getMillis()-lockStart>=30000)
            {
                locked=0;
                wrongAttempts=0;
            }
            else
                continue;
        }


        uint8_t b=readButton();


        if(b)
        {

            entered[index++]=b;


            while(readButton());


            if(index==4)
            {

                uint8_t correct=1;


                for(uint8_t i=0;i<4;i++)
                {
                    if(password[i]!=entered[i])
                    {
                        correct=0;
                        break;
                    }
                }


                if(correct)
                {

                    // Green ON
                    PORTD |= 0b00100000;


                    uint32_t t=getMillis();

                    while(getMillis()-t<3000);


                    // Green OFF
                    PORTD &= 0b11011111;


                    wrongAttempts=0;

                }


                else
                {

                    wrongAttempts++;


                    for(uint8_t i=0;i<5;i++)
                    {

                        // Red ON
                        PORTD |= 0b01000000;


                        // Buzzer ON
                        PORTD |= 0b10000000;


                        uint32_t t=getMillis();


                        while(getMillis()-t<300);


                        // Red OFF
                        PORTD &= 0b10111111;


                        // Buzzer OFF
                        PORTD &= 0b01111111;


                        t=getMillis();


                        while(getMillis()-t<300);

                    }


                    if(wrongAttempts>=3)
                    {
                        locked=1;

                        lockStart=getMillis();
                    }

                }


                index=0;

            }

        }
    }
}