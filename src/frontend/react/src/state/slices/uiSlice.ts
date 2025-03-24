import { createSlice, PayloadAction } from '@reduxjs/toolkit';

export type Theme = 'light' | 'dark' | 'system';
export type SidebarState = 'expanded' | 'collapsed';
export type ActiveView = 'emulator' | 'roms' | 'settings' | 'tools' | 'debug';

interface UIState {
    theme: Theme;
    sidebarState: SidebarState;
    activeView: ActiveView;
    notification: {
        show: boolean;
        message: string;
        type: 'success' | 'error' | 'info' | 'warning';
    };
    modal: {
        open: boolean;
        type: string | null;
        data: any;
    };
    fullscreen: boolean;
    controlsVisible: boolean;
    showFPS: boolean;
    activeToolTab: string | null;
}

const initialState: UIState = {
    theme: 'system',
    sidebarState: 'expanded',
    activeView: 'emulator',
    notification: {
        show: false,
        message: '',
        type: 'info',
    },
    modal: {
        open: false,
        type: null,
        data: null,
    },
    fullscreen: false,
    controlsVisible: true,
    showFPS: true,
    activeToolTab: null,
};

const uiSlice = createSlice({
    name: 'ui',
    initialState,
    reducers: {
        setTheme: (state, action: PayloadAction<Theme>) => {
            state.theme = action.payload;
        },
        toggleSidebar: (state) => {
            state.sidebarState = state.sidebarState === 'expanded' ? 'collapsed' : 'expanded';
        },
        setSidebarState: (state, action: PayloadAction<SidebarState>) => {
            state.sidebarState = action.payload;
        },
        setActiveView: (state, action: PayloadAction<ActiveView>) => {
            state.activeView = action.payload;
        },
        showNotification: (
            state,
            action: PayloadAction<{
                message: string;
                type: 'success' | 'error' | 'info' | 'warning';
            }>
        ) => {
            state.notification = {
                show: true,
                message: action.payload.message,
                type: action.payload.type,
            };
        },
        hideNotification: (state) => {
            state.notification.show = false;
        },
        openModal: (
            state,
            action: PayloadAction<{
                type: string;
                data?: any;
            }>
        ) => {
            state.modal = {
                open: true,
                type: action.payload.type,
                data: action.payload.data || null,
            };
        },
        closeModal: (state) => {
            state.modal.open = false;
        },
        setFullscreen: (state, action: PayloadAction<boolean>) => {
            state.fullscreen = action.payload;
        },
        toggleFullscreen: (state) => {
            state.fullscreen = !state.fullscreen;
        },
        setControlsVisible: (state, action: PayloadAction<boolean>) => {
            state.controlsVisible = action.payload;
        },
        toggleControlsVisible: (state) => {
            state.controlsVisible = !state.controlsVisible;
        },
        setShowFPS: (state, action: PayloadAction<boolean>) => {
            state.showFPS = action.payload;
        },
        toggleShowFPS: (state) => {
            state.showFPS = !state.showFPS;
        },
        setActiveToolTab: (state, action: PayloadAction<string | null>) => {
            state.activeToolTab = action.payload;
        },
        resetUI: () => initialState,
    },
});

export const {
    setTheme,
    toggleSidebar,
    setSidebarState,
    setActiveView,
    showNotification,
    hideNotification,
    openModal,
    closeModal,
    setFullscreen,
    toggleFullscreen,
    setControlsVisible,
    toggleControlsVisible,
    setShowFPS,
    toggleShowFPS,
    setActiveToolTab,
    resetUI,
} = uiSlice.actions;

export default uiSlice.reducer;
