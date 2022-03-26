#include "headers.h"

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

void copy_file_dates(const char* from, const char* to)
{
	struct stat st;
	stat(from, &st);
	const long mtime = st.st_mtime;
	struct utimbuf ubuf;
	time(&ubuf.actime);
	ubuf.modtime = mtime;
	const int err = utime(to, &ubuf);
	if (err < 0)
	{
		printf("copy_file_dates() utime() %s %s %s", from, to, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void copy_file(const char* from, const char* to, const ssize_t buffor)
{
	const int src = open(from, O_RDONLY);
	if (src < 0)
	{
		printf("copy_file() open(from) %s %s", from, strerror(errno));
		exit(EXIT_FAILURE);
	}
	const int dst = open(to, O_WRONLY | O_CREAT | O_APPEND, 0666);
	if (dst < 0)
	{
		printf("copy_file() open(to) %s %s", to, strerror(errno));
		exit(EXIT_FAILURE);
	}

	void* buf = malloc(buffor);
	ssize_t bytes_read = read(src, buf, buffor);

	do
	{
		write(dst, buf, bytes_read);
	} while ((bytes_read = read(src, buf, buffor)) != 0);

	free(buf);

	int err = close(src);
	if (err < 0)
	{
		printf("copy_file() close(src) %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	err = close(dst);
	if (err < 0)
	{
		printf("copy_file() close(dst) %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// Zmiana daty modyfikacji pliku
	copy_file_dates(from, to);
}

void delete_file(const char* path)
{
	const int err = remove(path);

	if (err < 0)
	{
		printf("delete_file() %s %s", path, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void delete_directory(const char* path)
{
	struct dirent** files_list;
	const int no_of_files = scandir(path, &files_list, NULL, alphasort);
	char* src;

	for (int i = 0; i < no_of_files; i++)
	{
		const char* file_name = files_list[i]->d_name;
		src = concat_path(path, file_name);

		if (skip_location(file_name))
			continue;

		if (is_directory(files_list[i]))
			delete_directory(src);

		delete_file(src);
		free(src);
	}

	free(files_list);
}

void copy_and_delete_all_files(const char* source_path, const char* destination_path, const int buffor_size)
{
	struct dirent** source_files_list;
	const int no_of_source_files = scandir(source_path, &source_files_list, NULL, alphasort);

	struct dirent** dest_files_list;
	const int no_of_dest_files = scandir(destination_path, &dest_files_list, NULL, alphasort);

	char* src, *dst;

	for (int i = 0; i < no_of_source_files; i++)
	{
		const char* source_file_name = source_files_list[i]->d_name;

		if (skip_location(source_file_name))
			continue;

		if (mode == DEFAULT && is_directory(source_files_list[i]))
			continue;

		src = concat_path(source_path, source_file_name);
		dst = concat_path(destination_path, source_file_name);

		int copy = 1, was = 0;

		for (int j = 0; j < no_of_dest_files; j++)
		{
			const char* dest_file_name = dest_files_list[j]->d_name;

			if (skip_location(dest_file_name))
				continue;

			free(dst);
			dst = concat_path(destination_path, dest_file_name);

			if (strcmp(source_file_name, dest_file_name) == 0)
			{
				was = 1;
				const long time = compare_files_times(src, dst);

				if (!time)
					copy = 0;

				break;
			}
		}

		if (!copy)
			continue;

		if (is_directory(source_files_list[i]) && mode == RECURSIVE)
		{
			free(dst);
			dst = concat_path(destination_path, source_file_name);

			if (mkdir(dst, 0700) < 0)
			{
				printf("copy_and_delete_all_files() mkdir() %s %s", dst, strerror(errno));
				exit(EXIT_FAILURE);
			}

			copy_and_delete_all_files(src, dst, buffor_size);
			copy_file_dates(src, dst);

			continue;
		}

		if (was)
		{
			delete_file(dst);
			copy_file(src, dst, buffor_size);
		}
		else
		{
			free(dst);
			dst = concat_path(destination_path, source_file_name);
			copy_file(src, dst, buffor_size);
		}

		free(src);
		free(dst);
	}

	for (int i = 0; i < no_of_dest_files; i++)
	{
		const char* dest_file_name = dest_files_list[i]->d_name;

		if (skip_location(dest_file_name))
			continue;

		if (mode == DEFAULT && is_directory(dest_files_list[i]))
			continue;

		int delete = 1;
		dst = concat_path(destination_path, dest_file_name);

		for (int j = 0; j < no_of_source_files; j++)
		{
			const char* source_file_name = source_files_list[j]->d_name;

			if (skip_location(source_file_name))
				continue;

			if (strcmp(source_file_name, dest_file_name) == 0)
			{
				delete = 0;
				break;
			}
		}

		if (delete)
		{
			if (!is_regular_file(dst))
				delete_directory(dst);

			delete_file(dst);
		}

		free(dst);
	}

	for (int i = 0; i < no_of_source_files; i++)
		free(source_files_list[i]);

	for (int i = 0; i < no_of_dest_files; i++)
		free(dest_files_list[i]);

	free(dest_files_list);
	free(source_files_list);
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("Za malo argumentow\n");
		exit(EXIT_FAILURE);
	}

	int opt, sleep = 300;

	ssize_t buffor_size = 16384;

	while ((opt = getopt(argc, argv, "Rb:s:")) != -1)
	{
		switch (opt)
		{
		case 'R':
			mode = RECURSIVE;
			break;
		case 'b':
			buffor_size = atoi(optarg);
			if (buffor_size <= 0)
			{
				fprintf(stderr, "Expected argument after option -b\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 's':
			sleep = atoi(optarg);
			if (sleep <= 0)
			{
				fprintf(stderr, "Expected argument after option -s\n");
				exit(EXIT_FAILURE);
			}
			break;
		default: /* '?' */
			fprintf(stderr, "Usage: %s [-b size] [-s time] [-R] directory directory\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (is_regular_file(argv[optind]))
	{
		printf("%s nie jest folderem\n", argv[optind]);
		exit(EXIT_FAILURE);
	}

	if (is_regular_file(argv[optind + 1]))
	{
		printf("%s nie jest folderem\n", argv[optind + 1]);
		exit(EXIT_FAILURE);
	}

	copy_and_delete_all_files(argv[optind], argv[optind + 1], buffor_size);

	exit(EXIT_SUCCESS);
}
