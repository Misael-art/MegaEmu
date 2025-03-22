/**
 * @file audio_test.c
 * @brief Ferramenta de teste para o sistema de áudio do Mega Drive
 * @author Mega_Emu Team
 * @version 1.0.0
 * @date 2025-03-21
 * 
 * Esta ferramenta gera uma sequência de tons usando os chips YM2612 e SN76489
 * para testar a funcionalidade do sistema de áudio do Mega Drive.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "../src/platforms/megadrive/audio/audio_system.h"
#include "../src/common/logging.h"

#define SAMPLE_RATE 44100
#define BUFFER_SIZE 2048
#define CLOCK_NTSC 7670454
#define CLOCK_PAL  7600489

// Estrutura para armazenar o estado do teste
typedef struct {
    md_audio_system_t audio;
    SDL_AudioDeviceID device;
    SDL_AudioSpec spec;
    uint8_t running;
    uint32_t time_ms;
    uint32_t note_duration_ms;
    uint32_t current_note;
    uint32_t total_notes;
} audio_test_t;

// Notas musicais (frequências em Hz)
typedef struct {
    const char* name;
    float frequency;
} note_t;

// Tabela de notas musicais (C4 a B4)
const note_t notes[] = {
    {"C4", 261.63f},
    {"C#4", 277.18f},
    {"D4", 293.66f},
    {"D#4", 311.13f},
    {"E4", 329.63f},
    {"F4", 349.23f},
    {"F#4", 369.99f},
    {"G4", 392.00f},
    {"G#4", 415.30f},
    {"A4", 440.00f},
    {"A#4", 466.16f},
    {"B4", 493.88f}
};

// Callback de áudio SDL
void audio_callback(void* userdata, Uint8* stream, int len) {
    audio_test_t* test = (audio_test_t*)userdata;
    int16_t* buffer = (int16_t*)stream;
    int samples = len / (2 * sizeof(int16_t)); // Estéreo, 16 bits
    
    // Limpar o buffer
    memset(stream, 0, len);
    
    // Gerar amostras
    md_audio_update(&test->audio, buffer, buffer + 1, samples);
    
    // Atualizar o tempo
    test->time_ms += (uint32_t)(1000.0f * samples / SAMPLE_RATE);
    
    // Verificar se é hora de mudar de nota
    if (test->time_ms >= (test->current_note + 1) * test->note_duration_ms) {
        test->current_note = (test->current_note + 1) % test->total_notes;
        
        // Configurar a próxima nota no YM2612
        float freq = notes[test->current_note].frequency;
        uint32_t f_number = (uint32_t)((144.0f * freq * (1 << 20)) / CLOCK_NTSC);
        uint8_t block = 4; // Oitava
        uint8_t f_number_high = (f_number >> 8) & 0x07;
        uint8_t f_number_low = f_number & 0xFF;
        
        // Desligar a nota anterior
        md_audio_write_ym2612(&test->audio, 0, 0x28, 0x00);
        
        // Configurar a nova frequência
        md_audio_write_ym2612(&test->audio, 0, 0xA0, f_number_low);
        md_audio_write_ym2612(&test->audio, 0, 0xA4, (block << 3) | f_number_high);
        
        // Ligar a nova nota
        md_audio_write_ym2612(&test->audio, 0, 0x28, 0xF0);
        
        // Configurar a nota no SN76489
        uint16_t tone = (uint16_t)(3579545 / (32 * freq));
        md_audio_write_sn76489(&test->audio, 0x80 | (0 << 5) | (tone & 0x0F));
        md_audio_write_sn76489(&test->audio, (tone >> 4) & 0x3F);
        
        // Exibir informações da nota
        log_info("Tocando nota %s (%.2f Hz) - YM2612 F-Number: %04X, SN76489 Tone: %04X",
                 notes[test->current_note].name, freq, f_number, tone);
    }
}

// Inicializar o YM2612 com um patch de piano
void init_ym2612_piano(md_audio_system_t* audio) {
    // Desativar LFO
    md_audio_write_ym2612(audio, 0, 0x22, 0x00);
    
    // Desativar timers
    md_audio_write_ym2612(audio, 0, 0x27, 0x00);
    
    // Configurar operadores para o canal 1 (algoritmo 0, feedback 0)
    md_audio_write_ym2612(audio, 0, 0xB0, 0x32);
    
    // Estéreo e sem LFO
    md_audio_write_ym2612(audio, 0, 0xB4, 0xC0);
    
    // Operador 1 (modulador)
    md_audio_write_ym2612(audio, 0, 0x30, 0x71); // Multiplicador: 7
    md_audio_write_ym2612(audio, 0, 0x40, 0x23); // Nível total: 35
    md_audio_write_ym2612(audio, 0, 0x50, 0x5F); // Attack rate: 31
    md_audio_write_ym2612(audio, 0, 0x60, 0x05); // Decay rate: 5
    md_audio_write_ym2612(audio, 0, 0x70, 0x02); // Sustain rate: 2
    md_audio_write_ym2612(audio, 0, 0x80, 0x11); // Release rate: 1, Sustain level: 1
    md_audio_write_ym2612(audio, 0, 0x90, 0x00); // SSG-EG: off
    
    // Operador 2 (modulador)
    md_audio_write_ym2612(audio, 0, 0x34, 0x0D); // Multiplicador: 13
    md_audio_write_ym2612(audio, 0, 0x44, 0x2D); // Nível total: 45
    md_audio_write_ym2612(audio, 0, 0x54, 0x99); // Attack rate: 25
    md_audio_write_ym2612(audio, 0, 0x64, 0x05); // Decay rate: 5
    md_audio_write_ym2612(audio, 0, 0x74, 0x02); // Sustain rate: 2
    md_audio_write_ym2612(audio, 0, 0x84, 0x11); // Release rate: 1, Sustain level: 1
    md_audio_write_ym2612(audio, 0, 0x94, 0x00); // SSG-EG: off
    
    // Operador 3 (modulador)
    md_audio_write_ym2612(audio, 0, 0x38, 0x33); // Multiplicador: 3
    md_audio_write_ym2612(audio, 0, 0x48, 0x26); // Nível total: 38
    md_audio_write_ym2612(audio, 0, 0x58, 0x5F); // Attack rate: 31
    md_audio_write_ym2612(audio, 0, 0x68, 0x05); // Decay rate: 5
    md_audio_write_ym2612(audio, 0, 0x78, 0x02); // Sustain rate: 2
    md_audio_write_ym2612(audio, 0, 0x88, 0x11); // Release rate: 1, Sustain level: 1
    md_audio_write_ym2612(audio, 0, 0x98, 0x00); // SSG-EG: off
    
    // Operador 4 (portadora)
    md_audio_write_ym2612(audio, 0, 0x3C, 0x01); // Multiplicador: 1
    md_audio_write_ym2612(audio, 0, 0x4C, 0x00); // Nível total: 0
    md_audio_write_ym2612(audio, 0, 0x5C, 0x94); // Attack rate: 20
    md_audio_write_ym2612(audio, 0, 0x6C, 0x07); // Decay rate: 7
    md_audio_write_ym2612(audio, 0, 0x7C, 0x02); // Sustain rate: 2
    md_audio_write_ym2612(audio, 0, 0x8C, 0xA6); // Release rate: 6, Sustain level: 10
    md_audio_write_ym2612(audio, 0, 0x9C, 0x00); // SSG-EG: off
}

// Inicializar o SN76489
void init_sn76489(md_audio_system_t* audio) {
    // Configurar volume para os canais
    md_audio_write_sn76489(audio, 0x90 | 0x00 | 0x0F); // Canal 0, volume mínimo
    md_audio_write_sn76489(audio, 0x90 | 0x20 | 0x0F); // Canal 1, volume mínimo
    md_audio_write_sn76489(audio, 0x90 | 0x40 | 0x0F); // Canal 2, volume mínimo
    md_audio_write_sn76489(audio, 0x90 | 0x60 | 0x0F); // Canal 3 (ruído), volume mínimo
    
    // Configurar canal 0 para tocar
    md_audio_write_sn76489(audio, 0x90 | 0x00 | 0x02); // Canal 0, volume quase máximo
    
    // Configurar estéreo (todos os canais em ambos os lados)
    md_audio_set_sn76489_stereo(audio, 0xFF);
}

int main(int argc, char** argv) {
    audio_test_t test;
    SDL_AudioSpec wanted;
    
    // Inicializar SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        log_error("Não foi possível inicializar SDL: %s", SDL_GetError());
        return 1;
    }
    
    // Inicializar o sistema de áudio do Mega Drive
    if (md_audio_init(&test.audio, CLOCK_NTSC, SAMPLE_RATE) != EMU_ERROR_NONE) {
        log_error("Falha ao inicializar o sistema de áudio do Mega Drive");
        SDL_Quit();
        return 1;
    }
    
    // Configurar volumes
    md_audio_set_ym2612_volume(&test.audio, 0.7f);
    md_audio_set_sn76489_volume(&test.audio, 0.5f);
    md_audio_set_master_volume(&test.audio, 1.0f);
    
    // Inicializar o YM2612 com um patch de piano
    init_ym2612_piano(&test.audio);
    
    // Inicializar o SN76489
    init_sn76489(&test.audio);
    
    // Configurar o teste
    test.running = 1;
    test.time_ms = 0;
    test.note_duration_ms = 500; // 500ms por nota
    test.current_note = 0;
    test.total_notes = sizeof(notes) / sizeof(notes[0]);
    
    // Configurar o áudio SDL
    wanted.freq = SAMPLE_RATE;
    wanted.format = AUDIO_S16SYS;
    wanted.channels = 2;
    wanted.samples = BUFFER_SIZE;
    wanted.callback = audio_callback;
    wanted.userdata = &test;
    
    // Abrir o dispositivo de áudio
    test.device = SDL_OpenAudioDevice(NULL, 0, &wanted, &test.spec, 0);
    if (test.device == 0) {
        log_error("Falha ao abrir o dispositivo de áudio: %s", SDL_GetError());
        md_audio_shutdown(&test.audio);
        SDL_Quit();
        return 1;
    }
    
    // Iniciar a reprodução
    SDL_PauseAudioDevice(test.device, 0);
    
    log_info("Teste de áudio iniciado. Tocando escala de C4 a B4.");
    log_info("Pressione Enter para sair...");
    
    // Aguardar entrada do usuário
    getchar();
    
    // Limpar
    SDL_CloseAudioDevice(test.device);
    md_audio_shutdown(&test.audio);
    SDL_Quit();
    
    log_info("Teste de áudio finalizado.");
    
    return 0;
}
