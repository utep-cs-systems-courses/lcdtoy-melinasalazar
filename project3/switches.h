#ifndef switches_included
#define switches_included

#define SW1 BIT0                    //switch one is S1
#define SW2 BIT1                    //switch two is S2
#define SW3 BIT2                    //switch three is S3
#define SW4 BIT3                    //switch four is S4
#define SWITCHES (SW1|SW2|SW3|SW4)  //initialize all switches


void switch_init();
void switch_interrupt_handler();

extern char switch_state_down, switch_state_changed, tempo; /* effectively boolean */

#endif // included
