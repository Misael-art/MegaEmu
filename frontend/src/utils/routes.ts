import { ActiveView } from '../state/slices/uiSlice';

// Interface para definir informações de rotas
export interface RouteInfo {
    path: string;
    name: string;
    view: ActiveView;
    showInNav: boolean;
}

// Lista de rotas principais da aplicação
export const routes: RouteInfo[] = [
    {
        path: '/emulator',
        name: 'Emulador',
        view: 'emulator',
        showInNav: true,
    },
    {
        path: '/roms',
        name: 'Biblioteca de ROMs',
        view: 'roms',
        showInNav: true,
    },
    {
        path: '/settings',
        name: 'Configurações',
        view: 'settings',
        showInNav: true,
    },
    {
        path: '/tools',
        name: 'Ferramentas',
        view: 'tools',
        showInNav: true,
    },
    {
        path: '/debug',
        name: 'Debug',
        view: 'debug',
        showInNav: true,
    },
];

// Função auxiliar para obter rota com base na visualização ativa
export const getRouteByView = (view: ActiveView): RouteInfo | undefined => {
    return routes.find(route => route.view === view);
};

// Função auxiliar para obter view com base no caminho da rota
export const getViewByPath = (path: string): ActiveView | undefined => {
    const route = routes.find(route => route.path === path);
    return route?.view;
};
