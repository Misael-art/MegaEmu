import { RomInfo, ConsoleType, consoleAliases, consoleFileExtensions, SaveState } from '../../types/emulator.types';
import { fs, path, base64 } from './fsMock';
import { getEnvironment, envLogger, isFeatureAvailable } from '../../utils/environment';

// Caminho base para as ROMs - pode ser alterado via configuração
const DEFAULT_ROMS_PATH = './resources/roms';
const SAVESTATES_PATH = './resources/savestates';

// Dados de exemplo para desenvolvimento
const mockRoms: RomInfo[] = [
    {
        id: base64.encode('/roms/megadrive/sonic.md'),
        name: 'Sonic The Hedgehog',
        path: '/roms/megadrive/sonic.md',
        size: 512000,
        consoleType: 'megadrive',
        region: 'USA',
        lastPlayed: Date.now() - 1000 * 60 * 60 * 24, // 1 dia atrás
        favorite: true,
        playCount: 15,
        totalPlayTime: 1000 * 60 * 60 * 8, // 8 horas
        metadata: {
            publisher: 'SEGA',
            saveStates: []
        }
    },
    {
        id: base64.encode('/roms/nes/mario3.nes'),
        name: 'Super Mario Bros. 3',
        path: '/roms/nes/mario3.nes',
        size: 384000,
        consoleType: 'nes',
        region: 'Japan',
        lastPlayed: Date.now() - 1000 * 60 * 60 * 48, // 2 dias atrás
        favorite: false,
        playCount: 7,
        totalPlayTime: 1000 * 60 * 60 * 4, // 4 horas
        metadata: {
            publisher: 'Nintendo',
            saveStates: []
        }
    },
    {
        id: base64.encode('/roms/snes/chronotrigger.sfc'),
        name: 'Chrono Trigger',
        path: '/roms/snes/chronotrigger.sfc',
        size: 4194304,
        consoleType: 'snes',
        region: 'USA',
        lastPlayed: Date.now() - 1000 * 60 * 60 * 72, // 3 dias atrás
        favorite: true,
        playCount: 3,
        totalPlayTime: 1000 * 60 * 60 * 12, // 12 horas
        metadata: {
            publisher: 'Square',
            saveStates: []
        }
    },
    {
        id: base64.encode('/roms/mastersystem/alexkidd.sms'),
        name: 'Alex Kidd in Miracle World',
        path: '/roms/mastersystem/alexkidd.sms',
        size: 262144,
        consoleType: 'mastersystem',
        region: 'Europe',
        lastPlayed: null,
        favorite: false,
        playCount: 0,
        totalPlayTime: 0,
        metadata: {
            publisher: 'SEGA',
            saveStates: []
        }
    },
    {
        id: base64.encode('/roms/gameboy/tetris.gb'),
        name: 'Tetris',
        path: '/roms/gameboy/tetris.gb',
        size: 32768,
        consoleType: 'gameboy',
        region: 'Japan',
        lastPlayed: Date.now() - 1000 * 60 * 60 * 24 * 7, // 1 semana atrás
        favorite: false,
        playCount: 20,
        totalPlayTime: 1000 * 60 * 60 * 5, // 5 horas
        metadata: {
            publisher: 'Nintendo',
            saveStates: []
        }
    }
];

class RomService {
    private romsBasePath: string;
    private saveStatesPath: string;
    private isDesktopEnv: boolean;

    constructor(basePath: string = DEFAULT_ROMS_PATH, saveStatesPath: string = SAVESTATES_PATH) {
        this.romsBasePath = basePath;
        this.saveStatesPath = saveStatesPath;
        this.isDesktopEnv = getEnvironment() === 'desktop';

        envLogger.info(`RomService inicializado em ambiente ${getEnvironment()}`);
        envLogger.info(`Caminho base ROMs: ${this.romsBasePath}`);
        envLogger.info(`Caminho saveStates: ${this.saveStatesPath}`);
    }

    /**
     * Define um novo caminho base para as ROMs
     */
    setRomsPath(newPath: string): void {
        this.romsBasePath = newPath;
        envLogger.info(`Novo caminho de ROMs definido: ${newPath}`);
    }

    /**
     * Obtém o caminho base atual para as ROMs
     */
    getRomsPath(): string {
        return this.romsBasePath;
    }

    /**
     * Define um novo caminho para os save states
     */
    setSaveStatesPath(newPath: string): void {
        this.saveStatesPath = newPath;
        envLogger.info(`Novo caminho de save states definido: ${newPath}`);
    }

    /**
     * Obtém o caminho atual para os save states
     */
    getSaveStatesPath(): string {
        return this.saveStatesPath;
    }

    /**
     * Detecta o tipo de console com base no nome do diretório ou extensão de arquivo
     */
    detectConsoleType(dirName: string, fileName: string): ConsoleType | null {
        // Primeiro tenta pelo nome do diretório (case insensitive)
        const dirLower = dirName.toLowerCase();

        // Verificar se o nome do diretório está diretamente nos aliases
        if (consoleAliases[dirLower]) {
            return consoleAliases[dirLower];
        }

        // Verificar também por correspondências parciais (ex: "Master System" corresponde a "mastersystem")
        for (const [alias, consoleType] of Object.entries(consoleAliases)) {
            if (dirLower.includes(alias) || alias.includes(dirLower)) {
                return consoleType;
            }
        }

        // Se não conseguir pelo diretório, tenta pela extensão
        const ext = path.extname(fileName).toLowerCase();

        for (const [consoleType, extensions] of Object.entries(consoleFileExtensions)) {
            if (extensions.includes(ext)) {
                return consoleType as ConsoleType;
            }
        }

        return null;
    }

    /**
     * Lista todas as ROMs disponíveis
     * No desktop usa as APIs do Node.js, no navegador usa dados simulados
     */
    async scanRoms(): Promise<RomInfo[]> {
        if (!this.isDesktopEnv) {
            envLogger.info('Ambiente navegador: retornando ROMs simuladas');
            return mockRoms;
        }

        envLogger.info('Ambiente desktop: escaneando diretório de ROMs');
        const roms: RomInfo[] = [];

        try {
            // Verifica se o diretório existe
            if (!fs.existsSync(this.romsBasePath)) {
                envLogger.error(`Diretório de ROMs não encontrado: ${this.romsBasePath}`);
                return roms;
            }

            // Lista os subdiretórios (consoles)
            const consoleDirs = fs.readdirSync(this.romsBasePath);

            for (const consoleDir of consoleDirs) {
                const consolePath = path.join(this.romsBasePath, consoleDir);
                const stats = fs.statSync(consolePath);

                // Verifica se é um diretório
                if (!stats.isDirectory()) {
                    continue;
                }

                // Detecta o tipo de console pelo nome do diretório
                let consoleType = this.detectConsoleType(consoleDir, "dummy.rom");

                // Lista os arquivos de ROM no diretório
                const romFiles = fs.readdirSync(consolePath);

                for (const romFile of romFiles) {
                    const romPath = path.join(consolePath, romFile);
                    const fileStats = fs.statSync(romPath);

                    // Pula arquivos como README, etc.
                    if (romFile.startsWith('.') ||
                        romFile === 'README.md' ||
                        fileStats.isDirectory()) {
                        continue;
                    }

                    // Se não definiu o console pelo diretório, tenta pela extensão
                    const fileConsoleType = this.detectConsoleType(consoleDir, romFile);
                    if (!consoleType && fileConsoleType) {
                        consoleType = fileConsoleType;
                    }

                    if (!consoleType) {
                        envLogger.warn(`Não foi possível determinar o tipo de console para: ${romPath}`);
                        continue;
                    }

                    // Extrai nome base do arquivo sem extensão
                    const baseName = path.basename(romFile, path.extname(romFile));

                    // Verifica se há um arquivo de save state associado
                    const saveStates = await this.findSaveStates(consoleType, baseName);

                    // Cria as informações da ROM
                    const romInfo: RomInfo = {
                        id: base64.encode(romPath),
                        name: this.formatRomName(baseName),
                        path: romPath,
                        size: fileStats.size,
                        consoleType: consoleType,
                        region: this.detectRegion(romFile),
                        lastPlayed: null,
                        favorite: false,
                        playCount: 0,
                        totalPlayTime: 0,
                        metadata: {
                            saveStates: saveStates,
                            realTitle: this.formatRomName(baseName),
                            filename: romFile,
                        }
                    };

                    roms.push(romInfo);
                }
            }

            return roms;
        } catch (error) {
            envLogger.error('Erro ao escanear ROMs: ' + String(error));
            // Se ocorrer um erro, retorna dados simulados
            return mockRoms;
        }
    }

    /**
     * Tenta detectar a região da ROM pelo nome do arquivo
     */
    private detectRegion(fileName: string): string {
        const lowerName = fileName.toLowerCase();

        if (lowerName.includes('(usa)') || lowerName.includes('(u)') || lowerName.includes('[u]')) {
            return 'USA';
        }

        if (lowerName.includes('(europe)') || lowerName.includes('(e)') || lowerName.includes('[e]')) {
            return 'Europe';
        }

        if (lowerName.includes('(japan)') || lowerName.includes('(j)') || lowerName.includes('[j]')) {
            return 'Japan';
        }

        if (lowerName.includes('(brazil)') || lowerName.includes('(b)') || lowerName.includes('[b]')) {
            return 'Brazil';
        }

        return 'Unknown';
    }

    /**
     * Formata o nome da ROM para exibição, removendo códigos de região, etc.
     */
    private formatRomName(name: string): string {
        // Remove códigos de região e outros artefatos comuns em nomes de ROM
        return name
            .replace(/\([^)]*\)/g, '') // Remove qualquer coisa entre parênteses
            .replace(/\[[^\]]*\]/g, '') // Remove qualquer coisa entre colchetes
            .replace(/\.(sms|md|gen|nes|sfc|gb|gba)$/i, '') // Remove extensões comuns
            .replace(/\s+/g, ' ') // Remove espaços extras
            .trim();
    }

    /**
     * Encontra os save states associados a uma ROM
     * Funciona em ambos os ambientes (desktop e navegador)
     */
    private async findSaveStates(consoleType: ConsoleType, baseName: string): Promise<SaveState[]> {
        if (!this.isDesktopEnv) {
            return []; // No navegador, retorna lista vazia
        }

        const saveStates: SaveState[] = [];
        // Verificar primeiro no diretório de save states dedicado
        const saveStatePath = path.join(this.saveStatesPath, consoleType.toString(), baseName);

        // Se não existir, verificar dentro do diretório de ROMs (retrocompatibilidade)
        const alternativePath = path.join(this.romsBasePath, '../savestates', consoleType.toString(), baseName);

        try {
            // Tenta primeiro no caminho principal
            if (fs.existsSync(saveStatePath) && fs.statSync(saveStatePath).isDirectory()) {
                await this.loadSaveStatesFromDir(saveStatePath, baseName, consoleType, saveStates);
            }
            // Se não encontrar, tenta no caminho alternativo
            else if (fs.existsSync(alternativePath) && fs.statSync(alternativePath).isDirectory()) {
                await this.loadSaveStatesFromDir(alternativePath, baseName, consoleType, saveStates);
            }
        } catch (error) {
            envLogger.error(`Erro ao buscar save states para ${baseName}: ${String(error)}`);
        }

        return saveStates;
    }

    /**
     * Carrega save states de um diretório específico (apenas desktop)
     */
    private async loadSaveStatesFromDir(
        dirPath: string,
        baseName: string,
        consoleType: ConsoleType,
        saveStates: SaveState[]
    ): Promise<void> {
        if (!this.isDesktopEnv) return;

        const files = fs.readdirSync(dirPath);

        for (const file of files) {
            if (file.endsWith('.state')) {
                const statePath = path.join(dirPath, file);
                const stats = fs.statSync(statePath);

                // Nome do save state (sem a extensão)
                const stateName = path.basename(file, '.state');

                // Tenta ler metadados adicionais se existir um arquivo .json correspondente
                let metadata: any = {
                    realName: this.formatRomName(baseName),
                    playTime: null,
                };

                // Metadados do save state
                const metadataPath = statePath.replace('.state', '.json');
                if (fs.existsSync(metadataPath)) {
                    try {
                        const metadataContent = fs.readFileSync(metadataPath, 'utf8') as string;
                        const parsedMetadata = JSON.parse(metadataContent);
                        metadata = { ...metadata, ...parsedMetadata };
                    } catch (error) {
                        envLogger.warn(`Erro ao ler metadados para ${stateName}: ${String(error)}`);
                    }
                }

                // Cria um thumbnail a partir do screenshot se disponível
                let thumbnail: string | null = null;
                const screenshotFormats = ['.png', '.jpg', '.jpeg'];

                // Procura por screenshots em diferentes formatos
                for (const format of screenshotFormats) {
                    const screenshotPath = statePath.replace('.state', format);
                    if (fs.existsSync(screenshotPath)) {
                        try {
                            // No desktop, podemos usar o fs para ler o arquivo
                            const imageBuffer = fs.readFileSync(screenshotPath);
                            thumbnail = `data:image/${format.substring(1)};base64,${Buffer.from(
                                imageBuffer as any, 'binary'
                            ).toString('base64')}`;
                            break;
                        } catch (error) {
                            envLogger.error(`Erro ao ler screenshot para ${stateName}: ${String(error)}`);
                        }
                    }
                }

                saveStates.push({
                    id: base64.encode(statePath),
                    name: stateName,
                    romName: baseName,
                    consoleType: consoleType,
                    timestamp: stats.mtime.getTime(),
                    thumbnail: thumbnail,
                    metadata: metadata
                });
            }
        }
    }

    /**
     * Carrega os metadados do cabeçalho da ROM
     */
    async loadRomMetadata(romId: string): Promise<any> {
        try {
            // Decodifica o ID para obter o caminho
            const romPath = base64.decode(romId);

            // Buscar o nome do arquivo
            const fileName = path.basename(romPath);

            // Aqui chamaríamos o backend para ler o cabeçalho da ROM
            // Por enquanto, retornamos informações simuladas
            return {
                publisher: 'Desconhecido',
                year: 'Desconhecido',
                realTitle: path.basename(fileName, path.extname(fileName)),
            };
        } catch (error) {
            envLogger.error('Erro ao carregar metadados da ROM: ' + String(error));
            return null;
        }
    }

    /**
     * Obtém a lista de save states para uma ROM específica
     */
    async getSaveStates(romId: string): Promise<SaveState[]> {
        try {
            // Decodifica o ID para obter o caminho
            const romPath = base64.decode(romId);
            const fileName = path.basename(romPath);
            const consoleDir = path.basename(path.dirname(romPath));

            // Busca o nome base do arquivo (sem extensão)
            const baseName = path.basename(fileName, path.extname(fileName));

            // Detecta o tipo de console
            const consoleType = this.detectConsoleType(consoleDir, fileName);

            if (!consoleType) {
                envLogger.error(`Não foi possível determinar o tipo de console para: ${romPath}`);
                return [];
            }

            // Busca os save states
            return await this.findSaveStates(consoleType, baseName);
        } catch (error) {
            envLogger.error('Erro ao carregar save states: ' + String(error));
            return [];
        }
    }
}

// Cria uma instância do serviço
const romService = new RomService();

export default romService;
