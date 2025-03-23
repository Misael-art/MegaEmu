/**
 * Mock das funções fs e path para uso no navegador
 *
 * Este arquivo fornece implementações simuladas de funções comuns do Node.js
 * para permitir que o código que depende de fs e path funcione no navegador.
 */

// Criamos tipos para garantir compatibilidade com as implementações reais
interface FileStats {
    isDirectory: () => boolean;
    isFile: () => boolean;
    size: number;
    mtime: Date;
}

// Mock para o módulo 'fs'
const fsMock = {
    existsSync: (path: string): boolean => {
        console.log('[FS MOCK] existsSync:', path);
        // No navegador, simulamos que o arquivo/diretório sempre existe
        return true;
    },

    readdirSync: (path: string): string[] => {
        console.log('[FS MOCK] readdirSync:', path);
        // Retorna uma lista de arquivos mockada
        return [
            'rom1.md',
            'rom2.sms',
            'save1.state',
            'rom3.nes',
            'rom4.sfc'
        ];
    },

    statSync: (path: string): FileStats => {
        console.log('[FS MOCK] statSync:', path);
        // Simula um objeto de estatísticas de arquivo
        return {
            isDirectory: () => path.indexOf('.') === -1, // Se não tem extensão, consideramos como diretório
            isFile: () => path.indexOf('.') !== -1,      // Se tem extensão, consideramos como arquivo
            size: 1024 * 1024,                          // 1MB como tamanho padrão
            mtime: new Date()                            // Data atual como data de modificação
        };
    },

    readFileSync: (path: string, encoding?: string): string | Uint8Array => {
        console.log('[FS MOCK] readFileSync:', path, encoding);

        // Se a opção de codificação for especificada como utf8, retorna um JSON mockado
        if (encoding === 'utf8') {
            if (path.endsWith('.json')) {
                return JSON.stringify({
                    name: "Mock File",
                    data: {
                        type: "mock",
                        value: "Este é um arquivo JSON mockado"
                    }
                });
            }
            return "Conteúdo de texto mockado para " + path;
        }

        // Caso contrário, retorna um array vazio representando dados binários
        return new Uint8Array([0, 1, 2, 3, 4, 5]);
    }
};

// Mock para o módulo 'path'
const pathMock = {
    join: (...paths: string[]): string => {
        console.log('[PATH MOCK] join:', paths);
        // Simples concatenação de caminhos com '/'
        return paths
            .filter(Boolean)
            .join('/')
            .replace(/\/+/g, '/'); // Remove barras duplicadas
    },

    basename: (path: string, ext?: string): string => {
        console.log('[PATH MOCK] basename:', path, ext);
        // Pega o último segmento do caminho
        const base = path.split('/').pop() || '';

        // Se a extensão for especificada, remove-a do resultado
        if (ext && base.endsWith(ext)) {
            return base.slice(0, -ext.length);
        }

        return base;
    },

    dirname: (path: string): string => {
        console.log('[PATH MOCK] dirname:', path);
        // Retorna o diretório do caminho
        const segments = path.split('/');
        segments.pop(); // Remove o último segmento (arquivo)
        return segments.join('/') || '.';
    },

    extname: (path: string): string => {
        console.log('[PATH MOCK] extname:', path);
        // Retorna a extensão do arquivo
        const base = path.split('/').pop() || '';
        const lastDotIndex = base.lastIndexOf('.');
        return lastDotIndex < 0 ? '' : base.slice(lastDotIndex);
    }
};

// Utilitário para trabalhar com base64
export const base64 = {
    encode: (str: string): string => {
        return btoa(str);
    },
    decode: (str: string): string => {
        return atob(str);
    }
};

// Exportação padrão para permitir importações como 'import fs from "fs"'
export default fsMock;

// Exportações nomeadas para diferentes estilos de importação
export const fs = fsMock;
export const path = pathMock;
