#include "emu_effects.h"#include <stdlib.h>#include <string.h>#include <stdio.h>#include <time.h>// Initialização do sistema de efeitosgui_result_t emu_effects_init(emu_effect_system_t *system, gui_shader_system_t *shaders){    if (!system || !shaders)        return GUI_ERROR_INVALID_PARAM;    system->presets = NULL;    system->preset_count = 0;    system->shader_system = shaders;    system->previous_frame = NULL;    system->initialized = GUI_TRUE;    // Inicializar com preset padrão    system->current_preset = emu_preset_default();    return GUI_SUCCESS;}// Finalização do sistemavoid emu_effects_shutdown(emu_effect_system_t *system){    if (!system)        return;    // Liberar presets    free(system->presets);    // Liberar frame anterior    if (system->previous_frame)    {        SDL_DestroyTexture(system->previous_frame);    }    system->presets = NULL;    system->preset_count = 0;    system->initialized = GUI_FALSE;}// Adicionar preset ao sistemagui_result_t emu_effects_add_preset(emu_effect_system_t *system, const emu_preset_t *preset){    if (!system || !preset)        return GUI_ERROR_INVALID_PARAM;    // Verificar se já existe um preset com este nome    for (size_t i = 0; i < system->preset_count; i++)    {        if (strcmp(system->presets[i].name, preset->name) == 0)        {            // Atualizar preset existente            system->presets[i] = *preset;            return GUI_SUCCESS;        }    }    // Criar novo preset    emu_preset_t *new_presets = realloc(system->presets,                                        (system->preset_count + 1) * sizeof(emu_preset_t));    if (!new_presets)        return GUI_ERROR_MEMORY;    system->presets = new_presets;    system->presets[system->preset_count++] = *preset;    return GUI_SUCCESS;}// Carregar preset por nomegui_result_t emu_effects_load_preset(emu_effect_system_t *system, const char *name){    if (!system || !name)        return GUI_ERROR_INVALID_PARAM;    // Buscar preset pelo nome    for (size_t i = 0; i < system->preset_count; i++)    {        if (strcmp(system->presets[i].name, name) == 0)        {            system->current_preset = system->presets[i];            return GUI_SUCCESS;        }    }    return GUI_ERROR_NOT_FOUND;}// Configurar consolegui_result_t emu_effects_set_console(emu_effect_system_t *system, emu_console_type_t console){    if (!system)        return GUI_ERROR_INVALID_PARAM;    // Configurar preset baseado no tipo de console    emu_preset_t preset;    switch (console)    {    case EMU_CONSOLE_NES:        preset = emu_preset_nes();        break;    case EMU_CONSOLE_SNES:        preset = emu_preset_snes();        break;    case EMU_CONSOLE_GAMEBOY:        preset = emu_preset_gameboy();        break;    case EMU_CONSOLE_GBC:        preset = emu_preset_gbc();        break;    case EMU_CONSOLE_GBA:        preset = emu_preset_gba();        break;    case EMU_CONSOLE_MEGADRIVE:        preset = emu_preset_megadrive();        break;    case EMU_CONSOLE_MASTERSYSTEM:        preset = emu_preset_mastersystem();        break;    case EMU_CONSOLE_ARCADE_CRT:        preset = emu_preset_arcade();        break;    default:        preset = emu_preset_default();        break;    }    system->current_preset = preset;    return GUI_SUCCESS;}// Personalizar parâmetrosgui_result_t emu_effects_customize(emu_effect_system_t *system, emu_effect_params_t params){    if (!system)        return GUI_ERROR_INVALID_PARAM;    // Manter o nome e console, mas atualizar os parâmetros    system->current_preset.params = params;    return GUI_SUCCESS;}// Início de framegui_result_t emu_effects_begin_frame(emu_effect_system_t *system, SDL_Texture *target){    if (!system || !system->initialized || !target)        return GUI_ERROR_INVALID_PARAM;    // Iniciar o sistema de shaders    return gui_shaders_begin(system->shader_system, target);}// Aplicar efeitosgui_result_t emu_effects_apply(emu_effect_system_t *system,                               SDL_Texture *source,                               SDL_Texture *target){    if (!system || !system->initialized || !source || !target)    {        return GUI_ERROR_INVALID_PARAM;    }    // Verificar se precisamos criar um frame anterior    if (!system->previous_frame)    {        int w, h;        SDL_QueryTexture(source, NULL, NULL, &w, &h);        system->previous_frame = SDL_CreateTexture(            system->shader_system->renderer,            SDL_PIXELFORMAT_RGBA8888,            SDL_TEXTUREACCESS_TARGET,            w, h);        if (!system->previous_frame)        {            return GUI_ERROR_INIT_FAILED;        }        // Inicializar com o frame atual        SDL_SetRenderTarget(system->shader_system->renderer, system->previous_frame);        SDL_RenderCopy(system->shader_system->renderer, source, NULL, NULL);        SDL_SetRenderTarget(system->shader_system->renderer, NULL);    }    // Textura temporária para os efeitos em cadeia    SDL_Texture *temp = SDL_CreateTexture(        system->shader_system->renderer,        SDL_PIXELFORMAT_RGBA8888,        SDL_TEXTUREACCESS_TARGET,        system->shader_system->renderer->w,        system->shader_system->renderer->h);    if (!temp)    {        return GUI_ERROR_MEMORY;    }    // Copiar source para temp    SDL_SetRenderTarget(system->shader_system->renderer, temp);    SDL_RenderCopy(system->shader_system->renderer, source, NULL, NULL);    // Aplicar efeitos específicos de emulador    emu_effect_params_t params = system->current_preset.params;    // Efeito de phosphor (persistência)    if (params.phosphor_persistence > 0.0f)    {        emu_effect_apply_phosphor(system, temp, temp, params.phosphor_persistence);    }    // Efeito de LCD    if (params.pixel_grid > 0.0f || params.ghosting > 0.0f)    {        emu_effect_apply_lcd(system, temp, temp, params.pixel_grid, params.ghosting);    }    // Efeito de TV noise    if (params.noise_intensity > 0.0f)    {        emu_effect_apply_tv_noise(system, temp, temp, params.noise_intensity);    }    // Efeito de color shift    if (params.color_shift > 0.0f)    {        emu_effect_apply_color_shift(system, temp, temp, params.tint, params.color_shift);    }    // Efeito de NTSC bleeding    if (params.color_bleed > 0.0f)    {        emu_effect_apply_ntsc_bleed(system, temp, temp, params.color_bleed);    }    // Aplicar shaders na ordem configurada    if (system->current_preset.shader_count > 0)    {        for (size_t i = 0; i < system->current_preset.shader_count; i++)        {            gui_shader_type_t shader_type = system->current_preset.shaders[i];            gui_shaders_apply(system->shader_system, shader_type, temp, temp);        }    }    // Renderizar resultado final para o destino    SDL_SetRenderTarget(system->shader_system->renderer, target);    SDL_RenderCopy(system->shader_system->renderer, temp, NULL, NULL);    // Atualizar frame anterior    SDL_SetRenderTarget(system->shader_system->renderer, system->previous_frame);    SDL_RenderCopy(system->shader_system->renderer, source, NULL, NULL);    // Limpar    SDL_DestroyTexture(temp);    return GUI_SUCCESS;}// Fim do framegui_result_t emu_effects_end_frame(emu_effect_system_t *system){    if (!system || !system->initialized)        return GUI_ERROR_INVALID_PARAM;    // Finalizar o sistema de shaders    return gui_shaders_end(system->shader_system);}// Efeito de persistência de fósforo (monitores CRT)gui_result_t emu_effect_apply_phosphor(emu_effect_system_t *system,                                       SDL_Texture *source,                                       SDL_Texture *target,                                       float persistence){    if (!system || !source || !target)        return GUI_ERROR_INVALID_PARAM;    // Configurar renderização para o destino    SDL_SetRenderTarget(system->shader_system->renderer, target);    // Renderizar o frame atual    SDL_RenderCopy(system->shader_system->renderer, source, NULL, NULL);    // Renderizar o frame anterior com alpha para criar o efeito de persistência    SDL_SetTextureAlphaMod(system->previous_frame, (Uint8)(persistence * 255));    SDL_SetTextureBlendMode(system->previous_frame, SDL_BLENDMODE_BLEND);    SDL_RenderCopy(system->shader_system->renderer, system->previous_frame, NULL, NULL);    // Restaurar    SDL_SetTextureAlphaMod(system->previous_frame, 255);    SDL_SetTextureBlendMode(system->previous_frame, SDL_BLENDMODE_NONE);    return GUI_SUCCESS;}// Efeito de ruído de TVgui_result_t emu_effect_apply_tv_noise(emu_effect_system_t *system,                                       SDL_Texture *source,                                       SDL_Texture *target,                                       float intensity){    if (!system || !source || !target)        return GUI_ERROR_INVALID_PARAM;    // Configurar renderização para o destino    SDL_SetRenderTarget(system->shader_system->renderer, target);    // Renderizar textura original    SDL_RenderCopy(system->shader_system->renderer, source, NULL, NULL);    // Adicionar ruído    int w, h;    SDL_QueryTexture(target, NULL, NULL, &w, &h);    // Quantidade de linhas de ruído    int num_noise_lines = (int)(intensity * 10);    // Desenhar linhas horizontais aleatórias    SDL_SetRenderDrawBlendMode(system->shader_system->renderer, SDL_BLENDMODE_BLEND);    SDL_SetRenderDrawColor(system->shader_system->renderer, 255, 255, 255, 128);    for (int i = 0; i < num_noise_lines; i++)    {        int y = rand() % h;        int length = rand() % w;        int x = rand() % (w - length);        SDL_RenderDrawLine(system->shader_system->renderer, x, y, x + length, y);    }    // Adicionar ruído estático    int num_static_points = (int)(intensity * w * h * 0.01f);    for (int i = 0; i < num_static_points; i++)    {        int x = rand() % w;        int y = rand() % h;        Uint8 bright = rand() % 256;        SDL_SetRenderDrawColor(system->shader_system->renderer, bright, bright, bright, 128);        SDL_RenderDrawPoint(system->shader_system->renderer, x, y);    }    return GUI_SUCCESS;}// Efeito de LCD (grade de pixels e ghosting)gui_result_t emu_effect_apply_lcd(emu_effect_system_t *system,                                  SDL_Texture *source,                                  SDL_Texture *target,                                  float grid_intensity,                                  float ghosting){    if (!system || !source || !target)        return GUI_ERROR_INVALID_PARAM;    // Configurar renderização para o destino    SDL_SetRenderTarget(system->shader_system->renderer, target);    // Renderizar textura original    SDL_RenderCopy(system->shader_system->renderer, source, NULL, NULL);    // Desenhar grade de pixels    if (grid_intensity > 0.0f)    {        int w, h;        SDL_QueryTexture(target, NULL, NULL, &w, &h);        int cell_size = 4; // Tamanho do pixel LCD        SDL_SetRenderDrawBlendMode(system->shader_system->renderer, SDL_BLENDMODE_BLEND);        SDL_SetRenderDrawColor(system->shader_system->renderer, 0, 0, 0,                               (Uint8)(grid_intensity * 128));        // Linhas horizontais        for (int y = 0; y < h; y += cell_size)        {            SDL_RenderDrawLine(system->shader_system->renderer, 0, y, w, y);        }        // Linhas verticais        for (int x = 0; x < w; x += cell_size)        {            SDL_RenderDrawLine(system->shader_system->renderer, x, 0, x, h);        }    }    // Aplicar ghosting com o frame anterior    if (ghosting > 0.0f)    {        SDL_SetTextureAlphaMod(system->previous_frame, (Uint8)(ghosting * 128));        SDL_SetTextureBlendMode(system->previous_frame, SDL_BLENDMODE_BLEND);        SDL_RenderCopy(system->shader_system->renderer, system->previous_frame, NULL, NULL);        // Restaurar        SDL_SetTextureAlphaMod(system->previous_frame, 255);        SDL_SetTextureBlendMode(system->previous_frame, SDL_BLENDMODE_NONE);    }    return GUI_SUCCESS;}// Filtro Game Boy (4 tons de verde)gui_result_t emu_effect_apply_gameboy(emu_effect_system_t *system,                                      SDL_Texture *source,                                      SDL_Texture *target){    if (!system || !source || !target)        return GUI_ERROR_INVALID_PARAM;    // Criar tabela de cores do Game Boy (4 tons de verde)    gui_color_t gb_palette[4] = {        {15, 56, 15, 255}, // Mais escuro        {48, 98, 48, 255},        {139, 172, 15, 255},        {155, 188, 15, 255} // Mais claro    };    // Configurar renderização para o destino    SDL_SetRenderTarget(system->shader_system->renderer, target);    SDL_SetRenderDrawColor(system->shader_system->renderer,                           gb_palette[0].r, gb_palette[0].g, gb_palette[0].b, 255);    SDL_RenderClear(system->shader_system->renderer);    // Renderizar original com a paleta limitada    // Nota: Em SDL2 puro, isso é uma aproximação, idealmente usaríamos shaders    // Primeiro, transformar para escala de cinza    SDL_SetTextureColorMod(source, 77, 77, 77); // Multiplicador para aproximar luminância (0.3R + 0.59G + 0.11B)    // Dividir a tela em 4 regiões para simular os 4 tons    int w, h;    SDL_QueryTexture(target, NULL, NULL, &w, &h);    // Renderizar com diferentes níveis de alpha para simular os tons    for (int i = 0; i < 4; i++)    {        SDL_Rect mask = {0, i * (h / 4), w, h / 4};        SDL_SetRenderDrawColor(system->shader_system->renderer,                               gb_palette[i].r, gb_palette[i].g, gb_palette[i].b, 255);        SDL_RenderFillRect(system->shader_system->renderer, &mask);        // Blend com a fonte original usando diferentes intensidades        SDL_SetTextureAlphaMod(source, 128 + (i * 32));        SDL_SetTextureBlendMode(source, SDL_BLENDMODE_BLEND);        SDL_RenderCopy(system->shader_system->renderer, source, NULL, &mask);    }    // Restaurar    SDL_SetTextureAlphaMod(source, 255);    SDL_SetTextureBlendMode(source, SDL_BLENDMODE_NONE);    SDL_SetTextureColorMod(source, 255, 255, 255);    return GUI_SUCCESS;}// Efeito de mudança de cor (tint)gui_result_t emu_effect_apply_color_shift(emu_effect_system_t *system,                                          SDL_Texture *source,                                          SDL_Texture *target,                                          gui_color_t tint,                                          float intensity){    if (!system || !source || !target)        return GUI_ERROR_INVALID_PARAM;    // Configurar renderização para o destino    SDL_SetRenderTarget(system->shader_system->renderer, target);    // Renderizar textura original    SDL_RenderCopy(system->shader_system->renderer, source, NULL, NULL);    // Aplicar tint    SDL_SetRenderDrawBlendMode(system->shader_system->renderer, SDL_BLENDMODE_BLEND);    SDL_SetRenderDrawColor(system->shader_system->renderer,                           tint.r, tint.g, tint.b, (Uint8)(tint.a * intensity));    SDL_Rect fullscreen = {0, 0};    SDL_QueryTexture(target, NULL, NULL, &fullscreen.w, &fullscreen.h);    SDL_RenderFillRect(system->shader_system->renderer, &fullscreen);    return GUI_SUCCESS;}// Efeito de sangramento de cores NTSCgui_result_t emu_effect_apply_ntsc_bleed(emu_effect_system_t *system,                                         SDL_Texture *source,                                         SDL_Texture *target,                                         float intensity){    if (!system || !source || !target)        return GUI_ERROR_INVALID_PARAM;    // Configurar renderização para o destino    SDL_SetRenderTarget(system->shader_system->renderer, target);    // Em SDL2 puro, simulamos o efeito separando os canais RGB    // e deslocando-os ligeiramente    int offset = (int)(intensity * 2.0f);    if (offset < 1)        offset = 1;    // Desenhar fundo preto    SDL_SetRenderDrawColor(system->shader_system->renderer, 0, 0, 0, 255);    SDL_RenderClear(system->shader_system->renderer);    // Canal vermelho (deslocado para a esquerda)    SDL_SetTextureColorMod(source, 255, 0, 0);    SDL_Rect r_rect = {-offset, 0};    SDL_QueryTexture(source, NULL, NULL, &r_rect.w, &r_rect.h);    SDL_RenderCopy(system->shader_system->renderer, source, NULL, &r_rect);    // Canal verde (centralizado)    SDL_SetTextureColorMod(source, 0, 255, 0);    SDL_Rect g_rect = {0, 0};    SDL_QueryTexture(source, NULL, NULL, &g_rect.w, &g_rect.h);    SDL_SetTextureBlendMode(source, SDL_BLENDMODE_ADD);    SDL_RenderCopy(system->shader_system->renderer, source, NULL, &g_rect);    // Canal azul (deslocado para a direita)    SDL_SetTextureColorMod(source, 0, 0, 255);    SDL_Rect b_rect = {offset, 0};    SDL_QueryTexture(source, NULL, NULL, &b_rect.w, &b_rect.h);    SDL_RenderCopy(system->shader_system->renderer, source, NULL, &b_rect);    // Restaurar    SDL_SetTextureColorMod(source, 255, 255, 255);    SDL_SetTextureBlendMode(source, SDL_BLENDMODE_NONE);    return GUI_SUCCESS;}// Salvar preset para jogo específicogui_result_t emu_effects_save_game_preset(const char *rom_path, const emu_preset_t *preset){    if (!rom_path || !preset)        return GUI_ERROR_INVALID_PARAM;    // Criar nome de arquivo baseado no caminho da ROM    char config_file[512];    snprintf(config_file, sizeof(config_file), "presets/%s.cfg",             strrchr(rom_path, '/') ? strrchr(rom_path, '/') + 1 : rom_path);// Criar diretório se não existir#ifdef _WIN32    _mkdir("presets");#else    mkdir("presets", 0777);#endif    // Salvar configuração    FILE *file = fopen(config_file, "wb");    if (!file)        return GUI_ERROR_FILE_WRITE;    fwrite(preset, sizeof(emu_preset_t), 1, file);    fclose(file);    return GUI_SUCCESS;}// Carregar preset para jogo específicogui_result_t emu_effects_load_game_preset(emu_effect_system_t *system, const char *rom_path){    if (!system || !rom_path)        return GUI_ERROR_INVALID_PARAM;    // Criar nome de arquivo baseado no caminho da ROM    char config_file[512];    snprintf(config_file, sizeof(config_file), "presets/%s.cfg",             strrchr(rom_path, '/') ? strrchr(rom_path, '/') + 1 : rom_path);    // Carregar configuração    FILE *file = fopen(config_file, "rb");    if (!file)        return GUI_ERROR_FILE_READ;    emu_preset_t preset;    size_t read = fread(&preset, sizeof(emu_preset_t), 1, file);    fclose(file);    if (read != 1)        return GUI_ERROR_FILE_READ;    // Aplicar preset    system->current_preset = preset;    return GUI_SUCCESS;}// ========================================================================// Presets padrão para diferentes consoles// ========================================================================// Preset padrão (sem efeitos)emu_preset_t emu_preset_default(void){    emu_preset_t preset;    memset(&preset, 0, sizeof(emu_preset_t));    preset.name = "Default";    preset.console = EMU_CONSOLE_NONE;    preset.shader_count = 0;    // Sem efeitos    preset.params.phosphor_persistence = 0.0f;    preset.params.noise_intensity = 0.0f;    preset.params.scanline_intensity = 0.0f;    preset.params.curvature = 0.0f;    preset.params.color_bleed = 0.0f;    preset.params.ghosting = 0.0f;    preset.params.pixel_grid = 0.0f;    preset.params.color_shift = 0.0f;    preset.params.pixel_perfect = GUI_TRUE;    return preset;}// Preset NESemu_preset_t emu_preset_nes(void){    emu_preset_t preset;    memset(&preset, 0, sizeof(emu_preset_t));    preset.name = "NES";    preset.console = EMU_CONSOLE_NES;    // Efeitos CRT suaves    preset.params.phosphor_persistence = 0.2f;    preset.params.noise_intensity = 0.1f;    preset.params.scanline_intensity = 0.3f;    preset.params.curvature = 0.1f;    preset.params.color_bleed = 0.2f;    preset.params.ghosting = 0.0f;    preset.params.pixel_grid = 0.0f;    preset.params.color_shift = 0.0f;    preset.params.pixel_perfect = GUI_FALSE;    // Shaders para NES    preset.shaders[0] = GUI_SHADER_CRT;    preset.shader_count = 1;    return preset;}// Preset SNESemu_preset_t emu_preset_snes(void){    emu_preset_t preset;    memset(&preset, 0, sizeof(emu_preset_t));    preset.name = "SNES";    preset.console = EMU_CONSOLE_SNES;    // Efeitos CRT suaves    preset.params.phosphor_persistence = 0.15f;    preset.params.noise_intensity = 0.05f;    preset.params.scanline_intensity = 0.25f;    preset.params.curvature = 0.1f;    preset.params.color_bleed = 0.15f;    preset.params.ghosting = 0.0f;    preset.params.pixel_grid = 0.0f;    preset.params.color_shift = 0.0f;    preset.params.pixel_perfect = GUI_FALSE;    // Shaders para SNES    preset.shaders[0] = GUI_SHADER_CRT;    preset.shader_count = 1;    return preset;}// Preset Game Boyemu_preset_t emu_preset_gameboy(void){    emu_preset_t preset;    memset(&preset, 0, sizeof(emu_preset_t));    preset.name = "Game Boy";    preset.console = EMU_CONSOLE_GAMEBOY;    // Efeitos LCD    preset.params.phosphor_persistence = 0.0f;    preset.params.noise_intensity = 0.0f;    preset.params.scanline_intensity = 0.0f;    preset.params.curvature = 0.0f;    preset.params.color_bleed = 0.0f;    preset.params.ghosting = 0.4f;    preset.params.pixel_grid = 0.3f;    preset.params.color_shift = 0.9f;    preset.params.pixel_perfect = GUI_FALSE;    // Cor verde do Game Boy    preset.params.tint.r = 15;    preset.params.tint.g = 56;    preset.params.tint.b = 15;    preset.params.tint.a = 255;    // Nenhum shader específico    preset.shader_count = 0;    return preset;}// Preset Game Boy Coloremu_preset_t emu_preset_gbc(void){    emu_preset_t preset;    memset(&preset, 0, sizeof(emu_preset_t));    preset.name = "Game Boy Color";    preset.console = EMU_CONSOLE_GBC;    // Efeitos LCD    preset.params.phosphor_persistence = 0.0f;    preset.params.noise_intensity = 0.0f;    preset.params.scanline_intensity = 0.0f;    preset.params.curvature = 0.0f;    preset.params.color_bleed = 0.0f;    preset.params.ghosting = 0.3f;    preset.params.pixel_grid = 0.3f;    preset.params.color_shift = 0.0f;    preset.params.pixel_perfect = GUI_FALSE;    // Nenhum shader específico    preset.shader_count = 0;    return preset;}// Preset Game Boy Advanceemu_preset_t emu_preset_gba(void){    emu_preset_t preset;    memset(&preset, 0, sizeof(emu_preset_t));    preset.name = "Game Boy Advance";    preset.console = EMU_CONSOLE_GBA;    // Efeitos LCD    preset.params.phosphor_persistence = 0.0f;    preset.params.noise_intensity = 0.0f;    preset.params.scanline_intensity = 0.0f;    preset.params.curvature = 0.0f;    preset.params.color_bleed = 0.0f;    preset.params.ghosting = 0.2f;    preset.params.pixel_grid = 0.2f;    preset.params.color_shift = 0.0f;    preset.params.pixel_perfect = GUI_FALSE;    // Nenhum shader específico    preset.shader_count = 0;    return preset;}// Preset Mega Drive/Genesisemu_preset_t emu_preset_megadrive(void){    emu_preset_t preset;    memset(&preset, 0, sizeof(emu_preset_t));    preset.name = "Mega Drive";    preset.console = EMU_CONSOLE_MEGADRIVE;    // Efeitos CRT mais intensos    preset.params.phosphor_persistence = 0.2f;    preset.params.noise_intensity = 0.1f;    preset.params.scanline_intensity = 0.4f;    preset.params.curvature = 0.15f;    preset.params.color_bleed = 0.25f;    preset.params.ghosting = 0.0f;    preset.params.pixel_grid = 0.0f;    preset.params.color_shift = 0.0f;    preset.params.pixel_perfect = GUI_FALSE;    // Shaders para Mega Drive    preset.shaders[0] = GUI_SHADER_CRT;    preset.shader_count = 1;    return preset;}// Preset Master Systememu_preset_t emu_preset_mastersystem(void){    emu_preset_t preset;    memset(&preset, 0, sizeof(emu_preset_t));    preset.name = "Master System";    preset.console = EMU_CONSOLE_MASTERSYSTEM;    // Efeitos CRT    preset.params.phosphor_persistence = 0.25f;    preset.params.noise_intensity = 0.15f;    preset.params.scanline_intensity = 0.35f;    preset.params.curvature = 0.12f;    preset.params.color_bleed = 0.2f;    preset.params.ghosting = 0.0f;    preset.params.pixel_grid = 0.0f;    preset.params.color_shift = 0.0f;    preset.params.pixel_perfect = GUI_FALSE;    // Shaders para Master System    preset.shaders[0] = GUI_SHADER_CRT;    preset.shader_count = 1;    return preset;}// Preset Arcadeemu_preset_t emu_preset_arcade(void){    emu_preset_t preset;    memset(&preset, 0, sizeof(emu_preset_t));    preset.name = "Arcade";    preset.console = EMU_CONSOLE_ARCADE_CRT;    // Efeitos CRT intensos para arcade    preset.params.phosphor_persistence = 0.3f;    preset.params.noise_intensity = 0.1f;    preset.params.scanline_intensity = 0.5f;    preset.params.curvature = 0.2f;    preset.params.color_bleed = 0.3f;    preset.params.ghosting = 0.0f;    preset.params.pixel_grid = 0.0f;    preset.params.color_shift = 0.0f;    preset.params.pixel_perfect = GUI_FALSE;    // Shaders para Arcade    preset.shaders[0] = GUI_SHADER_CRT;    preset.shader_count = 1;    return preset;}