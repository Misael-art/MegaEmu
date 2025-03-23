import React, { useCallback } from 'react';
import { Box } from '@mui/material';
import GameDisplay from '../../components/emulator/GameDisplay';
import ControlPanel from '../../components/emulator/ControlPanel';
import { useAppDispatch } from '../../state/store';
import { EmulatorCommand } from '../../types/emulator.types';

const EmulatorPage: React.FC = () => {
    const dispatch = useAppDispatch();

    // Manipular eventos de teclado
    const handleKeyDown = useCallback((event: KeyboardEvent) => {
        // Converter teclas pressionadas em comandos para o emulador
        const command: EmulatorCommand = {
            type: 'INPUT_KEYDOWN',
            payload: {
                key: event.code,
                timestamp: Date.now(),
            },
        };

        // TODO: Enviar para o emulador via WebSocket
        console.log('Key down:', event.code);
    }, []);

    const handleKeyUp = useCallback((event: KeyboardEvent) => {
        // Converter teclas liberadas em comandos para o emulador
        const command: EmulatorCommand = {
            type: 'INPUT_KEYUP',
            payload: {
                key: event.code,
                timestamp: Date.now(),
            },
        };

        // TODO: Enviar para o emulador via WebSocket
        console.log('Key up:', event.code);
    }, []);

    return (
        <Box
            sx={{
                display: 'flex',
                flexDirection: 'column',
                height: '100%',
                width: '100%',
            }}
        >
            {/* Área principal do emulador */}
            <Box
                sx={{
                    flexGrow: 1,
                    overflow: 'hidden',
                    position: 'relative',
                    backgroundColor: 'black',
                }}
            >
                <GameDisplay onKeyDown={handleKeyDown} onKeyUp={handleKeyUp} />
            </Box>

            {/* Painel de controle */}
            <ControlPanel />
        </Box>
    );
};

export default EmulatorPage;
