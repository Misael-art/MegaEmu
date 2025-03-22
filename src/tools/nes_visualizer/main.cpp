/** * @file main.cpp * @brief Ponto de entrada para o visualizador de NES * @author Mega_Emu Team * @version 1.0.0 * @date 2024-03-13 */#include "nes_visualizer.hpp"#include <iostream>#include <string>#include <cstdlib>#include <filesystem>namespace fs = std::filesystem;void printUsage(const char *programName){    std::cout << "Uso: " << programName << " <arquivo_rom>" << std::endl;    std::cout << std::endl;    std::cout << "Opções:" << std::endl;    std::cout << "  --help, -h       Exibe esta ajuda" << std::endl;    std::cout << "  --scale=N, -s N  Define o fator de escala (padrão: 3)" << std::endl;    std::cout << std::endl;    std::cout << "Controles:" << std::endl;    std::cout << "  Setas direcionais / WASD: Direcional" << std::endl;    std::cout << "  Z/K: Botão B" << std::endl;    std::cout << "  X/J: Botão A" << std::endl;    std::cout << "  Enter: Start" << std::endl;    std::cout << "  Right Shift: Select" << std::endl;    std::cout << "  F12: Tirar screenshot" << std::endl;    std::cout << "  ESC: Sair" << std::endl;}int main(int argc, char *argv[]){    std::string romFilename;    int scale = 3;    // Processar argumentos de linha de comando    if (argc < 2)    {        std::cerr << "Erro: É necessário especificar um arquivo ROM." << std::endl;        printUsage(argv[0]);        return 1;    }    for (int i = 1; i < argc; ++i)    {        std::string arg = argv[i];        if (arg == "--help" || arg == "-h")        {            printUsage(argv[0]);            return 0;        }        else if (arg.find("--scale=") == 0)        {            try            {                scale = std::stoi(arg.substr(8));            }            catch (const std::exception &e)            {                std::cerr << "Erro ao processar escala: " << e.what() << std::endl;                return 1;            }        }        else if (arg == "-s" && i + 1 < argc)        {            try            {                scale = std::stoi(argv[++i]);            }            catch (const std::exception &e)            {                std::cerr << "Erro ao processar escala: " << e.what() << std::endl;                return 1;            }        }        else if (romFilename.empty())        {            romFilename = arg;        }    }    // Validar ROM    if (!fs::exists(romFilename))    {        std::cerr << "Erro: O arquivo ROM '" << romFilename << "' não existe." << std::endl;        return 1;    }    // Validar escala    if (scale < 1 || scale > 10)    {        std::cerr << "Erro: Escala inválida. Use um valor entre 1 e 10." << std::endl;        return 1;    }    try    {        // Inicializar visualizador        std::cout << "Iniciando visualizador NES..." << std::endl;        MegaEmu::Tools::NESVisualizer visualizer;        if (!visualizer.initialize("Mega_Emu - NES Visualizer", scale))        {            std::cerr << "Erro ao inicializar visualizador" << std::endl;            return 1;        }        // Carregar ROM        std::cout << "Carregando ROM: " << romFilename << std::endl;        if (!visualizer.loadROM(romFilename))        {            std::cerr << "Erro ao carregar ROM" << std::endl;            return 1;        }        // Executar visualizador        visualizer.run();        return 0;    }    catch (const std::exception &e)    {        std::cerr << "Erro fatal: " << e.what() << std::endl;        return 1;    }}