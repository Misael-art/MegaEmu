import React, { useState, useEffect } from 'react';
import { Provider } from 'react-redux';
import { store } from './state/store';
import './App.css';
import { CssBaseline, Container, Grid, Box, ThemeProvider, createTheme } from '@mui/material';
import GameDisplay from './components/emulator/GameDisplay';
import ControlPanel from './components/emulator/ControlPanel';
import RomSelector from './components/emulator/RomSelector';
import Header from './components/layout/Header';

const darkTheme = createTheme({
  palette: {
    mode: 'dark',
    primary: {
      main: '#90caf9',
    },
    secondary: {
      main: '#f48fb1',
    },
    background: {
      default: '#121212',
      paper: '#1e1e1e',
    },
  },
});

const App: React.FC = () => {
  const [isElectron, setIsElectron] = useState(false);
  const [loaded, setLoaded] = useState(false);

  useEffect(() => {
    console.log('Aplicativo MegaEmu inicializado');

    // Verifica se está no ambiente Electron
    if (typeof window.electron !== 'undefined') {
      console.log('Electron detectado', window.electron);
      setIsElectron(true);
      // Garante que a página ocupe todo o espaço no Electron
      document.documentElement.classList.add('electron-mode');
    } else {
      console.log('Electron não detectado, rodando como aplicação web');
    }

    // Força um reflow/repaint para garantir que os estilos sejam aplicados
    setTimeout(() => {
      setLoaded(true);
    }, 100);
  }, []);

  // Tela de carregamento com mensagem visual
  if (!loaded) {
    return (
      <div className="loading-screen">
        <h1>Carregando Mega Emu...</h1>
        <div className="loading-spinner"></div>
      </div>
    );
  }

  return (
    <Provider store={store}>
      <ThemeProvider theme={darkTheme}>
        <CssBaseline />
        <div className={`app ${isElectron ? 'electron-mode' : ''}`}>
          <Header />
          <Container maxWidth="lg" className="main-container">
            <Box sx={{
              flexGrow: 1,
              display: 'flex',
              flexDirection: 'column',
              height: 'calc(100vh - 100px)'  // Ajusta para a altura do cabeçalho e rodapé
            }}>
              <Grid container spacing={2} style={{ flex: 1 }}>
                <Grid item xs={12} md={9} style={{ display: 'flex', flexDirection: 'column' }}>
                  <div style={{ flex: 1, marginBottom: '16px' }}>
                    <GameDisplay />
                  </div>
                  <ControlPanel />
                </Grid>
                <Grid item xs={12} md={3}>
                  <RomSelector />
                </Grid>
              </Grid>
            </Box>
          </Container>
          <div className="app-status">
            {isElectron ? 'Modo Desktop (Electron)' : 'Modo Navegador'}
          </div>
        </div>
      </ThemeProvider>
    </Provider>
  );
};

export default App;
