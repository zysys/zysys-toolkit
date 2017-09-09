/*
 * zysys-toolkit.h
 * Written by Zach Bornheimer (ZYSYS)
 *
 * Outlines a library of functions that allow for
 * easy implementation of server technology.
 *
 * Zysys C Toolkit
 */


/*** macros ***/
#define ERROR(a) exit(a);
#define THREAD_SPAWN 2
#define TRY_FREE(a) if (a) free(a);

/*** sigint_catching ***/
static volatile int sigint_caught = 0;
void set_sigint_caught(int dummy);

/*** structs and typedefs ***/

/* struct s_param: used for pthread args */
typedef struct s_param s_param;


/*** internal functions ***/

/* listdir: mechanics of get_rfiles_by_extension.
 * 	INTERNAL FUNCTION
 * 	inputs:
 * 		-name = path
 * 		-exension = file extension to search for
 * 		-count = how many have files in path have we found with extension
 * 		-files = the data structure holding file names: ['file name 1', 'file name 2']
 * 		-allocated_file_arr_size = size of the files array
 * 	outputs:
 * 		none
 * 	inline edits:
 * 		-count
 * 		-files
 * 		-allocated_file_arr_size
 */
void listdir(const char* name, char* extension, unsigned int* count, char*** files, unsigned int* allocated_file_arr_size);

/* filetrack: adds filename to file_arr
 * 	INTERNAL FUNCTION
 * 	inputs:
 * 		-file_arr = pointer to filename array
 * 		-file = the filename to add
 * 		-i = the index of the last filename in the filename array
 * 	outputs:
 * 		0 if successful
 * 	inline edits:
 * 		-file_arr
 * 		-i
 */

int filetrack(char*** file_arr, char** file, unsigned int* i);


/* progress_bar_animate_thread: mechanics fo progress_bar_animate
 * 	INTERNAL FUNCTION
 * 	inputs:
 * 		-thread_arg = a pointer to an s_param obj holding pthread args
 * 	outputs:
 * 		DISPLAY (updates inline): a message with incrementing/decrementing periods
 * 	inline edits:
 * 		-NONE
 */
void* progress_bar_animate_thread(void* thread_arg);

/*** internal/external functions ***/

/* escape_internal_chars: escapes certain problem characters, ignoring wrapping characters
 * 	INTERNAL/EXTERNAL FUNCTION
 * 	inputs:
 * 		-pos_one = the position of the first wrapping character (ie a quote mark).
 * 			   Negative values to indicate pos from end are okay (-1 is last char)
 * 		-pos_two = the position of the second wrapping character (ie a quote mark).
 * 			   Negative values to indicate pos from end are okay (-1 is last char)
 * 		-string = the string to escape
 * 	outputs:
 * 		-char* = string with characters replaced
 * 	inline edits:
 * 		-NONE
 */
char* escape_internal_chars(int pos_one, int pos_two, char* string);

/*** external functions ***/

/* get_rfiles_by_extension: equivant of find.
 * 	EXTERNAL FUNCTION
 * 	inputs:
 * 		-exension = file extension to search for
 * 		-count = how many have files in path have we found with extension
 * 		-allocated_file_arr_size = size of the files array
 * 	outputs:
 * 		-char** = the data structure holding file names: ['file name 1', 'file name 2']
 * 	inline edits:
 * 		-count
 */
char** get_rfiles_by_extension(char* extension, int* count);

/* progress_bar: shows info to user about progress during a loop
 * 	EXTERNAL FUNCTION
 * 	inputs:
 * 		-current = the current iteration in the array
 * 		-total = the total iterations scheduled in the loop
 * 	outputs:
 * 		DISPLAY (updates inline): __ of __ .. __% complete
 * 	inline edits:
 * 		-NONE
 */

void progress_bar(unsigned int current, unsigned int total);

/* progress_bar_animate: shows an animated message for tasks
 * 			 that can't have a prediction completion time
 * 	EXTERNAL FUNCTION
 * 	inputs:
 * 		-progress_bar_messag = the message to be displayed before the animation
 * 	outputs:
 * 		s_param obj: returns the parameter object containing a
 * 			     pointer to the thread and its args
 * 	inline edits:
 * 		-NONE
 */

s_param* progress_bar_animate(char* progress_bar_message);

/* end_progress_bar_animation: ends the progress bar animation
 * 	EXTERNAL FUNCTION
 * 	inputs:
 * 		-thread = the thread obj returned by progress_bar_animate
 * 	outputs:
 * 		-NONE
 * 	inline edits:
 * 		-NONE
 */

void end_progress_bar_animation(s_param* thread);

/* replace_wapper_char: replaces char at given positions with given char
 * 	EXTERNAL FUNCTION
 * 	inputs:
 * 		-character = the replacement character
 * 		-pos_one = the position of the first character.
 * 			   Negative values to indicate pos from end are okay (-1 is last char)
 * 		-pos_two = the position of the second character.
 * 			   Negative values to indicate pos from end are okay (-1 is last char)
 * 	outputs:
 * 		-char* = string with characters replaced
 * 	inline edits:
 * 		-NONE
 */

char* replace_wrapper_char(char character, int pos_one, int pos_two, char* string);

/* escape_chars: escapes certain problem characters
 * 	EXTERNAL FUNCTION
 * 	inputs:
 * 		-string = the string to escape
 * 	outputs:
 * 		-char* = string with characters replaced
 * 	inline edits:
 * 		-NONE
 */
char* escape_chars(char* string);

/* process_placeholder: convers %s into arg2
 * 	INTERNAL FUNCTION
 * 	inputs:
 * 		-cmd = the command that has placeholders
 * 		-escaped = the escaped replacment
 * 	outputs:
 * 		-cmd with the first placeholder replaced with escaped
 * 	inline edits:
 * 		-NONE
 */
char* process_placeholder(char* cmd, char* escaped);

/* execute_command_on_file_list: runs a command on a list of file_names
 * 	EXTERNAL FUNCTION
 * 	inputs:
 * 		-count = the iteration number
 * 		-cmd = the command to run.  Use %s as a replacement for the filename.
 * 		-total = the total number of iterations to be run
 * 		-file_list = the file list object that iterated on
 * 	outputs:
 * 		-NONE
 * 	inline edits:
 * 		-NONE
 */
void execute_command_on_file_list(unsigned int count, char* cmd, unsigned int total, char** file_list);

/* clean_string: removes characters with an ascii value over 128 and under 0
 * 	EXTERNAL FUNCTION
 * 	inputs:
 * 		-string = the string to work on
 * 	outputs:
 * 		-char * = the cleaned string
 * 	inline edits:
 * 		-NONE
 */
char* clean_string(char* string);


