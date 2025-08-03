#include "countlines.h"

// Create and initialize exclude list
ExcludeList* create_exclude_list(void) {
    ExcludeList *list = malloc(sizeof(ExcludeList));
    if (!list) return NULL;
    
    list->capacity = 10;
    list->patterns = malloc(sizeof(char*) * list->capacity);
    if (!list->patterns) {
        free(list);
        return NULL;
    }
    
    list->count = 0;
    return list;
}

// Add exclusion pattern to list
void add_exclude_pattern(ExcludeList *list, const char *pattern) {
    if (!list || !pattern) return;
    
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        char **new_patterns = realloc(list->patterns, sizeof(char*) * list->capacity);
        if (!new_patterns) return;
        list->patterns = new_patterns;
    }
    
    list->patterns[list->count] = malloc(strlen(pattern) + 1);
    if (list->patterns[list->count]) {
        strcpy(list->patterns[list->count], pattern);
        list->count++;
    }
}

// Free exclude list memory
void free_exclude_list(ExcludeList *list) {
    if (!list) return;
    
    for (int i = 0; i < list->count; i++) {
        free(list->patterns[i]);
    }
    free(list->patterns);
    free(list);
}

// Check if path matches any exclusion pattern
bool is_excluded(const char *path, const ExcludeList *exclude_list) {
    if (!path || !exclude_list) return false;
    
    for (int i = 0; i < exclude_list->count; i++) {
        if (strstr(path, exclude_list->patterns[i]) != NULL) {
            return true;
        }
    }
    return false;
}

// Check if file is likely a text file based on extension
bool is_text_file(const char *filename) {
    if (!filename) return false;
    
    const char *ext = strrchr(filename, '.');
    if (!ext) return false;
    
    // Common text file extensions
    const char *text_extensions[] = {
        ".c", ".cpp", ".cc", ".cxx", ".c++",
        ".h", ".hpp", ".hh", ".hxx", ".h++",
        ".java", ".js", ".ts", ".jsx", ".tsx",
        ".py", ".rb", ".php", ".go", ".rs",
        ".cs", ".vb", ".fs", ".swift", ".kt",
        ".scala", ".clj", ".hs", ".ml", ".r",
        ".sql", ".html", ".htm", ".xml", ".css",
        ".scss", ".sass", ".less", ".json", ".yaml",
        ".yml", ".toml", ".ini", ".cfg", ".conf",
        ".txt", ".md", ".rst", ".tex", ".sh",
        ".bash", ".zsh", ".fish", ".ps1", ".bat",
        ".cmd", ".vim", ".el", ".lua", ".perl",
        ".pl", ".tcl", ".awk", ".sed", ".m",
        ".mm", ".f", ".f90", ".f95", ".pas",
        ".ada", ".d", ".dart", ".elm", ".ex",
        ".exs", ".erl", ".hrl", ".jl", ".nim",
        ".v", ".vhd", ".vhdl", ".sv", ".svh",
        NULL
    };
    
    for (int i = 0; text_extensions[i]; i++) {
        if (strcmp(ext, text_extensions[i]) == 0) {
            return true;
        }
    }
    
    return false;
}

// Count lines in a single file
unsigned long long count_lines_in_file(const char *filepath, CountResult *result) {
    FILE *file = fopen(filepath, "r");
    if (!file) return 0;
    
    unsigned long long lines = 0;
    unsigned long long blank = 0;
    unsigned long long comments = 0;
    int ch, prev_ch = '\n';
    bool in_line_comment = false;
    bool in_block_comment = false;
    bool line_has_code = false;
    
    while ((ch = fgetc(file)) != EOF) {
        // Handle line comments (// style)
        if (prev_ch == '/' && ch == '/' && !in_block_comment) {
            in_line_comment = true;
        }
        
        // Handle block comments (/* style)
        if (prev_ch == '/' && ch == '*' && !in_line_comment) {
            in_block_comment = true;
        }
        
        // End block comment
        if (prev_ch == '*' && ch == '/' && in_block_comment) {
            in_block_comment = false;
            prev_ch = ch;
            continue;
        }
        
        // Check if character is code (not whitespace or comment)
        if (!in_line_comment && !in_block_comment && ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n') {
            line_has_code = true;
        }
        
        // Handle end of line
        if (ch == '\n') {
            lines++;
            if (!line_has_code && !in_line_comment && !in_block_comment) {
                blank++;
            } else if (in_line_comment || in_block_comment) {
                comments++;
            }
            
            in_line_comment = false;
            line_has_code = false;
        }
        
        prev_ch = ch;
    }
    
    // Handle file not ending with newline
    if (prev_ch != '\n' && prev_ch != EOF) {
        lines++;
        if (!line_has_code && !in_line_comment && !in_block_comment) {
            blank++;
        } else if (in_line_comment || in_block_comment) {
            comments++;
        }
    }
    
    fclose(file);
    
    if (result) {
        result->total_files++;
        result->blank_lines += blank;
        result->comment_lines += comments;
        result->code_lines += (lines - blank - comments);
    }
    
    return lines;
}

// Recursively count lines in directory
void count_lines_in_directory(const char *dirpath, const ExcludeList *exclude_list, CountResult *result) {
    if (!dirpath || !result) return;
    
    if (is_excluded(dirpath, exclude_list)) {
        return;
    }
    
#ifdef _WIN32
    char search_path[MAX_PATH_LEN];
    snprintf(search_path, sizeof(search_path), "%s\\*", dirpath);
    
    WIN32_FIND_DATA find_data;
    HANDLE hFind = FindFirstFile(search_path, &find_data);
    
    if (hFind == INVALID_HANDLE_VALUE) return;
    
    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }
        
        char full_path[MAX_PATH_LEN];
        snprintf(full_path, sizeof(full_path), "%s\\%s", dirpath, find_data.cFileName);
        
        if (is_excluded(full_path, exclude_list)) {
            continue;
        }
        
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            count_lines_in_directory(full_path, exclude_list, result);
        } else if (is_text_file(find_data.cFileName)) {
            unsigned long long file_lines = count_lines_in_file(full_path, result);
            result->total_lines += file_lines;
        }
    } while (FindNextFile(hFind, &find_data));
    
    FindClose(hFind);
#else
    DIR *dir = opendir(dirpath);
    if (!dir) return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char full_path[MAX_PATH_LEN];
        snprintf(full_path, sizeof(full_path), "%s/%s", dirpath, entry->d_name);
        
        if (is_excluded(full_path, exclude_list)) {
            continue;
        }
        
        struct stat file_stat;
        if (stat(full_path, &file_stat) == 0) {
            if (S_ISDIR(file_stat.st_mode)) {
                count_lines_in_directory(full_path, exclude_list, result);
            } else if (S_ISREG(file_stat.st_mode) && is_text_file(entry->d_name)) {
                unsigned long long file_lines = count_lines_in_file(full_path, result);
                result->total_lines += file_lines;
            }
        }
    }
    
    closedir(dir);
#endif
}

// Print usage information
void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS] <directory>\n", program_name);
    printf("\nA high-performance CLI tool for counting lines of code in projects.\n");
    printf("\nOptions:\n");
    printf("  -e, --exclude DIR     Exclude directory or pattern (can be used multiple times)\n");
    printf("  -h, --help           Show this help message\n");
    printf("  -v, --version        Show version information\n");
    printf("\nExamples:\n");
    printf("  %s /path/to/project\n", program_name);
    printf("  %s -e node_modules -e .git /path/to/project\n", program_name);
    printf("  %s --exclude=build --exclude=dist /path/to/project\n", program_name);
    printf("\nSupported file types:\n");
    printf("  C/C++, Java, JavaScript, TypeScript, Python, Ruby, PHP, Go, Rust,\n");
    printf("  C#, Visual Basic, F#, Swift, Kotlin, Scala, HTML, CSS, JSON, YAML,\n");
    printf("  Shell scripts, and many more text-based source files.\n");
}

// Print counting results
void print_results(const CountResult *result, const char *target_path) {
    printf("\n=== Code Line Count Results ===\n");
    printf("Target: %s\n", target_path);
    printf("Files processed: %llu\n", result->total_files);
    printf("Total lines: %llu\n", result->total_lines);
    printf("Code lines: %llu\n", result->code_lines);
    printf("Comment lines: %llu\n", result->comment_lines);
    printf("Blank lines: %llu\n", result->blank_lines);
    
    if (result->total_lines > 0) {
        double code_ratio = (double)result->code_lines / result->total_lines * 100.0;
        double comment_ratio = (double)result->comment_lines / result->total_lines * 100.0;
        double blank_ratio = (double)result->blank_lines / result->total_lines * 100.0;
        
        printf("\nBreakdown:\n");
        printf("Code:     %.1f%%\n", code_ratio);
        printf("Comments: %.1f%%\n", comment_ratio);
        printf("Blank:    %.1f%%\n", blank_ratio);
    }
}