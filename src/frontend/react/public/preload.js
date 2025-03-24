const { contextBridge, ipcRenderer } = require('electron');

// Log para verificar que o preload foi carregado
console.log('Preload script iniciado');

// API Electron exposta para o frontend
contextBridge.exposeInMainWorld('electron', {
    // Informações do sistema
    system: {
        isElectron: true,
        platform: process.platform,
        version: process.versions.electron
    },

    // Sistema de arquivos
    fs: {
        readDir: (dir) => ipcRenderer.invoke('fs-read-dir', dir),
        readFile: (path) => ipcRenderer.invoke('fs-read-file', path)
    },

    // Funções de diálogo e seleção de arquivos
    openFile: () => ipcRenderer.invoke('open-file-dialog'),
    getRomsDirectory: () => ipcRenderer.invoke('get-roms-directory'),

    // Comunicação
    send: (channel, data) => {
        // Lista de canais permitidos para envio
        const validChannels = ['load-rom', 'save-state', 'load-state'];
        if (validChannels.includes(channel)) {
            ipcRenderer.send(channel, data);
        } else {
            console.warn(`Canal de IPC não permitido: ${channel}`);
        }
    },

    receive: (channel, func) => {
        // Lista de canais permitidos para recebimento
        const validChannels = ['emulator-status', 'rom-loaded', 'state-saved'];
        if (validChannels.includes(channel)) {
            // Remover o listener antigo para evitar duplicação
            ipcRenderer.removeAllListeners(channel);
            // Adicionar o novo listener
            ipcRenderer.on(channel, (event, ...args) => func(...args));
        } else {
            console.warn(`Canal de IPC não permitido: ${channel}`);
        }
    },

    // Notificações do sistema
    notification: {
        show: (title, body) => {
            new Notification(title, { body });
        }
    },

    // Função para enviar logs para o console do Electron
    log: (message) => {
        console.log(`[Frontend]: ${message}`);
    }
});

// Log para debug quando o DOM estiver carregado
document.addEventListener('DOMContentLoaded', () => {
    console.log('DOM carregado no contexto do preload');
});

// Adicione aqui outras APIs que deseja expor ao frontend
