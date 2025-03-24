/**
 * Utilitários para detecção de ambiente e compatibilidade entre desktop e navegador
 */

interface ElectronAPI {
    fsReadDir: (dir: string) => Promise<string[]>;
    fsReadFile: (path: string) => Promise<Buffer>;
    isElectron: () => boolean;
    getVersion: () => string;
    showNotification: (title: string, body: string) => void;
}

// Declara a extensão da interface Window para incluir o objeto electron
declare global {
    interface Window {
        electron?: ElectronAPI;
    }
}

// Verifica se estamos em ambiente desktop (Electron) ou navegador web
export const isElectron = (): boolean => {
    // Usando o método exposto pelo contextBridge se disponível
    if (window.electron && typeof window.electron.isElectron === 'function') {
        return window.electron.isElectron();
    }

    // Fallback para detecção baseada no objeto process
    // @ts-ignore - process.type existe no Electron mas não é reconhecido pelo TypeScript
    return window && window.process && process.type ? true : false;
};

// Retorna o ambiente atual
export const getEnvironment = (): 'desktop' | 'browser' => {
    return isElectron() ? 'desktop' : 'browser';
};

// Verifica se o recurso está disponível no ambiente atual
export function isFeatureAvailable(feature: 'fs' | 'path' | 'native-dialogs'): boolean {
    const env = getEnvironment();

    switch (feature) {
        case 'fs':
        case 'path':
        case 'native-dialogs':
            return env === 'desktop';
        default:
            return false;
    }
}

// Obtém a API do Electron
export const getElectronAPI = (): ElectronAPI | null => {
    if (isElectron() && window.electron) {
        return window.electron;
    }
    return null;
};

// Logs específicos de ambiente
export const envLogger = {
    info: (message: string) => {
        console.info(`[${getEnvironment().toUpperCase()}] ${message}`);
    },
    warn: (message: string) => {
        console.warn(`[${getEnvironment().toUpperCase()}] ${message}`);
    },
    error: (message: string) => {
        console.error(`[${getEnvironment().toUpperCase()}] ${message}`);
    }
};
