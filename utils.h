#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

int is_directory(const struct dirent* dir);
int is_regular_file(const char* path);
char* concat_path(const char* source, const char* file);
int skip_location(const char* name);
long compare_files_times(const char* file1, const char* file2);