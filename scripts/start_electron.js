/**
 * Script para iniciar o Electron com o frontend React em modo de desenvolvimento
 */

const { execSync, spawn } = require('child_process');
const path = require('path');
const fs = require('fs');
const os = require('os');

const isWindows = os.platform() === 'win32';
const rootDir = path.resolve(__dirname, '../..');
const frontendDir = path.join(rootDir, 'frontend');

console.log('======================================');
console.log('    Mega Emu - Modo Desktop (Electron)');
console.log('======================================');
console.log('');

// Verificar se o diretório frontend existe
if (!fs.existsSync(frontendDir)) {
    console.error(`Erro: Diretório do frontend não encontrado em ${frontendDir}`);
    process.exit(1);
}

// Mudar para o diretório do frontend
console.log(`Navegando para o diretório do frontend: ${frontendDir}`);
process.chdir(frontendDir);

// Verificar se o arquivo package.json existe
if (!fs.existsSync(path.join(frontendDir, 'package.json'))) {
    console.error('Erro: package.json não encontrado no diretório do frontend');
    process.exit(1);
}

// Configurações
const electronExecPath = path.join(frontendDir, 'node_modules', '.bin', 'electron');
const electronMainPath = path.join(frontendDir, 'public', 'electron.js');

// Verifica se a compilação existe
const buildDir = path.join(frontendDir, 'build');

try {
    // Verifica se o diretório build existe e contém o index.html
    if (fs.existsSync(buildDir) && fs.existsSync(path.join(buildDir, 'index.html'))) {
        console.log('Build encontrado. Iniciando Electron em modo de produção...');
    } else {
        console.log('Build não encontrado! Executando npm run build...');
        process.chdir(frontendDir);
        execSync('npm run build', { stdio: 'inherit' });
    }

    // Inicia o Electron apontando para o arquivo principal
    const electronProcess = spawn(electronExecPath, [electronMainPath], {
        stdio: 'inherit',
        env: {
            ...process.env,
            NODE_ENV: 'production',
            ELECTRON_START_URL: null
        }
    });

    electronProcess.on('close', (code) => {
        console.log(`Electron encerrado com código ${code}`);
    });

    electronProcess.on('error', (err) => {
        console.error('Erro ao iniciar o Electron:', err);
    });

} catch (error) {
    console.error('Erro:', error);
}
