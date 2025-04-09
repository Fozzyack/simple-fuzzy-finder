#include "src/headers/fuzzy_find.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <vector>
#include <filesystem>
#include <ncurses.h>

namespace fs = std::filesystem;


void get_directories(const fs::path path, std::vector<std::string> &dirs) {

    for (fs::directory_entry dir_entry: fs::recursive_directory_iterator(path, fs::directory_options::skip_permission_denied)) {
        dirs.push_back(dir_entry.path());
    }
}

void print_dirs(const std::vector<std::string> &dirs, int entries) {
    for (int i = 0; i < entries && i < dirs.size(); i++) {
        printw("%s\n", dirs[i].c_str());
    }
}

int main(int argc, char *argv[]) {

    char *home = getenv("HOME");


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
            std::cout << "Unknown argument given -- Try using -h for help" << std::endl;
        }
    }




    std::vector<std::string> dirs;
    dirs.push_back(path.string());
    std::thread create_dir_thread(get_directories, std::cref(path), std::ref(dirs));


    SCREEN *s;
    FILE *out = stdout;
    if(!isatty(fileno(stdout))) {
        out = fopen("/dev/tty", "w");
        setbuf(out, NULL);
    }

    s = newterm(NULL, out, stdin);
    initscr();
    scrollok(stdscr, true);
    keypad(stdscr, true);
    cbreak();


    create_dir_thread.join();

    

    int c;
    int entries = 30;
    int choice = 0;
    std::string selected_path = "";
    std::string search = "";
    std::vector<std::string> result;


    search_paths(dirs, search, result);
    selected_path = render(result, choice, entries);
    printw("\nChoice Index: %d\n", choice);
    printw("Selected Choice: %s\n", selected_path.c_str());
    printw("Search: %s ", search.c_str());
    refresh();

    while (c != '\n') {

        c = getch();
        switch(c) {
            case KEY_BACKSPACE:
                if (search.size() > 0) search.pop_back();
                search_paths(dirs, search, result);
                break;
            case KEY_UP:
                choice--;
                break;
            case KEY_LEFT:
                if(entries > 0) entries--;
                break;
            case KEY_RIGHT:
                entries++;
                break;
            case '\t':
            case KEY_STAB:
            case KEY_DOWN:
                choice++;
                break;
            case '\n':
            case KEY_ENTER:
                break;
            default:
                search += c;
                search_paths(dirs, search, result);
                choice = 0;
                break;
        }

        clear();
        selected_path = render(result, choice, entries);
        printw("\nChoice Index: %d\n", choice);
        printw("Selected Choice: %s\n", selected_path.c_str());
        printw("\nSearch: %s", search.c_str());
        refresh();
    }
    endwin();

    if (selected_path == "No Directory Chosen") {
        std::cout << selected_path << std::endl;
        exit(1);
    }

    fs::path res(selected_path);
    std::cout << fs::absolute(res).string() << std::endl;

    return 0;
}
