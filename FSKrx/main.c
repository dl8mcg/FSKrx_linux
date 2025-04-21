/*
   by dl8mcg January to April 2025
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <termios.h>
#include <fcntl.h>
#include <stdint.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "config.h"
#include <unistd.h>

#include "fsk_demod.h"
#include "fsk_decode_rtty.h"
#include "sampleprocessing.h"
#include "fsk_decode_ascii.h"
#include "fsk_decode_ax25.h"

#include "buffer.h"


struct termios orig_termios;

void disable_raw_mode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode()
{
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode); // automatische Wiederherstellung bei Programmende

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);  // kein Line-Buffering, kein Echo
    raw.c_cc[VMIN] = 0;   // keine Mindestanzahl von Zeichen für read()
    raw.c_cc[VTIME] = 1;  // Timeout: 0.1 Sekunden (optional)

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int kbhit()
{
    int oldf = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    int ch = getchar();

    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}


int main()
{
    setlocale(LC_ALL, "de_DE.UTF-8");

    enable_raw_mode();

    printf("                      RYTL TYTL                                       by dl8mcg 2025\n\n");

    printf("Mit 1, 2, 3, 4, 5 oder 6 den Modus auszuwählen              Mit 8 das Programm beenden\n");

    initialize_audiostream();

    init_fsk_demod(FSK_RTTY_45_BAUD_170Hz);

    while (1)
    {
        if (kbhit())
        {
            int key = getchar(); // Erstes Zeichen lesen

                switch (key)
                {
                    case 0x31: // 1
                        init_fsk_demod(FSK_RTTY_45_BAUD_170Hz);
                        break;
                    case 0x32: // 2
                        init_fsk_demod(FSK_RTTY_50_BAUD_85Hz);
                        break;
                    case 0x33: // 3
                        init_fsk_demod(FSK_RTTY_50_BAUD_450Hz);
                        break;
                    case 0x34: // 4
                        init_fsk_demod(FSK_EFR_200_BAUD_340Hz);
                        break;
                    case 0x35: // 5
                        init_fsk_demod(FSK_ASCII_300_BAUD_850Hz);
                        break;
                    case 0x36: // 6
                        init_fsk_demod(FSK_AX25_1200_BAUD_1000Hz);
                        break;
                    case 0x38: // 8
                        printf("\n\nProgramm beendet.\n\n");
                        stop_audiostream();
                        printf("73\n");
                        usleep(1000 * 1000);
                        return 0;
                    default:
                            ;
                }
        }

        char value;
        if (readbuf(&value))
        {
            printf("%c", value);
        }
    }
}

