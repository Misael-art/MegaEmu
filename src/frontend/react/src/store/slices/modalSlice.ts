import { createSlice, PayloadAction } from '@reduxjs/toolkit';

type ModalType = 'saveState' | 'loadState' | 'settings' | 'none';

interface ModalState {
    type: ModalType;
    open: boolean;
    data?: any;
}

const initialState: ModalState = {
    type: 'none',
    open: false,
    data: null
};

interface OpenModalPayload {
    type: ModalType;
    data?: any;
}

const modalSlice = createSlice({
    name: 'modal',
    initialState,
    reducers: {
        openModal: (state, action: PayloadAction<OpenModalPayload>) => {
            state.type = action.payload.type;
            state.open = true;
            state.data = action.payload.data || null;
        },
        closeModal: (state) => {
            state.open = false;
        }
    }
});

export const { openModal, closeModal } = modalSlice.actions;
export default modalSlice.reducer;
