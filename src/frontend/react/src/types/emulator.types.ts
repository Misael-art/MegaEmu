export type ConsoleType =
    | 'megadrive'
    | 'sega32x'
    | 'segacd'
    | 'mastersystem'
    | 'gamegear'
    | 'sg1000'
    | 'nes'
    | 'snes'
    | 'gameboy'
    | 'gba'
    | 'n64'
    | 'psx'
    | 'tms9918'
    | 'atari7800'
    | 'gameboyadvanced'
    | 'genesis'
    | 'pcengine'
    | 'neogeo'
    | 'saturn'
    | 'playstation'
    | '32x';

// Definição de aliases para sistemas para compatibilidade com nomes diferentes
export const consoleAliases: Record<string, ConsoleType> = {
    'sega genesis': 'megadrive',
    'genesis': 'megadrive',
    'mega drive': 'megadrive',
    'megadrive': 'megadrive',
    'md': 'megadrive',
    'sega mega drive': 'megadrive',
    'sega md': 'megadrive',

    'master system': 'mastersystem',
    'mastersystem': 'mastersystem',
    'sms': 'mastersystem',
    'sega master system': 'mastersystem',
    'sega ms': 'mastersystem',

    'game gear': 'gamegear',
    'gamegear': 'gamegear',
    'gg': 'gamegear',
    'sega game gear': 'gamegear',
    'sega gg': 'gamegear',

    'sg-1000': 'sg1000',
    'sg1000': 'sg1000',
    'sega sg1000': 'sg1000',
    'sega sg-1000': 'sg1000',

    'nintendo': 'nes',
    'nes': 'nes',
    'famicom': 'nes',
    'family computer': 'nes',

    'super nintendo': 'snes',
    'snes': 'snes',
    'super famicom': 'snes',
    'super nes': 'snes',

    'turbografx': 'pcengine',
    'turbografx-16': 'pcengine',
    'pc engine': 'pcengine',
    'pcengine': 'pcengine',
    'pce': 'pcengine',

    'neo geo': 'neogeo',
    'neogeo': 'neogeo',
    'neo-geo': 'neogeo',
    'snk': 'neogeo',

    'sega saturn': 'saturn',
    'saturn': 'saturn',

    'psx': 'playstation',
    'playstation': 'playstation',
    'ps1': 'playstation',
    'sony playstation': 'playstation',

    'gameboy': 'gameboy',
    'game boy': 'gameboy',
    'gb': 'gameboy',
    'nintendo gameboy': 'gameboy',

    'gameboy advance': 'gba',
    'game boy advance': 'gba',
    'gba': 'gba',
    'nintendo gameboy advance': 'gba',

    'nintendo 64': 'n64',
    'n64': 'n64',

    '32x': 'sega32x',
    'sega 32x': 'sega32x',

    'sega cd': 'segacd',
    'segacd': 'segacd',
    'mega-cd': 'segacd',
    'mega cd': 'segacd',
};

// Nomes amigáveis para exibição
export const consoleDisplayNames: Record<ConsoleType, string> = {
    'megadrive': 'Mega Drive / Genesis',
    'genesis': 'Genesis',
    'sega32x': 'Sega 32X',
    'segacd': 'Sega CD / Mega CD',
    'mastersystem': 'Master System',
    'gamegear': 'Game Gear',
    'sg1000': 'SG-1000',
    'nes': 'Nintendo Entertainment System',
    'snes': 'Super Nintendo',
    'gameboy': 'Game Boy',
    'gba': 'Game Boy Advance',
    'n64': 'Nintendo 64',
    'psx': 'PlayStation',
    'tms9918': 'TMS9918',
    'atari7800': 'Atari 7800',
    'gameboyadvanced': 'Game Boy Advance',
    'pcengine': 'PC Engine / TurboGrafx-16',
    'neogeo': 'Neo Geo',
    'saturn': 'Sega Saturn',
    'playstation': 'PlayStation',
    '32x': 'Sega 32X'
};

// Definição de extensões de arquivo para cada tipo de console
export const consoleFileExtensions: Record<ConsoleType, string[]> = {
    'megadrive': ['.md', '.bin', '.gen', '.smd'],
    'genesis': ['.md', '.bin', '.gen', '.smd'],
    'sega32x': ['.32x', '.bin'],
    'segacd': ['.iso', '.bin', '.img', '.chd'],
    'mastersystem': ['.sms', '.bin'],
    'gamegear': ['.gg', '.bin'],
    'sg1000': ['.sg', '.bin'],
    'nes': ['.nes', '.unf', '.unif', '.fds'],
    'snes': ['.sfc', '.smc', '.fig', '.swc'],
    'gameboy': ['.gb', '.gbc'],
    'gba': ['.gba'],
    'n64': ['.n64', '.v64', '.z64', '.rom'],
    'psx': ['.bin', '.iso', '.img', '.chd', '.pbp'],
    'tms9918': ['.rom', '.bin'],
    'atari7800': ['.a78', '.bin'],
    'gameboyadvanced': ['.gba'],
    'pcengine': ['.pce', '.bin', '.chd'],
    'neogeo': ['.neo', '.bin', '.zip'],
    'saturn': ['.iso', '.bin', '.img', '.chd'],
    'playstation': ['.bin', '.iso', '.img', '.chd', '.pbp'],
    '32x': ['.32x', '.bin'],
};

export interface EmulatorState {
    isRunning: boolean;
    isPaused: boolean;
    currentConsole: ConsoleType | null;
    fps: number;
    loadedRom: string | null;
    frameSkip: number;
    audioEnabled: boolean;
    volume: number;
    error: string | null;
    saveStates: SaveState[];
    rewindEnabled: boolean;
    rewindBufferSize: number;
    controllerConfig: ControllerConfig[];
}

export interface SaveState {
    id: string;
    name: string;
    romName: string;
    consoleType: ConsoleType;
    timestamp: number;
    thumbnail: string | null;
    metadata: any;
}

export interface ControllerConfig {
    controllerId: number;
    controllerType: 'keyboard' | 'gamepad';
    mappings: {
        [key: string]: string | number;
    };
}

export interface RomInfo {
    id: string;
    name: string;
    path: string;
    size: number;
    consoleType: ConsoleType;
    region: string;
    lastPlayed: number | null;
    favorite: boolean;
    playCount: number;
    totalPlayTime: number;
    metadata: {
        publisher?: string;
        year?: string;
        realTitle?: string;
        saveStates?: SaveState[];
        headerInfo?: any;
        [key: string]: any;
    };
}

export interface FrameData {
    imageData: Uint8Array;
    width: number;
    height: number;
    timestamp: number;
}

export interface EmulatorCommand {
    type: string;
    payload: any;
}

export interface EmulatorResponse {
    type: string;
    success: boolean;
    payload?: any;
    error?: string;
}

// Configuração do emulador
export interface EmulatorConfig {
    romPath: string;
    saveStatesPath: string;
    controllerMapping: Record<string, any>;
    videoSettings: {
        fullscreen: boolean;
        filteringMode: 'nearest' | 'linear' | 'crt';
        aspectRatio: 'auto' | '4:3' | '16:9' | 'original';
    };
    audioSettings: {
        enabled: boolean;
        volume: number;
    };
}
