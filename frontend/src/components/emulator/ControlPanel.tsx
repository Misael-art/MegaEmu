import React from 'react';
import {
    Button,
    Typography,
    Grid,
    Paper,
} from '@mui/material';
import PlayArrowIcon from '@mui/icons-material/PlayArrow';
import PauseIcon from '@mui/icons-material/Pause';
import StopIcon from '@mui/icons-material/Stop';
import SaveIcon from '@mui/icons-material/Save';
import SettingsIcon from '@mui/icons-material/Settings';
import RestoreIcon from '@mui/icons-material/Restore';
import './ControlPanel.css';

import { useAppSelector, useAppDispatch } from '../../state/store';
import { useEmulatorState } from '../../hooks/useEmulatorState';
import { openModal } from '../../state/slices/uiSlice';
// eslint-disable-next-line @typescript-eslint/no-unused-vars
import { setAudioEnabled, setVolume } from '../../state/slices/emulatorSlice';

const ControlPanel: React.FC = () => {
    const dispatch = useAppDispatch();
    const { state: emulatorState } = useAppSelector((state) => state.emulator);
    const { connected } = useAppSelector((state) => state.emulator);

    const {
        startEmulation,
        pauseEmulation,
        resumeEmulation,
        stopEmulation,
        resetEmulation,
        // eslint-disable-next-line @typescript-eslint/no-unused-vars
        updateConfig,
    } = useEmulatorState({ autoConnect: false });

    const handlePlayPause = () => {
        if (emulatorState.isRunning) {
            if (emulatorState.isPaused) {
                resumeEmulation();
            } else {
                pauseEmulation();
            }
        } else {
            startEmulation();
        }
    };

    const handleStop = () => {
        stopEmulation();
    };

    const handleReset = () => {
        resetEmulation();
    };

    const handleSaveState = () => {
        dispatch(openModal({ type: 'saveState' }));
    };

    const handleOpenSettings = () => {
        dispatch(openModal({ type: 'emulatorSettings' }));
    };

    return (
        <Paper className="control-panel">
            <Typography variant="h6" className="panel-title">
                Controles
            </Typography>

            <Grid container spacing={1} className="button-container">
                <Grid item xs={4}>
                    <Button
                        variant="contained"
                        color="primary"
                        startIcon={<PlayArrowIcon />}
                        fullWidth
                        size="small"
                        onClick={handlePlayPause}
                        disabled={!connected || !emulatorState.loadedRom}
                    >
                        {emulatorState.isRunning && !emulatorState.isPaused ? 'Pausar' : 'Iniciar'}
                    </Button>
                </Grid>
                <Grid item xs={4}>
                    <Button
                        variant="contained"
                        color="secondary"
                        startIcon={<PauseIcon />}
                        fullWidth
                        size="small"
                        onClick={handleStop}
                        disabled={!connected || !emulatorState.isRunning}
                    >
                        Parar
                    </Button>
                </Grid>
                <Grid item xs={4}>
                    <Button
                        variant="contained"
                        color="error"
                        startIcon={<StopIcon />}
                        fullWidth
                        size="small"
                        onClick={handleReset}
                        disabled={!connected || !emulatorState.loadedRom}
                    >
                        Reiniciar
                    </Button>
                </Grid>

                <Grid item xs={4}>
                    <Button
                        variant="outlined"
                        startIcon={<SaveIcon />}
                        fullWidth
                        size="small"
                        onClick={handleSaveState}
                        disabled={!connected || !emulatorState.isRunning}
                    >
                        Salvar Estado
                    </Button>
                </Grid>
                <Grid item xs={4}>
                    <Button
                        variant="outlined"
                        startIcon={<RestoreIcon />}
                        fullWidth
                        size="small"
                        disabled={!connected || emulatorState.saveStates.length === 0}
                    >
                        Carregar Estado Anterior
                    </Button>
                </Grid>
                <Grid item xs={4}>
                    <Button
                        variant="outlined"
                        startIcon={<SettingsIcon />}
                        fullWidth
                        size="small"
                        onClick={handleOpenSettings}
                    >
                        Configurações
                    </Button>
                </Grid>
            </Grid>

            <div className="status-bar">
                <Typography variant="body2" className="status-text">
                    {emulatorState.loadedRom
                        ? `ROM: ${emulatorState.loadedRom}`
                        : 'Nenhuma ROM carregada'}
                </Typography>
            </div>
        </Paper>
    );
};

export default ControlPanel;
