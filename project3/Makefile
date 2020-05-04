CPU             	= msp430g2553
CFLAGS          	= -mmcu=${CPU} -Os -I../h
LDFLAGS			= -L../lib -L/opt/ti/msp430_gcc/include/

#switch the compiler (for the internal make rules)
CC              = msp430-elf-gcc
AS              = msp430-elf-as -mmcu=${CPU} -c

all:shapemotion.elf

#additional rules for files
shapemotion.elf: buzzer.o wdt_handler.o main.o
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ -lTimer -lLcd -lShape -lCircle -lp2sw

load: shapemotion.elf
	mspdebug rf2500 "prog $^"

clean:
	rm -f *.o *.elf
