/** * @file state_interface.h * @brief Interface do sistema de estado do
 * emulador */                                                                 \
#ifndef STATE_INTERFACE_H #define STATE_INTERFACE_H #ifdef __cplusplusextern   \
    "C" {                                                                      \
        #endif // Tipos de estado    typedef enum    { EMU_STATE_TYPE_NONE = 0,
               // EMU_STATE_TYPE_SAVE,        EMU_STATE_TYPE_LOAD,
               // EMU_STATE_TYPE_SAVE_SNAPSHOT, EMU_STATE_TYPE_LOAD_SNAPSHOT,
               // EMU_STATE_TYPE_RESET,        EMU_STATE_TYPE_POWER_CYCLE,
               // EMU_STATE_TYPE_MAX    } emu_state_type_t;    // Flags de
               // estado    typedef enum    {        EMU_STATE_FLAG_NONE =
               // 0x00000000,        EMU_STATE_FLAG_CPU = 0x00000001,
               // EMU_STATE_FLAG_MEMORY = 0x00000002, EMU_STATE_FLAG_VIDEO =
               // 0x00000004,        EMU_STATE_FLAG_AUDIO = 0x00000008,
               // EMU_STATE_FLAG_INPUT = 0x00000010, EMU_STATE_FLAG_REGISTERS =
               // 0x00000020,        EMU_STATE_FLAG_VRAM = 0x00000040,
               // EMU_STATE_FLAG_PALETTES = 0x00000080, EMU_STATE_FLAG_SPRITES =
               // 0x00000100,        EMU_STATE_FLAG_CARTRIDGE = 0x00000200,
               // EMU_STATE_FLAG_TIMERS = 0x00000400,        EMU_STATE_FLAG_DMA
               // = 0x00000800,        EMU_STATE_FLAG_INTERRUPTS = 0x00001000,
               // EMU_STATE_FLAG_ALL = 0xFFFFFFFF    } emu_state_flags_t;    //
               // Erro de estado    typedef enum    { EMU_STATE_ERROR_NONE = 0,
               // EMU_STATE_ERROR_NOT_INITIALIZED, EMU_STATE_ERROR_INVALID_TYPE,
               // EMU_STATE_ERROR_INVALID_SLOT, EMU_STATE_ERROR_FILE_NOT_FOUND,
               // EMU_STATE_ERROR_FILE_ACCESS, EMU_STATE_ERROR_INVALID_FORMAT,
               // EMU_STATE_ERROR_INVALID_VERSION,
               // EMU_STATE_ERROR_OUT_OF_MEMORY,
               // EMU_STATE_ERROR_PLATFORM_NOT_SUPPORTED,
               // EMU_STATE_ERROR_DATA_CORRUPTION,
               // EMU_STATE_ERROR_NOT_IMPLEMENTED,        EMU_STATE_ERROR_MAX }
               // emu_state_error_t;    // Informações do arquivo de estado
               // typedef struct    {        char platform_id[16];      /**<
               // Identificador da plataforma */        char rom_name[64]; /**<
               // Nome da ROM */        char rom_hash[64];         /**< Hash da
               // ROM */        uint32_t state_version;    /**< Versão do
               // formato de estado */        uint32_t flags;            /**<
               // Flags de dados incluídos */        uint64_t timestamp; /**<
               // Timestamp de criação */        uint64_t emulator_version; /**<
               // Versão do emulador */        char description[128];     /**<
               // Descrição do estado */    } emu_state_info_t;    // Callback
               // para progresso    typedef void
               // (*emu_state_progress_callback_t)(int32_t percentage, const
               // char *message, void *userdata);    // Callback para
               // verificação de ROM    typedef int32_t
               // (*emu_state_rom_verify_callback_t)(const char *rom_hash, void
               // *userdata);    // Interface do sistema de estado    typedef
               // struct    {        // Funções de gerenciamento        int32_t
               // (*init)(void);        void (*shutdown)(void);        int32_t
               // (*set_platform)(emu_platform_t *platform);        // Funções
               // de estado        int32_t (*save_state)(int32_t slot, const
               // char *description);        int32_t (*load_state)(int32_t
               // slot);        int32_t (*save_state_to_file)(const char
               // *filename, const char *description);        int32_t
               // (*load_state_from_file)(const char *filename);        //
               // Funções de snapshot        int32_t (*create_snapshot)(void);
               // int32_t (*restore_snapshot)(int32_t snapshot_id); int32_t
               // (*delete_snapshot)(int32_t snapshot_id);        int32_t
               // (*get_snapshot_count)(void);        // Funções de reset
               // int32_t (*reset)(emu_state_type_t reset_type);        int32_t
               // (*rewind)(int32_t frames);        // Funções de informação
               // int32_t (*get_state_info)(int32_t slot, emu_state_info_t
               // *info);        int32_t (*get_file_info)(const char *filename,
               // emu_state_info_t *info);        int32_t
               // (*get_state_slots)(int32_t *filled_slots, int32_t max_slots);
               // // Funções de callback        int32_t
               // (*set_progress_callback)(emu_state_progress_callback_t
               // callback, void *userdata);        int32_t
               // (*set_rom_verify_callback)(emu_state_rom_verify_callback_t
               // callback, void *userdata);        // Funções de configuração
               // int32_t (*enable_rewind)(int32_t enable);        int32_t
               // (*set_rewind_buffer_frames)(int32_t frames);        int32_t
               // (*set_autosave_interval)(int32_t seconds);        // Funções
               // de utilitário        const char
               // *(*get_error_string)(emu_state_error_t error);    }
               // emu_state_interface_t;    // Obter interface do sistema de
               // estado    const emu_state_interface_t
               // *emu_state_get_interface(void);#ifdef __cplusplus}#endif#endif
               // /* STATE_INTERFACE_H */
