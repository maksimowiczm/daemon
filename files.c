#include "files.h"
#include "utils.h"

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
		fprintf(stderr, "copy_file_dates() utime() %s %s %s", from, to, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void copy_file(const char* from, const char* to, const ssize_t buffor, const ssize_t large_file_size_limit)
{
	const int src = open(from, O_RDONLY);
	if (src < 0)
	{
		fprintf(stderr, "copy_file() open(from) %s %s", from, strerror(errno));
		exit(EXIT_FAILURE);
	}

	const int prem = get_permission(from);
	const int dst = open(to, O_WRONLY | O_CREAT | O_APPEND, prem);
	if (dst < 0)
	{
		fprintf(stderr, "copy_file() open(to) %s %s", to, strerror(errno));
		exit(EXIT_FAILURE);
	}

	struct stat st;
	stat(from, &st);
	const ssize_t size = st.st_size;

	syslog(LOG_INFO, "trying to copy %s to %s", from, to);

	if (size > large_file_size_limit)
	{
		char* addr = mmap(NULL, size, PROT_READ, MAP_PRIVATE, src, 0);
		if (addr == MAP_FAILED)
		{
			fprintf(stderr, "copy_file() mmap() %s", from);
			exit(EXIT_FAILURE);
		}

		write(dst, addr, size);
		munmap(addr, size);
	}
	else
	{
		void* buf = malloc(buffor);
		ssize_t bytes_read = read(src, buf, buffor);

		do
		{
			write(dst, buf, bytes_read);
		}
		while ((bytes_read = read(src, buf, buffor)) != 0);

		free(buf);
	}

	syslog(LOG_INFO, "copied %s to %s", from, to);

	int err = close(src);
	if (err < 0)
	{
		fprintf(stderr, "copy_file() close(src) %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	err = close(dst);
	if (err < 0)
	{
		fprintf(stderr, "copy_file() close(dst) %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// Zmiana daty modyfikacji pliku
	copy_file_dates(from, to);
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

void delete_file(const char* path)
{
	syslog(LOG_INFO, "trying to delete %s", path);
	const int err = remove(path);

	if (err < 0)
	{
		fprintf(stderr, "delete_file() %s %s", path, strerror(errno));
		exit(EXIT_FAILURE);
	}
	syslog(LOG_INFO, "deleted %s", path);
}

void delete_directory(const char* path)
{
	struct dirent** files_list;
	const int no_of_files = scandir(path, &files_list, NULL, alphasort);
	char* src;

	syslog(LOG_INFO, "trying to delete directory %s", path);
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

	syslog(LOG_INFO, "deleted directory %s", path);
	free(files_list);
}
