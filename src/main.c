#include "countlines.h"
#include "webserver.h"
#include <time.h>

#define VERSION "1.0.0"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Check for web server mode
    if (argc == 2 && (strcmp(argv[1], "--web") == 0 || strcmp(argv[1], "-w") == 0)) {
        return start_web_server(WEB_PORT);
    }
    
    // Check for web server with custom port
    if (argc == 3 && (strcmp(argv[1], "--web") == 0 || strcmp(argv[1], "-w") == 0)) {
        int port = atoi(argv[2]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Error: Invalid port number '%s'\n", argv[2]);
            return 1;
        }
        return start_web_server(port);
    }
    
    ExcludeList *exclude_list = create_exclude_list();
    if (!exclude_list) {
        fprintf(stderr, "Error: Failed to initialize exclude list\n");
        return 1;
    }
    
    // Add common exclusions by default
    add_exclude_pattern(exclude_list, ".git");
    add_exclude_pattern(exclude_list, ".svn");
    add_exclude_pattern(exclude_list, ".hg");
    add_exclude_pattern(exclude_list, "node_modules");
    add_exclude_pattern(exclude_list, "__pycache__");
    add_exclude_pattern(exclude_list, ".vs");
    add_exclude_pattern(exclude_list, ".vscode");
    
    char *target_path = NULL;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            free_exclude_list(exclude_list);
            return 0;
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("CountLines version %s\n", VERSION);
            printf("A high-performance CLI tool for counting lines of code\n");
            free_exclude_list(exclude_list);
            return 0;
        }
        else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--exclude") == 0) {
            if (i + 1 < argc) {
                add_exclude_pattern(exclude_list, argv[i + 1]);
                i++; // Skip next argument as it's the exclude pattern
            } else {
                fprintf(stderr, "Error: --exclude option requires an argument\n");
                free_exclude_list(exclude_list);
                return 1;
            }
        }
        else if (strncmp(argv[i], "--exclude=", 10) == 0) {
            add_exclude_pattern(exclude_list, argv[i] + 10);
        }
        else if (argv[i][0] == '-') {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            print_usage(argv[0]);
            free_exclude_list(exclude_list);
            return 1;
        }
        else {
            // This should be the target directory
            if (target_path == NULL) {
                target_path = argv[i];
            } else {
                fprintf(stderr, "Error: Multiple target directories specified\n");
                print_usage(argv[0]);
                free_exclude_list(exclude_list);
                return 1;
            }
        }
    }
    
    if (target_path == NULL) {
        fprintf(stderr, "Error: No target directory specified\n");
        print_usage(argv[0]);
        free_exclude_list(exclude_list);
        return 1;
    }
    
    // Check if target path exists
#ifdef _WIN32
    DWORD attributes = GetFileAttributes(target_path);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        fprintf(stderr, "Error: Path '%s' does not exist\n", target_path);
        free_exclude_list(exclude_list);
        return 1;
    }
    if (!(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        fprintf(stderr, "Error: '%s' is not a directory\n", target_path);
        free_exclude_list(exclude_list);
        return 1;
    }
#else
    struct stat path_stat;
    if (stat(target_path, &path_stat) != 0) {
        fprintf(stderr, "Error: Path '%s' does not exist\n", target_path);
        free_exclude_list(exclude_list);
        return 1;
    }
    if (!S_ISDIR(path_stat.st_mode)) {
        fprintf(stderr, "Error: '%s' is not a directory\n", target_path);
        free_exclude_list(exclude_list);
        return 1;
    }
#endif
    
    printf("Counting lines in: %s\n", target_path);
    if (exclude_list->count > 0) {
        printf("Excluding patterns: ");
        for (int i = 0; i < exclude_list->count; i++) {
            printf("%s", exclude_list->patterns[i]);
            if (i < exclude_list->count - 1) printf(", ");
        }
        printf("\n");
    }
    printf("Processing...\n");
    
    // Initialize result structure
    CountResult result = {0, 0, 0, 0, 0};
    
    // Start counting
    clock_t start_time = clock();
    count_lines_in_directory(target_path, exclude_list, &result);
    clock_t end_time = clock();
    
    // Print results
    print_results(&result, target_path);
    
    double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("\nProcessing completed in %.3f seconds\n", elapsed_time);
    
    // Cleanup
    free_exclude_list(exclude_list);
    
    return 0;
}