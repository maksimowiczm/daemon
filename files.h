#pragma once
#include <stdlib.h>

/**
 *    \brief Kopiowanie czasu modyfikacji pliku do innego pliku
 *    \param from ścieżka do pliku źródłowego
 *    \param to ścieżka do docelowego
 */
void copy_file_dates(const char* from, const char* to);

/**
 *    \brief Kopiowanie pliku
 *    \param from ścieżka do pliku źródłowego
 *    \param to ścieżka do docelowego
 *    \param buffor rozmiar buforu używanego do kopiowania plików
 *    \param large_file_size_limit rozmiar, po którym plik jest traktowany jako duży
 */
void copy_file(const char* from, const char* to, ssize_t buffor, ssize_t large_file_size_limit);

/**
 *   \brief Usunięcie istniejącego pliku
 *   \param path ścieżka do pliku
 */
void delete_file(const char* path);

/**
 *    \brief Usuwanie istniejącego folderu
 *    \param path ścieżka do folderu
 */
void delete_directory(const char* path);
