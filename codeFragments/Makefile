CC				= gcc
CFLAGS		= -O4 -Wall -I/usr/local/include
LDFLAGS		= -L/usr/local/lib
LIBS			= -lpigpio -pthread -lm
OBJS		= PFCLastError.o PFCRGBColor.o
WAVEOBJ	= PFCWaveDriver.o
SPIOBJ		=  PFCSPIDriver.o
WAVE		= pfcl
SPI			= pfcls

all:	$(WAVE) $(SPI)

$(WAVE):	mainWave.c $(OBJS) $(WAVEOBJ)
	$(CC) mainWave.c $(OBJS) $(WAVEOBJ) $(LDFLAGS) $(LIBS) -o $(WAVE)

$(SPI):	mainSPI.c $(OBJS) $(SPIOBJ)
	$(CC) mainSPI.c $(OBJS) $(SPIOBJ) $(LDFLAGS) $(LIBS) -o $(WAVE)

clean:;	rm -f *.o *~ $(PROGRAM)
