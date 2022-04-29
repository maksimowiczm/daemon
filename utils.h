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
 * \brief Sprawdza czy plik jest zwykłym plikiem
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

/**
 *    \brief Łączy ścieżkę do pliku z jego nazwą
 *    \param source źródło pliku
 *    \param file nazwa pliku
 *    \return połączona ścieżka do pliku
 */
char* concat_path(const char* source, const char* file);

/**
 *    \brief Sprawdza czy plik nazywa się "." lub ".."
 *    \param name nazwa pliku
 *    \return true - jeśli plik nazywa się "." lub ".."\n false - w przeciwnym wypadku 
 */
int skip_location(const char* name);

/**
 *    \brief Funkcja porównująca czas modyfikacji pliku
 *    \param file1 ścieżka do pliku pierwszego
 *    \param file2 ścieżka do pliku drugiego
 *	  \return różnicę między czasami
 */
long compare_files_times(const char* file1, const char* file2);

/**
 *    \brief Rejestrowanie zdarzeń zachodzących w programie do logów systemowych
 *    \param type rodzaj zdarzenia
 *    \param format format napisu
 *	  \param ... napis
 */
void send_syslog(int type, const char* format, ...);
