/**
 * Agrupamento dos serviços relacionados ao emulador
 * Este módulo funciona tanto em ambiente desktop (Electron) quanto no navegador
 */

import { getEnvironment, envLogger } from '../../utils/environment';
import romService from './romService';
import emulatorApiService from './restApi';

// Informações sobre o ambiente atual
const environment = {
    type: getEnvironment(),
    isDesktop: getEnvironment() === 'desktop',
    isBrowser: getEnvironment() === 'browser'
};

// Exporta todos os serviços e utilitários relacionados ao emulador
export {
    romService,
    emulatorApiService,
    environment,
    getEnvironment,
    envLogger
};

// Exporta também um objeto unificado para facilitar importações
export default {
    rom: romService,
    api: emulatorApiService,
    environment,
    utils: {
        getEnvironment,
        logger: envLogger
    }
};
