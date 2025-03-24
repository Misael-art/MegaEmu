import React from 'react';
import { AppBar, Toolbar, Typography, IconButton } from '@mui/material';
import MenuIcon from '@mui/icons-material/Menu';
import './Header.css';

const Header: React.FC = () => {
    return (
        <AppBar position="static" className="header">
            <Toolbar>
                <IconButton
                    size="large"
                    edge="start"
                    color="inherit"
                    aria-label="menu"
                    sx={{ mr: 2 }}
                >
                    <MenuIcon />
                </IconButton>
                <Typography variant="h6" component="div" sx={{ flexGrow: 1 }}>
                    Mega Emu
                </Typography>
                <Typography variant="caption" color="inherit">
                    v1.2.5
                </Typography>
            </Toolbar>
        </AppBar>
    );
};

export default Header;
