#!/bin/bash

# Array com as dependências a serem instaladas
dependencies=(
  ts-jest
  identity-obj-proxy
  @types/jest
  jest-canvas-mock
  redux-mock-store
  @types/redux-mock-store
  @types/node # Definições de tipo para Node.js
  @testing-library/react # Para testes de componentes React
  @testing-library/jest-dom # Assertivas DOM para testes React
  @types/testing-library__jest-dom #definições de tipo para testing-library
)

# Loop para instalar cada dependência
for dependency in "${dependencies[@]}"; do
  echo "Instalando $dependency..."
  npm install --save-dev --legacy-peer-deps "$dependency"
  if [ $? -ne 0 ]; then
    echo "Falha ao instalar $dependency."
    exit 1
  fi
  echo "$dependency instalado com sucesso."
done

echo "Todas as dependências foram instaladas com sucesso."


# Abra o PowerShell como administrador.
# Execute o comando wsl --install. Isso instalará o WSL e uma distribuição Linux padrão (geralmente Ubuntu).
# Após a instalação, siga as instruções na tela para configurar sua distribuição Linux.
# Executando o Script:
# Salve o script (install_dev_deps.sh) em um local acessível no sistema de arquivos do WSL. Você pode colocar o arquivo em seu diretório inicial do Linux (por exemplo, /home/seu_usuario/).
# Abra o terminal do WSL (você pode encontrá-lo no menu Iniciar).
# Navegue até o diretório onde você salvou o script usando o comando cd. Por exemplo, cd /home/seu_usuario/.
# Torne o script executável com o comando chmod +x install_dev_deps.sh.
# Execute o script com o comando ./install_dev_deps.sh.

# 2. Usando o Git Bash:

# Se você tiver o Git para Windows instalado, ele vem com o Git Bash, que é um ambiente Bash que você pode usar para executar scripts Bash.
# Instalação do Git para Windows:
# Baixe o Git para Windows no site oficial do Git.
# Instale o Git para Windows, seguindo as opções padrões de instalação são suficientes para a maior parte dos casos.
# Executando o Script:
# Salve o script (install_dev_deps.sh) em um local acessível.
# Abra o Git Bash (você pode encontrá-lo no menu Iniciar).
# Navegue até o diretório onde você salvou o script usando o comando cd.
# Torne o script executável com o comando chmod +x install_dev_deps.sh.
# Execute o script com o comando ./install_dev_deps.sh.

# Execute o comando chmod +x install_dev_deps.sh para tornar o script executável.
# Execute o script digitando ./install_dev_deps.sh e pressione Enter.
# Dependências Globais:
# Caso deseje instalar alguma dessas dependências globalmente, remova a flag --save-dev.
# yarn:
# Caso seu projeto utilize yarn, troque os comandos npm install por yarn add -D.

# Recomendação:

# O WSL é geralmente a opção mais completa e recomendada para executar scripts Bash no Windows 11, pois fornece um ambiente Linux completo.