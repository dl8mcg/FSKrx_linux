/*
   by dl8mcg January to April 2025       2FSK AX25 - Decoder frame detection
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "fsk_demod.h"
#include "config.h"
#include "buffer.h"
//#include <windows.h>

// state machine frame
static void stateframe0();
static void stateframe1();
static void stateframe2();
static void (*smAX25)() = stateframe0;       // function pointer for state machine

// state machine content
static void statecontent0();
static void statecontent1();
static void statecontent2();
static void statecontent3();
static void (*smContent)() = statecontent0;       // function pointer for state machine

static uint8_t rxbyte = 0;              // shift-in register
static uint8_t oldbit = 0;              // old bit
static uint8_t bitcnt = 0;              // bit counter
static uint8_t onecnt = 0;              // counter for one-bits
static bool rxbit = false;              // received bit

static uint32_t rxword = 0;

static int stopcnt = 0;


void process_ax25(uint8_t bit)
{
    if (bit == oldbit)                  // differential decoding: no change = 1, change = 0
        rxbit = 1;
    else
        rxbit = 0;
    oldbit = bit;


    if ((onecnt == 5) && (rxbit == 0))
    {
        //printf("X");
        onecnt = 0;
        return;
    }

    rxbyte = (rxbyte >> 1) | (rxbit << 7);     // shift in new rxbit to MSB

    if (rxbit == 1)
    {
        onecnt++;
    }
    else
    {
        onecnt = 0;
    }
    if (onecnt > 7)                      // error when more than 7 one-bits
    {
        //printf(".");
        onecnt = 0;
        smAX25 = stateframe0;
        return;
    }

    smAX25();
}

void stateframe0()
{
    //OutputDebugStringA("s0 ");
    /*if (rxbyte == 0x7E)                 // start flag detected
    {
        //printf("S");
        bitcnt = 0;
        smAX25 = stateframe1;
    }*/

    rxword = (rxword << 1) | (rxbit);     // shift in new rxbit to LSB

    if (rxword == 0x7E7E7E7E)               // start flag detected
    {
        //printf("S");            
        bitcnt = 0;
        smAX25 = stateframe1;
    }

}

void stateframe1()
{
    //OutputDebugStringA("s1 ");
    bitcnt++;

    if (rxbyte == 0x7E)                 // an additional start flag detected
    {
        //printf("W");            
        bitcnt = 0;
        return;                         // Bleibe in state1
    }

    if (bitcnt < 8)                     // wait for 8 data bits
        return;

    bitcnt = 0;                         // reset bit-counter

    //printf("Z");                      // first payload databyte detected
    //callcnt = 0;
    smContent = statecontent0;
    smContent();

    smAX25 = stateframe2;
}

void stateframe2()
{
    //OutputDebugStringA("s2 ");
    bitcnt++;

    if (rxbyte == 0x7E)                 // stop flag detected
    {
        
        //printf("E");                  // End-Flag erkannt
        bitcnt = 0;                     // reset bit-counter
        smAX25 = stateframe1;           // back to state1
        return;
    }

    if (bitcnt == 8)                    // payload databyte received
    {
        bitcnt = 0;                     // reset bit-counter
        //printf("z");                    
        smContent();
    }
}


void statecontent0()
{
    //OutputDebugStringA("c0 ");
    //printf("%c", rxbyte >> 1);
    writebuf(rxbyte >> 1);

    if (rxbyte & 0x01)
    {
        smContent = statecontent1;
        return;
    }
}

void statecontent1()
{
    //OutputDebugStringA("c1 ");
    //printf("CTRL : %02X ", rxbyte);

    if (!(rxbyte & 0x01))
    {
        //printf("CTRL : I ");
        smContent = statecontent2;          // I-frame
        return;
    }
    if ((rxbyte & 0x03) == 0x01)
    {
        //printf("CTRL : S ");
        smContent = statecontent3;          // S-frame
        return;
    }
    //printf("CTRL : U ");
    smContent = statecontent3;              // U-frame
    return;
}

void statecontent2()
{
    //OutputDebugStringA("c2 ");
    //printf("PID : %02X ", rxbyte);
    smContent = statecontent3;
    return;
}


void statecontent3()
{
    //OutputDebugStringA("c3 ");
    //printf("%c", rxbyte);
    writebuf(rxbyte);
    smContent = statecontent3;
    return;
}


