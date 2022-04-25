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
	struct dirent** source_files_list; // Lista plików w folderze źródłowym
	// Ilość plików w folderze źródłowym
	const int no_of_source_files = scandir(source_path, &source_files_list, NULL, alphasort);

	struct dirent** dest_files_list; // Lista plików w folderze docelowym
	// Ilość plików w folderze docelowym
	const int no_of_dest_files = scandir(destination_path, &dest_files_list, NULL, alphasort);

	char *src, *dst; // Ścieżki do plików

	for (int i = 0; i < no_of_source_files; i++) // Wszystkie pliki w folderze źródłowym są sprawdzane
	{
		const char* source_file_name = source_files_list[i]->d_name; // Nazwa pliku źródłowego

		if (skip_location(source_file_name)) // Foldery "." ".." są omijane
			continue;

		if (mode == DEFAULT && is_directory(source_files_list[i])) // W trybie domyślnym foldery sa omijane
			continue;

		src = concat_path(source_path, source_file_name); // Ścieżka do pliku źródłowego
		dst = concat_path(destination_path, source_file_name); // Ścieżka do pliku docelowego

		if (mode == RECURSIVE && is_directory(source_files_list[i])) // W trybie rekursywnym fodlery są kopiowane
		{
			DIR* dir = opendir(dst);
			if (!dir) // Sprawdza czy folder istnieje
			{
				const int prem = get_permission(src);
				if (mkdir(dst, prem) < 0) // Jeśli nie istnieje tworzy nowy folder o takiej samej nazwie i uprawnieniach
				{
					fprintf(stderr, "copy_and_delete_all_files() mkdir() %s %s", dst, strerror(errno));
					exit(EXIT_FAILURE);
				}
			}

			free(dir);
			copy_and_delete_all_files(src, dst, buffor_size, large_file_size_limit);
			// Uruchomianie rekurencyjne funcji dla folderu
			copy_file_dates(src, dst); // Zmiana daty modyfikacji po skopiowaniu na prawdiłową

			free(dst);
			free(src);
			continue;
		}

		// Zmienne do zapisania informacji czy plik będzie kopiowany i czy istnieje w folderze docelowym
		int copy = 1, was = 0;

		for (int j = 0; j < no_of_dest_files; j++) // Wszystkie pliki w folderze docelowym są sprawdzane
		{
			const char* dest_file_name = dest_files_list[j]->d_name; // Nazwa pliku docelowego

			if (skip_location(dest_file_name)) // Foldery "." ".." są omijane
				continue;

			free(dst);
			dst = concat_path(destination_path, dest_file_name); // Ścieżka do pliku docelowego

			if (strcmp(source_file_name, dest_file_name) == 0) // Sprawdza czy pliki mają taką samą nazwę
			{
				was = 1; // Ustala czy plik był w folderze docelowym
				const long time = compare_files_times(src, dst); // Porównuje czasy modyfikacji plików

				if (!time) // Jeśli pliki są identyczne nie są kopiowane
					copy = 0;

				break; // Po znalezieniu pliku z identyczną nazwą pętla przeszukująca folder docelowy kończy się
			}
		}

		if (!copy)
		{
			free(src);
			free(dst);
			continue;
		}

		if (was) // Jeśli plik istnieje w folderze docelowym
		{
			delete_file(dst); // Istniejący plik jest usuwany
			copy_file(src, dst, buffor_size, large_file_size_limit); // i kopiowany
		}
		else // W przeciwnym wypadku jest tylko kopiowany
		{
			free(dst);
			dst = concat_path(destination_path, source_file_name);
			copy_file(src, dst, buffor_size, large_file_size_limit);
		}

		free(src);
		free(dst);
	}

	for (int i = 0; i < no_of_dest_files; i++) // Wszystkie pliki w folderze docelowym są sprawdzane
	{
		const char* dest_file_name = dest_files_list[i]->d_name; // Nazwa pliku w folderze docelowym

		if (skip_location(dest_file_name)) // Foldery "." ".." są omijane
			continue;

		if (mode == DEFAULT && is_directory(dest_files_list[i])) // W trybie domyślnym foldery sa omijane
			continue;

		int delete = 1; // Zmienna zapisująca informację o tym czy plik będzie usuwany
		for (int j = 0; j < no_of_source_files; j++) // Wszystkie pliki w folderze źródłowym są sprawdzane
		{
			const char* source_file_name = source_files_list[j]->d_name; // Nazwa pliku w folderze źródłowym

			if (skip_location(source_file_name)) // Foldery "." ".." są omijane
				continue;

			// Jeśli plik o tej nazwie istnieje w folderze źródłowym to plik nie jest usuwany
			if (strcmp(source_file_name, dest_file_name) == 0)
			{
				delete = 0;
				break; // Po znalezieniu pliku z identyczną nazwą pętla przeszukująca folder źródłowy kończy się
			}
		}

		dst = concat_path(destination_path, dest_file_name); // Ścieżka do pliku
		if (delete)
		{
			// W zależności czy plik jest folderem czy zwykłym plikiem odpowiednie funkcje są uruchamiane
			if (!is_regular_file(dst, 1))
				delete_directory(dst);

			delete_file(dst);
		}

		free(dst);
	}

	// Zwalnanie pamięci
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
	// Wczytanie argumentów
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

	// Stworzenie demona
	const pid_t pid = fork();

	if (pid < 0)
		exit(EXIT_FAILURE);
	else if (pid > 0)
	{
		wait(NULL);
		syslog(LOG_INFO, "Koniec demona.");
		exit(EXIT_SUCCESS);
	}

	signal(SIGUSR1, handle_signal);

	syslog(LOG_INFO, "Start demona. Uspanie na %d sekund", sleep_time);
	sleep(sleep_time);

	syslog(LOG_INFO, "Start kopiowania");
	copy_and_delete_all_files(argv[optind], argv[optind + 1], buffor_size, large_file_size_limit);
	syslog(LOG_INFO, "Skopiowane.");

	exit(EXIT_SUCCESS);
}
