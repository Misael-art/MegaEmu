import { createSlice, PayloadAction } from '@reduxjs/toolkit';

interface UploadRomState {
    selectedConsole: string;
    selectedFiles: File[];
    uploading: boolean;
    uploadProgress: number;
    error: string | null;
    success: boolean;
}

const initialState: UploadRomState = {
    selectedConsole: '',
    selectedFiles: [],
    uploading: false,
    uploadProgress: 0,
    error: null,
    success: false
};

const uploadRomSlice = createSlice({
    name: 'uploadRom',
    initialState,
    reducers: {
        setSelectedConsole: (state, action: PayloadAction<string>) => {
            state.selectedConsole = action.payload;
        },
        setSelectedFiles: (state, action: PayloadAction<File[]>) => {
            state.selectedFiles = action.payload;
        },
        setUploading: (state, action: PayloadAction<boolean>) => {
            state.uploading = action.payload;
        },
        setUploadProgress: (state, action: PayloadAction<number>) => {
            state.uploadProgress = action.payload;
        },
        setError: (state, action: PayloadAction<string | null>) => {
            state.error = action.payload;
        },
        setSuccess: (state, action: PayloadAction<boolean>) => {
            state.success = action.payload;
        },
        resetState: (state) => {
            state.selectedConsole = '';
            state.selectedFiles = [];
            state.uploading = false;
            state.uploadProgress = 0;
            state.error = null;
            state.success = false;
        }
    }
});

export const {
    setSelectedConsole,
    setSelectedFiles,
    setUploading,
    setUploadProgress,
    setError,
    setSuccess,
    resetState
} = uploadRomSlice.actions;

export const uploadRomReducer = uploadRomSlice.reducer;
