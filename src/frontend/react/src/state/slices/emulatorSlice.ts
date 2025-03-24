import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import { EmulatorState, FrameData, SaveState, ConsoleType } from '../../types/emulator.types';

interface EmulatorSliceState {
    state: EmulatorState;
    currentFrame: FrameData | null;
    connected: boolean;
    connecting: boolean;
    error: string | null;
}

const initialState: EmulatorSliceState = {
    state: {
        isRunning: false,
        isPaused: false,
        currentConsole: null,
        fps: 0,
        loadedRom: null,
        frameSkip: 0,
        audioEnabled: true,
        volume: 100,
        error: null,
        saveStates: [],
        rewindEnabled: false,
        rewindBufferSize: 60,
        controllerConfig: [],
    },
    currentFrame: null,
    connected: false,
    connecting: false,
    error: null,
};

const emulatorSlice = createSlice({
    name: 'emulator',
    initialState,
    reducers: {
        setEmulatorState: (state, action: PayloadAction<EmulatorState>) => {
            state.state = action.payload;
        },
        updateEmulatorState: (state, action: PayloadAction<Partial<EmulatorState>>) => {
            state.state = { ...state.state, ...action.payload };
        },
        setFrameData: (state, action: PayloadAction<FrameData>) => {
            state.currentFrame = action.payload;
        },
        setConnected: (state, action: PayloadAction<boolean>) => {
            state.connected = action.payload;
            if (action.payload) {
                state.connecting = false;
            }
        },
        setConnecting: (state, action: PayloadAction<boolean>) => {
            state.connecting = action.payload;
        },
        setError: (state, action: PayloadAction<string | null>) => {
            state.error = action.payload;
        },
        addSaveState: (state, action: PayloadAction<SaveState>) => {
            state.state.saveStates = [...state.state.saveStates, action.payload];
        },
        removeSaveState: (state, action: PayloadAction<string>) => {
            state.state.saveStates = state.state.saveStates.filter(
                (saveState) => saveState.id !== action.payload
            );
        },
        setCurrentConsole: (state, action: PayloadAction<ConsoleType | null>) => {
            state.state.currentConsole = action.payload;
        },
        setLoadedRom: (state, action: PayloadAction<string | null>) => {
            state.state.loadedRom = action.payload;
        },
        setAudioEnabled: (state, action: PayloadAction<boolean>) => {
            state.state.audioEnabled = action.payload;
        },
        setVolume: (state, action: PayloadAction<number>) => {
            state.state.volume = action.payload;
        },
        setRewindEnabled: (state, action: PayloadAction<boolean>) => {
            state.state.rewindEnabled = action.payload;
        },
        setFrameSkip: (state, action: PayloadAction<number>) => {
            state.state.frameSkip = action.payload;
        },
        resetState: () => initialState,
    },
});

export const {
    setEmulatorState,
    updateEmulatorState,
    setFrameData,
    setConnected,
    setConnecting,
    setError,
    addSaveState,
    removeSaveState,
    setCurrentConsole,
    setLoadedRom,
    setAudioEnabled,
    setVolume,
    setRewindEnabled,
    setFrameSkip,
    resetState,
} = emulatorSlice.actions;

export default emulatorSlice.reducer;
