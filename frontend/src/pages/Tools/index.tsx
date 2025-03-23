import React, { useState } from 'react';
import {
    Box,
    Typography,
    Grid,
    Paper,
    Tabs,
    Tab,
    Card,
    CardContent,
    CardHeader,
    Divider,
    Button,
    TextField,
    Select,
    MenuItem,
    FormControl,
    InputLabel,
    Switch,
    FormControlLabel,
    Alert,
    IconButton,
} from '@mui/material';
import DownloadIcon from '@mui/icons-material/Download';
import RefreshIcon from '@mui/icons-material/Refresh';
import BugReportIcon from '@mui/icons-material/BugReport';
import VisibilityIcon from '@mui/icons-material/Visibility';
import MemoryIcon from '@mui/icons-material/Memory';
import SpeedIcon from '@mui/icons-material/Speed';
import SettingsSuggestIcon from '@mui/icons-material/SettingsSuggest';
import MusicNoteIcon from '@mui/icons-material/MusicNote';

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
            id={`tools-tabpanel-${index}`}
            aria-labelledby={`tools-tab-${index}`}
            {...other}
            style={{ height: '100%' }}
        >
            {value === index && (
                <Box sx={{ p: 2, height: '100%' }}>
                    {children}
                </Box>
            )}
        </div>
    );
}

// Mock data for development (placeholder while the backend is not available)
const mockMemoryData = Array.from({ length: 256 }, (_, i) => ({
    address: (0x1000 + i).toString(16).toUpperCase().padStart(4, '0'),
    value: Math.floor(Math.random() * 256).toString(16).toUpperCase().padStart(2, '0'),
    ascii: String.fromCharCode(32 + Math.floor(Math.random() * 95)),
}));

// Placeholder sprite data (8x8 pixel grid)
const mockSpriteData = [
    [0, 0, 1, 1, 1, 1, 0, 0],
    [0, 1, 1, 1, 1, 1, 1, 0],
    [1, 1, 0, 1, 1, 0, 1, 1],
    [1, 1, 1, 1, 1, 1, 1, 1],
    [1, 1, 1, 1, 1, 1, 1, 1],
    [0, 1, 0, 1, 1, 0, 1, 0],
    [0, 0, 1, 0, 0, 1, 0, 0],
    [0, 1, 0, 0, 0, 0, 1, 0],
];

const ToolsPage: React.FC = () => {
    const [selectedTab, setSelectedTab] = useState(0);
    const [memorySearchQuery, setMemorySearchQuery] = useState('');
    const [memoryRegion, setMemoryRegion] = useState('ram');
    const [spriteSize, setSpriteSize] = useState('8x8');
    const [spritePalette, setSpritePalette] = useState('default');
    const [showGrid, setShowGrid] = useState(true);

    const handleTabChange = (_event: React.SyntheticEvent, newValue: number) => {
        setSelectedTab(newValue);
    };

    // Filter memory based on the search query
    const filteredMemory = memorySearchQuery
        ? mockMemoryData.filter(item =>
            item.address.includes(memorySearchQuery.toUpperCase()) ||
            item.value.includes(memorySearchQuery.toUpperCase()))
        : mockMemoryData;

    return (
        <Box sx={{ display: 'flex', flexDirection: 'column', height: 'calc(100vh - 64px)' }}>
            <Box sx={{ px: 3, py: 2, display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                <Typography variant="h4" component="h1">
                    Ferramentas de Desenvolvimento
                </Typography>
                <Alert severity="info" sx={{ flex: 1, ml: 2 }}>
                    Estas ferramentas são para desenvolvimento e podem não estar totalmente funcionais sem o backend.
                </Alert>
            </Box>

            <Box sx={{ display: 'flex', flexGrow: 1, px: 3, pb: 3, overflow: 'hidden' }}>
                <Paper sx={{ width: '100%', display: 'flex', flexDirection: 'column' }}>
                    <Tabs
                        value={selectedTab}
                        onChange={handleTabChange}
                        variant="scrollable"
                        scrollButtons="auto"
                        sx={{ borderBottom: 1, borderColor: 'divider' }}
                    >
                        <Tab icon={<VisibilityIcon />} label="Visualizador de Sprites" />
                        <Tab icon={<MemoryIcon />} label="Visualizador de Memória" />
                        <Tab icon={<MusicNoteIcon />} label="Monitor de Áudio" />
                        <Tab icon={<SpeedIcon />} label="Desempenho" />
                        <Tab icon={<BugReportIcon />} label="Debugger" />
                        <Tab icon={<SettingsSuggestIcon />} label="Avançado" />
                    </Tabs>

                    {/* Sprite Viewer */}
                    <TabPanel value={selectedTab} index={0}>
                        <Grid container spacing={2} sx={{ height: '100%' }}>
                            <Grid item xs={12} md={8} sx={{ height: '100%' }}>
                                <Paper
                                    elevation={3}
                                    sx={{
                                        height: '100%',
                                        backgroundColor: '#1e1e1e',
                                        display: 'flex',
                                        alignItems: 'center',
                                        justifyContent: 'center',
                                        position: 'relative',
                                        overflow: 'auto',
                                    }}
                                >
                                    {/* Sprite Viewer Canvas */}
                                    <Box sx={{ position: 'relative' }}>
                                        <Box
                                            sx={{
                                                width: spriteSize === '8x8' ? 320 : 640,
                                                height: spriteSize === '8x8' ? 320 : 640,
                                                display: 'grid',
                                                gridTemplateColumns: `repeat(${spriteSize === '8x8' ? 8 : 16}, 1fr)`,
                                                gridTemplateRows: `repeat(${spriteSize === '8x8' ? 8 : 16}, 1fr)`,
                                                gap: showGrid ? '1px' : '0',
                                                border: showGrid ? '1px solid #444' : 'none',
                                                backgroundColor: '#000',
                                            }}
                                        >
                                            {mockSpriteData.flat().map((pixel, idx) => (
                                                <Box
                                                    key={idx}
                                                    sx={{
                                                        backgroundColor: pixel ? (
                                                            spritePalette === 'default' ? '#fff' :
                                                                spritePalette === 'gameboy' ? '#0f380f' :
                                                                    spritePalette === 'nes' ? '#ff0000' : '#fff'
                                                        ) : 'transparent',
                                                        border: showGrid ? '1px solid #333' : 'none',
                                                    }}
                                                />
                                            ))}
                                        </Box>
                                    </Box>

                                    {/* Overlay with sprite information */}
                                    <Box
                                        sx={{
                                            position: 'absolute',
                                            bottom: 10,
                                            left: 10,
                                            backgroundColor: 'rgba(0,0,0,0.7)',
                                            padding: 1,
                                            borderRadius: 1,
                                            color: 'white',
                                            fontSize: '0.75rem',
                                        }}
                                    >
                                        Sprite #1 - Endereço: 0x1000 - Tamanho: 8x8
                                    </Box>
                                </Paper>
                            </Grid>

                            <Grid item xs={12} md={4} sx={{ height: '100%', display: 'flex', flexDirection: 'column' }}>
                                <Card sx={{ mb: 2, flexGrow: 1, display: 'flex', flexDirection: 'column', overflow: 'auto' }}>
                                    <CardHeader title="Controles" />
                                    <Divider />
                                    <CardContent>
                                        <FormControl fullWidth margin="normal">
                                            <InputLabel id="sprite-size-label">Tamanho do Sprite</InputLabel>
                                            <Select
                                                labelId="sprite-size-label"
                                                value={spriteSize}
                                                label="Tamanho do Sprite"
                                                onChange={(e) => setSpriteSize(e.target.value as string)}
                                            >
                                                <MenuItem value="8x8">8x8</MenuItem>
                                                <MenuItem value="16x16">16x16</MenuItem>
                                                <MenuItem value="32x32">32x32</MenuItem>
                                            </Select>
                                        </FormControl>

                                        <FormControl fullWidth margin="normal">
                                            <InputLabel id="palette-label">Paleta de Cores</InputLabel>
                                            <Select
                                                labelId="palette-label"
                                                value={spritePalette}
                                                label="Paleta de Cores"
                                                onChange={(e) => setSpritePalette(e.target.value as string)}
                                            >
                                                <MenuItem value="default">Padrão</MenuItem>
                                                <MenuItem value="gameboy">Game Boy</MenuItem>
                                                <MenuItem value="nes">NES</MenuItem>
                                                <MenuItem value="sega">SEGA</MenuItem>
                                            </Select>
                                        </FormControl>

                                        <FormControlLabel
                                            control={
                                                <Switch
                                                    checked={showGrid}
                                                    onChange={(e) => setShowGrid(e.target.checked)}
                                                />
                                            }
                                            label="Mostrar grade"
                                            sx={{ mt: 2 }}
                                        />

                                        <Box sx={{ mt: 2 }}>
                                            <Button
                                                variant="outlined"
                                                startIcon={<DownloadIcon />}
                                                fullWidth
                                                sx={{ mb: 1 }}
                                            >
                                                Exportar Sprite
                                            </Button>
                                            <Button
                                                variant="outlined"
                                                startIcon={<RefreshIcon />}
                                                fullWidth
                                            >
                                                Atualizar
                                            </Button>
                                        </Box>
                                    </CardContent>
                                </Card>

                                <Card sx={{ flexGrow: 1, display: 'flex', flexDirection: 'column', overflow: 'auto', maxHeight: '50%' }}>
                                    <CardHeader title="Sprites Disponíveis" />
                                    <Divider />
                                    <CardContent sx={{ overflow: 'auto', flexGrow: 1, padding: 0 }}>
                                        <Box sx={{ height: '100%', overflow: 'auto' }}>
                                            <Box component="ul" sx={{ listStyle: 'none', p: 0, m: 0 }}>
                                                {Array.from({ length: 20 }, (_, i) => (
                                                    <Box
                                                        component="li"
                                                        key={i}
                                                        sx={{
                                                            p: 1,
                                                            borderBottom: '1px solid rgba(0,0,0,0.1)',
                                                            '&:hover': { backgroundColor: 'action.hover' },
                                                            cursor: 'pointer',
                                                            display: 'flex',
                                                            justifyContent: 'space-between',
                                                            alignItems: 'center',
                                                        }}
                                                    >
                                                        <Typography variant="body2">Sprite #{i + 1} (0x{(0x1000 + i * 64).toString(16).toUpperCase()})</Typography>
                                                        <IconButton size="small">
                                                            <VisibilityIcon fontSize="small" />
                                                        </IconButton>
                                                    </Box>
                                                ))}
                                            </Box>
                                        </Box>
                                    </CardContent>
                                </Card>
                            </Grid>
                        </Grid>
                    </TabPanel>

                    {/* Memory Viewer */}
                    <TabPanel value={selectedTab} index={1}>
                        <Grid container spacing={2} sx={{ height: '100%' }}>
                            <Grid item xs={12} md={8} sx={{ height: '100%', display: 'flex', flexDirection: 'column' }}>
                                <Box sx={{ mb: 2, display: 'flex', gap: 2 }}>
                                    <TextField
                                        label="Buscar (endereço ou valor)"
                                        size="small"
                                        value={memorySearchQuery}
                                        onChange={(e) => setMemorySearchQuery(e.target.value)}
                                        sx={{ flexGrow: 1 }}
                                    />
                                    <FormControl sx={{ minWidth: 120 }} size="small">
                                        <InputLabel id="memory-region-label">Região</InputLabel>
                                        <Select
                                            labelId="memory-region-label"
                                            value={memoryRegion}
                                            label="Região"
                                            onChange={(e) => setMemoryRegion(e.target.value as string)}
                                        >
                                            <MenuItem value="ram">RAM</MenuItem>
                                            <MenuItem value="vram">VRAM</MenuItem>
                                            <MenuItem value="rom">ROM</MenuItem>
                                            <MenuItem value="sram">SRAM</MenuItem>
                                        </Select>
                                    </FormControl>
                                    <Button startIcon={<RefreshIcon />} variant="outlined">
                                        Atualizar
                                    </Button>
                                </Box>

                                <Paper
                                    sx={{
                                        flexGrow: 1,
                                        overflow: 'auto',
                                        fontFamily: 'monospace',
                                        fontSize: '0.875rem',
                                    }}
                                >
                                    <Box sx={{ display: 'table', width: '100%', borderCollapse: 'collapse' }}>
                                        <Box sx={{ display: 'table-header-group', backgroundColor: 'grey.100' }}>
                                            <Box sx={{ display: 'table-row' }}>
                                                <Box sx={{ display: 'table-cell', p: 1, fontWeight: 'bold', borderBottom: 1, borderColor: 'divider' }}>Endereço</Box>
                                                <Box sx={{ display: 'table-cell', p: 1, fontWeight: 'bold', borderBottom: 1, borderColor: 'divider' }}>+0</Box>
                                                <Box sx={{ display: 'table-cell', p: 1, fontWeight: 'bold', borderBottom: 1, borderColor: 'divider' }}>+1</Box>
                                                <Box sx={{ display: 'table-cell', p: 1, fontWeight: 'bold', borderBottom: 1, borderColor: 'divider' }}>+2</Box>
                                                <Box sx={{ display: 'table-cell', p: 1, fontWeight: 'bold', borderBottom: 1, borderColor: 'divider' }}>+3</Box>
                                                <Box sx={{ display: 'table-cell', p: 1, fontWeight: 'bold', borderBottom: 1, borderColor: 'divider' }}>+4</Box>
                                                <Box sx={{ display: 'table-cell', p: 1, fontWeight: 'bold', borderBottom: 1, borderColor: 'divider' }}>+5</Box>
                                                <Box sx={{ display: 'table-cell', p: 1, fontWeight: 'bold', borderBottom: 1, borderColor: 'divider' }}>+6</Box>
                                                <Box sx={{ display: 'table-cell', p: 1, fontWeight: 'bold', borderBottom: 1, borderColor: 'divider' }}>+7</Box>
                                                <Box sx={{ display: 'table-cell', p: 1, fontWeight: 'bold', borderBottom: 1, borderColor: 'divider' }}>ASCII</Box>
                                            </Box>
                                        </Box>
                                        <Box sx={{ display: 'table-row-group' }}>
                                            {Array.from({ length: Math.ceil(filteredMemory.length / 8) }, (_, rowIndex) => {
                                                const startIdx = rowIndex * 8;
                                                const rowData = filteredMemory.slice(startIdx, startIdx + 8);
                                                const baseAddress = rowData[0]?.address.slice(0, -1) + '0';

                                                return (
                                                    <Box sx={{ display: 'table-row' }} key={rowIndex}>
                                                        <Box sx={{ display: 'table-cell', p: 1, color: 'primary.main', borderBottom: 1, borderColor: 'divider' }}>
                                                            {baseAddress}
                                                        </Box>
                                                        {rowData.map((item, cellIndex) => (
                                                            <Box
                                                                sx={{
                                                                    display: 'table-cell',
                                                                    p: 1,
                                                                    borderBottom: 1,
                                                                    borderColor: 'divider',
                                                                    backgroundColor: memorySearchQuery &&
                                                                        (item.address.includes(memorySearchQuery.toUpperCase()) ||
                                                                            item.value.includes(memorySearchQuery.toUpperCase())) ?
                                                                        'rgba(255, 255, 0, 0.2)' : 'inherit',
                                                                }}
                                                                key={cellIndex}
                                                            >
                                                                {item.value}
                                                            </Box>
                                                        ))}
                                                        <Box sx={{ display: 'table-cell', p: 1, borderBottom: 1, borderColor: 'divider', fontFamily: 'monospace' }}>
                                                            {rowData.map(item => item.ascii).join('')}
                                                        </Box>
                                                    </Box>
                                                );
                                            })}
                                        </Box>
                                    </Box>
                                </Paper>
                            </Grid>

                            <Grid item xs={12} md={4} sx={{ height: '100%', display: 'flex', flexDirection: 'column' }}>
                                <Card sx={{ mb: 2 }}>
                                    <CardHeader title="Inspeção de Memória" />
                                    <Divider />
                                    <CardContent>
                                        <Alert severity="info" sx={{ mb: 2 }}>
                                            Esta ferramenta permite visualizar e editar a memória do emulador em tempo real.
                                        </Alert>

                                        <Box sx={{ mb: 2 }}>
                                            <TextField
                                                label="Ir para Endereço"
                                                size="small"
                                                placeholder="ex: 0x1000"
                                                fullWidth
                                                sx={{ mb: 1 }}
                                            />
                                            <Button variant="contained" size="small" fullWidth>
                                                Ir
                                            </Button>
                                        </Box>

                                        <Divider sx={{ my: 2 }} />

                                        <Box>
                                            <Typography variant="subtitle2" gutterBottom>
                                                Editar Valor
                                            </Typography>

                                            <Box sx={{ display: 'flex', gap: 1, mb: 1 }}>
                                                <TextField
                                                    label="Endereço"
                                                    size="small"
                                                    placeholder="0x1000"
                                                    sx={{ flexGrow: 1 }}
                                                />
                                                <TextField
                                                    label="Valor"
                                                    size="small"
                                                    placeholder="0x00"
                                                    sx={{ width: '80px' }}
                                                />
                                            </Box>

                                            <Button variant="contained" size="small" color="primary" fullWidth>
                                                Escrever Valor
                                            </Button>
                                        </Box>
                                    </CardContent>
                                </Card>

                                <Card sx={{ flexGrow: 1, display: 'flex', flexDirection: 'column' }}>
                                    <CardHeader title="Informações de Memória" />
                                    <Divider />
                                    <CardContent>
                                        <Typography variant="body2" paragraph>
                                            <strong>Tamanho total:</strong> 64 KB
                                        </Typography>
                                        <Typography variant="body2" paragraph>
                                            <strong>Regiões disponíveis:</strong>
                                        </Typography>

                                        <Box component="ul" sx={{ pl: 2, mt: 0 }}>
                                            <Box component="li">
                                                <Typography variant="body2">
                                                    RAM: 0x0000 - 0x1FFF (8 KB)
                                                </Typography>
                                            </Box>
                                            <Box component="li">
                                                <Typography variant="body2">
                                                    VRAM: 0x2000 - 0x3FFF (8 KB)
                                                </Typography>
                                            </Box>
                                            <Box component="li">
                                                <Typography variant="body2">
                                                    ROM: 0x4000 - 0xFFFF (48 KB)
                                                </Typography>
                                            </Box>
                                        </Box>

                                        <Divider sx={{ my: 2 }} />

                                        <Box sx={{ mb: 2 }}>
                                            <Button
                                                variant="outlined"
                                                startIcon={<DownloadIcon />}
                                                fullWidth
                                                sx={{ mb: 1 }}
                                            >
                                                Exportar Dump
                                            </Button>
                                            <Button
                                                variant="outlined"
                                                startIcon={<RefreshIcon />}
                                                fullWidth
                                            >
                                                Atualizar
                                            </Button>
                                        </Box>
                                    </CardContent>
                                </Card>
                            </Grid>
                        </Grid>
                    </TabPanel>

                    {/* Placeholder tabs - To be implemented */}
                    <TabPanel value={selectedTab} index={2}>
                        <Box sx={{ textAlign: 'center', p: 4 }}>
                            <Typography variant="h5" sx={{ mb: 2 }}>
                                Monitor de Áudio
                            </Typography>
                            <Alert severity="info">
                                O monitor de áudio estará disponível quando o backend estiver em execução.
                            </Alert>
                        </Box>
                    </TabPanel>

                    <TabPanel value={selectedTab} index={3}>
                        <Box sx={{ textAlign: 'center', p: 4 }}>
                            <Typography variant="h5" sx={{ mb: 2 }}>
                                Monitoramento de Desempenho
                            </Typography>
                            <Alert severity="info">
                                As ferramentas de monitoramento de desempenho estarão disponíveis quando o backend estiver em execução.
                            </Alert>
                        </Box>
                    </TabPanel>

                    <TabPanel value={selectedTab} index={4}>
                        <Box sx={{ textAlign: 'center', p: 4 }}>
                            <Typography variant="h5" sx={{ mb: 2 }}>
                                Debugger
                            </Typography>
                            <Alert severity="info">
                                O debugger estará disponível quando o backend estiver em execução.
                            </Alert>
                        </Box>
                    </TabPanel>

                    <TabPanel value={selectedTab} index={5}>
                        <Box sx={{ textAlign: 'center', p: 4 }}>
                            <Typography variant="h5" sx={{ mb: 2 }}>
                                Ferramentas Avançadas
                            </Typography>
                            <Alert severity="info">
                                As ferramentas avançadas estarão disponíveis quando o backend estiver em execução.
                            </Alert>
                        </Box>
                    </TabPanel>
                </Paper>
            </Box>
        </Box>
    );
};

export default ToolsPage;
