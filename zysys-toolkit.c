/*
 * zysys-toolkit.c
 * Written by Zach Bornheimer (ZYSYS)
 *
 * Outlines a library of functions that allow for
 * easy implementation of server technology.
 *
 * Zysys C Toolkit
 */

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <pthread.h>
#include <signal.h>
#include "zysys-toolkit.h"

void set_sigint_caught(int dummy) {
	sigint_caught = 1;
}

typedef struct s_param {
	pthread_t t;
	pthread_mutex_t* end_animation;
	char* message;
} s_param;

// From: https://stackoverflow.com/questions/41309872/count-files-with-a-specific-extension-in-directories-and-subdirectories-c
void listdir(const char* name, char* extension, unsigned int* count, char*** files, unsigned int* allocated_file_arr_size)
{
	DIR* dir;
	struct dirent* entry;

	if (!(dir = opendir(name)))
		return;
	if (!(entry = readdir(dir)))
		return;

	do {
		if (sigint_caught)
			break;
		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;

			char* path = malloc(strlen(name) + strlen(entry->d_name) + 2);
			strcpy(path, name);
			strcat(path, "/");
			strcat(path, entry->d_name);
			listdir(path, extension, count, files, allocated_file_arr_size);
			TRY_FREE(path);
		} else {
			const char* ext = strrchr(entry->d_name,'.');
			if( !ext || ext == entry->d_name ) {
				continue;
			} else if (strcmp(ext, extension) == 0) {
				char* file = malloc(strlen(name) + strlen(entry->d_name) + 2);
				strcpy(file, name);
				strcat(file, "/");
				strcat(file, entry->d_name);
				strcat(file, "\0");
				filetrack(files, &file, count);
				if ((*allocated_file_arr_size / sizeof(*files)) == *count) {
					*allocated_file_arr_size *= 2;
					*files = realloc(*files, *allocated_file_arr_size);
				}
				TRY_FREE(file);
			}
		}
	} while (entry = readdir(dir));
	closedir(dir);
}

char** get_rfiles_by_extension(char* extension, int* count) {
	unsigned int num_files = 0;
	char** file_list = malloc(sizeof(char*));
	unsigned int allocated_file_arr_size = sizeof(*file_list);
	listdir(".", extension, &num_files, &file_list, &allocated_file_arr_size);
	*count = num_files;
	return file_list;
}

int filetrack(char*** file_arr, char** file, unsigned int* i)
{
	char* file_to_append = *file;
	char** a = *file_arr;
	a[*i] = malloc(strlen(file_to_append) + 1);
	if (a[*i] == NULL)
		return -1;
	strcpy(a[*i], file_to_append);	
	*i += 1;
	file_arr = &a;
	return 0;
}

void progress_bar(unsigned int current, unsigned int total)
{
	if (sigint_caught)
		return;
	if (current < total) {
		float percentage = ( float ) current / (float) total;
		printf("%d of %d .. %.2f%% complete\t\r", current, total, percentage*100);
		fflush(stdout);
	} else {
		printf("%d of %d .. %d%% complete\t\n\n", current, total, 100);
		fflush(stdout);
	}

}

void* progress_bar_animate_thread(void* thread_arg) 
{
	s_param* thread = thread_arg;
	int periods = 0;
	int period_math = 1;
	char marker = '.';
	int max_length = 5;
	char* message = thread->message;
	while (1) {
		if (sigint_caught) {
			printf("SIGINT CAUGHT\t\n");
			break;
		}
		printf("%s", message);
		int q=0;
		for (q=0;q<=periods;q++)
			printf("%c", marker);
		for (q=max_length;q>periods;q--)
			printf(" ");
		printf("\r");
		if (periods > 3)
			period_math = -1;
		else if (periods == 0)
			period_math = 1;
		periods += period_math;

		fflush(stdout);
		sleep(1);

		if (pthread_mutex_trylock(thread->end_animation) == 0){
			pthread_mutex_unlock(thread->end_animation);
			pthread_exit(NULL);
		}
	}
	return thread_arg;
} 
s_param* progress_bar_animate(char* progress_bar_message) {
	int rc;
	static pthread_mutex_t end_animation = PTHREAD_MUTEX_INITIALIZER;
	s_param* thread = malloc(sizeof(s_param));

	thread->end_animation = &end_animation;
	pthread_mutex_lock(thread->end_animation);

	thread->message = malloc(strlen(progress_bar_message) + 1);
	strcpy(thread->message, progress_bar_message);

	rc = pthread_create(&thread->t, NULL, progress_bar_animate_thread, (void*)thread);

	if (rc)
		ERROR(THREAD_SPAWN);

	return thread;
}

void end_progress_bar_animation(s_param* thread)
{
	pthread_mutex_unlock(thread->end_animation); 
}

char* replace_wrapper_char(char character, int pos_one, int pos_two, char* string)
{
	int one = pos_one, two = pos_two;
	if (pos_one < 0)
		one = strlen(string) + pos_one;
	if (pos_two < 0)
		two = strlen(string) + pos_two;

	char* str = malloc(strlen(string) + 2);
	strcpy(str, string);
	str[one] = character;
	str[two] = character;
	str[strlen(string)+1] = '\0';
	return str;
}

char* escape_internal_chars(int pos_one, int pos_two, char* string)
{
	// pos_one and pos_two refer to quote locations
	// escape_internal_char(0, -1, ...):
	// ""which chars? it can't be these!"" => "\"which chars? it can\' be these!\""
	int one = pos_one, two = pos_two;
	int i = 0, j = 0;

	if (pos_one < 0)
		one = strlen(string) - pos_one;
	if (pos_two < 0)
		two = strlen(string) + pos_two;

	char* str = malloc(strlen(string) * 2 + 1);
	for (i = 0; i < strlen(string); i++) {
		if (i == one || i == two)
			str[j] = string[i];
		else
			switch(string[i]) {
				case '\'':
				case '"':
				case '(':
				case ')':
				case ' ':
				case '[':
				case ']':
					str[j++] = '\\';
					str[j] = string[i];
					break;
				default:
					str[j] = string[i];
			}
		j++;
	}

	str[j] = '\0';
	return str;
}


char* escape_chars(char* string)
{
	int i;
	char* str = malloc(strlen(string)+3);
	str[0] = '"';
	for (i=0;i<=strlen(string);++i)
		str[i+1] = string[i];
	str[strlen(string)+1] = '"';
	str[strlen(string)+2] = '\0';
	char* newstring = escape_internal_chars(0, -1, str);
	TRY_FREE(str);
	char* newstr = malloc(strlen(newstring)+1);
	for (i=1;i<strlen(newstring)-1;++i){
		newstr[i-1] = newstring[i];
	}

	free(newstring);
	
	newstr[i] = '\0';
	return newstr;
}

void execute_command_on_file_list(unsigned int count, char* cmd, unsigned int total, char** file_list)
{
	if (sigint_caught)
		return;
	
	progress_bar(count, total);

	if (strlen(file_list[count]) < 1)
		return;

	char* escaped = escape_internal_chars(0,-1,file_list[count]);
	char* substr = malloc((strstr(cmd, "%s")-cmd) + 1);
	char* exec_cmd_dirty = malloc(strlen(cmd) + 1 + strlen(escaped) + 1 + strlen(" >/dev/null") + 1);

	memcpy(substr, cmd, strstr(cmd, "%s")-cmd);
	substr[strstr(cmd, "%s")-cmd] = '\0';
	strcpy(exec_cmd_dirty, substr);

	strcat(exec_cmd_dirty, escaped);
	strcat(exec_cmd_dirty, strstr(cmd, "%s")+2);
	strcat(exec_cmd_dirty, " >/dev/null\0");
	TRY_FREE(substr);
	TRY_FREE(escaped);

	char* exec_cmd = clean_string(exec_cmd_dirty);
	free(exec_cmd_dirty);

	if (!sigint_caught && system(exec_cmd) == SIGINT)
		set_sigint_caught(SIGINT);

	if (count+1 < total && !sigint_caught)
		execute_command_on_file_list(count+1, cmd, total, file_list);
	else if (sigint_caught)
		printf("SIGINT CAUGHT\t\n");
	else
		progress_bar(count+1, total);

	TRY_FREE(exec_cmd);
}

char* clean_string(char* string)
{
	char* clean = malloc(strlen(string) + 1);
	int w, x;
	x=0;
	for (w = 0; w <= strlen(string); ++w)
		if (string[w] < 128 && string[w] >= 0 )
			clean[x++] = string[w];

	clean = realloc(clean, strlen(clean) + 1);
	return clean;
}
