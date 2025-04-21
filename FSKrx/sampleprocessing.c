/*
   by dl8mcg January to April 2025
*/

#define MINIAUDIO_IMPLEMENTATION        // damit auch der Programmcode von miniaudio eingebunden wird
#include "miniaudio.h"
#include <stdio.h>
#include <locale.h>
//#include <windows.h>
#include "config.h"
#include "fsk_demod.h"

ma_result result;
ma_device_config deviceConfig;
ma_device device;

void audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) 
{
    if (pInput == NULL || pOutput == NULL) 
    {
        return;
    }

    const float* input = (const float*)pInput;
    float* output = (float*)pOutput;

    for (ma_uint32 i = 0; i < frameCount; i++) 
    {
        output[i * 2] = input[i];               // Mikrofon auf Lautsprecher ausgeben
        output[i * 2 + 1] = input[i];
       process_fsk_demodulation(input[i]);     // FSK-Demodulation                               <--- wieder aktivieren
    }
}

int initialize_audiostream()
{
    // Konfiguration des Audio-Ger�ts
    deviceConfig = ma_device_config_init(ma_device_type_duplex);
    deviceConfig.sampleRate = (ma_uint32)SAMPLING_RATE; 
    deviceConfig.playback.format = ma_format_f32;           // Ausgabe: 32-Bit Float
    deviceConfig.playback.channels = 2;                     // Stereo-Ausgabe
    deviceConfig.capture.format = ma_format_f32;            // Eingabe: 32-Bit Float
    deviceConfig.capture.channels = 1;                      // Mono-Eingabe
    deviceConfig.dataCallback = audioCallback;              // Callback-Funktion
    deviceConfig.periodSizeInFrames = FRAMES_PER_BUFFER;    // Puffergr��e (frames)

    // Initialisierung Soundkarte
    result = ma_device_init(NULL, &deviceConfig, &device);
    if (result != MA_SUCCESS) 
    {
        printf("Fehler beim Initialisieren des Ger�ts: %d\n", result);
        return -1;
    }

    // Start Soundkarte
    result = ma_device_start(&device);
    if (result != MA_SUCCESS) 
    {
        printf("Fehler beim Starten des Ger�ts: %d\n", result);
        ma_device_uninit(&device);
        return -1;
    }

    return 0;
}

void stop_audiostream()
{
    // Stop Soundkarte
    ma_device_uninit(&device);
}