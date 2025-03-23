import { Socket, io } from 'socket.io-client';
import { EmulatorCommand, EmulatorResponse } from '../../types/emulator.types';

type MessageHandler = (response: EmulatorResponse) => void;
type ConnectionStatusHandler = (status: boolean) => void;

class EmulatorWebSocket {
    private socket: Socket | null = null;
    private isConnected: boolean = false;
    private messageHandlers: Map<string, MessageHandler[]> = new Map();
    private connectionStatusHandlers: ConnectionStatusHandler[] = [];
    private reconnectTimer: NodeJS.Timeout | null = null;
    private reconnectAttempts: number = 0;
    private MAX_RECONNECT_ATTEMPTS = 5;
    private RECONNECT_DELAY = 2000; // 2 segundos

    constructor(private url: string = 'ws://localhost:8080') { }

    connect(): Promise<boolean> {
        return new Promise((resolve, reject) => {
            if (this.socket) {
                this.socket.close();
            }

            this.socket = io(this.url, {
                reconnection: false, // Vamos gerenciar reconexão manualmente
                timeout: 10000, // 10 segundos
            });

            this.socket.on('connect', () => {
                this.isConnected = true;
                this.reconnectAttempts = 0;
                this.notifyConnectionStatusHandlers(true);
                resolve(true);
            });

            this.socket.on('disconnect', () => {
                this.isConnected = false;
                this.notifyConnectionStatusHandlers(false);
                this.attemptReconnect();
            });

            this.socket.on('error', (error) => {
                console.error('Socket error:', error);
                reject(error);
            });

            this.socket.on('message', (message: string) => {
                try {
                    const response = JSON.parse(message) as EmulatorResponse;
                    this.handleMessage(response);
                } catch (error) {
                    console.error('Error parsing message:', error);
                }
            });
        });
    }

    disconnect(): void {
        if (this.socket) {
            this.socket.close();
            this.socket = null;
            this.isConnected = false;
            this.notifyConnectionStatusHandlers(false);
        }

        if (this.reconnectTimer) {
            clearTimeout(this.reconnectTimer);
            this.reconnectTimer = null;
        }
    }

    sendCommand(command: EmulatorCommand): void {
        if (!this.isConnected || !this.socket) {
            console.warn('Cannot send command: WebSocket is not connected');
            return;
        }

        this.socket.emit('command', JSON.stringify(command));
    }

    onMessage(type: string, handler: MessageHandler): () => void {
        if (!this.messageHandlers.has(type)) {
            this.messageHandlers.set(type, []);
        }

        const handlers = this.messageHandlers.get(type)!;
        handlers.push(handler);

        // Retorna uma função para remover o handler
        return () => {
            const index = handlers.indexOf(handler);
            if (index !== -1) {
                handlers.splice(index, 1);
            }
        };
    }

    onConnectionStatus(handler: ConnectionStatusHandler): () => void {
        this.connectionStatusHandlers.push(handler);

        // Retorna uma função para remover o handler
        return () => {
            const index = this.connectionStatusHandlers.indexOf(handler);
            if (index !== -1) {
                this.connectionStatusHandlers.splice(index, 1);
            }
        };
    }

    private handleMessage(response: EmulatorResponse): void {
        const { type } = response;

        if (this.messageHandlers.has(type)) {
            const handlers = this.messageHandlers.get(type)!;
            handlers.forEach(handler => handler(response));
        }

        // Handlers para todos os tipos de mensagens
        if (this.messageHandlers.has('*')) {
            const handlers = this.messageHandlers.get('*')!;
            handlers.forEach(handler => handler(response));
        }
    }

    private notifyConnectionStatusHandlers(status: boolean): void {
        this.connectionStatusHandlers.forEach(handler => handler(status));
    }

    private attemptReconnect(): void {
        if (this.reconnectTimer || this.reconnectAttempts >= this.MAX_RECONNECT_ATTEMPTS) {
            return;
        }

        this.reconnectAttempts++;

        this.reconnectTimer = setTimeout(() => {
            this.reconnectTimer = null;
            console.log(`Attempting to reconnect (${this.reconnectAttempts}/${this.MAX_RECONNECT_ATTEMPTS})...`);

            this.connect().catch(() => {
                if (this.reconnectAttempts < this.MAX_RECONNECT_ATTEMPTS) {
                    this.attemptReconnect();
                } else {
                    console.error('Max reconnect attempts reached');
                }
            });
        }, this.RECONNECT_DELAY);
    }
}

// Instância singleton
export const emulatorWebSocket = new EmulatorWebSocket();
export default emulatorWebSocket;
