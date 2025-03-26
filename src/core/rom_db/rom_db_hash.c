/**
 * @file rom_db_hash.c
 * @brief Implementação das funções de hash para o banco de dados de ROMs
 */

#include "../global_defines.h"
#include "../logging/log.h"
#include "rom_db.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Bibliotecas para cálculo de hashes
#include "../../utils/crc32.h"
#include "../../utils/md5.h"
#include "../../utils/sha1.h"

#define ROM_DB_HASH_BUFFER_SIZE 8192

/**
 * @brief Calcula os hashes de um arquivo de ROM
 *
 * @param file_path Caminho do arquivo de ROM
 * @param hash Estrutura que receberá os hashes calculados
 * @param callback Função de callback para notificar progresso (opcional)
 * @param user_data Dados do usuário para o callback
 * @return true Se os hashes foram calculados com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_calculate_hash(
    const char *file_path, mega_emu_rom_db_hash_t *hash,
    mega_emu_rom_db_progress_callback_t callback, void *user_data) {

  if (file_path == NULL || hash == NULL) {
    return false;
  }

  // Abrir o arquivo
  FILE *file = fopen(file_path, "rb");
  if (file == NULL) {
    MEGA_LOG_ERROR(
        "ROM Database: Falha ao abrir arquivo para calcular hash: %s",
        file_path);
    return false;
  }

  // Obter o tamanho do arquivo
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (file_size <= 0) {
    MEGA_LOG_ERROR("ROM Database: Arquivo vazio ou inválido: %s", file_path);
    fclose(file);
    return false;
  }

  // Inicializar os contextos de hash
  MD5_CTX md5_ctx;
  SHA1_CTX sha1_ctx;
  uint32_t crc32_value = 0;

  MD5_Init(&md5_ctx);
  SHA1_Init(&sha1_ctx);

  // Buffer para leitura do arquivo
  unsigned char buffer[ROM_DB_HASH_BUFFER_SIZE];
  size_t bytes_read;
  long total_read = 0;

  // Ler o arquivo em blocos e atualizar os hashes
  while ((bytes_read = fread(buffer, 1, ROM_DB_HASH_BUFFER_SIZE, file)) > 0) {
    MD5_Update(&md5_ctx, buffer, bytes_read);
    SHA1_Update(&sha1_ctx, buffer, bytes_read);
    crc32_value = mega_emu_crc32_update(crc32_value, buffer, bytes_read);

    // Atualizar progresso
    total_read += bytes_read;
    if (callback != NULL) {
      callback((uint32_t)total_read, (uint32_t)file_size, user_data);
    }
  }

  // Finalizar os hashes
  MD5_Final(hash->md5, &md5_ctx);
  SHA1_Final(hash->sha1, &sha1_ctx);

  // Converter o CRC32 para bytes
  hash->crc32[0] = (crc32_value >> 24) & 0xFF;
  hash->crc32[1] = (crc32_value >> 16) & 0xFF;
  hash->crc32[2] = (crc32_value >> 8) & 0xFF;
  hash->crc32[3] = crc32_value & 0xFF;

  fclose(file);

  return true;
}

/**
 * @brief Função de utilidade para converter hash em formato hexadecimal para
 * string
 *
 * @param hash Hash a ser convertido
 * @param hash_str Buffer para receber a string
 * @param buffer_size Tamanho do buffer
 * @param hash_type Tipo de hash (0=CRC32, 1=MD5, 2=SHA1)
 * @return true Se convertido com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_hash_to_string(const mega_emu_rom_db_hash_t *hash,
                                    char *hash_str, size_t buffer_size,
                                    uint8_t hash_type) {
  if (hash == NULL || hash_str == NULL) {
    return false;
  }

  // Verificar o tamanho do buffer
  size_t min_size;
  const unsigned char *src;
  size_t src_len;

  switch (hash_type) {
  case 0:         // CRC32
    min_size = 9; // 8 chars + null terminator
    src = hash->crc32;
    src_len = 4;
    break;
  case 1:          // MD5
    min_size = 33; // 32 chars + null terminator
    src = hash->md5;
    src_len = 16;
    break;
  case 2:          // SHA1
    min_size = 41; // 40 chars + null terminator
    src = hash->sha1;
    src_len = 20;
    break;
  default:
    return false;
  }

  if (buffer_size < min_size) {
    return false;
  }

  // Converter para string hexadecimal
  for (size_t i = 0; i < src_len; i++) {
    snprintf(hash_str + (i * 2), 3, "%02x", src[i]);
  }

  return true;
}

/**
 * @brief Função de utilidade para converter string hexadecimal em hash
 *
 * @param hash_str String contendo o hash em formato hexadecimal
 * @param hash Estrutura que receberá o hash
 * @param hash_type Tipo de hash (0=CRC32, 1=MD5, 2=SHA1)
 * @return true Se convertido com sucesso
 * @return false Se ocorrer erro
 */
bool mega_emu_rom_db_string_to_hash(const char *hash_str,
                                    mega_emu_rom_db_hash_t *hash,
                                    uint8_t hash_type) {
  if (hash_str == NULL || hash == NULL) {
    return false;
  }

  // Verificar o comprimento da string
  size_t expected_len;
  unsigned char *dest;
  size_t dest_len;

  switch (hash_type) {
  case 0: // CRC32
    expected_len = 8;
    dest = hash->crc32;
    dest_len = 4;
    break;
  case 1: // MD5
    expected_len = 32;
    dest = hash->md5;
    dest_len = 16;
    break;
  case 2: // SHA1
    expected_len = 40;
    dest = hash->sha1;
    dest_len = 20;
    break;
  default:
    return false;
  }

  size_t str_len = strlen(hash_str);
  if (str_len != expected_len) {
    return false;
  }

  // Converter string hexadecimal para bytes
  for (size_t i = 0; i < dest_len; i++) {
    unsigned int value;
    if (sscanf(hash_str + (i * 2), "%02x", &value) != 1) {
      return false;
    }
    dest[i] = (unsigned char)value;
  }

  return true;
}

/**
 * @brief Compara dois hashes
 *
 * @param hash1 Primeiro hash
 * @param hash2 Segundo hash
 * @param hash_type Tipo de hash a comparar (0=CRC32, 1=MD5, 2=SHA1)
 * @return true Se os hashes são iguais
 * @return false Se os hashes são diferentes
 */
bool mega_emu_rom_db_compare_hash(const mega_emu_rom_db_hash_t *hash1,
                                  const mega_emu_rom_db_hash_t *hash2,
                                  uint8_t hash_type) {
  if (hash1 == NULL || hash2 == NULL) {
    return false;
  }

  switch (hash_type) {
  case 0: // CRC32
    return memcmp(hash1->crc32, hash2->crc32, 4) == 0;
  case 1: // MD5
    return memcmp(hash1->md5, hash2->md5, 16) == 0;
  case 2: // SHA1
    return memcmp(hash1->sha1, hash2->sha1, 20) == 0;
  default:
    return false;
  }
}

/**
 * @brief Verifica se dois conjuntos de hashes têm alguma correspondência
 *
 * @param hash1 Primeiro conjunto de hashes
 * @param hash2 Segundo conjunto de hashes
 * @return true Se há correspondência em qualquer um dos hashes
 * @return false Se nenhum hash corresponde
 */
bool mega_emu_rom_db_has_matching_hash(const mega_emu_rom_db_hash_t *hash1,
                                       const mega_emu_rom_db_hash_t *hash2) {
  return mega_emu_rom_db_compare_hash(hash1, hash2, 0) || // CRC32
         mega_emu_rom_db_compare_hash(hash1, hash2, 1) || // MD5
         mega_emu_rom_db_compare_hash(hash1, hash2, 2);   // SHA1
}
