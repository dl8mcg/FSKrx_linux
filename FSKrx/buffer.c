/*
   by dl8mcg January to April 2025       ASCII - decode
*/

#include "buffer.h"

#define RINGBUFFER_SIZE 256  // Gr��e des Buffers 

char ringbuffer[RINGBUFFER_SIZE];
volatile uint16_t write_index = 0;  // Schreibzeiger 
volatile uint16_t read_index = 0;   // Lesezeiger 

void writebuf(char val)
{
    // Schreibe das Zeichen in den Ringbuffer
    uint16_t next_write_index = (write_index + 1) % RINGBUFFER_SIZE;

    // �berpr�fe, ob der Ringbuffer voll ist
    if (next_write_index != read_index)
    {
        ringbuffer[write_index] = val; 
        write_index = next_write_index;        
    }

}

bool readbuf(char* val)
{
    // Pr�fe, ob Daten im Ringbuffer vorhanden sind
    if (read_index != write_index)
    {
        // Zeichen aus dem Ringbuffer lesen
        *val = ringbuffer[read_index];
        read_index = (read_index + 1) % RINGBUFFER_SIZE;
        return true;
    }
    return false;
}