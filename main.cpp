/**
 * @file main.cpp
 * @brief Main entry point for the CLI fuzzy finder application
 * 
 * This file contains the main application logic including:
 * - Command line argument parsing
 * - Directory scanning in background thread
 * - ncurses terminal interface setup
 * - Main event loop for user interaction
 * - Path selection and output
 */

#include "src/headers/fuzzy_find.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <set>
#include <vector>
#include <filesystem>

#include <locale.h>
#include <ncurses.h>

namespace fs = std::filesystem;


/**
 * @brief Recursively scans directory for all files and subdirectories
 * 
 * Traverses the filesystem starting from the given path, collecting
 * all accessible files and directories. Skips directories that cause
 * permission errors.
 * 
 * @param path Starting directory path to scan
 * @param dirs Output vector to store all discovered paths
 */
void get_directories(const fs::path path, std::vector<std::string> &dirs) {

    for (fs::directory_entry dir_entry: fs::recursive_directory_iterator(path, fs::directory_options::skip_permission_denied)) {
        dirs.push_back(dir_entry.path());
    }
}

/**
 * @brief Prints a limited number of directory paths (unused in current implementation)
 * 
 * @param dirs Vector of directory paths
 * @param entries Maximum number of entries to print
 */
void print_dirs(const std::vector<std::string> &dirs, int entries) {
    for (int i = 0; i < entries && i < dirs.size(); i++) {
        printw("%s\n", dirs[i].c_str());
    }
}

/**
 * @brief Creates a character set from search string for highlighting
 * 
 * Converts the search string into a set of lowercase characters
 * used for syntax highlighting in the terminal interface.
 * 
 * @param search Search string to process
 * @param char_set Output set to store unique characters
 */
void get_search_hashmap(const std::string &search, std::set<char> &char_set) {
    char_set.clear();
    for(int i = 0; i < search.size(); i++) char_set.insert(tolower(search[i]));
}


/**
 * @brief Main application entry point
 * 
 * Handles command line arguments, sets up the terminal interface,
 * launches background directory scanning, and runs the main
 * interactive loop for fuzzy finding.
 * 
 * Command line options:
 * - No args: Search from current directory
 * - -h, --help: Show help (exits with code 5)
 * - -s, --start: Search from home directory
 * - [path]: Search from specified directory
 * 
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return 0 on success, 1 on error
 */
int main(int argc, char *argv[]) {

    // Set locale for proper Unicode support
    setlocale(LC_ALL, "");

    char *home = getenv("HOME");

    // Parse command line arguments
    fs::path path(".");
    if (argc > 2) {
        std::cout << "Incorrect Amount of Arguments Given" << std::endl;
        exit(1);
    }
    if (argc == 2) {
        if (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help") {
            exit(5);
        } else if ( std::string(argv[1]) == "-s" || std::string(argv[1]) == "--start") {
            path = home;
        } else {
            // Validate custom path argument
            if (!fs::exists(argv[1])) {
                std::cout << "Unknown argument given -- Try using -h for help" << std::endl;
                exit(1);
            }
            path = std::string(argv[1]);
        }
    }

    // Initialize directory list and start background scanning
    std::vector<std::string> dirs;
    dirs.push_back(path.string());
    std::thread create_dir_thread(get_directories, std::cref(path), std::ref(dirs));

    // Set up ncurses terminal interface
    SCREEN *s;
    FILE *out = stdout;
    
    // Handle non-TTY output (e.g., when piped)
    if(!isatty(fileno(stdout))) {
        out = fopen("/dev/tty", "w");
        setbuf(out, NULL);
    }

    s = newterm(NULL, out, stdin);

    // Configure ncurses settings
    scrollok(stdscr, true);
    keypad(stdscr, true);     // Enable function keys
    cbreak();                 // Disable line buffering
    start_color();
    use_default_colors();
    
    // Set up color pairs for syntax highlighting
    init_pair(1, COLOR_MAGENTA, -1);  // Matched characters
    init_pair(2, COLOR_WHITE, -1);    // Normal characters
    
    printw("Loading ...");
    refresh();
    clear();

    // Wait for directory scanning to complete
    create_dir_thread.join();

    // Initialize search state
    int c;
    int entries = 10;                    // Number of results to display
    int choice = 0;                      // Currently selected result
    std::string selected_path = "";
    std::string search = "";
    std::set<char> char_set;             // Characters to highlight
    std::vector<std::string> result;

    // Initial search and render
    search_paths(dirs, search, result);
    selected_path = render(result, choice, entries, char_set);
    
    // Display UI instructions
    printw("\n Choice Index: %d\n", choice);
    printw(" Selected Choice: %s\n\n", selected_path.c_str());
    printw(" Use left or right arrow keys to increase/decrease entries\n");
    printw(" Use up / down / tab to navigate up or down\n");
    printw(" Search 🔍: %s", search.c_str());
    refresh();

    // Main event loop
    while (c != '\n') {

        c = getch();
        switch(c) {
            case KEY_BACKSPACE:
                // Remove last character from search
                if (search.size() > 0) search.pop_back();
                search_paths(dirs, search, result);
                get_search_hashmap(search, char_set);
                break;
            case KEY_UP:
                // Navigate up in results
                choice--;
                break;
            case KEY_LEFT:
                // Decrease number of displayed entries
                if(entries > 0) entries--;
                break;
            case KEY_RIGHT:
                // Increase number of displayed entries
                entries++;
                break;
            case '\t':
            case KEY_STAB:
            case KEY_DOWN:
                // Navigate down in results
                choice++;
                break;
            case '\n':
            case KEY_ENTER:
                // Select current choice (exit loop)
                break;
            default:
                // Add character to search string
                search += c;
                search_paths(dirs, search, result);
                get_search_hashmap(search, char_set);
                choice = 0;  // Reset selection to top
                break;
        }

        // Update display
        clear();
        selected_path = render(result, choice, entries, char_set);
        printw("\n Choice Index: %d\n", choice);
        printw(" Selected Choice: %s\n\n", selected_path.c_str());
        printw(" Use left or right arrow keys to increase/decrease entries\n");
        printw(" Use up / down / tab to navigate up or down\n");
        printw(" Search 🔍: %s", search.c_str());
        refresh();
    }
    
    // Clean up ncurses
    endwin();

    // Handle no selection case
    if (selected_path == "No Directory Chosen") {
        std::cout << selected_path << std::endl;
        exit(1);
    }

    // Output the selected directory path
    fs::path res(selected_path);
    if(!fs::is_directory(res)) {
        res.remove_filename();  // If file selected, use its directory
    }
    std::cout << fs::absolute(res).string() << std::endl;

    return 0;
}
