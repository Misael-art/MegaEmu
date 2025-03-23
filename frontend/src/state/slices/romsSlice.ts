import { createSlice, PayloadAction, createAsyncThunk } from '@reduxjs/toolkit';
import { RomInfo, ConsoleType } from '../../types/emulator.types';
import emulatorApiService from '../../services/emulator/restApi';

interface RomsState {
    romsList: RomInfo[];
    filteredRoms: RomInfo[];
    searchQuery: string;
    selectedConsoleType: ConsoleType | 'all' | null;
    selectedRom: RomInfo | null;
    loading: boolean;
    error: string | null;
    uploadProgress: number | null;
}

const initialState: RomsState = {
    romsList: [],
    filteredRoms: [],
    searchQuery: '',
    selectedConsoleType: 'all',
    selectedRom: null,
    loading: false,
    error: null,
    uploadProgress: null,
};

// Thunks para operações assíncronas
export const fetchRoms = createAsyncThunk('roms/fetchRoms', async (_, { rejectWithValue }) => {
    try {
        return await emulatorApiService.getRomsList();
    } catch (error) {
        return rejectWithValue(error instanceof Error ? error.message : 'Erro ao buscar ROMs');
    }
});

export const searchRoms = createAsyncThunk(
    'roms/searchRoms',
    async (query: string, { rejectWithValue }) => {
        try {
            return await emulatorApiService.searchRoms(query);
        } catch (error) {
            return rejectWithValue(error instanceof Error ? error.message : 'Erro ao buscar ROMs');
        }
    }
);

export const uploadRom = createAsyncThunk(
    'roms/uploadRom',
    async (
        { file, consoleType }: { file: File; consoleType: ConsoleType },
        { rejectWithValue }
    ) => {
        try {
            return await emulatorApiService.uploadRom(file, consoleType);
        } catch (error) {
            return rejectWithValue(error instanceof Error ? error.message : 'Erro ao fazer upload da ROM');
        }
    }
);

const romsSlice = createSlice({
    name: 'roms',
    initialState,
    reducers: {
        setSearchQuery: (state, action: PayloadAction<string>) => {
            state.searchQuery = action.payload;
            state.filteredRoms = filterRoms(state.romsList, action.payload, state.selectedConsoleType);
        },
        setSelectedConsoleType: (state, action: PayloadAction<ConsoleType | 'all' | null>) => {
            state.selectedConsoleType = action.payload;
            state.filteredRoms = filterRoms(state.romsList, state.searchQuery, action.payload);
        },
        setSelectedRom: (state, action: PayloadAction<RomInfo | null>) => {
            state.selectedRom = action.payload;
        },
        addRom: (state, action: PayloadAction<RomInfo>) => {
            state.romsList.push(action.payload);
            state.filteredRoms = filterRoms(state.romsList, state.searchQuery, state.selectedConsoleType);
        },
        updateRom: (state, action: PayloadAction<RomInfo>) => {
            const index = state.romsList.findIndex((rom) => rom.id === action.payload.id);
            if (index !== -1) {
                state.romsList[index] = action.payload;
                if (state.selectedRom?.id === action.payload.id) {
                    state.selectedRom = action.payload;
                }
            }
            state.filteredRoms = filterRoms(state.romsList, state.searchQuery, state.selectedConsoleType);
        },
        removeRom: (state, action: PayloadAction<string>) => {
            state.romsList = state.romsList.filter((rom) => rom.id !== action.payload);
            if (state.selectedRom?.id === action.payload) {
                state.selectedRom = null;
            }
            state.filteredRoms = filterRoms(state.romsList, state.searchQuery, state.selectedConsoleType);
        },
        setUploadProgress: (state, action: PayloadAction<number | null>) => {
            state.uploadProgress = action.payload;
        },
        clearError: (state) => {
            state.error = null;
        },
        resetRomsState: () => initialState,
    },
    extraReducers: (builder) => {
        // Gerenciamento dos estados das chamadas assíncronas
        builder
            // fetchRoms
            .addCase(fetchRoms.pending, (state) => {
                state.loading = true;
                state.error = null;
            })
            .addCase(fetchRoms.fulfilled, (state, action) => {
                state.loading = false;
                state.romsList = action.payload;
                state.filteredRoms = filterRoms(action.payload, state.searchQuery, state.selectedConsoleType);
            })
            .addCase(fetchRoms.rejected, (state, action) => {
                state.loading = false;
                state.error = action.payload as string;
            })
            // searchRoms
            .addCase(searchRoms.pending, (state) => {
                state.loading = true;
                state.error = null;
            })
            .addCase(searchRoms.fulfilled, (state, action) => {
                state.loading = false;
                state.filteredRoms = action.payload;
            })
            .addCase(searchRoms.rejected, (state, action) => {
                state.loading = false;
                state.error = action.payload as string;
            })
            // uploadRom
            .addCase(uploadRom.pending, (state) => {
                state.loading = true;
                state.error = null;
                state.uploadProgress = 0;
            })
            .addCase(uploadRom.fulfilled, (state, action) => {
                state.loading = false;
                state.uploadProgress = null;

                if (action.payload) {
                    state.romsList.push(action.payload);
                    state.filteredRoms = filterRoms(state.romsList, state.searchQuery, state.selectedConsoleType);
                }
            })
            .addCase(uploadRom.rejected, (state, action) => {
                state.loading = false;
                state.error = action.payload as string;
                state.uploadProgress = null;
            });
    },
});

// Função auxiliar para filtrar ROMs com base na consulta e no tipo de console
function filterRoms(
    roms: RomInfo[],
    query: string,
    consoleType: ConsoleType | 'all' | null
): RomInfo[] {
    if (!roms.length) return [];

    let filtered = [...roms];

    // Filtrar por tipo de console, se especificado
    if (consoleType && consoleType !== 'all') {
        filtered = filtered.filter((rom) => rom.consoleType === consoleType);
    }

    // Filtrar por termo de pesquisa
    if (query) {
        const lowerQuery = query.toLowerCase();
        filtered = filtered.filter(
            (rom) =>
                rom.name.toLowerCase().includes(lowerQuery) ||
                rom.region.toLowerCase().includes(lowerQuery)
        );
    }

    return filtered;
}

export const {
    setSearchQuery,
    setSelectedConsoleType,
    setSelectedRom,
    addRom,
    updateRom,
    removeRom,
    setUploadProgress,
    clearError,
    resetRomsState,
} = romsSlice.actions;

export default romsSlice.reducer;
