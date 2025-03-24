// Definição de tipos para a API Electron exposta pelo preload.js

interface ElectronAPI {
    // Sistema
    system: {
        isElectron: boolean;
        platform: string;
        version: string;
    };

    // Sistema de arquivos
    fs: {
        readDir: (dir: string) => Promise<string[]>;
        readFile: (path: string) => Promise<Buffer>;
    };

    // Diálogos e seleção de arquivos
    openFile: () => Promise<string | null>;
    getRomsDirectory: () => Promise<string | null>;

    // Comunicação
    send: (channel: string, data: any) => void;
    receive: (channel: string, func: (...args: any[]) => void) => void;

    // Notificações
    notification: {
        show: (title: string, body: string) => void;
    };

    // Logs
    log: (message: string) => void;
}

declare global {
    interface Window {
        electron?: ElectronAPI;
    }
}

export { };
