import axios, { AxiosError } from 'axios';
import { RomInfo, SaveState, EmulatorConfig, EmulatorState } from '../../types/emulator.types';
import romService from './romService';
import { getEnvironment, envLogger } from '../../utils/environment';

// Configuração base da API
const API_URL = process.env.REACT_APP_API_URL || 'http://localhost:3001';
const API_TIMEOUT = 5000; // 5 segundos de timeout

class EmulatorApiService {
    private api;
    private isDesktopEnv: boolean;

    constructor() {
        this.api = axios.create({
            baseURL: API_URL,
            timeout: API_TIMEOUT,
            headers: {
                'Content-Type': 'application/json',
            },
        });

        this.isDesktopEnv = getEnvironment() === 'desktop';
        envLogger.info(`EmulatorApiService inicializado em ambiente ${getEnvironment()}`);
        envLogger.info(`URL da API: ${API_URL}`);
    }

    /**
     * Verifica se o backend está online
     */
    async checkBackendStatus(): Promise<{ online: boolean; message: string }> {
        try {
            const response = await this.api.get('/status');
            return {
                online: true,
                message: response.data.message || 'Backend online'
            };
        } catch (error) {
            const axiosError = error as AxiosError;

            // Se o erro for de conexão recusada (backend offline)
            if (axiosError.code === 'ECONNREFUSED' || axiosError.code === 'ERR_NETWORK') {
                return {
                    online: false,
                    message: 'Backend offline, usando serviços locais'
                };
            }

            // Outros erros (ex: 500, 404, etc.)
            return {
                online: false,
                message: `Erro ao conectar: ${axiosError.message}`
            };
        }
    }

    /**
     * Obtém a lista de ROMs disponíveis
     */
    async getRomsList(): Promise<RomInfo[]> {
        try {
            // Verifica se o backend está online
            const status = await this.checkBackendStatus();

            if (status.online) {
                // Se o backend está online, usa o API
                envLogger.info('Usando API para buscar lista de ROMs');
                const response = await this.api.get('/roms');
                return response.data;
            } else {
                // Se o backend está offline, usa o serviço local
                envLogger.info('Backend offline, usando serviço local para ROMs');
                return await romService.scanRoms();
            }
        } catch (error) {
            envLogger.error('Erro ao obter lista de ROMs: ' + String(error));
            // Em caso de erro, fallback para o serviço local
            return await romService.scanRoms();
        }
    }

    /**
     * Obtém detalhes de uma ROM específica por ID
     */
    async getRomDetails(romId: string): Promise<RomInfo | null> {
        try {
            // Verifica se o backend está online
            const status = await this.checkBackendStatus();

            if (status.online) {
                // Se o backend está online, usa o API
                envLogger.info(`Usando API para buscar detalhes da ROM: ${romId}`);
                const response = await this.api.get(`/roms/${romId}`);
                return response.data;
            } else {
                // Se o backend está offline, tenta encontrar na lista
                envLogger.info(`Backend offline, buscando ROM localmente: ${romId}`);
                const romsList = await romService.scanRoms();
                return romsList.find(rom => rom.id === romId) || null;
            }
        } catch (error) {
            envLogger.error('Erro ao obter detalhes da ROM: ' + String(error));
            // Em caso de erro, tenta encontrar na lista local
            const romsList = await romService.scanRoms();
            return romsList.find(rom => rom.id === romId) || null;
        }
    }

    /**
     * Busca ROMs por termo de pesquisa
     */
    async searchRoms(query: string): Promise<RomInfo[]> {
        try {
            // Verifica se o backend está online
            const status = await this.checkBackendStatus();

            if (status.online) {
                // Se o backend está online, usa o API
                envLogger.info(`Usando API para pesquisar ROMs: ${query}`);
                const response = await this.api.get(`/roms/search?q=${encodeURIComponent(query)}`);
                return response.data;
            } else {
                // Se o backend está offline, faz a busca local
                envLogger.info(`Backend offline, pesquisando ROMs localmente: ${query}`);
                const romsList = await romService.scanRoms();
                const lowercaseQuery = query.toLowerCase();
                return romsList.filter(rom =>
                    rom.name.toLowerCase().includes(lowercaseQuery) ||
                    (rom.metadata.realTitle && rom.metadata.realTitle.toLowerCase().includes(lowercaseQuery))
                );
            }
        } catch (error) {
            envLogger.error('Erro ao pesquisar ROMs: ' + String(error));
            // Em caso de erro, faz a busca local
            const romsList = await romService.scanRoms();
            const lowercaseQuery = query.toLowerCase();
            return romsList.filter(rom =>
                rom.name.toLowerCase().includes(lowercaseQuery) ||
                (rom.metadata.realTitle && rom.metadata.realTitle.toLowerCase().includes(lowercaseQuery))
            );
        }
    }

    /**
     * Faz upload de uma ROM para o sistema
     */
    async uploadRom(file: File, consoleType: string): Promise<RomInfo | null> {
        try {
            // Verifica se o backend está online (necessário para upload)
            const status = await this.checkBackendStatus();

            if (!status.online) {
                throw new Error('Backend offline, não é possível fazer upload.');
            }

            const formData = new FormData();
            formData.append('file', file);
            formData.append('consoleType', consoleType);

            envLogger.info(`Fazendo upload de ROM: ${file.name} (${consoleType})`);
            const response = await this.api.post('/roms/upload', formData, {
                headers: {
                    'Content-Type': 'multipart/form-data'
                }
            });

            return response.data;
        } catch (error) {
            envLogger.error('Erro ao fazer upload da ROM: ' + String(error));
            return null;
        }
    }

    /**
     * Obtém a lista de save states para uma ROM
     */
    async getSaveStates(romId: string): Promise<SaveState[]> {
        try {
            // Verifica se o backend está online
            const status = await this.checkBackendStatus();

            if (status.online) {
                // Se o backend está online, usa o API
                envLogger.info(`Usando API para buscar save states: ${romId}`);
                const response = await this.api.get(`/savestates/rom/${romId}`);
                return response.data;
            } else {
                // Se o backend está offline, usa o serviço local
                envLogger.info(`Backend offline, buscando save states localmente: ${romId}`);
                return await romService.getSaveStates(romId);
            }
        } catch (error) {
            envLogger.error('Erro ao obter save states: ' + String(error));
            // Em caso de erro, usa o serviço local
            return await romService.getSaveStates(romId);
        }
    }

    /**
     * Cria um novo save state
     */
    async createSaveState(romId: string, saveData: any, screenshot?: string): Promise<SaveState | null> {
        try {
            // Verifica se o backend está online
            const status = await this.checkBackendStatus();

            if (!status.online) {
                throw new Error('Backend offline, não é possível criar save state.');
            }

            const data = {
                romId,
                saveData,
                screenshot,
                timestamp: Date.now()
            };

            envLogger.info(`Criando save state para ROM: ${romId}`);
            const response = await this.api.post('/savestates', data);
            return response.data;
        } catch (error) {
            envLogger.error('Erro ao criar save state: ' + String(error));
            return null;
        }
    }

    /**
     * Carrega um save state
     */
    async loadSaveState(saveStateId: string): Promise<any> {
        try {
            // Verifica se o backend está online
            const status = await this.checkBackendStatus();

            if (!status.online) {
                throw new Error('Backend offline, não é possível carregar save state.');
            }

            envLogger.info(`Carregando save state: ${saveStateId}`);
            const response = await this.api.get(`/savestates/${saveStateId}`);
            return response.data;
        } catch (error) {
            envLogger.error('Erro ao carregar save state: ' + String(error));
            return null;
        }
    }

    /**
     * Deleta um save state
     */
    async deleteSaveState(saveStateId: string): Promise<boolean> {
        try {
            // Verifica se o backend está online
            const status = await this.checkBackendStatus();

            if (!status.online) {
                throw new Error('Backend offline, não é possível deletar save state.');
            }

            envLogger.info(`Deletando save state: ${saveStateId}`);
            await this.api.delete(`/savestates/${saveStateId}`);
            return true;
        } catch (error) {
            envLogger.error('Erro ao deletar save state: ' + String(error));
            return false;
        }
    }

    /**
     * Converte uma configuração EmulatorConfig para EmulatorState
     * @param config Configuração do EmulatorConfig
     */
    private configToState(config: EmulatorConfig): Partial<EmulatorState> {
        return {
            audioEnabled: config.audioSettings.enabled,
            volume: config.audioSettings.volume,
            // Adicione outros mapeamentos conforme necessário
        };
    }

    /**
     * Converte um estado EmulatorState para EmulatorConfig
     * @param state Estado do EmulatorState
     */
    private stateToConfig(state: Partial<EmulatorState>): Partial<EmulatorConfig> {
        const config: Partial<EmulatorConfig> = {};

        // Configurações de áudio
        if (state.audioEnabled !== undefined || state.volume !== undefined) {
            config.audioSettings = {
                enabled: state.audioEnabled ?? true,
                volume: state.volume ?? 100
            };
        }

        // Se um diretório personalizado foi definido no serviço, incluí-lo na configuração
        const currentRomsPath = romService.getRomsPath();
        if (currentRomsPath) {
            config.romPath = currentRomsPath;
        }

        // Obter também o caminho de saves se disponível
        const currentSavesPath = romService.getSaveStatesPath();
        if (currentSavesPath) {
            config.saveStatesPath = currentSavesPath;
        }

        // Adicione outros mapeamentos conforme necessário

        return config;
    }

    /**
     * Obtém a configuração atual do emulador
     */
    async getEmulatorConfig(): Promise<EmulatorState> {
        try {
            // Verifica se o backend está online
            const status = await this.checkBackendStatus();

            if (status.online) {
                // Se o backend está online, usa o API
                envLogger.info('Buscando configuração do emulador');
                const response = await this.api.get('/emulator/config');
                const configData = response.data as EmulatorConfig;

                // Converte para EmulatorState
                const stateData = this.configToState(configData);

                // Cria um novo estado com valores padrão e os valores convertidos
                return {
                    ...defaultEmulatorState,
                    ...stateData
                };
            } else {
                // Se o backend está offline, retorna configuração padrão
                envLogger.info('Backend offline, usando configuração padrão do emulador');
                return defaultEmulatorState;
            }
        } catch (error) {
            envLogger.error('Erro ao obter configuração do emulador: ' + String(error));
            // Em caso de erro, retorna configuração padrão
            return defaultEmulatorState;
        }
    }

    /**
     * Atualiza a configuração do emulador
     */
    async updateEmulatorConfig(emulatorState: Partial<EmulatorState>): Promise<EmulatorState> {
        try {
            // Verifica se o backend está online
            const status = await this.checkBackendStatus();

            if (!status.online) {
                throw new Error('Backend offline, não é possível atualizar configuração.');
            }

            envLogger.info('Atualizando configuração do emulador');

            // Converte o estado para formato de configuração
            const config = this.stateToConfig(emulatorState);

            // Atualiza o romService se o caminho das ROMs for alterado
            if ('romPath' in config) {
                romService.setRomsPath(config.romPath as string);
            }

            // Atualiza o romService se o caminho dos save states for alterado
            if ('saveStatesPath' in config) {
                romService.setSaveStatesPath(config.saveStatesPath as string);
            }

            // Envia a configuração para o backend
            const response = await this.api.put('/emulator/config', config);
            const updatedConfig = response.data as EmulatorConfig;

            // Converte de volta para formato de estado
            const updatedState = this.configToState(updatedConfig);

            // Retorna o estado atualizado
            return {
                ...defaultEmulatorState,
                ...updatedState
            };
        } catch (error) {
            envLogger.error('Erro ao atualizar configuração do emulador: ' + String(error));
            throw error;
        }
    }

    /**
     * Seleciona um diretório para ROMs usando o diálogo nativo (apenas desktop)
     */
    async selectRomsDirectory(): Promise<string | null> {
        if (!this.isDesktopEnv) {
            envLogger.warn('Seleção de diretório não disponível no navegador');
            return null;
        }

        try {
            // Verifica se o backend está online
            const status = await this.checkBackendStatus();

            if (!status.online) {
                throw new Error('Backend offline, não é possível selecionar diretório.');
            }

            envLogger.info('Abrindo diálogo para selecionar diretório de ROMs');
            const response = await this.api.get('/emulator/selectDirectory?type=roms');

            if (response.data && response.data.path) {
                romService.setRomsPath(response.data.path);
                return response.data.path;
            }

            return null;
        } catch (error) {
            envLogger.error('Erro ao selecionar diretório de ROMs: ' + String(error));
            return null;
        }
    }

    /**
     * Seleciona um diretório para save states usando o diálogo nativo (apenas desktop)
     */
    async selectSaveStatesDirectory(): Promise<string | null> {
        if (!this.isDesktopEnv) {
            envLogger.warn('Seleção de diretório não disponível no navegador');
            return null;
        }

        try {
            // Verifica se o backend está online
            const status = await this.checkBackendStatus();

            if (!status.online) {
                throw new Error('Backend offline, não é possível selecionar diretório.');
            }

            envLogger.info('Abrindo diálogo para selecionar diretório de save states');
            const response = await this.api.get('/emulator/selectDirectory?type=savestates');

            if (response.data && response.data.path) {
                romService.setSaveStatesPath(response.data.path);
                return response.data.path;
            }

            return null;
        } catch (error) {
            envLogger.error('Erro ao selecionar diretório de save states: ' + String(error));
            return null;
        }
    }
}

// Estado padrão do emulador
const defaultEmulatorState: EmulatorState = {
    isRunning: false,
    isPaused: false,
    currentConsole: null,
    fps: 60,
    loadedRom: null,
    frameSkip: 0,
    audioEnabled: true,
    volume: 100,
    error: null,
    saveStates: [],
    rewindEnabled: false,
    rewindBufferSize: 60,
    controllerConfig: []
};

// Cria uma instância do serviço
const emulatorApiService = new EmulatorApiService();

export default emulatorApiService;
