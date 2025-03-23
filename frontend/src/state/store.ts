import { configureStore } from '@reduxjs/toolkit';
import { useDispatch, useSelector, TypedUseSelectorHook } from 'react-redux';
import emulatorReducer from './slices/emulatorSlice';
import uiReducer from './slices/uiSlice';
import romsReducer from './slices/romsSlice';

export const store = configureStore({
    reducer: {
        emulator: emulatorReducer,
        ui: uiReducer,
        roms: romsReducer,
    },
    middleware: (getDefaultMiddleware) =>
        getDefaultMiddleware({
            serializableCheck: {
                // Ignorar algumas ações não serializáveis que podem conter File, ArrayBuffer, etc.
                ignoredActions: ['roms/uploadRomStart', 'roms/uploadRomSuccess', 'emulator/setFrameData'],
                // Ignorar alguns caminhos de estado que podem conter dados não serializáveis
                ignoredPaths: ['emulator.currentFrame.imageData'],
            },
        }),
});

// Tipos para hooks Redux tipados
export type RootState = ReturnType<typeof store.getState>;
export type AppDispatch = typeof store.dispatch;

// Hooks personalizados e tipados para usar no lugar de useDispatch e useSelector
export const useAppDispatch = () => useDispatch<AppDispatch>();
export const useAppSelector: TypedUseSelectorHook<RootState> = useSelector;
