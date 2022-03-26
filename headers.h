#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>

enum
{
	DEFAULT,
	RECURSIVE
} mode = DEFAULT;

int is_directory(const struct dirent* dir);
// Sprawdza czy podany plik jest zwykym plikiem
int is_regular_file(const char* path);

char* concat_path(const char* source, const char* file);
int skip_location(const char* name);

long compare_files_times(const char* file1, const char* file2);

void copy_file_dates(const char* from, const char* to);
void copy_file(const char* from, const char* to, ssize_t buffor);
void delete_file(const char* path);
void delete_directory(const char* path);

void copy_and_delete_all_files(const char* source_path, const char* destination_path, int buffor_size);