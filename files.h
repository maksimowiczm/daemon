#pragma once
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <utime.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

void copy_file_dates(const char* from, const char* to);
void copy_file(const char* from, const char* to, ssize_t buffor, ssize_t large_file_size_limit);
void delete_file(const char* path);
void delete_directory(const char* path);
