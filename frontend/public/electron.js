const { app, BrowserWindow, ipcMain, dialog } = require('electron');
const path = require('path');
const fs = require('fs');
const url = require('url');

// Mantenha uma referência global do objeto da janela.
// Se você não fizer isso, a janela será fechada automaticamente
// quando o objeto JavaScript for coletado pelo garbage collector.
let mainWindow;

// Determinar se estamos em modo de desenvolvimento
const isDev = process.env.NODE_ENV === 'development' || !app.isPackaged;
console.log('Electron iniciando em modo:', isDev ? 'desenvolvimento' : 'produção');

// Diretório base para os arquivos do aplicativo
const appPath = app.getAppPath();

function createWindow() {
    // Criar a janela do navegador.
    mainWindow = new BrowserWindow({
        width: 1280,
        height: 800,
        minWidth: 1024,
        minHeight: 768,
        title: 'Mega Emu',
        icon: path.join(__dirname, 'favicon.ico'),
        backgroundColor: '#121212', // Adiciona cor de fundo para evitar tela branca durante o carregamento
        show: false, // Não mostra até que o conteúdo esteja pronto
        webPreferences: {
            nodeIntegration: false,
            contextIsolation: true,
            enableRemoteModule: false,
            webSecurity: !isDev, // Desativado apenas em desenvolvimento
            preload: path.join(__dirname, 'preload.js')
        }
    });

    // Carregar o arquivo HTML apropriado baseado no ambiente
    let startURL;

    if (isDev) {
        // Em desenvolvimento, carrega do servidor local
        startURL = 'http://localhost:3000';
    } else {
        // Em produção, carrega do arquivo local
        let indexPath;
        if (fs.existsSync(path.join(__dirname, '../build/index.html'))) {
            indexPath = path.join(__dirname, '../build/index.html');
        } else if (fs.existsSync(path.join(__dirname, './index.html'))) {
            indexPath = path.join(__dirname, './index.html');
        } else {
            console.error('Não foi possível encontrar index.html');
            app.quit();
            return;
        }
        startURL = url.format({
            pathname: indexPath,
            protocol: 'file:',
            slashes: true
        });
    }

    console.log('Carregando URL:', startURL);

    // Mostra a janela quando o conteúdo estiver pronto
    mainWindow.once('ready-to-show', () => {
        mainWindow.show();
    });

    mainWindow.loadURL(startURL)
        .then(() => {
            console.log('Aplicação carregada com sucesso');
        })
        .catch(err => {
            console.error('Erro ao carregar aplicação:', err);
        });

    // Abre o DevTools em modo de desenvolvimento
    if (isDev) {
        console.log('Abrindo DevTools');
        mainWindow.webContents.openDevTools();
    }

    // Adiciona manipuladores de eventos para a janela
    mainWindow.webContents.on('did-finish-load', () => {
        console.log('Conteúdo da janela carregado');
    });

    mainWindow.webContents.on('did-fail-load', (event, errorCode, errorDescription) => {
        console.error('Falha ao carregar a janela:', errorCode, errorDescription);
    });

    // Emitido quando a janela é fechada.
    mainWindow.on('closed', function () {
        // Elimina a referência do objeto da janela, geralmente você armazenaria as janelas
        // em um array se seu aplicativo suportar várias janelas, este é o momento
        // em que você deve excluir o elemento correspondente.
        mainWindow = null;
    });
}

// Este método será chamado quando o Electron terminar de inicializar
app.whenReady().then(() => {
    console.log('Electron pronto');
    createWindow();

    app.on('activate', function () {
        // No macOS é comum recriar uma janela no aplicativo quando o
        // ícone da doca é clicado e não existem outras janelas abertas.
        if (mainWindow === null) {
            createWindow();
        }
    });
});

// Sair quando todas as janelas estiverem fechadas.
app.on('window-all-closed', function () {
    // No macOS é comum para aplicativos e sua barra de menu
    // permanecerem ativos até que o usuário explicitamente encerre com Cmd + Q
    if (process.platform !== 'darwin') {
        app.quit();
    }
});

// Handlers para acesso ao sistema de arquivos
ipcMain.handle('fs-read-dir', async (event, dir) => {
    try {
        return fs.readdirSync(dir);
    } catch (error) {
        console.error('Erro ao ler diretório:', error);
        throw error;
    }
});

ipcMain.handle('fs-read-file', async (event, filePath) => {
    try {
        return fs.readFileSync(filePath);
    } catch (error) {
        console.error('Erro ao ler arquivo:', error);
        throw error;
    }
});

// Configurar IPC Handlers para comunicação com o frontend
ipcMain.handle('open-file-dialog', async () => {
    try {
        const { canceled, filePaths } = await dialog.showOpenDialog({
            properties: ['openFile'],
            filters: [
                { name: 'ROMs', extensions: ['bin', 'rom', 'nes', 'smd', 'sms', 'gb', 'gbc', 'a78'] },
                { name: 'Todos os Arquivos', extensions: ['*'] }
            ]
        });

        if (!canceled && filePaths.length > 0) {
            return filePaths[0];
        }
        return null;
    } catch (error) {
        console.error('Erro ao abrir diálogo de arquivo:', error);
        return null;
    }
});

// Manipulador para obter o diretório de roms
ipcMain.handle('get-roms-directory', async () => {
    try {
        const appDataPath = app.getPath('userData');
        const romsPath = path.join(appDataPath, 'roms');

        // Garante que o diretório existe
        if (!fs.existsSync(romsPath)) {
            fs.mkdirSync(romsPath, { recursive: true });
        }

        return romsPath;
    } catch (error) {
        console.error('Erro ao obter diretório de ROMs:', error);
        return null;
    }
});

// Adicione outros handlers conforme necessário para interação com o sistema operacional
