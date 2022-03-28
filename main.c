#include "files.h"
#include "headers.h"
#include "utils.h"

enum
{
	DEFAULT,
	RECURSIVE
} mode = DEFAULT;

void copy_and_delete_all_files(const char* source_path, const char* destination_path, const ssize_t buffor_size,
                               const ssize_t large_file_size_limit)
{
	struct dirent** source_files_list;
	const int no_of_source_files = scandir(source_path, &source_files_list, NULL, alphasort);

	struct dirent** dest_files_list;
	const int no_of_dest_files = scandir(destination_path, &dest_files_list, NULL, alphasort);

	char *src, *dst;

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

			if (!opendir(dst))
				if (mkdir(dst, 0700) < 0)
				{
					fprintf(stderr, "copy_and_delete_all_files() mkdir() %s %s", dst, strerror(errno));
					exit(EXIT_FAILURE);
				}

			copy_and_delete_all_files(src, dst, buffor_size, large_file_size_limit);
			copy_file_dates(src, dst);

			free(dst);
			free(src);
			continue;
		}

		if (was)
		{
			delete_file(dst);
			copy_file(src, dst, buffor_size, large_file_size_limit);
		}
		else
		{
			free(dst);
			dst = concat_path(destination_path, source_file_name);
			copy_file(src, dst, buffor_size, large_file_size_limit);
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
			if (!is_regular_file(dst, 1))
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

void handle_signal(const int signum)
{
	if (signum == SIGUSR1)
		syslog(LOG_INFO, "SIGUSR1");
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s [-b size] [-s time] [-l size] [-R] source_directory destination_directory\n",
		        argv[0]);
		exit(EXIT_FAILURE);
	}

	int opt, sleep_time = 300;

	ssize_t buffor_size = 16384,
	        large_file_size_limit = 1048576;

	while ((opt = getopt(argc, argv, "Rb:s:l:")) != -1)
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
			sleep_time = atoi(optarg);
			if (sleep_time <= 0)
			{
				fprintf(stderr, "Expected argument after option -s\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'l':
			large_file_size_limit = atoi(optarg);
			if (large_file_size_limit <= 0)
			{
				fprintf(stderr, "Expected argument after option -l\n");
				exit(EXIT_FAILURE);
			}
			break;
		default:
			fprintf(stderr, "Usage: %s [-b size] [-s time] [-l size] [-R] source_directory destination_directory\n",
			        argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (is_regular_file(argv[optind], 0))
	{
		fprintf(stderr, "%s nie jest folderem\n", argv[optind]);
		exit(EXIT_FAILURE);
	}

	if (is_regular_file(argv[optind + 1], 0))
	{
		fprintf(stderr, "%s nie jest folderem\n", argv[optind + 1]);
		exit(EXIT_FAILURE);
	}

	pid_t pid = fork();

	if (pid < 0)
		exit(EXIT_FAILURE);
	else if (pid > 0)
		exit(EXIT_SUCCESS);

	const int ssid = setsid();
	if (ssid < 0)
		exit(EXIT_FAILURE);

	pid = fork();

	if (pid < 0)
		exit(EXIT_FAILURE);
	else if (pid > 0)
		exit(EXIT_SUCCESS);

	signal(SIGUSR1, handle_signal);

	syslog(LOG_INFO, "Starting daemon. Sleeping for %d seconds", sleep_time);
	sleep(sleep_time);

	syslog(LOG_INFO, "Starting copying");
	copy_and_delete_all_files(argv[optind], argv[optind + 1], buffor_size, large_file_size_limit);
	syslog(LOG_INFO, "Copied");
	syslog(LOG_INFO, "Exiting daemon");

	exit(EXIT_SUCCESS);
}
