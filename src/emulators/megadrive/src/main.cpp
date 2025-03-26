#include "mega_tools.h"
#include <iostream>

int main() {
  std::cout << "Mega Drive Emulator" << std::endl;
  mega_tools::initialize();

  std::cout << "\nPressione ENTER para sair..." << std::endl;
  std::cin.get();

  return 0;
}
