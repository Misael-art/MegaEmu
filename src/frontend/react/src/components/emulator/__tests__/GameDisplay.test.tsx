import React from 'react';
import { render, screen, fireEvent } from '@testing-library/react';
import GameDisplay from '../GameDisplay';

describe('GameDisplay', () => {
    const mockOnKeyDown = jest.fn();
    const mockOnKeyUp = jest.fn();

    beforeEach(() => {
        jest.clearAllMocks();
    });

    it('renderiza o canvas corretamente', () => {
        render(<GameDisplay />);
        const canvas = screen.getByTestId('game-canvas');
        expect(canvas).toBeInTheDocument();
        expect(canvas).toHaveAttribute('width', '640');
        expect(canvas).toHaveAttribute('height', '480');
    });

    it('chama onKeyDown quando uma tecla é pressionada', () => {
        render(<GameDisplay onKeyDown={mockOnKeyDown} />);

        // Simula pressionar uma tecla
        fireEvent.keyDown(window, { key: 'ArrowUp', code: 'ArrowUp' });

        expect(mockOnKeyDown).toHaveBeenCalledTimes(1);
        expect(mockOnKeyDown).toHaveBeenCalledWith(expect.any(KeyboardEvent));
    });

    it('chama onKeyUp quando uma tecla é solta', () => {
        render(<GameDisplay onKeyUp={mockOnKeyUp} />);

        // Simula soltar uma tecla
        fireEvent.keyUp(window, { key: 'ArrowUp', code: 'ArrowUp' });

        expect(mockOnKeyUp).toHaveBeenCalledTimes(1);
        expect(mockOnKeyUp).toHaveBeenCalledWith(expect.any(KeyboardEvent));
    });

    it('remove event listeners ao desmontar', () => {
        const { unmount } = render(
            <GameDisplay onKeyDown={mockOnKeyDown} onKeyUp={mockOnKeyUp} />
        );

        // Simula eventos antes de desmontar
        fireEvent.keyDown(window, { key: 'ArrowUp', code: 'ArrowUp' });
        fireEvent.keyUp(window, { key: 'ArrowUp', code: 'ArrowUp' });

        expect(mockOnKeyDown).toHaveBeenCalledTimes(1);
        expect(mockOnKeyUp).toHaveBeenCalledTimes(1);

        // Desmonta o componente
        unmount();

        // Simula eventos após desmontar
        fireEvent.keyDown(window, { key: 'ArrowUp', code: 'ArrowUp' });
        fireEvent.keyUp(window, { key: 'ArrowUp', code: 'ArrowUp' });

        // Verifica se não houve novas chamadas
        expect(mockOnKeyDown).toHaveBeenCalledTimes(1);
        expect(mockOnKeyUp).toHaveBeenCalledTimes(1);
    });

    it('desenha o conteúdo inicial no canvas', () => {
        // Mock do contexto do canvas
        const mockContext = {
            clearRect: jest.fn(),
            fillRect: jest.fn(),
            fillText: jest.fn(),
            strokeRect: jest.fn()
        };

        // Mock do getContext
        HTMLCanvasElement.prototype.getContext = jest.fn(() => mockContext);

        render(<GameDisplay />);

        expect(mockContext.clearRect).toHaveBeenCalled();
        expect(mockContext.fillRect).toHaveBeenCalled();
        expect(mockContext.fillText).toHaveBeenCalledWith('Emulador Mega Emu', expect.any(Number), expect.any(Number));
        expect(mockContext.fillText).toHaveBeenCalledWith('Carregue uma ROM para iniciar', expect.any(Number), expect.any(Number));
        expect(mockContext.strokeRect).toHaveBeenCalled();
    });

    it('redesenha o conteúdo quando a janela é redimensionada', () => {
        // Mock do contexto do canvas
        const mockContext = {
            clearRect: jest.fn(),
            fillRect: jest.fn(),
            fillText: jest.fn(),
            strokeRect: jest.fn()
        };

        // Mock do getContext
        HTMLCanvasElement.prototype.getContext = jest.fn(() => mockContext);

        render(<GameDisplay />);

        // Limpa as chamadas anteriores
        mockContext.clearRect.mockClear();
        mockContext.fillRect.mockClear();
        mockContext.fillText.mockClear();
        mockContext.strokeRect.mockClear();

        // Simula o redimensionamento da janela
        fireEvent(window, new Event('resize'));

        expect(mockContext.clearRect).toHaveBeenCalled();
        expect(mockContext.fillRect).toHaveBeenCalled();
        expect(mockContext.fillText).toHaveBeenCalledWith('Emulador Mega Emu', expect.any(Number), expect.any(Number));
        expect(mockContext.fillText).toHaveBeenCalledWith('Carregue uma ROM para iniciar', expect.any(Number), expect.any(Number));
        expect(mockContext.strokeRect).toHaveBeenCalled();
    });
});
