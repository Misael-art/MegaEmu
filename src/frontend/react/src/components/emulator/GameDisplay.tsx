import React, { useRef, useEffect } from 'react';
import './GameDisplay.css';

interface GameDisplayProps {
    onKeyDown?: (event: KeyboardEvent) => void;
    onKeyUp?: (event: KeyboardEvent) => void;
}

const GameDisplay: React.FC<GameDisplayProps> = ({ onKeyDown, onKeyUp }) => {
    const canvasRef = useRef<HTMLCanvasElement>(null);

    // Função para desenhar o conteúdo inicial do canvas
    const drawInitialContent = () => {
        const canvas = canvasRef.current;
        if (!canvas) return;

        const ctx = canvas.getContext('2d');
        if (!ctx) return;

        // Limpa o canvas
        ctx.clearRect(0, 0, canvas.width, canvas.height);

        // Desenha um background preto
        ctx.fillStyle = '#000';
        ctx.fillRect(0, 0, canvas.width, canvas.height);

        // Desenha um texto informativo
        ctx.fillStyle = '#fff';
        ctx.font = '24px Arial';
        ctx.textAlign = 'center';
        ctx.fillText('Emulador Mega Emu', canvas.width / 2, canvas.height / 2 - 20);
        ctx.font = '16px Arial';
        ctx.fillText('Carregue uma ROM para iniciar', canvas.width / 2, canvas.height / 2 + 20);

        // Desenha uma borda para verificar os limites do canvas
        ctx.strokeStyle = '#333';
        ctx.lineWidth = 2;
        ctx.strokeRect(2, 2, canvas.width - 4, canvas.height - 4);
    };

    useEffect(() => {
        // Desenha o conteúdo inicial
        drawInitialContent();

        // Redimensiona o canvas quando a janela for redimensionada
        const handleResize = () => {
            drawInitialContent();
        };

        window.addEventListener('resize', handleResize);

        return () => {
            window.removeEventListener('resize', handleResize);
        };
    }, []);

    // Adicionar event listeners para teclas
    useEffect(() => {
        if (onKeyDown) {
            window.addEventListener('keydown', onKeyDown);
        }

        if (onKeyUp) {
            window.addEventListener('keyup', onKeyUp);
        }

        // Cleanup
        return () => {
            if (onKeyDown) {
                window.removeEventListener('keydown', onKeyDown);
            }

            if (onKeyUp) {
                window.removeEventListener('keyup', onKeyUp);
            }
        };
    }, [onKeyDown, onKeyUp]);

    return (
        <div className="game-display">
            <canvas
                ref={canvasRef}
                width={640}
                height={480}
                className="game-canvas"
                tabIndex={0} // Para permitir foco e eventos de teclado
                data-testid="game-canvas"
            />
        </div>
    );
};

export default GameDisplay;
