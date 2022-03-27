#include "utils.h"

int is_directory(const struct dirent* dir)
{
	return dir->d_type == DT_DIR;
}

int is_regular_file(const char* path)
{
	struct stat path_stat;
	const int err = stat(path, &path_stat);

	if (err == -1)
	{
		printf("is_regular_file() %s %s", path, strerror(errno));
		exit(EXIT_FAILURE);
	}

	return S_ISREG(path_stat.st_mode);
}

char* concat_path(const char* source, const char* file)
{
	char* path = malloc(strlen(source) + strlen(file) + 2);

	strcpy(path, source);

	strcat(path, "/");
	strcat(path, file);

	return path;
}

int skip_location(const char* name)
{
	return strcmp(name, ".") == 0 || strcmp(name, "..") == 0;
}

long compare_files_times(const char* file1, const char* file2)
{
	struct stat stat1;
	if (stat(file1, &stat1) == -1)
	{
		printf("compare_files_times() %s %s", file1, strerror(errno));
		exit(EXIT_FAILURE);
	}

	struct stat stat2;
	if (stat(file2, &stat2) == -1)
	{
		printf("compare_files_times() %s %s", file2, strerror(errno));
		exit(EXIT_FAILURE);
	}

	const long time1 = stat1.st_mtime;
	const long time2 = stat2.st_mtime;

	return time1 - time2;
}
