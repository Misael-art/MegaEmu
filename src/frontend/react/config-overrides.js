// Arquivo de configuração para sobrescrever o webpack do Create React App
const webpack = require('webpack');

module.exports = function override(config, env) {
    // Adiciona fallbacks para os módulos do Node.js
    config.resolve.fallback = {
        ...config.resolve.fallback,
        "fs": false,
        "path": require.resolve("path-browserify"),
        "crypto": require.resolve("crypto-browserify"),
        "stream": require.resolve("stream-browserify"),
        "buffer": require.resolve("buffer/"),
        "process": require.resolve("process/browser"),
    };

    // Adiciona polyfills necessários
    config.plugins = [
        ...config.plugins,
        new webpack.ProvidePlugin({
            Buffer: ['buffer', 'Buffer'],
            process: "process/browser",
        }),
    ];

    return config;
};
