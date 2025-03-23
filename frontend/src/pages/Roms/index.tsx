import React, { useEffect, useState } from 'react';
import {
    Box,
    Typography,
    Grid,
    Card,
    CardContent,
    CardMedia,
    TextField,
    InputAdornment,
    IconButton,
    Button,
    FormControl,
    InputLabel,
    Select,
    MenuItem,
    CircularProgress,
    Divider,
    Chip,
    Tooltip,
    SelectChangeEvent,
    Badge,
    Dialog,
    DialogTitle,
    DialogContent,
    DialogActions,
    ListItemButton,
    List,
    ListItem,
    ListItemText,
    ListItemIcon,
    Avatar,
} from '@mui/material';
import SearchIcon from '@mui/icons-material/Search';
import UploadIcon from '@mui/icons-material/Upload';
import SortIcon from '@mui/icons-material/Sort';
import FavoriteIcon from '@mui/icons-material/Favorite';
import FavoriteBorderIcon from '@mui/icons-material/FavoriteBorder';
import PlayArrowIcon from '@mui/icons-material/PlayArrow';
import FilterListIcon from '@mui/icons-material/FilterList';
import SaveIcon from '@mui/icons-material/Save';
import RestoreIcon from '@mui/icons-material/Restore';
import HistoryIcon from '@mui/icons-material/History';
import InfoIcon from '@mui/icons-material/Info';
import GridViewIcon from '@mui/icons-material/GridView';
import ViewListIcon from '@mui/icons-material/ViewList';
import FolderIcon from '@mui/icons-material/Folder';
import SettingsIcon from '@mui/icons-material/Settings';
import CloudUploadIcon from '@mui/icons-material/CloudUpload';
import DeleteIcon from '@mui/icons-material/Delete';
import DirectoryTreeIcon from '@mui/icons-material/AccountTree';

import { useAppDispatch, useAppSelector } from '../../state/store';
import { fetchRoms, setSearchQuery, setSelectedConsoleType, setSelectedRom } from '../../state/slices/romsSlice';
import { RomInfo, ConsoleType, SaveState, consoleDisplayNames } from '../../types/emulator.types';
import { setCurrentConsole, setLoadedRom } from '../../state/slices/emulatorSlice';
import { useEmulatorState } from '../../hooks/useEmulatorState';
import emulatorApiService from '../../services/emulator/restApi';
import SaveStateCard from '../../components/SaveStateCard';
import UploadRomDialog from '../../components/UploadRomDialog';
import romService from '../../services/emulator/romService';

// Imagens de placeholder para consoles
const consolePlaceholders: Record<ConsoleType | 'default', string> = {
    'megadrive': 'https://placehold.co/320x240/001155/ffffff?text=Mega+Drive',
    'genesis': 'https://placehold.co/320x240/001155/ffffff?text=Genesis',
    'sega32x': 'https://placehold.co/320x240/000055/ffffff?text=32X',
    'segacd': 'https://placehold.co/320x240/222222/ffffff?text=Sega+CD',
    'mastersystem': 'https://placehold.co/320x240/0000AA/ffffff?text=Master+System',
    'gamegear': 'https://placehold.co/320x240/0055AA/ffffff?text=Game+Gear',
    'sg1000': 'https://placehold.co/320x240/003377/ffffff?text=SG-1000',
    'nes': 'https://placehold.co/320x240/8B0000/ffffff?text=NES',
    'snes': 'https://placehold.co/320x240/6A0DAD/ffffff?text=SNES',
    'gameboy': 'https://placehold.co/320x240/004d00/ffffff?text=Game+Boy',
    'gba': 'https://placehold.co/320x240/553311/ffffff?text=GBA',
    'n64': 'https://placehold.co/320x240/006633/ffffff?text=N64',
    'psx': 'https://placehold.co/320x240/444444/ffffff?text=PlayStation',
    'tms9918': 'https://placehold.co/320x240/666666/ffffff?text=TMS9918',
    'atari7800': 'https://placehold.co/320x240/00008B/ffffff?text=Atari7800',
    'gameboyadvanced': 'https://placehold.co/320x240/553311/ffffff?text=Game+Boy+Advance',
    'pcengine': 'https://placehold.co/320x240/B22222/ffffff?text=PC+Engine',
    'neogeo': 'https://placehold.co/320x240/2F4F4F/ffffff?text=Neo+Geo',
    'saturn': 'https://placehold.co/320x240/000080/ffffff?text=Saturn',
    'playstation': 'https://placehold.co/320x240/444444/ffffff?text=PlayStation',
    '32x': 'https://placehold.co/320x240/000055/ffffff?text=32X',
    'default': 'https://placehold.co/320x240/333333/ffffff?text=ROM'
};

// MockData para ROMs (para desenvolvimento sem backend)
const mockRoms: RomInfo[] = [
    {
        id: '1',
        name: 'Sonic The Hedgehog',
        path: '/roms/sonic.md',
        size: 512000,
        consoleType: 'megadrive',
        region: 'USA',
        lastPlayed: Date.now() - 1000 * 60 * 60 * 24, // 1 dia atrás
        favorite: true,
        playCount: 15,
        totalPlayTime: 1000 * 60 * 60 * 8, // 8 horas
        metadata: { publisher: 'SEGA' }
    },
    {
        id: '2',
        name: 'Super Mario Bros. 3',
        path: '/roms/mario3.nes',
        size: 384000,
        consoleType: 'nes',
        region: 'Japan',
        lastPlayed: Date.now() - 1000 * 60 * 60 * 48, // 2 dias atrás
        favorite: false,
        playCount: 7,
        totalPlayTime: 1000 * 60 * 60 * 4, // 4 horas
        metadata: { publisher: 'Nintendo' }
    },
    {
        id: '3',
        name: 'Chrono Trigger',
        path: '/roms/chronotrigger.sfc',
        size: 4194304,
        consoleType: 'snes',
        region: 'USA',
        lastPlayed: Date.now() - 1000 * 60 * 60 * 72, // 3 dias atrás
        favorite: true,
        playCount: 3,
        totalPlayTime: 1000 * 60 * 60 * 12, // 12 horas
        metadata: { publisher: 'Square' }
    },
    {
        id: '4',
        name: 'Alex Kidd in Miracle World',
        path: '/roms/alexkidd.sms',
        size: 262144,
        consoleType: 'mastersystem',
        region: 'Europe',
        lastPlayed: null,
        favorite: false,
        playCount: 0,
        totalPlayTime: 0,
        metadata: { publisher: 'SEGA' }
    },
    {
        id: '5',
        name: 'Tetris',
        path: '/roms/tetris.gb',
        size: 32768,
        consoleType: 'gameboy',
        region: 'Japan',
        lastPlayed: Date.now() - 1000 * 60 * 60 * 24 * 7, // 1 semana atrás
        favorite: false,
        playCount: 20,
        totalPlayTime: 1000 * 60 * 60 * 5, // 5 horas
        metadata: { publisher: 'Nintendo' }
    }
];

const RomsPage: React.FC = () => {
    const dispatch = useAppDispatch();
    const { romsList, filteredRoms, searchQuery, selectedConsoleType, loading, error } = useAppSelector(state => state.roms);
    const { loadRom } = useEmulatorState({ autoConnect: false });

    const [viewType, setViewType] = useState<'grid' | 'list'>('grid');
    const [sortBy, setSortBy] = useState<'name' | 'lastPlayed' | 'size'>('name');
    const [uploadDialogOpen, setUploadDialogOpen] = useState(false);
    const [saveStatesDialogOpen, setSaveStatesDialogOpen] = useState(false);
    const [selectedRomForStates, setSelectedRomForStates] = useState<RomInfo | null>(null);
    const [backendStatus, setBackendStatus] = useState<boolean>(false);
    const [directoryDialogOpen, setDirectoryDialogOpen] = useState(false);
    const [customRomsDir, setCustomRomsDir] = useState<string>('');
    const [activeDirectory, setActiveDirectory] = useState<string>('');

    // Verificar a disponibilidade do backend
    useEffect(() => {
        const checkBackend = async () => {
            const isOnlineStatus = await emulatorApiService.checkBackendStatus();
            setBackendStatus(isOnlineStatus.online);

            // Carregar diretório atual de ROMs do romService
            const currentDir = romService.getRomsPath();
            setActiveDirectory(currentDir);
            setCustomRomsDir(currentDir);
        };
        checkBackend();
    }, []);

    // Efeito para carregar ROMs ao montar o componente
    useEffect(() => {
        dispatch(fetchRoms());
    }, [dispatch]);

    const handleSearchChange = (e: React.ChangeEvent<HTMLInputElement>) => {
        dispatch(setSearchQuery(e.target.value));
    };

    const handleConsoleFilterChange = (e: SelectChangeEvent) => {
        dispatch(setSelectedConsoleType(e.target.value as ConsoleType | 'all'));
    };

    const handleSortChange = (e: SelectChangeEvent) => {
        setSortBy(e.target.value as 'name' | 'lastPlayed' | 'size');
    };

    const handleRomPlay = (rom: RomInfo) => {
        dispatch(setSelectedRom(rom));
        dispatch(setCurrentConsole(rom.consoleType));
        dispatch(setLoadedRom(rom.id));
        loadRom(rom.id);
        // Navegar para a página do emulador
        window.location.href = '/emulator';
    };

    const handleShowSaveStates = (rom: RomInfo) => {
        setSelectedRomForStates(rom);
        setSaveStatesDialogOpen(true);
    };

    const handlePlayWithSaveState = (rom: RomInfo, saveState: SaveState) => {
        dispatch(setSelectedRom(rom));
        dispatch(setCurrentConsole(rom.consoleType));
        dispatch(setLoadedRom(rom.id));

        // Carregar ROM com saveState
        loadRom(rom.id, saveState.id);
        setSaveStatesDialogOpen(false);

        // Navegar para a página do emulador
        window.location.href = '/emulator';
    };

    const handleDeleteSaveState = async (saveStateId: string) => {
        if (!backendStatus) {
            alert('O backend não está disponível para excluir estados salvos');
            return;
        }

        const confirmed = window.confirm('Tem certeza que deseja excluir este estado salvo?');
        if (!confirmed) return;

        const success = await emulatorApiService.deleteSaveState(saveStateId);
        if (success) {
            // Atualizar a lista de ROMs para refletir a exclusão
            dispatch(fetchRoms());
        } else {
            alert('Erro ao excluir o estado salvo');
        }
    };

    // Função para mudar o diretório de ROMs
    const handleChangeRomsDirectory = async () => {
        if (!customRomsDir) return;

        try {
            // Buscar a configuração atual primeiro
            const currentConfig = await emulatorApiService.getEmulatorConfig();

            // Atualizar apenas a propriedade que precisamos no serviço de ROM
            romService.setRomsPath(customRomsDir);

            // Chamar a API com um objeto EmulatorState parcial
            const updatedConfig = await emulatorApiService.updateEmulatorConfig({
                // Mantemos as propriedades existentes
                ...currentConfig,
                // Atualizamos alguma propriedade para forçar a atualização
                // Isso será tratado pelo método stateToConfig na API
                audioEnabled: currentConfig.audioEnabled
            });

            if (updatedConfig) {
                setActiveDirectory(customRomsDir);
                setDirectoryDialogOpen(false);

                // Recarregar a lista de ROMs
                dispatch(fetchRoms());
            } else {
                // Mostrar mensagem de erro
                alert('Não foi possível alterar o diretório de ROMs');
            }
        } catch (error) {
            console.error('Erro ao atualizar o diretório:', error);
            alert('Erro ao alterar o diretório de ROMs');
        }
    };

    // Processar ROMs para exibição
    const processedRoms = [...filteredRoms].sort((a, b) => {
        switch (sortBy) {
            case 'name':
                return a.name.localeCompare(b.name);
            case 'lastPlayed':
                if (a.lastPlayed === null) return 1;
                if (b.lastPlayed === null) return -1;
                return b.lastPlayed - a.lastPlayed;
            case 'size':
                return b.size - a.size;
            default:
                return 0;
        }
    });

    const formatSize = (bytes: number): string => {
        const megabytes = bytes / (1024 * 1024);
        return megabytes < 1
            ? `${(bytes / 1024).toFixed(1)} KB`
            : `${megabytes.toFixed(1)} MB`;
    };

    const formatLastPlayed = (timestamp: number | null): string => {
        if (timestamp === null) return 'Nunca jogado';

        const date = new Date(timestamp);
        const now = new Date();
        const diffHours = Math.floor((now.getTime() - date.getTime()) / (1000 * 60 * 60));

        if (diffHours < 24) {
            return `${diffHours} hora${diffHours !== 1 ? 's' : ''} atrás`;
        } else {
            const diffDays = Math.floor(diffHours / 24);
            if (diffDays < 30) {
                return `${diffDays} dia${diffDays !== 1 ? 's' : ''} atrás`;
            } else {
                return date.toLocaleDateString();
            }
        }
    };

    // Função para obter imagem do console ou save state
    const getRomImage = (rom: RomInfo): string => {
        // Se tem save states com thumbnail, usar primeira thumbnail
        if (rom.metadata.saveStates && rom.metadata.saveStates.length > 0) {
            // Ordenar para pegar o save state mais recente com thumbnail
            const sortedStates = [...rom.metadata.saveStates]
                .filter(state => state.thumbnail)
                .sort((a, b) => b.timestamp - a.timestamp);

            if (sortedStates.length > 0 && sortedStates[0].thumbnail) {
                return sortedStates[0].thumbnail;
            }
        }

        // Senão, usar placeholder do console
        return consolePlaceholders[rom.consoleType] || consolePlaceholders.default;
    };

    // Função para obter o nome real da ROM se disponível
    const getRomDisplayName = (rom: RomInfo): string => {
        if (rom.metadata.realTitle) {
            return rom.metadata.realTitle;
        }
        return rom.name;
    };

    // Função para verificar se a ROM tem save states
    const hasSaveStates = (rom: RomInfo): boolean => {
        return !!(rom.metadata.saveStates && rom.metadata.saveStates.length > 0);
    };

    // Renderização da visão em lista
    const renderListView = () => (
        <List sx={{ width: '100%' }}>
            {processedRoms.map((rom) => (
                <ListItem
                    key={rom.id}
                    divider
                    secondaryAction={
                        <Box>
                            {hasSaveStates(rom) && (
                                <Tooltip title="Ver estados salvos">
                                    <IconButton
                                        edge="end"
                                        aria-label="save states"
                                        onClick={() => handleShowSaveStates(rom)}
                                        sx={{ mr: 1 }}
                                    >
                                        <Badge badgeContent={rom.metadata.saveStates?.length || 0} color="primary">
                                            <RestoreIcon />
                                        </Badge>
                                    </IconButton>
                                </Tooltip>
                            )}
                            <Tooltip title="Jogar">
                                <IconButton
                                    edge="end"
                                    aria-label="play"
                                    onClick={() => handleRomPlay(rom)}
                                    color="primary"
                                >
                                    <PlayArrowIcon />
                                </IconButton>
                            </Tooltip>
                        </Box>
                    }
                >
                    <ListItemButton onClick={() => hasSaveStates(rom) ? handleShowSaveStates(rom) : handleRomPlay(rom)}>
                        <ListItemIcon>
                            <Avatar
                                src={getRomImage(rom)}
                                variant="rounded"
                                sx={{ width: 48, height: 48 }}
                            />
                        </ListItemIcon>
                        <ListItemText
                            primary={
                                <Box sx={{ display: 'flex', alignItems: 'center' }}>
                                    {getRomDisplayName(rom)}
                                    {rom.favorite && <FavoriteIcon color="error" fontSize="small" sx={{ ml: 1 }} />}
                                </Box>
                            }
                            secondary={
                                <Box>
                                    <Chip
                                        label={consoleDisplayNames[rom.consoleType] || rom.consoleType.toUpperCase()}
                                        size="small"
                                        sx={{ mr: 1, mb: 0.5 }}
                                    />
                                    <Chip
                                        label={rom.region}
                                        size="small"
                                        sx={{ mr: 1, mb: 0.5 }}
                                        variant="outlined"
                                    />
                                    <Typography variant="caption" component="span" sx={{ ml: 1 }}>
                                        {formatSize(rom.size)}
                                    </Typography>
                                </Box>
                            }
                        />
                    </ListItemButton>
                </ListItem>
            ))}
        </List>
    );

    // Renderização da visão em grade
    const renderGridView = () => (
        <Grid container spacing={2}>
            {processedRoms.map((rom) => (
                <Grid item xs={12} sm={6} md={4} lg={3} key={rom.id}>
                    <Card
                        sx={{
                            height: '100%',
                            display: 'flex',
                            flexDirection: 'column',
                            transition: 'transform 0.2s, box-shadow 0.2s',
                            '&:hover': {
                                transform: 'translateY(-4px)',
                                boxShadow: 4,
                            }
                        }}
                    >
                        <CardMedia
                            component="img"
                            height="160"
                            image={getRomImage(rom)}
                            alt={rom.name}
                            onClick={() => hasSaveStates(rom) ? handleShowSaveStates(rom) : handleRomPlay(rom)}
                            sx={{ cursor: 'pointer' }}
                        />
                        <CardContent sx={{ flexGrow: 1 }}>
                            <Box sx={{ display: 'flex', justifyContent: 'space-between', mb: 1 }}>
                                <Typography variant="h6" component="h2" noWrap>
                                    {getRomDisplayName(rom)}
                                </Typography>
                                <IconButton size="small">
                                    {rom.favorite ? <FavoriteIcon color="error" /> : <FavoriteBorderIcon />}
                                </IconButton>
                            </Box>

                            {/* Se tiver um nome de arquivo diferente do título real, mostrar abaixo */}
                            {rom.metadata.realTitle && rom.metadata.realTitle !== rom.name && (
                                <Typography variant="caption" color="text.secondary" sx={{ display: 'block', mb: 1 }}>
                                    {rom.name}
                                </Typography>
                            )}

                            <Box sx={{ mb: 1 }}>
                                <Chip
                                    label={consoleDisplayNames[rom.consoleType] || rom.consoleType.toUpperCase()}
                                    size="small"
                                    sx={{ mr: 1, mb: 0.5 }}
                                />
                                <Chip
                                    label={rom.region}
                                    size="small"
                                    sx={{ mr: 1, mb: 0.5 }}
                                    variant="outlined"
                                />
                            </Box>

                            <Typography variant="body2" color="text.secondary">
                                Tamanho: {formatSize(rom.size)}
                            </Typography>

                            {rom.lastPlayed && (
                                <Typography variant="body2" color="text.secondary">
                                    Jogado: {formatLastPlayed(rom.lastPlayed)}
                                </Typography>
                            )}

                            {rom.playCount > 0 && (
                                <Typography variant="body2" color="text.secondary">
                                    Vezes jogado: {rom.playCount}
                                </Typography>
                            )}

                            {hasSaveStates(rom) && (
                                <Typography variant="body2" color="primary" sx={{ mt: 1, display: 'flex', alignItems: 'center' }}>
                                    <SaveIcon fontSize="small" sx={{ mr: 0.5 }} />
                                    {rom.metadata.saveStates?.length} {rom.metadata.saveStates?.length === 1 ? 'estado salvo' : 'estados salvos'}
                                </Typography>
                            )}
                        </CardContent>

                        <Divider />

                        <Box sx={{ p: 1, display: 'flex', justifyContent: 'space-between' }}>
                            {hasSaveStates(rom) ? (
                                <>
                                    <Button
                                        variant="outlined"
                                        size="small"
                                        startIcon={<RestoreIcon />}
                                        onClick={() => handleShowSaveStates(rom)}
                                    >
                                        Estados
                                    </Button>
                                    <Button
                                        variant="contained"
                                        color="primary"
                                        size="small"
                                        startIcon={<PlayArrowIcon />}
                                        onClick={() => handleRomPlay(rom)}
                                    >
                                        Jogar
                                    </Button>
                                </>
                            ) : (
                                <Button
                                    variant="contained"
                                    color="primary"
                                    size="small"
                                    startIcon={<PlayArrowIcon />}
                                    onClick={() => handleRomPlay(rom)}
                                    sx={{ ml: 'auto' }}
                                >
                                    Jogar
                                </Button>
                            )}
                        </Box>
                    </Card>
                </Grid>
            ))}
        </Grid>
    );

    // Dialog de Save States
    const renderSaveStatesDialog = () => (
        <Dialog
            open={saveStatesDialogOpen}
            onClose={() => setSaveStatesDialogOpen(false)}
            maxWidth="md"
            fullWidth
        >
            <DialogTitle>
                <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                    <Typography variant="h6" component="div">
                        {selectedRomForStates ? `Estados Salvos - ${getRomDisplayName(selectedRomForStates)}` : 'Estados Salvos'}
                    </Typography>
                    <Chip
                        label={selectedRomForStates?.consoleType
                            ? (consoleDisplayNames[selectedRomForStates.consoleType] || selectedRomForStates.consoleType)
                            : ''
                        }
                        size="small"
                    />
                </Box>
            </DialogTitle>
            <DialogContent dividers>
                {selectedRomForStates && selectedRomForStates.metadata.saveStates && (
                    <Grid container spacing={2}>
                        {selectedRomForStates.metadata.saveStates.map((saveState) => (
                            <Grid item xs={12} sm={6} md={4} key={saveState.id}>
                                <SaveStateCard
                                    saveState={saveState}
                                    consoleType={selectedRomForStates.consoleType}
                                    onLoad={() => handlePlayWithSaveState(selectedRomForStates, saveState)}
                                    onDelete={backendStatus ? () => handleDeleteSaveState(saveState.id) : undefined}
                                />
                            </Grid>
                        ))}
                    </Grid>
                )}

                {selectedRomForStates && (!selectedRomForStates.metadata.saveStates || selectedRomForStates.metadata.saveStates.length === 0) && (
                    <Box sx={{ textAlign: 'center', p: 4 }}>
                        <Typography variant="h6">Nenhum estado salvo encontrado</Typography>
                        <Typography variant="body2" color="text.secondary">
                            Crie um estado salvo enquanto joga esta ROM
                        </Typography>
                    </Box>
                )}
            </DialogContent>
            <DialogActions>
                <Button onClick={() => handleRomPlay(selectedRomForStates!)} color="primary" startIcon={<PlayArrowIcon />}>
                    Jogar sem estado salvo
                </Button>
                <Button onClick={() => setSaveStatesDialogOpen(false)}>
                    Fechar
                </Button>
            </DialogActions>
        </Dialog>
    );

    // Dialog para configuração de diretório de ROMs
    const renderDirectoryDialog = () => (
        <Dialog
            open={directoryDialogOpen}
            onClose={() => setDirectoryDialogOpen(false)}
            maxWidth="md"
            fullWidth
        >
            <DialogTitle>
                <Box sx={{ display: 'flex', alignItems: 'center' }}>
                    <FolderIcon sx={{ mr: 1 }} />
                    <Typography variant="h6">Configurar Diretório de ROMs</Typography>
                </Box>
            </DialogTitle>
            <DialogContent dividers>
                <Typography variant="body2" paragraph>
                    Diretório atual: <strong>{activeDirectory}</strong>
                </Typography>

                <TextField
                    fullWidth
                    label="Novo diretório de ROMs"
                    variant="outlined"
                    value={customRomsDir}
                    onChange={(e) => setCustomRomsDir(e.target.value)}
                    placeholder="Ex: ./resources/roms"
                    helperText="Digite o caminho completo para o diretório que contém suas ROMs"
                    sx={{ mb: 2 }}
                />

                <Typography variant="body2" color="text.secondary">
                    Dicas:
                </Typography>
                <ul>
                    <li>Use <code>./resources/roms</code> para o diretório padrão do aplicativo</li>
                    <li>Cada console deve ter seu próprio subdiretório</li>
                    <li>O emulador detectará automaticamente o tipo de console com base no nome da pasta</li>
                </ul>
            </DialogContent>
            <DialogActions>
                <Button onClick={() => setDirectoryDialogOpen(false)}>
                    Cancelar
                </Button>
                <Button
                    onClick={handleChangeRomsDirectory}
                    variant="contained"
                    color="primary"
                    startIcon={<FolderIcon />}
                    disabled={!customRomsDir}
                >
                    Alterar Diretório
                </Button>
            </DialogActions>
        </Dialog>
    );

    return (
        <Box sx={{ p: 3 }}>
            <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
                <Typography variant="h4" component="h1">
                    Biblioteca de ROMs
                </Typography>
                <Box>
                    {!backendStatus && (
                        <Chip
                            icon={<InfoIcon />}
                            label="Modo offline - usando ROMs locais"
                            color="warning"
                            sx={{ mr: 2 }}
                        />
                    )}
                    <Tooltip title="Configurar diretório de ROMs">
                        <IconButton
                            color="primary"
                            onClick={() => setDirectoryDialogOpen(true)}
                            sx={{ mr: 1 }}
                        >
                            <FolderIcon />
                        </IconButton>
                    </Tooltip>
                    <Button
                        variant="contained"
                        color="primary"
                        startIcon={<UploadIcon />}
                        onClick={() => setUploadDialogOpen(true)}
                        disabled={!backendStatus}
                    >
                        Fazer Upload
                    </Button>
                </Box>
            </Box>

            {/* Diretório atual */}
            <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                <FolderIcon color="action" sx={{ mr: 1 }} />
                <Typography variant="body2" color="text.secondary">
                    Diretório: {activeDirectory}
                </Typography>
            </Box>

            {/* Barra de filtros */}
            <Box sx={{ display: 'flex', flexWrap: 'wrap', gap: 2, mb: 3 }}>
                <TextField
                    label="Pesquisar ROMs"
                    variant="outlined"
                    size="small"
                    value={searchQuery}
                    onChange={handleSearchChange}
                    sx={{ flexGrow: 1, minWidth: '200px' }}
                    InputProps={{
                        startAdornment: (
                            <InputAdornment position="start">
                                <SearchIcon />
                            </InputAdornment>
                        ),
                    }}
                />

                <FormControl sx={{ minWidth: '180px' }} size="small">
                    <InputLabel id="console-filter-label">Console</InputLabel>
                    <Select
                        labelId="console-filter-label"
                        value={selectedConsoleType || 'all'}
                        onChange={handleConsoleFilterChange}
                        label="Console"
                    >
                        <MenuItem value="all">Todos</MenuItem>
                        <MenuItem value="megadrive">Mega Drive / Genesis</MenuItem>
                        <MenuItem value="nes">NES</MenuItem>
                        <MenuItem value="snes">SNES</MenuItem>
                        <MenuItem value="mastersystem">Master System</MenuItem>
                        <MenuItem value="gameboy">Game Boy</MenuItem>
                        <MenuItem value="gameboyadvanced">Game Boy Advance</MenuItem>
                        <MenuItem value="pcengine">PC Engine</MenuItem>
                        <MenuItem value="neogeo">Neo Geo</MenuItem>
                        <MenuItem value="saturn">Saturn</MenuItem>
                        <MenuItem value="playstation">PlayStation</MenuItem>
                        <MenuItem value="32x">32X</MenuItem>
                        <MenuItem value="atari7800">Atari 7800</MenuItem>
                    </Select>
                </FormControl>

                <FormControl sx={{ minWidth: '180px' }} size="small">
                    <InputLabel id="sort-label">Ordenar por</InputLabel>
                    <Select
                        labelId="sort-label"
                        value={sortBy}
                        onChange={handleSortChange}
                        label="Ordenar por"
                    >
                        <MenuItem value="name">Nome</MenuItem>
                        <MenuItem value="lastPlayed">Jogado recentemente</MenuItem>
                        <MenuItem value="size">Tamanho</MenuItem>
                    </Select>
                </FormControl>

                <Box>
                    <Tooltip title="Visualização em grade">
                        <IconButton
                            color={viewType === 'grid' ? 'primary' : 'default'}
                            onClick={() => setViewType('grid')}
                        >
                            <GridViewIcon />
                        </IconButton>
                    </Tooltip>
                    <Tooltip title="Visualização em lista">
                        <IconButton
                            color={viewType === 'list' ? 'primary' : 'default'}
                            onClick={() => setViewType('list')}
                        >
                            <ViewListIcon />
                        </IconButton>
                    </Tooltip>
                </Box>
            </Box>

            {/* Status e estatísticas */}
            <Box sx={{ display: 'flex', justifyContent: 'space-between', mb: 2 }}>
                <Typography variant="body2" color="text.secondary">
                    {processedRoms.length} ROMs encontradas
                </Typography>
            </Box>

            {/* Lista de ROMs */}
            {loading ? (
                <Box sx={{ display: 'flex', justifyContent: 'center', p: 4 }}>
                    <CircularProgress />
                </Box>
            ) : error ? (
                <Typography color="error" sx={{ p: 2 }}>
                    Erro ao carregar ROMs: {error}
                </Typography>
            ) : processedRoms.length === 0 ? (
                <Box sx={{ textAlign: 'center', p: 4 }}>
                    <Typography variant="h6">Nenhuma ROM encontrada</Typography>
                    <Typography variant="body2" color="text.secondary">
                        Faça upload de uma ROM ou altere os filtros de pesquisa
                    </Typography>
                </Box>
            ) : (
                viewType === 'grid' ? renderGridView() : renderListView()
            )}

            {/* Dialog para SaveStates */}
            {renderSaveStatesDialog()}

            {/* Dialog para configuração de diretório */}
            {renderDirectoryDialog()}

            {/* Dialog para Upload de ROMs */}
            <UploadRomDialog
                open={uploadDialogOpen}
                onClose={() => setUploadDialogOpen(false)}
            />
        </Box>
    );
};

export default RomsPage;
