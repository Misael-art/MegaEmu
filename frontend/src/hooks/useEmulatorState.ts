import { useState, useEffect, useCallback } from 'react';
import { EmulatorState, FrameData, ConsoleType, EmulatorCommand, EmulatorConfig, SaveState } from '../types/emulator.types';
import emulatorWebSocket from '../services/emulator/websocket';
import emulatorApiService from '../services/emulator/restApi';
import { useAppDispatch } from '../state/store';
import { addSaveState, updateEmulatorState } from '../state/slices/emulatorSlice';

interface UseEmulatorStateOptions {
    autoConnect?: boolean;
}

// Configuração padrão do emulador
const defaultState: EmulatorState = {
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

export function useEmulatorState(options: UseEmulatorStateOptions = {}) {
    const { autoConnect = true } = options;
    const dispatch = useAppDispatch();

    const [connected, setConnected] = useState(false);
    const [emulatorState, setEmulatorState] = useState<EmulatorState>(defaultState);
    const [currentFrame, setCurrentFrame] = useState<FrameData | null>(null);
    const [connecting, setConnecting] = useState(false);
    const [error, setError] = useState<string | null>(null);
    const [loading, setLoading] = useState<boolean>(true);

    // Função para conectar ao emulador
    const connect = useCallback(async () => {
        if (connected || connecting) return;

        setConnecting(true);
        setError(null);

        try {
            // Conecta ao WebSocket
            await emulatorWebSocket.connect();

            // Busca o estado atual do emulador via API REST
            const emulatorStateData = await emulatorApiService.getEmulatorConfig();

            // Atualiza o estado com as configurações disponíveis
            setEmulatorState(prev => ({
                ...prev,
                ...emulatorStateData
            }));

            setConnected(true);
        } catch (err) {
            setError('Falha ao conectar ao emulador: ' + (err instanceof Error ? err.message : String(err)));
        } finally {
            setConnecting(false);
        }
    }, []);

    // Função para desconectar do emulador
    const disconnect = useCallback(() => {
        emulatorWebSocket.disconnect();
        setConnected(false);
    }, []);

    // Função para enviar comandos ao emulador
    const sendCommand = useCallback((command: EmulatorCommand) => {
        if (!connected) {
            console.warn('Não é possível enviar comando: não conectado ao emulador');
            return;
        }

        emulatorWebSocket.sendCommand(command);
    }, [connected]);

    // Funções para controle do emulador
    const startEmulation = useCallback(() => {
        sendCommand({ type: 'START', payload: {} });
    }, [sendCommand]);

    const pauseEmulation = useCallback(() => {
        sendCommand({ type: 'PAUSE', payload: {} });
    }, [sendCommand]);

    const resumeEmulation = useCallback(() => {
        sendCommand({ type: 'RESUME', payload: {} });
    }, [sendCommand]);

    const stopEmulation = useCallback(() => {
        sendCommand({ type: 'STOP', payload: {} });
    }, [sendCommand]);

    const loadRom = useCallback(async (romId: string, saveStateId?: string) => {
        try {
            // Verifica se o emulador está conectado
            if (!connected) {
                console.error('Emulador não está conectado');
                return false;
            }

            // Atualiza o estado localmente para feedback imediato
            setEmulatorState(prev => ({
                ...prev,
                isRunning: true,
                isPaused: false,
                loadedRom: romId,
                error: null
            }));

            // Despacha a ação para atualizar o estado no Redux
            dispatch(updateEmulatorState({
                isRunning: true,
                isPaused: false,
                loadedRom: romId,
                error: null
            }));

            return true;
        } catch (err) {
            console.error('Erro ao carregar ROM:', err);

            // Atualiza o estado com o erro
            const errorMessage = err instanceof Error ? err.message : 'Erro desconhecido';

            setEmulatorState(prev => ({
                ...prev,
                isRunning: false,
                error: errorMessage
            }));

            dispatch(updateEmulatorState({
                isRunning: false,
                error: errorMessage
            }));

            return false;
        }
    }, [connected, dispatch]);

    const resetEmulation = useCallback(() => {
        sendCommand({ type: 'RESET', payload: {} });
    }, [sendCommand]);

    const createSaveState = useCallback(async (name: string) => {
        if (!emulatorState.loadedRom) {
            console.error('Nenhuma ROM carregada para criar save state');
            return false;
        }

        try {
            const result = await emulatorApiService.createSaveState(
                emulatorState.loadedRom,
                { name }, // Dados do save state
                undefined // Screenshot opcional - usando undefined em vez de null
            );

            if (result) {
                // Adiciona o save state ao Redux
                dispatch(addSaveState(result));
                return true;
            }
            return false;
        } catch (err) {
            console.error('Erro ao criar save state:', err);
            return false;
        }
    }, [emulatorState.loadedRom, dispatch]);

    const loadSaveState = useCallback(async (saveStateId: string) => {
        try {
            const saveStateData = await emulatorApiService.loadSaveState(saveStateId);

            if (saveStateData) {
                // Busca o estado atualizado do emulador
                const emulatorStateData = await emulatorApiService.getEmulatorConfig();

                // Atualiza o estado do emulador
                dispatch(updateEmulatorState(emulatorStateData));
                return true;
            }
            return false;
        } catch (err) {
            console.error('Erro ao carregar save state:', err);
            return false;
        }
    }, [dispatch]);

    const updateConfig = useCallback(async (config: Partial<EmulatorState>) => {
        try {
            const updatedState = await emulatorApiService.updateEmulatorConfig(config);

            if (updatedState) {
                dispatch(updateEmulatorState(updatedState));
                return true;
            }
            return false;
        } catch (err) {
            console.error('Falha ao atualizar configuração:', err);
            return false;
        }
    }, [dispatch]);

    // Configura ouvintes para WebSocket
    useEffect(() => {
        // Ouvinte para status de conexão
        const disconnectConnectionStatus = emulatorWebSocket.onConnectionStatus(status => {
            setConnected(status);
            if (!status) {
                setError('Conexão WebSocket perdida');
            }
        });

        // Ouvinte para atualizações de estado
        const disconnectStateUpdate = emulatorWebSocket.onMessage('STATE_UPDATE', response => {
            if (response.success && response.payload) {
                setEmulatorState(prev => ({ ...prev, ...response.payload }));
            }
        });

        // Ouvinte para frames
        const disconnectFrameUpdate = emulatorWebSocket.onMessage('FRAME', response => {
            if (response.success && response.payload) {
                setCurrentFrame(response.payload as FrameData);
            }
        });

        // Ouvinte para erros
        const disconnectError = emulatorWebSocket.onMessage('ERROR', response => {
            setError(response.error || 'Erro desconhecido do emulador');
        });

        return () => {
            disconnectConnectionStatus();
            disconnectStateUpdate();
            disconnectFrameUpdate();
            disconnectError();
        };
    }, []);

    // Conecta automaticamente se autoConnect = true
    useEffect(() => {
        if (autoConnect) {
            connect();
        }

        return () => {
            disconnect();
        };
    }, [autoConnect, connect, disconnect]);

    return {
        connected,
        connecting,
        error,
        emulatorState,
        currentFrame,
        connect,
        disconnect,
        sendCommand,
        startEmulation,
        pauseEmulation,
        resumeEmulation,
        stopEmulation,
        loadRom,
        resetEmulation,
        createSaveState,
        loadSaveState,
        updateConfig,
        loading,
        initEmulator: connect
    };
}
