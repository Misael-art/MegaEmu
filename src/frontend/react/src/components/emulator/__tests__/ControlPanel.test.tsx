import React from 'react';
import { render, screen, fireEvent } from '@testing-library/react';
import { Provider } from 'react-redux';
import configureStore from 'redux-mock-store';
import ControlPanel from '../ControlPanel';
import { openModal } from '../../../state/slices/uiSlice';

// Mock dos hooks personalizados
jest.mock('../../../hooks/useEmulatorState', () => ({
    __esModule: true,
    default: () => ({
        isRunning: true,
        isPaused: false,
        loadedRom: 'test.rom',
        play: jest.fn(),
        pause: jest.fn(),
        reset: jest.fn(),
        saveState: jest.fn(),
        loadState: jest.fn()
    })
}));

const mockStore = configureStore([]);

describe('ControlPanel', () => {
    let store;

    beforeEach(() => {
        store = mockStore({
            emulator: {
                state: {
                    isRunning: true,
                    isPaused: false,
                    loadedRom: 'test.rom',
                    saveStates: []
                },
                connected: true
            }
        });
        store.dispatch = jest.fn();
    });

    const renderComponent = () => {
        return render(
            <Provider store={store}>
                <ControlPanel />
            </Provider>
        );
    };

    it('renderiza todos os botões corretamente', () => {
        renderComponent();

        expect(screen.getByText('Iniciar')).toBeInTheDocument();
        expect(screen.getByText('Pausar')).toBeInTheDocument();
        expect(screen.getByText('Resetar')).toBeInTheDocument();
        expect(screen.getByText('Carregar Estado')).toBeInTheDocument();
        expect(screen.getByText('Salvar Estado')).toBeInTheDocument();
        expect(screen.getByText('Configurações')).toBeInTheDocument();
    });

    it('desabilita botões apropriados quando não há ROM carregada', () => {
        renderComponent();

        expect(screen.getByText('Iniciar')).toBeDisabled();
        expect(screen.getByText('Pausar')).toBeDisabled();
        expect(screen.getByText('Resetar')).toBeDisabled();
        expect(screen.getByText('Carregar Estado')).toBeDisabled();
        expect(screen.getByText('Salvar Estado')).toBeDisabled();
        expect(screen.getByText('Configurações')).not.toBeDisabled();
    });

    it('habilita botões apropriados quando há ROM carregada', () => {
        store = mockStore({
            emulator: {
                state: {
                    isRunning: false,
                    isPaused: false,
                    loadedRom: 'test.rom',
                    saveStates: []
                },
                connected: true
            }
        });

        renderComponent();

        expect(screen.getByText('Iniciar')).not.toBeDisabled();
        expect(screen.getByText('Resetar')).not.toBeDisabled();
    });

    it('mostra "Pausar" quando o emulador está rodando', () => {
        store = mockStore({
            emulator: {
                state: {
                    isRunning: true,
                    isPaused: false,
                    loadedRom: 'test.rom',
                    saveStates: []
                },
                connected: true
            }
        });

        renderComponent();

        expect(screen.getByText('Pausar')).toBeInTheDocument();
    });

    it('mostra "Iniciar" quando o emulador está pausado', () => {
        store = mockStore({
            emulator: {
                state: {
                    isRunning: true,
                    isPaused: true,
                    loadedRom: 'test.rom',
                    saveStates: []
                },
                connected: true
            }
        });

        renderComponent();

        expect(screen.getByText('Iniciar')).toBeInTheDocument();
    });

    it('abre modal de configurações ao clicar no botão', () => {
        renderComponent();

        fireEvent.click(screen.getByText('Configurações'));

        expect(store.dispatch).toHaveBeenCalledWith(
            openModal({ type: 'emulatorSettings' })
        );
    });

    it('abre modal de save state ao clicar no botão', () => {
        render(
            <Provider store={store}>
                <ControlPanel />
            </Provider>
        );

        fireEvent.click(screen.getByText('Salvar Estado'));

        expect(store.dispatch).toHaveBeenCalledWith(
            openModal({ type: 'saveState' })
        );
    });

    it('mostra o nome da ROM carregada', () => {
        store = mockStore({
            emulator: {
                state: {
                    isRunning: false,
                    isPaused: false,
                    loadedRom: 'test.rom',
                    saveStates: []
                },
                connected: true
            }
        });

        renderComponent();

        expect(screen.getByText('ROM: test.rom')).toBeInTheDocument();
    });

    it('mostra mensagem quando não há ROM carregada', () => {
        renderComponent();

        expect(screen.getByText('Nenhuma ROM carregada')).toBeInTheDocument();
    });

    it('desabilita todos os botões quando não está conectado', () => {
        store = mockStore({
            emulator: {
                state: {
                    isRunning: false,
                    isPaused: false,
                    loadedRom: 'test.rom',
                    saveStates: []
                },
                connected: false
            }
        });

        renderComponent();

        const buttons = screen.getAllByRole('button');
        buttons.forEach(button => {
            if (button.textContent !== 'Configurações') {
                expect(button).toBeDisabled();
            }
        });
    });
});
