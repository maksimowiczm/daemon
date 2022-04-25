#pragma once
#include <dirent.h>

int is_directory(const struct dirent* dir);
int is_regular_file(const char* path, int error);
int get_permission(const char* path);
char* concat_path(const char* source, const char* file);
int skip_location(const char* name);
long compare_files_times(const char* file1, const char* file2);
void send_syslog(int type, const char* format, ...);
