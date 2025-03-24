import React from 'react';
import {
    Card,
    CardContent,
    CardMedia,
    Typography,
    Button,
    Box,
    Tooltip,
    IconButton,
    Divider,
    Chip
} from '@mui/material';
import PlayArrowIcon from '@mui/icons-material/PlayArrow';
import DeleteIcon from '@mui/icons-material/Delete';
import InfoIcon from '@mui/icons-material/Info';
import { SaveState, ConsoleType } from '../types/emulator.types';

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
    'default': 'https://placehold.co/320x240/333333/ffffff?text=Save+State'
};

interface SaveStateCardProps {
    saveState: SaveState;
    consoleType: ConsoleType;
    onLoad: () => void;
    onDelete?: () => void;
    showDetails?: boolean;
}

const SaveStateCard: React.FC<SaveStateCardProps> = ({
    saveState,
    consoleType,
    onLoad,
    onDelete,
    showDetails = true
}) => {
    // Formata o tempo de jogo para exibição
    const formatPlayTime = (seconds: number | undefined) => {
        if (!seconds) return null;

        const hours = Math.floor(seconds / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);

        if (hours > 0) {
            return `${hours}h ${minutes}m`;
        }
        return `${minutes}m`;
    };

    // Formata a data para exibição
    const formatDate = (timestamp: number) => {
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

    return (
        <Card sx={{
            height: '100%',
            display: 'flex',
            flexDirection: 'column',
            transition: 'transform 0.2s, box-shadow 0.2s',
            '&:hover': {
                transform: 'translateY(-4px)',
                boxShadow: 4,
            }
        }}>
            <CardMedia
                component="img"
                height="160"
                image={saveState.thumbnail || consolePlaceholders[consoleType] || consolePlaceholders.default}
                alt={saveState.name}
                sx={{
                    cursor: 'pointer',
                    objectFit: 'cover'
                }}
                onClick={onLoad}
            />
            <CardContent sx={{ flexGrow: 1, pb: 1 }}>
                {/* Nome do save state com data em chip */}
                <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start', mb: 1 }}>
                    <Typography variant="h6" component="div" noWrap sx={{ mr: 1 }}>
                        {saveState.name}
                    </Typography>
                    <Chip
                        label={formatDate(saveState.timestamp)}
                        size="small"
                        color="primary"
                        variant="outlined"
                    />
                </Box>

                {showDetails && (
                    <>
                        {/* Detalhes do save state */}
                        {saveState.metadata?.realName && (
                            <Typography variant="body2" color="text.secondary" noWrap>
                                {saveState.metadata.realName}
                            </Typography>
                        )}

                        {saveState.metadata?.playTime && (
                            <Typography variant="body2" color="text.secondary" sx={{ display: 'flex', alignItems: 'center', mt: 1 }}>
                                <Box component="span" sx={{ mr: 1 }}>⏱️</Box>
                                Tempo: {formatPlayTime(saveState.metadata.playTime)}
                            </Typography>
                        )}

                        {saveState.metadata?.level && (
                            <Typography variant="body2" color="text.secondary">
                                <Box component="span" sx={{ mr: 1 }}>🏆</Box>
                                {typeof saveState.metadata.level === 'number'
                                    ? `Fase ${saveState.metadata.level}`
                                    : saveState.metadata.level}
                            </Typography>
                        )}

                        {/* Exibir outras informações relevantes */}
                        {saveState.metadata?.screenInfo && (
                            <Typography variant="body2" color="text.secondary">
                                {saveState.metadata.screenInfo}
                            </Typography>
                        )}
                    </>
                )}
            </CardContent>

            <Divider />
            <Box sx={{ p: 1, display: 'flex', justifyContent: 'space-between' }}>
                {onDelete && (
                    <Tooltip title="Excluir save state">
                        <IconButton
                            size="small"
                            color="error"
                            onClick={onDelete}
                        >
                            <DeleteIcon />
                        </IconButton>
                    </Tooltip>
                )}
                <Button
                    variant="contained"
                    color="primary"
                    size="small"
                    startIcon={<PlayArrowIcon />}
                    onClick={onLoad}
                    sx={{ ml: onDelete ? 0 : 'auto' }}
                >
                    Carregar
                </Button>
            </Box>
        </Card>
    );
};

export default SaveStateCard;
