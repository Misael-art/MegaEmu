import React, { useState, useRef } from 'react';
import {
    Box,
    Button,
    Dialog,
    DialogActions,
    DialogContent,
    DialogTitle,
    FormControl,
    InputLabel,
    MenuItem,
    Select,
    SelectChangeEvent,
    Typography,
    LinearProgress,
    Alert,
    Chip,
    Grid
} from '@mui/material';
import CloudUploadIcon from '@mui/icons-material/CloudUpload';
import CheckCircleIcon from '@mui/icons-material/CheckCircle';
import CancelIcon from '@mui/icons-material/Cancel';
import { ConsoleType, consoleDisplayNames, consoleFileExtensions } from '../types/emulator.types';
import emulatorApiService from '../services/emulator/restApi';
import { useAppDispatch } from '../state/store';
import { fetchRoms } from '../state/slices/romsSlice';

interface UploadRomDialogProps {
    open: boolean;
    onClose: () => void;
}

const UploadRomDialog: React.FC<UploadRomDialogProps> = ({ open, onClose }) => {
    const dispatch = useAppDispatch();
    const [selectedConsole, setSelectedConsole] = useState<ConsoleType | ''>('');
    const [selectedFiles, setSelectedFiles] = useState<File[]>([]);
    const [uploading, setUploading] = useState(false);
    const [uploadProgress, setUploadProgress] = useState(0);
    const [error, setError] = useState<string | null>(null);
    const [success, setSuccess] = useState(false);
    const fileInputRef = useRef<HTMLInputElement>(null);

    const handleConsoleChange = (event: SelectChangeEvent<ConsoleType | ''>) => {
        setSelectedConsole(event.target.value as ConsoleType | '');
    };

    const handleFileSelect = (event: React.ChangeEvent<HTMLInputElement>) => {
        if (event.target.files && event.target.files.length > 0) {
            const files = Array.from(event.target.files);
            setSelectedFiles(files);
            setError(null);
        }
    };

    const handleUpload = async () => {
        if (!selectedConsole) {
            setError('Por favor, selecione um console');
            return;
        }

        if (selectedFiles.length === 0) {
            setError('Por favor, selecione pelo menos um arquivo');
            return;
        }

        setUploading(true);
        setUploadProgress(0);
        setError(null);

        try {
            // Upload de cada arquivo selecionado
            for (let i = 0; i < selectedFiles.length; i++) {
                const file = selectedFiles[i];

                // Calcula o progresso de upload com base no número de arquivos
                const progressPerFile = 100 / selectedFiles.length;

                // Simula o progresso de upload para o arquivo atual
                for (let p = 0; p < 100; p += 10) {
                    setUploadProgress(Math.min(100, (i * progressPerFile) + (p * progressPerFile / 100)));
                    await new Promise(resolve => setTimeout(resolve, 50));
                }

                // Faz o upload real do arquivo
                await emulatorApiService.uploadRom(file, selectedConsole);

                // Marca o progresso completo para este arquivo
                setUploadProgress((i + 1) * progressPerFile);
            }

            // Upload completo com sucesso
            setUploadProgress(100);
            setSuccess(true);

            // Recarrega a lista de ROMs para refletir os novos uploads
            dispatch(fetchRoms());

            // Reseta após 2 segundos
            setTimeout(() => {
                setSelectedFiles([]);
                setSuccess(false);
                setUploadProgress(0);
                setUploading(false);
                // Mantém o console selecionado para uploads adicionais
            }, 2000);
        } catch (err) {
            setError('Erro ao fazer upload: ' + (err instanceof Error ? err.message : String(err)));
            setUploading(false);
        }
    };

    const resetForm = () => {
        setSelectedConsole('');
        setSelectedFiles([]);
        setUploading(false);
        setUploadProgress(0);
        setError(null);
        setSuccess(false);

        // Reseta o input de arquivo
        if (fileInputRef.current) {
            fileInputRef.current.value = '';
        }
    };

    const handleClose = () => {
        if (!uploading) {
            resetForm();
            onClose();
        }
    };

    // Gera a lista de extensões suportadas para o console selecionado
    const getSupportedExtensions = () => {
        if (!selectedConsole) return '';

        return consoleFileExtensions[selectedConsole].join(', ');
    };

    return (
        <Dialog open={open} onClose={handleClose} maxWidth="sm" fullWidth>
            <DialogTitle>
                <Box sx={{ display: 'flex', alignItems: 'center' }}>
                    <CloudUploadIcon sx={{ mr: 1 }} />
                    <Typography variant="h6">Upload de ROMs</Typography>
                </Box>
            </DialogTitle>
            <DialogContent dividers>
                {error && (
                    <Alert severity="error" sx={{ mb: 2 }} role="alert">
                        {error}
                    </Alert>
                )}

                {success && (
                    <Alert icon={<CheckCircleIcon />} severity="success" sx={{ mb: 2 }} role="alert">
                        Upload concluído com sucesso!
                    </Alert>
                )}

                <FormControl fullWidth sx={{ mb: 2 }}>
                    <InputLabel id="console-select-label">Tipo de Console</InputLabel>
                    <Select
                        labelId="console-select-label"
                        value={selectedConsole}
                        label="Tipo de Console"
                        onChange={handleConsoleChange}
                        disabled={uploading}
                    >
                        {Object.entries(consoleDisplayNames).map(([value, label]) => (
                            <MenuItem key={value} value={value}>
                                {label}
                            </MenuItem>
                        ))}
                    </Select>
                </FormControl>

                {selectedConsole && (
                    <Typography variant="body2" color="text.secondary" sx={{ mb: 2 }}>
                        Extensões suportadas: {getSupportedExtensions()}
                    </Typography>
                )}

                <Box
                    sx={{
                        border: '2px dashed #ccc',
                        borderRadius: 2,
                        p: 3,
                        textAlign: 'center',
                        mb: 2,
                        bgcolor: 'background.paper'
                    }}
                >
                    <input
                        type="file"
                        multiple
                        onChange={handleFileSelect}
                        style={{ display: 'none' }}
                        ref={fileInputRef}
                        disabled={uploading || !selectedConsole}
                        accept={selectedConsole ? consoleFileExtensions[selectedConsole].join(',') : undefined}
                        aria-label="Selecionar Arquivos"
                        data-testid="file-input"
                    />

                    <Button
                        variant="outlined"
                        onClick={() => fileInputRef.current?.click()}
                        startIcon={<CloudUploadIcon />}
                        disabled={uploading || !selectedConsole}
                        sx={{ mb: 2 }}
                        aria-label="Selecionar Arquivos"
                    >
                        Selecionar Arquivos
                    </Button>

                    <Typography variant="body2" color="text.secondary">
                        {selectedFiles.length > 0
                            ? `${selectedFiles.length} arquivo(s) selecionado(s)`
                            : 'Arraste arquivos aqui ou clique para selecionar'}
                    </Typography>
                </Box>

                {selectedFiles.length > 0 && (
                    <Box sx={{ mb: 2 }}>
                        <Typography variant="subtitle2" gutterBottom>
                            Arquivos selecionados:
                        </Typography>
                        <Grid container spacing={1}>
                            {selectedFiles.map((file, index) => (
                                <Grid item key={index}>
                                    <Chip
                                        label={file.name}
                                        onDelete={uploading ? undefined : () => {
                                            const newFiles = selectedFiles.filter((_, i) => i !== index);
                                            setSelectedFiles(newFiles);
                                        }}
                                        deleteIcon={<CancelIcon data-testid="CancelIcon" />}
                                    />
                                </Grid>
                            ))}
                        </Grid>
                    </Box>
                )}

                {uploading && (
                    <Box sx={{ mb: 2 }}>
                        <Typography variant="body2" color="text.secondary" gutterBottom>
                            Progresso: {Math.round(uploadProgress)}%
                        </Typography>
                        <LinearProgress
                            variant="determinate"
                            value={uploadProgress}
                            sx={{ height: 10, borderRadius: 5 }}
                        />
                    </Box>
                )}
            </DialogContent>
            <DialogActions>
                <Button
                    onClick={handleClose}
                    disabled={uploading}
                >
                    {success ? 'Fechar' : 'Cancelar'}
                </Button>
                <Button
                    variant="contained"
                    color="primary"
                    onClick={handleUpload}
                    disabled={uploading || selectedFiles.length === 0 || !selectedConsole}
                    startIcon={<CloudUploadIcon />}
                >
                    Fazer Upload
                </Button>
            </DialogActions>
        </Dialog>
    );
};

export default UploadRomDialog;
