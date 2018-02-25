/*
@file    FT8_config.h
@brief   configuration information for some TFTs and some pre-defined colors
@version 3.2
@date    2017-05-02
@author  Rudolph Riedel

@section History

2.2
- added prototypes for hardware abstraction funtions
- added hardware-selection defines, first implemented for AVR8
- added FT8_VM800B50A TFT setup as copy of FT8_VM800B43A since FTDI uses the same setup for both

2.3
- moved pin definitions for FT8_CS and FR_PDN to here from FT8_config.c
- added a set of definitions for Arduino

2.4
- switched from custom AVR8 to standard __AVR__ symbol to automatically generate plattform specific code
- removed the definition of the ARDUINO symbol at it already is defined by the Arduino enviroment

2.5
- added support for RH850
- switched to standard-C compliant comment-style
- added FT8_HAS_CRYSTAL to make FT8_init() automatically use the correct option

3.0
- renamed from FT800_config.h to FT8_config.h
- changed FT_ prefixes to FT8_
- changed ft800_ prefixes to FT8_

3.1
- added "#if defined (__ESP8266__)" for the Arduino side, now empty

3.2
- added config for FT811CB_HY50HD

*/

#ifndef FT8_CONFIG_H_
#define FT8_CONFIG_H_

/* switch over to FT81x */
#define FT8_81X_ENABLE


/* select the settings for the TFT attached */
#if 0
    #define FT8_VM800B35A
    #define FT8_VM800B43A
    #define FT8_VM800B50A
    #define FT8_FT810CB_HY50HD
    #define FT8_FT811CB_HY50HD
    #define FT8_ET07
    #define FT8_RVT70AQ
#endif

#define FT8_FT811CB_HY50HD


/* some pre-definded colors */
#define RED     0xff0000UL
#define ORANGE  0xffa500UL
#define GREEN   0x00ff00UL
#define BLUE    0x0000ffUL
#define YELLOW  0xffff00UL
/*#define PINK  0xff00ffUL*/
#define PURPLE  0x800080UL
#define WHITE   0xffffffUL
#define BLACK   0x000000UL


#ifndef ARDUINO
    #if defined (__GNUC__)
        #if defined (__AVR__)

            #include <avr/io.h>
            #include <avr/pgmspace.h>
            #define F_CPU 16000000UL
            #include <util/delay.h>

            #define DELAY_MS(ms) _delay_ms(ms)

            #define FT8_CS_PORT PORTB
            #define FT8_CS      (1<<PB5)
            #define FT8_PDN_PORT    PORTB
            #define FT8_PDN     (1<<PB4)

        #endif

        #if defined (__v851__)

            #include <stdint.h>
            #include "rh850_regs.h"
            #include "os.h"

            #define DELAY_MS(ms)    OS_Wait(ms * 1000)  

        #endif

    #endif
#endif

#ifdef ARDUINO
    #include <stdio.h>
    #include <SPI.h>

    #define FT8_CS      PB12
    #define FT8_PDN     PA8

    #define DELAY_MS(ms) delay(ms)

    #if defined (__ESP8266__)
    
    #endif
    
    #if defined (__AVR__)
        #include <avr/pgmspace.h>
    #endif

#endif


void FT8_pdn_set(void);
void FT8_pdn_clear(void);
void FT8_cs_set(void);
void FT8_cs_clear(void);
void spi_transmit(uint8_t data);
uint8_t spi_receive(uint8_t data);
uint8_t fetch_flash_byte(const uint8_t *data);


/* VM800B35A: FT800 320x240 3.5" FTDI */
#ifdef FT8_VM800B35A
#define FT8_VSYNC0  (0L)    /* Tvf Vertical Front Porch */
#define FT8_VSYNC1  (2L)    /* Tvf + Tvp Vertical Front Porch plus Vsync Pulse width */
#define FT8_VOFFSET (13L)   /* Tvf + Tvp + Tvb Number of non-visible lines (in lines) */
#define FT8_VCYCLE  (263L)  /* Tv Total number of lines (visible and non-visible) (in lines) */
#define FT8_VSIZE   (240L)  /* Tvd Number of visible lines (in lines) - display height */
#define FT8_HSYNC0  (0L)    /* Thf Horizontal Front Porch */
#define FT8_HSYNC1  (10L)   /* Thf + Thp Horizontal Front Porch plus Hsync Pulse width */
#define FT8_HOFFSET     (70L)   /* Thf + Thp + Thb Length of non-visible part of line (in PCLK cycles) */
#define FT8_HCYCLE  (408L)  /* Th Total length of line (visible and non-visible) (in PCLKs) */
#define FT8_HSIZE   (320L)  /* Thd Length of visible part of line (in PCLKs) - display width */
#define FT8_PCLKPOL     (0L)    /* PCLK polarity (0 = rising edge, 1 = falling edge) */
#define FT8_SWIZZLE     (2L)    /* Defines the arrangement of the RGB pins of the FT800 */
#define FT8_PCLK        (8L)    /* 48MHz / REG_PCLK = PCLK frequency */
#define FT8_TOUCH_RZTHRESH (1200L)  /* touch-sensitivity */
#define FT8_HAS_CRYSTAL 1   /* use external crystal or internal oscillator? */
#endif

/* VM800B43A: FT800 480x272 4.3" FTDI */
#ifdef FT8_VM800B43A
#define FT8_VSYNC0  (0L)
#define FT8_VSYNC1  (10L)
#define FT8_VOFFSET (12L)
#define FT8_VCYCLE  (292L)
#define FT8_VSIZE   (272L)
#define FT8_HSYNC0  (0L)
#define FT8_HSYNC1  (41L)
#define FT8_HOFFSET     (43L)
#define FT8_HCYCLE  (548L)
#define FT8_HSIZE   (480L)
#define FT8_PCLKPOL     (1L)
#define FT8_SWIZZLE     (0L)
#define FT8_PCLK        (5L)
#define FT8_TOUCH_RZTHRESH (1200L)
#define FT8_HAS_CRYSTAL 1
#endif

/* VM800B50A: FT800 480x272 5.0" FTDI */
#ifdef FT8_VM800B50A
#define FT8_VSYNC0  (0L)
#define FT8_VSYNC1  (10L)
#define FT8_VOFFSET (12L)
#define FT8_VCYCLE  (292L)
#define FT8_VSIZE   (272L)
#define FT8_HSYNC0  (0L)
#define FT8_HSYNC1  (41L)
#define FT8_HOFFSET     (43L)
#define FT8_HCYCLE  (548L)
#define FT8_HSIZE   (480L)
#define FT8_PCLKPOL     (1L)
#define FT8_SWIZZLE     (0L)
#define FT8_PCLK        (5L)
#define FT8_TOUCH_RZTHRESH (1200L)
#define FT8_HAS_CRYSTAL 1
#endif

/* FT810CB-HY50HD: FT810 800x480 5" HAOYU */
#ifdef FT8_FT810CB_HY50HD
#define FT8_VSYNC0  (0L)
#define FT8_VSYNC1  (2L)
#define FT8_VOFFSET (13L)
#define FT8_VCYCLE  (525L)
#define FT8_VSIZE   (480L)
#define FT8_HSYNC0  (0L)
#define FT8_HSYNC1  (20L)
#define FT8_HOFFSET     (64L)
#define FT8_HCYCLE  (952L)
#define FT8_HSIZE   (800L)
#define FT8_PCLKPOL     (1L)
#define FT8_SWIZZLE     (0L)
#define FT8_PCLK        (2L)
#define FT8_TOUCH_RZTHRESH (2000L)  /* touch-sensitivity */
#define FT8_HAS_CRYSTAL 1
#endif

/* FT811CB-HY50HD: FT811 800x480 5" HAOYU */
#ifdef FT8_FT811CB_HY50HD
#define FT8_VSYNC0  (0L)
#define FT8_VSYNC1  (2L)
#define FT8_VOFFSET (13L)
#define FT8_VCYCLE  (525L)
#define FT8_VSIZE   (480L)
#define FT8_HSYNC0  (0L)
#define FT8_HSYNC1  (20L)
#define FT8_HOFFSET     (64L)
#define FT8_HCYCLE  (952L)
#define FT8_HSIZE   (800L)
#define FT8_PCLKPOL     (1L)
#define FT8_SWIZZLE     (0L)
#define FT8_PCLK        (2L)
#define FT8_TOUCH_RZTHRESH (1200L)  /* touch-sensitivity */
#define FT8_HAS_CRYSTAL 1
#endif

/* some test setup */
#ifdef FT8_800x480x
#define FT8_VSYNC0  (0L) /* Tvf Vertical Front Porch */
#define FT8_VSYNC1  (10L) /* Tvf + Tvp Vertical Front Porch plus Vsync Pulse width */
#define FT8_VOFFSET (35L) /* Tvf + Tvp + Tvb Number of non-visible lines (in lines) */
#define FT8_VCYCLE  (516L)  /* Tv Total number of lines (visible and non-visible) (in lines) */
#define FT8_VSIZE   (480L)  /* Tvd Number of visible lines (in lines) - display height */
#define FT8_HSYNC0  (0L) /* (40L)   // Thf Horizontal Front Porch */
#define FT8_HSYNC1  (88L)   /* Thf + Thp Horizontal Front Porch plus Hsync Pulse width */
#define FT8_HOFFSET     (169L) /* Thf + Thp + Thb Length of non-visible part of line (in PCLK cycles) */
#define FT8_HCYCLE  (969L) /* Th Total length of line (visible and non-visible) (in PCLKs) */
#define FT8_HSIZE   (800L)  /* Thd Length of visible part of line (in PCLKs) - display width */
#define FT8_PCLKPOL     (1L)    /* PCLK polarity (0 = rising edge, 1 = falling edge) */
#define FT8_SWIZZLE     (0L)    /* Defines the arrangement of the RGB pins of the FT800 */
#define FT8_PCLK        (2L)    /* 60MHz / REG_PCLK = PCLK frequency    30 MHz */
#define FT8_TOUCH_RZTHRESH (1200L)  /* touch-sensitivity */
#define FT8_HAS_CRYSTAL 1
#endif

/* G-ET0700G0DM6 800x480 7" Glyn, untested */
#ifdef FT8_ET07
#define FT8_VSYNC0  (0L)
#define FT8_VSYNC1  (2L)
#define FT8_VOFFSET (35L)
#define FT8_VCYCLE  (525L)
#define FT8_VSIZE   (480L)
#define FT8_HSYNC0  (0L)
#define FT8_HSYNC1  (128L)
#define FT8_HOFFSET     (203L)
#define FT8_HCYCLE  (1056L)
#define FT8_HSIZE   (800L)
#define FT8_PCLKPOL     (1L)
#define FT8_SWIZZLE     (0L)
#define FT8_PCLK        (2L)
#define FT8_TOUCH_RZTHRESH (1200L)
#define FT8_HAS_CRYSTAL 0   /* no idea if these come with a crystal populated or not */
#endif

/* RVT70AQxxxxxx 800x480 7" Riverdi, various options, FT812/FT813, tested with RVT70UQFNWC0x */
#ifdef FT8_RVT70AQ
#define FT8_VSYNC0  (0L)    /* Tvf Vertical Front Porch */
#define FT8_VSYNC1  (10L)   /* Tvf + Tvp Vertical Front Porch plus Vsync Pulse width */
#define FT8_VOFFSET (23L)   /* Tvf + Tvp + Tvb Number of non-visible lines (in lines) */
#define FT8_VCYCLE  (525L)  /* Tv Total number of lines (visible and non-visible) (in lines) */
#define FT8_VSIZE   (480L)  /* Tvd Number of visible lines (in lines) - display height */
#define FT8_HSYNC0  (0L)    /* Thf Horizontal Front Porch */
#define FT8_HSYNC1  (10L)   /* Thf + Thp Horizontal Front Porch plus Hsync Pulse width */
#define FT8_HOFFSET     (46L)   /* Thf + Thp + Thb Length of non-visible part of line (in PCLK cycles) */
#define FT8_HCYCLE  (1056L) /* Th Total length of line (visible and non-visible) (in PCLKs) */
#define FT8_HSIZE   (800L)  /* Thd Length of visible part of line (in PCLKs) - display width */
#define FT8_PCLKPOL     (1L)    /* PCLK polarity (0 = rising edge, 1 = falling edge) */
#define FT8_SWIZZLE     (0L)    /* Defines the arrangement of the RGB pins of the FT800 */
#define FT8_PCLK        (2L)    /* 60MHz / REG_PCLK = PCLK frequency 30 MHz */
#define FT8_TOUCH_RZTHRESH (1800L)  /* touch-sensitivity */
#define FT8_HAS_CRYSTAL 0
#endif

#endif /* FT8_CONFIG_H */
