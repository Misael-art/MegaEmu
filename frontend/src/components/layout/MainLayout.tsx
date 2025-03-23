import React, { ReactNode } from 'react';
import { Box, CssBaseline, createTheme, ThemeProvider } from '@mui/material';
import Header from './Header';
import Sidebar from './Sidebar';
import { useAppSelector } from '../../state/store';

interface MainLayoutProps {
    children: ReactNode;
}

const MainLayout: React.FC<MainLayoutProps> = ({ children }) => {
    const { theme: themeMode, sidebarState } = useAppSelector(state => state.ui);

    // Criar tema baseado na preferência do usuário
    const theme = React.useMemo(
        () =>
            createTheme({
                palette: {
                    mode: themeMode === 'system'
                        ? (window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light')
                        : themeMode === 'dark' ? 'dark' : 'light',
                    primary: {
                        main: '#007bff',
                    },
                    secondary: {
                        main: '#ff4081',
                    },
                },
                typography: {
                    fontFamily: [
                        '-apple-system',
                        'BlinkMacSystemFont',
                        '"Segoe UI"',
                        'Roboto',
                        '"Helvetica Neue"',
                        'Arial',
                        'sans-serif',
                    ].join(','),
                },
                components: {
                    MuiCssBaseline: {
                        styleOverrides: {
                            body: {
                                scrollbarWidth: 'thin',
                                '&::-webkit-scrollbar': {
                                    width: '8px',
                                    height: '8px',
                                },
                                '&::-webkit-scrollbar-track': {
                                    background: themeMode === 'dark' ? '#1e1e1e' : '#f5f5f5',
                                },
                                '&::-webkit-scrollbar-thumb': {
                                    background: themeMode === 'dark' ? '#555' : '#bbb',
                                    borderRadius: '4px',
                                },
                                '&::-webkit-scrollbar-thumb:hover': {
                                    background: themeMode === 'dark' ? '#777' : '#999',
                                },
                            },
                        },
                    },
                },
            }),
        [themeMode]
    );

    return (
        <ThemeProvider theme={theme}>
            <CssBaseline />
            <Box sx={{ display: 'flex', height: '100vh', overflow: 'hidden' }}>
                <Sidebar />
                <Box
                    component="main"
                    sx={{
                        flexGrow: 1,
                        display: 'flex',
                        flexDirection: 'column',
                        overflow: 'hidden',
                        ml: sidebarState === 'expanded' ? '240px' : '72px',
                        transition: theme => theme.transitions.create('margin', {
                            easing: theme.transitions.easing.sharp,
                            duration: theme.transitions.duration.enteringScreen,
                        }),
                    }}
                >
                    <Header />
                    <Box
                        sx={{
                            flexGrow: 1,
                            overflow: 'auto',
                            p: 0,
                            backgroundColor: 'background.default',
                            position: 'relative',
                        }}
                    >
                        {children}
                    </Box>
                </Box>
            </Box>
        </ThemeProvider>
    );
};

export default MainLayout;
