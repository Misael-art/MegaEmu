import React, { useState, useEffect } from 'react';
import {
    Button,
    Typography,
    Paper,
    List,
    ListItem,
    ListItemText,
    ListItemIcon,
    Divider,
    TextField,
    InputAdornment,
    ListItemButton
} from '@mui/material';
import SearchIcon from '@mui/icons-material/Search';
import SportsEsportsIcon from '@mui/icons-material/SportsEsports';
import FolderOpenIcon from '@mui/icons-material/FolderOpen';
import './RomSelector.css';

// Lista simulada de ROMs
const mockRoms = [
    { id: 1, name: 'Sonic the Hedgehog', system: 'Mega Drive' },
    { id: 2, name: 'Super Mario Bros', system: 'NES' },
    { id: 3, name: 'Donkey Kong Country', system: 'SNES' },
    { id: 4, name: 'Alex Kidd', system: 'Master System' },
    { id: 5, name: 'Pokémon Red', system: 'Game Boy' },
];

const RomSelector: React.FC = () => {
    const [searchTerm, setSearchTerm] = useState('');
    const [selectedRom, setSelectedRom] = useState<number | null>(null);
    const [isElectron, setIsElectron] = useState(false);

    useEffect(() => {
        // Verificar se o Electron está disponível
        if (typeof window.electron !== 'undefined') {
            setIsElectron(true);
            console.log('Electron detectado no RomSelector');
        }
    }, []);

    const filteredRoms = mockRoms.filter(rom =>
        rom.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
        rom.system.toLowerCase().includes(searchTerm.toLowerCase())
    );

    const handleSearchChange = (event: React.ChangeEvent<HTMLInputElement>) => {
        setSearchTerm(event.target.value);
    };

    const handleRomClick = (id: number) => {
        setSelectedRom(id);
    };

    const handleLoadRom = () => {
        if (selectedRom) {
            const rom = mockRoms.find(r => r.id === selectedRom);
            console.log(`Carregando ROM: ${rom?.name}`);
            // Aqui seria implementada a lógica para carregar a ROM
        }
    };

    const handleOpenFile = async () => {
        if (isElectron) {
            try {
                // @ts-ignore: Ignorar verificação de tipo para API Electron
                if (window.electron && typeof window.electron.openFile === 'function') {
                    // @ts-ignore: Ignorar verificação de tipo para API Electron
                    const filePath = await window.electron.openFile();
                    if (filePath) {
                        console.log(`ROM selecionada: ${filePath}`);
                    } else {
                        console.log('Nenhuma ROM selecionada');
                    }
                } else {
                    console.error('Função openFile não encontrada na API Electron');
                }
            } catch (error) {
                console.error('Erro ao abrir seletor de arquivo:', error);
            }
        } else {
            console.log('Funcionalidade disponível apenas no app desktop');
            alert('Funcionalidade disponível apenas no app desktop');
        }
    };

    return (
        <Paper className="rom-selector">
            <Typography variant="h6" className="selector-title">
                Biblioteca de ROMs
            </Typography>

            <div className="selector-toolbar">
                <TextField
                    size="small"
                    variant="outlined"
                    placeholder="Buscar ROMs..."
                    fullWidth
                    value={searchTerm}
                    onChange={handleSearchChange}
                    InputProps={{
                        startAdornment: (
                            <InputAdornment position="start">
                                <SearchIcon />
                            </InputAdornment>
                        ),
                    }}
                    className="search-field"
                />

                <Button
                    variant="outlined"
                    color="primary"
                    startIcon={<FolderOpenIcon />}
                    onClick={handleOpenFile}
                    size="medium"
                    className="open-button"
                >
                    Abrir
                </Button>
            </div>

            <Divider className="selector-divider" />

            <List className="rom-list">
                {filteredRoms.length > 0 ? (
                    filteredRoms.map(rom => (
                        <ListItem key={rom.id} disablePadding>
                            <ListItemButton
                                selected={selectedRom === rom.id}
                                onClick={() => handleRomClick(rom.id)}
                                className="rom-item"
                            >
                                <ListItemIcon>
                                    <SportsEsportsIcon color="primary" />
                                </ListItemIcon>
                                <ListItemText
                                    primary={rom.name}
                                    secondary={rom.system}
                                />
                            </ListItemButton>
                        </ListItem>
                    ))
                ) : (
                    <Typography variant="body2" className="no-results">
                        Nenhuma ROM encontrada
                    </Typography>
                )}
            </List>

            <div className="selector-actions">
                <Button
                    variant="contained"
                    color="primary"
                    fullWidth
                    disabled={selectedRom === null}
                    onClick={handleLoadRom}
                >
                    Carregar ROM
                </Button>
            </div>
        </Paper>
    );
};

export default RomSelector;
