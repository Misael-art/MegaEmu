#!/bin/bash

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}Iniciando limpeza e build do Mega Emu...${NC}"

# Limpa diretórios anteriores
echo "Limpando builds anteriores..."
rm -rf build/
rm -rf bin/
mkdir -p build
mkdir -p bin

# Verifica dependências
echo "Verificando dependências..."
DEPS=("cmake" "gcc" "g++" "make" "libsdl2-dev" "liblua5.4-dev" "libboost-all-dev")
MISSING_DEPS=()

for dep in "${DEPS[@]}"; do
    if ! dpkg -l | grep -q "^ii  $dep "; then
        MISSING_DEPS+=("$dep")
    fi
done

if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
    echo -e "${RED}Dependências faltando: ${MISSING_DEPS[*]}${NC}"
    echo "Por favor instale com: sudo apt-get install ${MISSING_DEPS[*]}"
    exit 1
fi

# Configura CMake
echo "Configurando CMake..."
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Compila
echo "Compilando..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}Erro na compilação!${NC}"
    exit 1
fi

# Copia executável e recursos
echo "Copiando arquivos..."
cp mega_emu ../bin/
cp -r ../resources ../bin/

# Executa testes
echo "Executando testes..."
./tests/mega_emu_tests

if [ $? -ne 0 ]; then
    echo -e "${RED}Alguns testes falharam!${NC}"
    exit 1
fi

echo -e "${GREEN}Build completado com sucesso!${NC}"
echo "Executável disponível em: bin/mega_emu"
