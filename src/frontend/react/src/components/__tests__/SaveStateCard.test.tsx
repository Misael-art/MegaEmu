import React from 'react';
import { render, screen, fireEvent } from '@testing-library/react';
import SaveStateCard from '../SaveStateCard';
import { SaveState, ConsoleType } from '../../types/emulator.types';

describe('SaveStateCard', () => {
    const mockSaveState: SaveState = {
        id: '1',
        name: 'Test Save',
        timestamp: Date.now(),
        data: new Uint8Array(),
        metadata: {
            realName: 'Test Game',
            playTime: 3600, // 1 hora
            level: 1,
            screenInfo: 'Test Screen Info'
        }
    };

    const mockProps = {
        saveState: mockSaveState,
        consoleType: 'megadrive' as ConsoleType,
        onLoad: jest.fn(),
        onDelete: jest.fn(),
        showDetails: true
    };

    beforeEach(() => {
        jest.clearAllMocks();
    });

    it('renderiza corretamente com todos os detalhes', () => {
        render(<SaveStateCard {...mockProps} />);

        // Verifica elementos básicos
        expect(screen.getByText('Test Save')).toBeInTheDocument();
        expect(screen.getByText('Test Game')).toBeInTheDocument();
        expect(screen.getByText(/Tempo: 1h 0m/)).toBeInTheDocument();
        expect(screen.getByText('Fase 1')).toBeInTheDocument();
        expect(screen.getByText('Test Screen Info')).toBeInTheDocument();
    });

    it('chama onLoad quando o botão de carregar é clicado', () => {
        render(<SaveStateCard {...mockProps} />);

        const loadButton = screen.getByText('Carregar');
        fireEvent.click(loadButton);

        expect(mockProps.onLoad).toHaveBeenCalledTimes(1);
    });

    it('chama onDelete quando o botão de excluir é clicado', () => {
        render(<SaveStateCard {...mockProps} />);

        const deleteButton = screen.getByRole('button', { name: /excluir save state/i });
        fireEvent.click(deleteButton);

        expect(mockProps.onDelete).toHaveBeenCalledTimes(1);
    });

    it('não mostra detalhes quando showDetails é false', () => {
        render(<SaveStateCard {...mockProps} showDetails={false} />);

        expect(screen.queryByText('Test Game')).not.toBeInTheDocument();
        expect(screen.queryByText(/Tempo:/)).not.toBeInTheDocument();
        expect(screen.queryByText('Fase 1')).not.toBeInTheDocument();
    });

    it('formata o tempo corretamente para diferentes durações', () => {
        const saveStateMinutes = {
            ...mockSaveState,
            metadata: { ...mockSaveState.metadata, playTime: 300 } // 5 minutos
        };

        const { rerender } = render(<SaveStateCard {...mockProps} saveState={saveStateMinutes} />);
        expect(screen.getByText(/Tempo: 5m/)).toBeInTheDocument();

        const saveStateHours = {
            ...mockSaveState,
            metadata: { ...mockSaveState.metadata, playTime: 7200 } // 2 horas
        };

        rerender(<SaveStateCard {...mockProps} saveState={saveStateHours} />);
        expect(screen.getByText(/Tempo: 2h 0m/)).toBeInTheDocument();
    });

    it('formata a data corretamente', () => {
        const now = Date.now();
        const oneDayAgo = now - (25 * 60 * 60 * 1000); // 25 horas atrás
        const saveStateOld = {
            ...mockSaveState,
            timestamp: oneDayAgo
        };

        render(<SaveStateCard {...mockProps} saveState={saveStateOld} />);
        expect(screen.getByText(/1 dia atrás/)).toBeInTheDocument();
    });
});
