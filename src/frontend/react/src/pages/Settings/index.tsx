import React, { useState, useEffect } from 'react';
import {
    Box,
    Typography,
    Tabs,
    Tab,
    Paper,
    FormControl,
    FormControlLabel,
    Select,
    MenuItem,
    Switch,
    Slider,
    TextField,
    Button,
    Divider,
    Grid,
    InputLabel,
    Alert,
    Snackbar,
    Card,
    CardContent,
    IconButton,
} from '@mui/material';
import SaveIcon from '@mui/icons-material/Save';
import RefreshIcon from '@mui/icons-material/Refresh';
import RestoreIcon from '@mui/icons-material/Restore';
import InfoOutlinedIcon from '@mui/icons-material/InfoOutlined';

import { useAppDispatch, useAppSelector } from '../../state/store';
import { useEmulatorState } from '../../hooks/useEmulatorState';
import { setTheme } from '../../state/slices/uiSlice';
import { setFrameSkip, setVolume, setAudioEnabled, setRewindEnabled, updateEmulatorState } from '../../state/slices/emulatorSlice';

interface TabPanelProps {
    children?: React.ReactNode;
    index: number;
    value: number;
}

function TabPanel({ children, value, index, ...other }: TabPanelProps) {
    return (
        <div
            role="tabpanel"
            hidden={value !== index}
            id={`settings-tabpanel-${index}`}
            aria-labelledby={`settings-tab-${index}`}
            {...other}
        >
            {value === index && (
                <Box sx={{ p: 3 }}>
                    {children}
                </Box>
            )}
        </div>
    );
}

function a11yProps(index: number) {
    return {
        id: `settings-tab-${index}`,
        'aria-controls': `settings-tabpanel-${index}`,
    };
}

const SettingsPage: React.FC = () => {
    const dispatch = useAppDispatch();
    const emulatorState = useAppSelector(state => state.emulator.state);
    const uiState = useAppSelector(state => state.ui);
    const { updateConfig } = useEmulatorState({ autoConnect: false });

    const [tabValue, setTabValue] = useState(0);
    const [showSaveMessage, setShowSaveMessage] = useState(false);
    const [saveMessage, setSaveMessage] = useState({ type: 'success', text: 'Configurações salvas com sucesso!' });

    // Formulários de configuração
    const [videoSettings, setVideoSettings] = useState({
        frameSkip: emulatorState.frameSkip,
        scalingMode: 'pixelPerfect', // nearest, bilinear, pixelPerfect
        aspectRatio: 'original', // original, 16_9, 4_3, stretched
        filterMode: 'none', // none, scanlines, crt, lcd
        vsync: true,
        fullscreenStartup: false,
    });

    const [audioSettings, setAudioSettings] = useState({
        enabled: emulatorState.audioEnabled,
        volume: emulatorState.volume,
        sampleRate: 44100, // 22050, 44100, 48000
        bufferSize: 1024, // 512, 1024, 2048, 4096
        synchronization: 'dynamic', // strict, dynamic, off
    });

    const [performanceSettings, setPerformanceSettings] = useState({
        threadCount: 2, // 1, 2, 4, 8, auto
        priorityMode: 'balanced', // accuracy, performance, balanced
        rewindEnabled: emulatorState.rewindEnabled,
        rewindBufferSize: emulatorState.rewindBufferSize,
    });

    const [interfaceSettings, setInterfaceSettings] = useState({
        theme: uiState.theme,
        language: 'pt-BR', // pt-BR, en-US, es-ES
        showFPS: uiState.showFPS,
        controlsVisible: uiState.controlsVisible,
        showNotifications: true,
        autoSaveStates: true,
    });

    const [inputSettings, setInputSettings] = useState({
        keyboard: {
            playerOne: {
                up: 'ArrowUp',
                down: 'ArrowDown',
                left: 'ArrowLeft',
                right: 'ArrowRight',
                a: 'z',
                b: 'x',
                x: 'a',
                y: 's',
                start: 'Enter',
                select: 'Shift',
            }
        },
        gamepad: {
            enabled: true,
            mappingProfile: 'xbox',
        }
    });

    const handleTabChange = (_event: React.SyntheticEvent, newValue: number) => {
        setTabValue(newValue);
    };

    const handleVideoSettingsChange = (key: keyof typeof videoSettings, value: any) => {
        if (key === 'frameSkip' && typeof value === 'number') {
            dispatch(setFrameSkip(value));
        }

        setVideoSettings(prev => ({ ...prev, [key]: value }));
    };

    const handleAudioSettingsChange = (key: keyof typeof audioSettings, value: any) => {
        if (key === 'volume' && typeof value === 'number') {
            dispatch(setVolume(value));
        } else if (key === 'enabled' && typeof value === 'boolean') {
            dispatch(setAudioEnabled(value));
        }

        setAudioSettings(prev => ({ ...prev, [key]: value }));
    };

    const handlePerformanceSettingsChange = (key: keyof typeof performanceSettings, value: any) => {
        if (key === 'rewindEnabled' && typeof value === 'boolean') {
            dispatch(setRewindEnabled(value));
        } else if (key === 'rewindBufferSize' && typeof value === 'number') {
            dispatch(updateEmulatorState({ rewindBufferSize: value }));
        }

        setPerformanceSettings(prev => ({ ...prev, [key]: value }));
    };

    const handleInterfaceSettingsChange = (key: keyof typeof interfaceSettings, value: any) => {
        if (key === 'theme') {
            dispatch(setTheme(value));
        }

        setInterfaceSettings(prev => ({ ...prev, [key]: value }));
    };

    const handleSaveSettings = () => {
        // Aplicar as configurações ao emulador (quando houver um backend)
        updateConfig({
            frameSkip: videoSettings.frameSkip,
            audioEnabled: audioSettings.enabled,
            volume: audioSettings.volume,
            rewindEnabled: performanceSettings.rewindEnabled,
            rewindBufferSize: performanceSettings.rewindBufferSize,
        });

        // Mostrar mensagem de sucesso
        setSaveMessage({ type: 'success', text: 'Configurações salvas com sucesso!' });
        setShowSaveMessage(true);
    };

    const handleResetSettings = () => {
        // Configurações padrão
        setVideoSettings({
            frameSkip: 0,
            scalingMode: 'pixelPerfect',
            aspectRatio: 'original',
            filterMode: 'none',
            vsync: true,
            fullscreenStartup: false,
        });

        setAudioSettings({
            enabled: true,
            volume: 100,
            sampleRate: 44100,
            bufferSize: 1024,
            synchronization: 'dynamic',
        });

        setPerformanceSettings({
            threadCount: 2,
            priorityMode: 'balanced',
            rewindEnabled: true,
            rewindBufferSize: 60,
        });

        setInterfaceSettings({
            theme: 'system',
            language: 'pt-BR',
            showFPS: true,
            controlsVisible: true,
            showNotifications: true,
            autoSaveStates: true,
        });

        setSaveMessage({ type: 'info', text: 'Configurações restauradas para os valores padrão.' });
        setShowSaveMessage(true);
    };

    return (
        <Box sx={{ p: 3 }}>
            <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
                <Typography variant="h4" component="h1" gutterBottom>
                    Configurações
                </Typography>
                <Box>
                    <Button
                        startIcon={<SaveIcon />}
                        variant="contained"
                        color="primary"
                        onClick={handleSaveSettings}
                        sx={{ mr: 1 }}
                    >
                        Salvar
                    </Button>
                    <Button
                        startIcon={<RestoreIcon />}
                        variant="outlined"
                        onClick={handleResetSettings}
                    >
                        Restaurar Padrões
                    </Button>
                </Box>
            </Box>

            <Paper sx={{ width: '100%' }}>
                <Tabs
                    value={tabValue}
                    onChange={handleTabChange}
                    aria-label="configurações do emulador"
                    variant="scrollable"
                    scrollButtons="auto"
                >
                    <Tab label="Vídeo" {...a11yProps(0)} />
                    <Tab label="Áudio" {...a11yProps(1)} />
                    <Tab label="Desempenho" {...a11yProps(2)} />
                    <Tab label="Interface" {...a11yProps(3)} />
                    <Tab label="Controles" {...a11yProps(4)} />
                    <Tab label="Pastas" {...a11yProps(5)} />
                    <Tab label="Avançado" {...a11yProps(6)} />
                </Tabs>

                {/* Configurações de Vídeo */}
                <TabPanel value={tabValue} index={0}>
                    <Grid container spacing={3}>
                        <Grid item xs={12} md={6}>
                            <Card>
                                <CardContent>
                                    <Typography variant="h6" gutterBottom>Qualidade da Imagem</Typography>

                                    <Box sx={{ mb: 3 }}>
                                        <Typography gutterBottom>
                                            Frame Skip
                                            <IconButton size="small" sx={{ ml: 1 }}>
                                                <InfoOutlinedIcon fontSize="small" />
                                            </IconButton>
                                        </Typography>
                                        <Slider
                                            value={videoSettings.frameSkip}
                                            onChange={(_e, value) => handleVideoSettingsChange('frameSkip', value as number)}
                                            step={1}
                                            marks
                                            min={0}
                                            max={5}
                                            valueLabelDisplay="auto"
                                        />
                                        <Typography variant="caption" color="text.secondary">
                                            0 = Sem pular frames (melhor qualidade). Valores maiores podem melhorar o desempenho.
                                        </Typography>
                                    </Box>

                                    <Box sx={{ mb: 3 }}>
                                        <FormControl fullWidth margin="normal">
                                            <InputLabel id="scaling-mode-label">Modo de Escala</InputLabel>
                                            <Select
                                                labelId="scaling-mode-label"
                                                value={videoSettings.scalingMode}
                                                label="Modo de Escala"
                                                onChange={(e) => handleVideoSettingsChange('scalingMode', e.target.value)}
                                            >
                                                <MenuItem value="pixelPerfect">Pixel Perfect</MenuItem>
                                                <MenuItem value="nearest">Nearest Neighbor</MenuItem>
                                                <MenuItem value="bilinear">Bilinear</MenuItem>
                                            </Select>
                                        </FormControl>
                                    </Box>

                                    <Box sx={{ mb: 3 }}>
                                        <FormControl fullWidth margin="normal">
                                            <InputLabel id="filter-mode-label">Filtro de Exibição</InputLabel>
                                            <Select
                                                labelId="filter-mode-label"
                                                value={videoSettings.filterMode}
                                                label="Filtro de Exibição"
                                                onChange={(e) => handleVideoSettingsChange('filterMode', e.target.value)}
                                            >
                                                <MenuItem value="none">Nenhum</MenuItem>
                                                <MenuItem value="scanlines">Scanlines</MenuItem>
                                                <MenuItem value="crt">CRT (Tubo)</MenuItem>
                                                <MenuItem value="lcd">LCD</MenuItem>
                                            </Select>
                                        </FormControl>
                                    </Box>
                                </CardContent>
                            </Card>
                        </Grid>

                        <Grid item xs={12} md={6}>
                            <Card>
                                <CardContent>
                                    <Typography variant="h6" gutterBottom>Exibição</Typography>

                                    <Box sx={{ mb: 3 }}>
                                        <FormControl fullWidth margin="normal">
                                            <InputLabel id="aspect-ratio-label">Proporção da Tela</InputLabel>
                                            <Select
                                                labelId="aspect-ratio-label"
                                                value={videoSettings.aspectRatio}
                                                label="Proporção da Tela"
                                                onChange={(e) => handleVideoSettingsChange('aspectRatio', e.target.value)}
                                            >
                                                <MenuItem value="original">Original</MenuItem>
                                                <MenuItem value="4_3">4:3</MenuItem>
                                                <MenuItem value="16_9">16:9</MenuItem>
                                                <MenuItem value="stretched">Esticado</MenuItem>
                                            </Select>
                                        </FormControl>
                                    </Box>

                                    <Box sx={{ mt: 3 }}>
                                        <FormControlLabel
                                            control={
                                                <Switch
                                                    checked={videoSettings.vsync}
                                                    onChange={(e) => handleVideoSettingsChange('vsync', e.target.checked)}
                                                />
                                            }
                                            label="V-Sync (Sincronização Vertical)"
                                        />
                                        <Typography variant="caption" display="block" color="text.secondary">
                                            Reduz o tearing da imagem, mas pode causar input lag.
                                        </Typography>
                                    </Box>

                                    <Box sx={{ mt: 2 }}>
                                        <FormControlLabel
                                            control={
                                                <Switch
                                                    checked={videoSettings.fullscreenStartup}
                                                    onChange={(e) => handleVideoSettingsChange('fullscreenStartup', e.target.checked)}
                                                />
                                            }
                                            label="Iniciar em Tela Cheia"
                                        />
                                    </Box>
                                </CardContent>
                            </Card>
                        </Grid>
                    </Grid>
                </TabPanel>

                {/* Configurações de Áudio */}
                <TabPanel value={tabValue} index={1}>
                    <Grid container spacing={3}>
                        <Grid item xs={12} md={6}>
                            <Card>
                                <CardContent>
                                    <Typography variant="h6" gutterBottom>Reprodução de Áudio</Typography>

                                    <Box sx={{ mb: 3 }}>
                                        <FormControlLabel
                                            control={
                                                <Switch
                                                    checked={audioSettings.enabled}
                                                    onChange={(e) => handleAudioSettingsChange('enabled', e.target.checked)}
                                                />
                                            }
                                            label="Ativar Áudio"
                                        />
                                    </Box>

                                    <Box sx={{ mb: 3 }}>
                                        <Typography gutterBottom>
                                            Volume
                                        </Typography>
                                        <Box sx={{ display: 'flex', alignItems: 'center' }}>
                                            <Slider
                                                value={audioSettings.volume}
                                                onChange={(_e, value) => handleAudioSettingsChange('volume', value as number)}
                                                step={1}
                                                min={0}
                                                max={100}
                                                disabled={!audioSettings.enabled}
                                                sx={{ flexGrow: 1, mr: 2 }}
                                            />
                                            <Typography>{audioSettings.volume}%</Typography>
                                        </Box>
                                    </Box>
                                </CardContent>
                            </Card>
                        </Grid>

                        <Grid item xs={12} md={6}>
                            <Card>
                                <CardContent>
                                    <Typography variant="h6" gutterBottom>Configurações Avançadas</Typography>

                                    <Box sx={{ mb: 3 }}>
                                        <FormControl fullWidth margin="normal">
                                            <InputLabel id="sample-rate-label">Taxa de Amostragem</InputLabel>
                                            <Select
                                                labelId="sample-rate-label"
                                                value={audioSettings.sampleRate}
                                                label="Taxa de Amostragem"
                                                onChange={(e) => handleAudioSettingsChange('sampleRate', Number(e.target.value))}
                                                disabled={!audioSettings.enabled}
                                            >
                                                <MenuItem value={22050}>22.05 kHz</MenuItem>
                                                <MenuItem value={44100}>44.1 kHz</MenuItem>
                                                <MenuItem value={48000}>48 kHz</MenuItem>
                                            </Select>
                                        </FormControl>
                                    </Box>

                                    <Box sx={{ mb: 3 }}>
                                        <FormControl fullWidth margin="normal">
                                            <InputLabel id="buffer-size-label">Tamanho do Buffer</InputLabel>
                                            <Select
                                                labelId="buffer-size-label"
                                                value={audioSettings.bufferSize}
                                                label="Tamanho do Buffer"
                                                onChange={(e) => handleAudioSettingsChange('bufferSize', Number(e.target.value))}
                                                disabled={!audioSettings.enabled}
                                            >
                                                <MenuItem value={512}>512 (Baixa latência)</MenuItem>
                                                <MenuItem value={1024}>1024 (Recomendado)</MenuItem>
                                                <MenuItem value={2048}>2048 (Maior estabilidade)</MenuItem>
                                                <MenuItem value={4096}>4096 (Máxima estabilidade)</MenuItem>
                                            </Select>
                                        </FormControl>
                                        <Typography variant="caption" color="text.secondary">
                                            Valores menores reduzem o delay de áudio, mas podem causar falhas na reprodução.
                                        </Typography>
                                    </Box>

                                    <Box sx={{ mb: 3 }}>
                                        <FormControl fullWidth margin="normal">
                                            <InputLabel id="sync-mode-label">Sincronização</InputLabel>
                                            <Select
                                                labelId="sync-mode-label"
                                                value={audioSettings.synchronization}
                                                label="Sincronização"
                                                onChange={(e) => handleAudioSettingsChange('synchronization', e.target.value)}
                                                disabled={!audioSettings.enabled}
                                            >
                                                <MenuItem value="strict">Rígida (Mais precisa)</MenuItem>
                                                <MenuItem value="dynamic">Dinâmica (Recomendado)</MenuItem>
                                                <MenuItem value="off">Desativada (Melhor desempenho)</MenuItem>
                                            </Select>
                                        </FormControl>
                                    </Box>
                                </CardContent>
                            </Card>
                        </Grid>
                    </Grid>
                </TabPanel>

                {/* Configurações de Desempenho */}
                <TabPanel value={tabValue} index={2}>
                    <Grid container spacing={3}>
                        <Grid item xs={12} md={6}>
                            <Card>
                                <CardContent>
                                    <Typography variant="h6" gutterBottom>Processamento</Typography>

                                    <Box sx={{ mb: 3 }}>
                                        <FormControl fullWidth margin="normal">
                                            <InputLabel id="thread-count-label">Número de Threads</InputLabel>
                                            <Select
                                                labelId="thread-count-label"
                                                value={performanceSettings.threadCount}
                                                label="Número de Threads"
                                                onChange={(e) => handlePerformanceSettingsChange('threadCount', Number(e.target.value))}
                                            >
                                                <MenuItem value={1}>1 (Compatibilidade)</MenuItem>
                                                <MenuItem value={2}>2 (Recomendado)</MenuItem>
                                                <MenuItem value={4}>4 (Multi-core)</MenuItem>
                                                <MenuItem value={0}>Auto (Baseado no CPU)</MenuItem>
                                            </Select>
                                        </FormControl>
                                    </Box>

                                    <Box sx={{ mb: 3 }}>
                                        <FormControl fullWidth margin="normal">
                                            <InputLabel id="priority-mode-label">Modo de Prioridade</InputLabel>
                                            <Select
                                                labelId="priority-mode-label"
                                                value={performanceSettings.priorityMode}
                                                label="Modo de Prioridade"
                                                onChange={(e) => handlePerformanceSettingsChange('priorityMode', e.target.value)}
                                            >
                                                <MenuItem value="accuracy">Precisão (Mais lento)</MenuItem>
                                                <MenuItem value="balanced">Balanceado</MenuItem>
                                                <MenuItem value="performance">Desempenho (Mais rápido)</MenuItem>
                                            </Select>
                                        </FormControl>
                                    </Box>
                                </CardContent>
                            </Card>
                        </Grid>

                        <Grid item xs={12} md={6}>
                            <Card>
                                <CardContent>
                                    <Typography variant="h6" gutterBottom>Funcionalidades</Typography>

                                    <Box sx={{ mb: 3 }}>
                                        <FormControlLabel
                                            control={
                                                <Switch
                                                    checked={performanceSettings.rewindEnabled}
                                                    onChange={(e) => handlePerformanceSettingsChange('rewindEnabled', e.target.checked)}
                                                />
                                            }
                                            label="Ativar Rewind (Voltar no tempo)"
                                        />
                                        <Typography variant="caption" display="block" color="text.secondary">
                                            Permite voltar o jogo no tempo. Consome mais memória e processamento.
                                        </Typography>
                                    </Box>

                                    <Box sx={{ mb: 3 }}>
                                        <Typography gutterBottom>
                                            Tamanho do Buffer de Rewind (segundos)
                                        </Typography>
                                        <Slider
                                            value={performanceSettings.rewindBufferSize}
                                            onChange={(_e, value) => handlePerformanceSettingsChange('rewindBufferSize', value as number)}
                                            step={10}
                                            marks
                                            min={10}
                                            max={120}
                                            valueLabelDisplay="auto"
                                            disabled={!performanceSettings.rewindEnabled}
                                        />
                                        <Typography variant="caption" color="text.secondary">
                                            Quanto maior o buffer, mais tempo você pode voltar, mas maior o consumo de memória.
                                        </Typography>
                                    </Box>
                                </CardContent>
                            </Card>
                        </Grid>
                    </Grid>
                </TabPanel>

                {/* Configurações de Interface */}
                <TabPanel value={tabValue} index={3}>
                    <Grid container spacing={3}>
                        <Grid item xs={12} md={6}>
                            <Card>
                                <CardContent>
                                    <Typography variant="h6" gutterBottom>Aparência</Typography>

                                    <Box sx={{ mb: 3 }}>
                                        <FormControl fullWidth margin="normal">
                                            <InputLabel id="theme-label">Tema</InputLabel>
                                            <Select
                                                labelId="theme-label"
                                                value={interfaceSettings.theme}
                                                label="Tema"
                                                onChange={(e) => handleInterfaceSettingsChange('theme', e.target.value)}
                                            >
                                                <MenuItem value="light">Claro</MenuItem>
                                                <MenuItem value="dark">Escuro</MenuItem>
                                                <MenuItem value="system">Sistema (Automático)</MenuItem>
                                            </Select>
                                        </FormControl>
                                    </Box>

                                    <Box sx={{ mb: 3 }}>
                                        <FormControl fullWidth margin="normal">
                                            <InputLabel id="language-label">Idioma</InputLabel>
                                            <Select
                                                labelId="language-label"
                                                value={interfaceSettings.language}
                                                label="Idioma"
                                                onChange={(e) => handleInterfaceSettingsChange('language', e.target.value)}
                                            >
                                                <MenuItem value="pt-BR">Português (Brasil)</MenuItem>
                                                <MenuItem value="en-US">English (US)</MenuItem>
                                                <MenuItem value="es-ES">Español</MenuItem>
                                            </Select>
                                        </FormControl>
                                    </Box>
                                </CardContent>
                            </Card>
                        </Grid>

                        <Grid item xs={12} md={6}>
                            <Card>
                                <CardContent>
                                    <Typography variant="h6" gutterBottom>Comportamento</Typography>

                                    <Box sx={{ mb: 2 }}>
                                        <FormControlLabel
                                            control={
                                                <Switch
                                                    checked={interfaceSettings.showFPS}
                                                    onChange={(e) => handleInterfaceSettingsChange('showFPS', e.target.checked)}
                                                />
                                            }
                                            label="Mostrar FPS"
                                        />
                                    </Box>

                                    <Box sx={{ mb: 2 }}>
                                        <FormControlLabel
                                            control={
                                                <Switch
                                                    checked={interfaceSettings.controlsVisible}
                                                    onChange={(e) => handleInterfaceSettingsChange('controlsVisible', e.target.checked)}
                                                />
                                            }
                                            label="Mostrar Controles"
                                        />
                                    </Box>

                                    <Box sx={{ mb: 2 }}>
                                        <FormControlLabel
                                            control={
                                                <Switch
                                                    checked={interfaceSettings.showNotifications}
                                                    onChange={(e) => handleInterfaceSettingsChange('showNotifications', e.target.checked)}
                                                />
                                            }
                                            label="Mostrar Notificações"
                                        />
                                    </Box>

                                    <Box sx={{ mb: 2 }}>
                                        <FormControlLabel
                                            control={
                                                <Switch
                                                    checked={interfaceSettings.autoSaveStates}
                                                    onChange={(e) => handleInterfaceSettingsChange('autoSaveStates', e.target.checked)}
                                                />
                                            }
                                            label="Auto-Salvar Estados"
                                        />
                                        <Typography variant="caption" display="block" color="text.secondary">
                                            Salva automaticamente o estado do jogo em intervalos regulares.
                                        </Typography>
                                    </Box>
                                </CardContent>
                            </Card>
                        </Grid>
                    </Grid>
                </TabPanel>

                {/* Controles (Placeholder) */}
                <TabPanel value={tabValue} index={4}>
                    <Alert severity="info" sx={{ mb: 3 }}>
                        A configuração de controles completa estará disponível na próxima atualização.
                    </Alert>

                    <Card>
                        <CardContent>
                            <Typography variant="h6" gutterBottom>Teclado - Jogador 1</Typography>
                            <Grid container spacing={2}>
                                <Grid item xs={6} md={3}>
                                    <TextField
                                        fullWidth
                                        label="Cima"
                                        value={inputSettings.keyboard.playerOne.up}
                                        InputProps={{ readOnly: true }}
                                        size="small"
                                        margin="normal"
                                    />
                                </Grid>
                                <Grid item xs={6} md={3}>
                                    <TextField
                                        fullWidth
                                        label="Baixo"
                                        value={inputSettings.keyboard.playerOne.down}
                                        InputProps={{ readOnly: true }}
                                        size="small"
                                        margin="normal"
                                    />
                                </Grid>
                                <Grid item xs={6} md={3}>
                                    <TextField
                                        fullWidth
                                        label="Esquerda"
                                        value={inputSettings.keyboard.playerOne.left}
                                        InputProps={{ readOnly: true }}
                                        size="small"
                                        margin="normal"
                                    />
                                </Grid>
                                <Grid item xs={6} md={3}>
                                    <TextField
                                        fullWidth
                                        label="Direita"
                                        value={inputSettings.keyboard.playerOne.right}
                                        InputProps={{ readOnly: true }}
                                        size="small"
                                        margin="normal"
                                    />
                                </Grid>
                                <Grid item xs={6} md={3}>
                                    <TextField
                                        fullWidth
                                        label="Botão A"
                                        value={inputSettings.keyboard.playerOne.a}
                                        InputProps={{ readOnly: true }}
                                        size="small"
                                        margin="normal"
                                    />
                                </Grid>
                                <Grid item xs={6} md={3}>
                                    <TextField
                                        fullWidth
                                        label="Botão B"
                                        value={inputSettings.keyboard.playerOne.b}
                                        InputProps={{ readOnly: true }}
                                        size="small"
                                        margin="normal"
                                    />
                                </Grid>
                                <Grid item xs={6} md={3}>
                                    <TextField
                                        fullWidth
                                        label="Start"
                                        value={inputSettings.keyboard.playerOne.start}
                                        InputProps={{ readOnly: true }}
                                        size="small"
                                        margin="normal"
                                    />
                                </Grid>
                                <Grid item xs={6} md={3}>
                                    <TextField
                                        fullWidth
                                        label="Select"
                                        value={inputSettings.keyboard.playerOne.select}
                                        InputProps={{ readOnly: true }}
                                        size="small"
                                        margin="normal"
                                    />
                                </Grid>
                            </Grid>
                        </CardContent>
                    </Card>
                </TabPanel>

                {/* Pastas (Placeholder) */}
                <TabPanel value={tabValue} index={5}>
                    <Alert severity="info" sx={{ mb: 3 }}>
                        A configuração de pastas completa estará disponível na próxima atualização.
                    </Alert>

                    <Card>
                        <CardContent>
                            <Typography variant="h6" gutterBottom>Diretórios do Sistema</Typography>

                            <Grid container spacing={2}>
                                <Grid item xs={12}>
                                    <TextField
                                        fullWidth
                                        label="Pasta de ROMs"
                                        value="D:\Steamapps\DevProjetos\PC Engines Projects\Mega_Emu\roms"
                                        InputProps={{
                                            endAdornment: (
                                                <Button variant="outlined" size="small">
                                                    Alterar
                                                </Button>
                                            ),
                                        }}
                                        margin="normal"
                                    />
                                </Grid>
                                <Grid item xs={12}>
                                    <TextField
                                        fullWidth
                                        label="Pasta de Save States"
                                        value="D:\Steamapps\DevProjetos\PC Engines Projects\Mega_Emu\saves"
                                        InputProps={{
                                            endAdornment: (
                                                <Button variant="outlined" size="small">
                                                    Alterar
                                                </Button>
                                            ),
                                        }}
                                        margin="normal"
                                    />
                                </Grid>
                                <Grid item xs={12}>
                                    <TextField
                                        fullWidth
                                        label="Pasta de Screenshots"
                                        value="D:\Steamapps\DevProjetos\PC Engines Projects\Mega_Emu\screenshots"
                                        InputProps={{
                                            endAdornment: (
                                                <Button variant="outlined" size="small">
                                                    Alterar
                                                </Button>
                                            ),
                                        }}
                                        margin="normal"
                                    />
                                </Grid>
                                <Grid item xs={12}>
                                    <TextField
                                        fullWidth
                                        label="Pasta de Gravações"
                                        value="D:\Steamapps\DevProjetos\PC Engines Projects\Mega_Emu\recordings"
                                        InputProps={{
                                            endAdornment: (
                                                <Button variant="outlined" size="small">
                                                    Alterar
                                                </Button>
                                            ),
                                        }}
                                        margin="normal"
                                    />
                                </Grid>
                            </Grid>
                        </CardContent>
                    </Card>
                </TabPanel>

                {/* Avançado (Placeholder) */}
                <TabPanel value={tabValue} index={6}>
                    <Alert severity="warning" sx={{ mb: 3 }}>
                        As configurações avançadas podem afetar a estabilidade e o desempenho do emulador.
                        Altere apenas se souber o que está fazendo.
                    </Alert>

                    <Card>
                        <CardContent>
                            <Typography variant="h6" gutterBottom>Configurações do Sistema</Typography>

                            <Box sx={{ mb: 3 }}>
                                <FormControlLabel
                                    control={
                                        <Switch
                                            defaultChecked
                                        />
                                    }
                                    label="Ativar logs detalhados"
                                />
                                <Typography variant="caption" display="block" color="text.secondary">
                                    Útil para depuração, mas pode reduzir o desempenho.
                                </Typography>
                            </Box>

                            <Box sx={{ mb: 3 }}>
                                <FormControlLabel
                                    control={
                                        <Switch
                                            defaultChecked
                                        />
                                    }
                                    label="Desativar limitação de FPS"
                                />
                                <Typography variant="caption" display="block" color="text.secondary">
                                    Permite que o emulador rode o mais rápido possível.
                                </Typography>
                            </Box>

                            <Box sx={{ mb: 3 }}>
                                <Typography gutterBottom>Intervalo do Watchdog (ms)</Typography>
                                <TextField
                                    value="5000"
                                    type="number"
                                    size="small"
                                />
                            </Box>

                            <Box sx={{ mb: 3 }}>
                                <Button variant="outlined" color="primary" startIcon={<RefreshIcon />}>
                                    Limpar Cache
                                </Button>
                            </Box>
                        </CardContent>
                    </Card>
                </TabPanel>
            </Paper>

            <Snackbar
                open={showSaveMessage}
                autoHideDuration={3000}
                onClose={() => setShowSaveMessage(false)}
            >
                <Alert severity={saveMessage.type as "success" | "error" | "info" | "warning"} onClose={() => setShowSaveMessage(false)}>
                    {saveMessage.text}
                </Alert>
            </Snackbar>
        </Box>
    );
};

export default SettingsPage;
