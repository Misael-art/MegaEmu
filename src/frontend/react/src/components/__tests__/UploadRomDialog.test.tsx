import React from 'react';
import { render, screen, fireEvent, waitFor } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import { Provider } from 'react-redux';
import configureStore from 'redux-mock-store';
import UploadRomDialog from '../UploadRomDialog';
import emulatorApiService from '../../services/emulator/restApi';

// Mock do serviço de API
jest.mock('../../services/emulator/restApi', () => ({
    uploadRom: jest.fn()
}));

const mockStore = configureStore([]);

describe('UploadRomDialog', () => {
    let store;

    beforeEach(() => {
        store = mockStore({});
        store.dispatch = jest.fn();

        render(
            <Provider store={store}>
                <UploadRomDialog open={true} onClose={() => {}} />
            </Provider>
        );
    });

    it('renderiza corretamente quando aberto', () => {
        expect(screen.getByText('Upload de ROMs')).toBeInTheDocument();
        expect(screen.getByLabelText('Tipo de Console')).toBeInTheDocument();
        expect(screen.getByText('Selecionar Arquivos')).toBeInTheDocument();
    });

    it('permite selecionar um console', () => {
        const select = screen.getByLabelText('Tipo de Console');
        fireEvent.mouseDown(select);

        const option = screen.getByRole('option', { name: 'Mega Drive / Genesis' });
        fireEvent.click(option);

        expect(select).toHaveTextContent('Mega Drive / Genesis');
    });

    it('mostra extensões suportadas após selecionar console', async () => {
        const select = screen.getByLabelText('Tipo de Console');
        fireEvent.mouseDown(select);

        const option = screen.getByRole('option', { name: 'Mega Drive / Genesis' });
        fireEvent.click(option);

        expect(screen.getByText(/Extensões suportadas:/)).toBeInTheDocument();
    });

    it('permite selecionar arquivos', async () => {
        const select = screen.getByLabelText('Tipo de Console');
        fireEvent.mouseDown(select);
        fireEvent.click(screen.getByRole('option', { name: 'Mega Drive / Genesis' }));

        const file = new File(['dummy content'], 'test.md', { type: 'application/octet-stream' });
        const fileInput = screen.getByTestId('file-input');

        Object.defineProperty(fileInput, 'files', {
            value: [file]
        });

        fireEvent.change(fileInput);

        expect(screen.getByText('test.md')).toBeInTheDocument();
    });

    it('permite remover arquivos selecionados', async () => {
        const select = screen.getByLabelText('Tipo de Console');
        fireEvent.mouseDown(select);
        fireEvent.click(screen.getByRole('option', { name: 'Mega Drive / Genesis' }));

        const file = new File(['dummy content'], 'test.md', { type: 'application/octet-stream' });
        const fileInput = screen.getByTestId('file-input');

        Object.defineProperty(fileInput, 'files', {
            value: [file]
        });

        fireEvent.change(fileInput);

        const removeButton = screen.getByTestId('CancelIcon');
        fireEvent.click(removeButton);

        expect(screen.queryByText('test.md')).not.toBeInTheDocument();
    });
});
