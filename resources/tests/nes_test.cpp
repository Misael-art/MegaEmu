#include <iostream>
#include <string>
#include <memory>

class NESEmulator
{
public:
    NESEmulator()
    {
        std::cout << "Iniciando emulador NES..." << std::endl;
    }

    bool initialize()
    {
        std::cout << "Inicializando NES..." << std::endl;
        return true;
    }

    bool loadROM(const std::string &path)
    {
        std::cout << "Carregando ROM: " << path << std::endl;
        // Verificar se o arquivo existe
        FILE *file = fopen(path.c_str(), "rb");
        if (!file)
        {
            std::cerr << "ERRO: Arquivo não encontrado: " << path << std::endl;
            return false;
        }

        // Obter tamanho do arquivo
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Ler alguns bytes para identificação
        unsigned char header[16];
        size_t read = fread(header, 1, sizeof(header), file);
        fclose(file);

        if (read < 16)
        {
            std::cerr << "ERRO: Arquivo muito pequeno para ser uma ROM de NES" << std::endl;
            return false;
        }

        // Verificar cabeçalho iNES
        if (header[0] != 'N' || header[1] != 'E' || header[2] != 'S' || header[3] != 0x1A)
        {
            std::cerr << "ERRO: Cabeçalho de ROM de NES inválido" << std::endl;
            return false;
        }

        std::cout << "ROM válida carregada com " << size << " bytes" << std::endl;
        std::cout << "PRG ROM: " << (int)header[4] << " x 16KB" << std::endl;
        std::cout << "CHR ROM: " << (int)header[5] << " x 8KB" << std::endl;
        std::cout << "Mapper: " << ((header[6] >> 4) | (header[7] & 0xF0)) << std::endl;

        return true;
    }

    bool runFrame()
    {
        // Simulação de execução de um frame
        static int frame_count = 0;
        frame_count++;
        if (frame_count % 10 == 0)
        {
            std::cout << "Executando frame " << frame_count << std::endl;
        }
        return true;
    }
};

int main(int argc, char *argv[])
{
    std::cout << "=== Teste de arquivo ROM de NES ===" << std::endl;

    // Verificar argumento de linha de comando
    std::string rom_path;
    if (argc > 1)
    {
        rom_path = argv[1];
    }
    else
    {
        rom_path = "D:/Steamapps/Dev/PC Engines Projects/Mega_Emu/resources/roms/nes/Super Mario Bros. (World).nes";
        std::cout << "Nenhum arquivo ROM especificado, usando padrão: " << rom_path << std::endl;
    }

    // Criar e inicializar emulador
    auto emulator = std::make_unique<NESEmulator>();
    if (!emulator->initialize())
    {
        std::cerr << "Falha ao inicializar emulador" << std::endl;
        return 1;
    }

    // Carregar ROM
    if (!emulator->loadROM(rom_path))
    {
        std::cerr << "Falha ao carregar ROM: " << rom_path << std::endl;
        return 1;
    }

    // Executar alguns frames
    std::cout << "Executando 60 frames..." << std::endl;
    for (int i = 0; i < 60; i++)
    {
        if (!emulator->runFrame())
        {
            std::cerr << "Falha ao executar frame " << i << std::endl;
            return 1;
        }
    }

    std::cout << "Teste concluído com sucesso!" << std::endl;
    return 0;
}
