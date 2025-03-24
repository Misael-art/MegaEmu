import React from 'react';
import { render, screen, fireEvent, waitFor } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import RomSelector from '../RomSelector';

// Mock da API do Electron
const mockElectron = {
    openFile: jest.fn()
};

describe('RomSelector', () => {
    beforeEach(() => {
        // Limpa todos os mocks antes de cada teste
        jest.clearAllMocks();
        // Remove a API do Electron do objeto window
        delete (window as any).electron;
        // Limpa os console.logs
        jest.spyOn(console, 'log').mockImplementation(() => { });
        jest.spyOn(console, 'error').mockImplementation(() => { });
    });

    it('renderiza corretamente com a lista de ROMs', () => {
        render(<RomSelector />);

        expect(screen.getByText('Biblioteca de ROMs')).toBeInTheDocument();
        expect(screen.getByPlaceholderText('Buscar ROMs...')).toBeInTheDocument();
        expect(screen.getByText('Sonic the Hedgehog')).toBeInTheDocument();
        expect(screen.getByText('Super Mario Bros')).toBeInTheDocument();
    });

    it('filtra ROMs baseado na busca', async () => {
        render(<RomSelector />);

        const searchInput = screen.getByPlaceholderText('Buscar ROMs...');
        await userEvent.type(searchInput, 'sonic');

        expect(screen.getByText('Sonic the Hedgehog')).toBeInTheDocument();
        expect(screen.queryByText('Super Mario Bros')).not.toBeInTheDocument();
    });

    it('filtra ROMs por sistema', async () => {
        render(<RomSelector />);

        const searchInput = screen.getByPlaceholderText('Buscar ROMs...');
        await userEvent.type(searchInput, 'SNES');

        expect(screen.getByText('Donkey Kong Country')).toBeInTheDocument();
        expect(screen.queryByText('Sonic the Hedgehog')).not.toBeInTheDocument();
    });

    it('mostra mensagem quando nenhuma ROM é encontrada', async () => {
        render(<RomSelector />);

        const searchInput = screen.getByPlaceholderText('Buscar ROMs...');
        await userEvent.type(searchInput, 'inexistente');

        expect(screen.getByText('Nenhuma ROM encontrada')).toBeInTheDocument();
    });

    it('permite selecionar uma ROM', () => {
        render(<RomSelector />);

        const romItem = screen.getByText('Sonic the Hedgehog');
        fireEvent.click(romItem);

        // O botão de carregar ROM deve estar habilitado
        const loadButton = screen.getByText('Carregar ROM');
        expect(loadButton).not.toBeDisabled();
    });

    it('desabilita o botão de carregar quando nenhuma ROM está selecionada', () => {
        render(<RomSelector />);

        const loadButton = screen.getByText('Carregar ROM');
        expect(loadButton).toBeDisabled();
    });

    it('loga quando uma ROM é carregada', () => {
        render(<RomSelector />);

        // Seleciona uma ROM
        const romItem = screen.getByText('Sonic the Hedgehog');
        fireEvent.click(romItem);

        // Clica no botão de carregar
        const loadButton = screen.getByText('Carregar ROM');
        fireEvent.click(loadButton);

        expect(console.log).toHaveBeenCalledWith('Carregando ROM: Sonic the Hedgehog');
    });

    describe('Funcionalidades do Electron', () => {
        beforeEach(() => {
            // Adiciona a API do Electron ao objeto window
            (window as any).electron = mockElectron;
        });

        it('habilita funcionalidades do Electron quando disponível', async () => {
            mockElectron.openFile.mockResolvedValue('/caminho/para/rom.md');
            render(<RomSelector />);

            // Clica no botão de abrir arquivo
            const openButton = screen.getByText('Abrir');
            fireEvent.click(openButton);

            await waitFor(() => {
                expect(mockElectron.openFile).toHaveBeenCalled();
                expect(console.log).toHaveBeenCalledWith('ROM selecionada: /caminho/para/rom.md');
            });
        });

        it('lida com erro quando o usuário cancela a seleção de arquivo', async () => {
            mockElectron.openFile.mockResolvedValue(null);
            render(<RomSelector />);

            const openButton = screen.getByText('Abrir');
            fireEvent.click(openButton);

            await waitFor(() => {
                expect(mockElectron.openFile).toHaveBeenCalled();
                expect(console.log).toHaveBeenCalledWith('Nenhuma ROM selecionada');
            });
        });

        it('lida com erros na API do Electron', async () => {
            mockElectron.openFile.mockRejectedValue(new Error('Erro de teste'));
            render(<RomSelector />);

            const openButton = screen.getByText('Abrir');
            fireEvent.click(openButton);

            await waitFor(() => {
                expect(console.error).toHaveBeenCalledWith('Erro ao abrir seletor de arquivo:', expect.any(Error));
            });
        });
    });

    it('mostra alerta quando tenta abrir arquivo sem Electron', async () => {
        const mockAlert = jest.spyOn(window, 'alert').mockImplementation(() => { });
        render(<RomSelector />);

        const openButton = screen.getByText('Abrir');
        fireEvent.click(openButton);

        await waitFor(() => {
            expect(mockAlert).toHaveBeenCalledWith('Funcionalidade disponível apenas no app desktop');
        });

        mockAlert.mockRestore();
    });
});
