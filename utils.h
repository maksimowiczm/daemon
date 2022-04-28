#pragma once
#include <dirent.h>

/**
 * \brief Sprawdza czy plik jest folderem
 * \param dir dirent do pliku
 * \return true - jeśli plik jest folderem\n
 * false - w przeciwnym wypadku
 */
int is_directory(const struct dirent* dir);

/**
 * \brief Sprawdza czy plik jest zwyklym plikiem
 * \param path ścieżka do pliku
 * \param error jeśli true raportowanie błedów uruchomione
 * \return true - jeśli plik jest zwykłym plikiem \n
 * false - w przeciwnym wypadku
 */
int is_regular_file(const char* path, int error);

/**
 * \brief Pobranie uprawnień pliku
 * \param path ścieżka do pliku
 * \return uprawnienia pliku
 */
int get_permission(const char* path);
char* concat_path(const char* source, const char* file);
int skip_location(const char* name);
long compare_files_times(const char* file1, const char* file2);
void send_syslog(int type, const char* format, ...);
