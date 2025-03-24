import React from 'react';
import { useNavigate } from 'react-router-dom';
import {
    Drawer,
    List,
    ListItem,
    ListItemButton,
    ListItemIcon,
    ListItemText,
    Divider,
    Box,
    Typography,
    useTheme,
} from '@mui/material';
import GamesIcon from '@mui/icons-material/Games';
import VideogameAssetIcon from '@mui/icons-material/VideogameAsset';
import SettingsIcon from '@mui/icons-material/Settings';
import SaveIcon from '@mui/icons-material/Save';
import BuildIcon from '@mui/icons-material/Build';
import BugReportIcon from '@mui/icons-material/BugReport';
import { useAppSelector, useAppDispatch } from '../../state/store';
import { setActiveView, ActiveView } from '../../state/slices/uiSlice';
import { routes, getRouteByView } from '../../utils/routes';

const Sidebar: React.FC = () => {
    const dispatch = useAppDispatch();
    const navigate = useNavigate();
    const theme = useTheme();
    const { sidebarState, activeView } = useAppSelector((state) => state.ui);

    const navItems = routes.filter(route => route.showInNav && ['emulator', 'roms', 'settings'].includes(route.view));
    const toolsItems = routes.filter(route => route.showInNav && ['tools', 'debug'].includes(route.view));

    const handleNavClick = (view: ActiveView) => {
        dispatch(setActiveView(view));
        const route = getRouteByView(view);
        if (route) {
            navigate(route.path);
        }
    };

    return (
        <Drawer
            variant="permanent"
            open={sidebarState === 'expanded'}
            sx={{
                width: sidebarState === 'expanded' ? 240 : 72,
                flexShrink: 0,
                '& .MuiDrawer-paper': {
                    width: sidebarState === 'expanded' ? 240 : 72,
                    boxSizing: 'border-box',
                    backgroundColor: theme.palette.background.paper,
                    transition: theme.transitions.create('width', {
                        easing: theme.transitions.easing.sharp,
                        duration: theme.transitions.duration.enteringScreen,
                    }),
                    overflowX: 'hidden',
                },
            }}
        >
            <Box
                sx={{
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: sidebarState === 'expanded' ? 'flex-start' : 'center',
                    padding: theme.spacing(2),
                    height: 64,
                }}
            >
                {sidebarState === 'expanded' ? (
                    <Typography variant="h6" noWrap component="div">
                        Mega Emu
                    </Typography>
                ) : (
                    <Box
                        component="img"
                        sx={{
                            height: 32,
                            width: 32,
                        }}
                        alt="Logo"
                        src="/logo192.png"
                    />
                )}
            </Box>
            <Divider />
            <List>
                {navItems.map((item) => (
                    <ListItem key={item.view} disablePadding>
                        <ListItemButton
                            selected={activeView === item.view}
                            onClick={() => handleNavClick(item.view as ActiveView)}
                            sx={{
                                minHeight: 48,
                                justifyContent: sidebarState === 'expanded' ? 'initial' : 'center',
                                px: 2.5,
                            }}
                        >
                            <ListItemIcon
                                sx={{
                                    minWidth: 0,
                                    mr: sidebarState === 'expanded' ? 3 : 'auto',
                                    justifyContent: 'center',
                                }}
                            >
                                {getIconForView(item.view as ActiveView)}
                            </ListItemIcon>
                            {sidebarState === 'expanded' && <ListItemText primary={item.name} />}
                        </ListItemButton>
                    </ListItem>
                ))}
            </List>
            <Divider />
            <List>
                {toolsItems.map((item) => (
                    <ListItem key={item.view} disablePadding>
                        <ListItemButton
                            selected={activeView === item.view}
                            onClick={() => handleNavClick(item.view as ActiveView)}
                            sx={{
                                minHeight: 48,
                                justifyContent: sidebarState === 'expanded' ? 'initial' : 'center',
                                px: 2.5,
                            }}
                        >
                            <ListItemIcon
                                sx={{
                                    minWidth: 0,
                                    mr: sidebarState === 'expanded' ? 3 : 'auto',
                                    justifyContent: 'center',
                                }}
                            >
                                {getIconForView(item.view as ActiveView)}
                            </ListItemIcon>
                            {sidebarState === 'expanded' && <ListItemText primary={item.name} />}
                        </ListItemButton>
                    </ListItem>
                ))}
            </List>

            {/* Área na parte inferior da barra lateral */}
            <Box sx={{ flexGrow: 1 }} />
            <Divider />
            <Box
                sx={{
                    p: 2,
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: sidebarState === 'expanded' ? 'space-between' : 'center',
                }}
            >
                <ListItemIcon
                    sx={{
                        minWidth: 0,
                        mr: sidebarState === 'expanded' ? 3 : 'auto',
                        justifyContent: 'center',
                    }}
                >
                    <SaveIcon />
                </ListItemIcon>
                {sidebarState === 'expanded' && <Typography variant="body2">Estados Salvos</Typography>}
            </Box>
        </Drawer>
    );
};

// Função auxiliar para obter ícone com base na visualização
function getIconForView(view: ActiveView) {
    switch (view) {
        case 'emulator':
            return <GamesIcon />;
        case 'roms':
            return <VideogameAssetIcon />;
        case 'settings':
            return <SettingsIcon />;
        case 'tools':
            return <BuildIcon />;
        case 'debug':
            return <BugReportIcon />;
        default:
            return <GamesIcon />;
    }
}

export default Sidebar;
