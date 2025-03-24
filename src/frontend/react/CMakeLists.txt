# Frontend React/TypeScript build configuration
cmake_minimum_required(VERSION 3.20)

# Opções de build para o frontend React
option(BUILD_FRONTEND_REACT "Build React frontend" ON)
option(FRONTEND_REACT_PRODUCTION "Build React frontend in production mode" OFF)

if(BUILD_FRONTEND_REACT)
    # Verificar se Node.js está instalado
    find_program(NODE_EXECUTABLE node)
    if(NOT NODE_EXECUTABLE)
        message(FATAL_ERROR "Node.js não encontrado. Por favor, instale Node.js para compilar o frontend React.")
    endif()

    # Definir diretórios
    set(FRONTEND_REACT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    set(FRONTEND_REACT_BUILD_DIR ${CMAKE_BINARY_DIR}/frontend/react)

    # Comandos de build
    if(FRONTEND_REACT_PRODUCTION)
        # Build de produção
        add_custom_target(frontend_react ALL
            COMMAND ${CMAKE_COMMAND} -E env
                NODE_ENV=production
                ${NODE_EXECUTABLE} npm ci
            COMMAND ${CMAKE_COMMAND} -E env
                NODE_ENV=production
                ${NODE_EXECUTABLE} npm run build
            WORKING_DIRECTORY ${FRONTEND_REACT_SOURCE_DIR}
            COMMENT "Building React frontend (production mode)"
        )
    else()
        # Build de desenvolvimento
        add_custom_target(frontend_react ALL
            COMMAND ${NODE_EXECUTABLE} npm install
            COMMAND ${NODE_EXECUTABLE} npm run build
            WORKING_DIRECTORY ${FRONTEND_REACT_SOURCE_DIR}
            COMMENT "Building React frontend (development mode)"
        )
    endif()

    # Copiar build para diretório de distribuição
    add_custom_command(TARGET frontend_react POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${FRONTEND_REACT_SOURCE_DIR}/build
            ${CMAKE_BINARY_DIR}/dist/frontend/react
        COMMENT "Copying React frontend build to distribution directory"
    )

    # Adicionar script de desenvolvimento
    add_custom_target(frontend_react_dev
        COMMAND ${NODE_EXECUTABLE} npm start
        WORKING_DIRECTORY ${FRONTEND_REACT_SOURCE_DIR}
        COMMENT "Starting React frontend development server"
    )

    # Adicionar script de testes
    add_custom_target(frontend_react_test
        COMMAND ${NODE_EXECUTABLE} npm test
        WORKING_DIRECTORY ${FRONTEND_REACT_SOURCE_DIR}
        COMMENT "Running React frontend tests"
    )

    # Adicionar script de lint
    add_custom_target(frontend_react_lint
        COMMAND ${NODE_EXECUTABLE} npm run lint
        WORKING_DIRECTORY ${FRONTEND_REACT_SOURCE_DIR}
        COMMENT "Running React frontend linter"
    )
endif()
