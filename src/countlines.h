#ifndef COUNTLINES_H
#define COUNTLINES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Platform-specific includes
#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #define PATH_SEPARATOR '\\'
    #define PATH_SEPARATOR_STR "\\"
#else
    #include <dirent.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #define PATH_SEPARATOR '/'
    #define PATH_SEPARATOR_STR "/"
#endif

// Maximum path length
#define MAX_PATH_LEN 4096
#define MAX_EXCLUDE_DIRS 100

// Structure to hold exclusion patterns
typedef struct {
    char **patterns;
    int count;
    int capacity;
} ExcludeList;

// Structure to hold counting results
typedef struct {
    unsigned long long total_lines;
    unsigned long long total_files;
    unsigned long long blank_lines;
    unsigned long long comment_lines;
    unsigned long long code_lines;
} CountResult;

// Function declarations
ExcludeList* create_exclude_list(void);
void add_exclude_pattern(ExcludeList *list, const char *pattern);
void free_exclude_list(ExcludeList *list);
bool is_excluded(const char *path, const ExcludeList *exclude_list);

bool is_text_file(const char *filename);
unsigned long long count_lines_in_file(const char *filepath, CountResult *result);
void count_lines_in_directory(const char *dirpath, const ExcludeList *exclude_list, CountResult *result);

void print_usage(const char *program_name);
void print_results(const CountResult *result, const char *target_path);

#endif // COUNTLINES_H