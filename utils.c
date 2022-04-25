#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <time.h>

int is_directory(const struct dirent* dir)
{
	return dir->d_type == DT_DIR;
}

int is_regular_file(const char* path, const int error)
{
	struct stat path_stat;
	const int err = stat(path, &path_stat);

	if (err == -1)
	{
		if (!error)
			return -1;

		fprintf(stderr, "is_regular_file() %s %s", path, strerror(errno));
		exit(EXIT_FAILURE);
	}

	return S_ISREG(path_stat.st_mode);
}

int get_permission(const char* path)
{
	struct stat st;
	stat(path, &st);
	mode_t perm = st.st_mode;

	return (perm & S_IRUSR) | (perm & S_IWUSR) | (perm & S_IXUSR)
		| (perm & S_IRGRP) | (perm & S_IWGRP) | (perm & S_IXGRP)
		| (perm & S_IROTH) | (perm & S_IWOTH) | (perm & S_IXOTH);
}

// Łączy dwa łańcuchy znaków w jeden
// "/home/user" + "file.txt" = "/home/user/file.txt"
char* concat_path(const char* source, const char* file)
{
	char* path = malloc(strlen(source) + strlen(file) + 2);

	strcpy(path, source);

	strcat(path, "/");
	strcat(path, file);

	return path;
}

// Sprawdza czy plik nazywa się "." lub ".."
int skip_location(const char* name)
{
	return strcmp(name, ".") == 0 || strcmp(name, "..") == 0;
}

// Zwraca różnicę pomiędzy czasami modyfikacji plików
long compare_files_times(const char* file1, const char* file2)
{
	struct stat stat1;
	if (stat(file1, &stat1) == -1)
	{
		fprintf(stderr, "compare_files_times() %s %s", file1, strerror(errno));
		exit(EXIT_FAILURE);
	}

	struct stat stat2;
	if (stat(file2, &stat2) == -1)
	{
		fprintf(stderr, "compare_files_times() %s %s", file2, strerror(errno));
		exit(EXIT_FAILURE);
	}

	const long time1 = stat1.st_mtime;
	const long time2 = stat2.st_mtime;

	return time1 - time2;
}

void send_syslog(const int type, const char* format, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);

	time_t rawtime;
	time(&rawtime);
	const struct tm* ti = localtime(&rawtime);

	syslog(type, "%d.%d.%d-%d:%d:%d %s",
	       ti->tm_mday, ti->tm_mon + 1, ti->tm_year + 1900,
	       ti->tm_hour, ti->tm_min, ti->tm_sec, buffer);
}
