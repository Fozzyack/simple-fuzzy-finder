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

int main() {

    char *home = getenv("HOME");
    fs::path home_dir(home);
    std::vector<std::string> dirs;
    std::thread create_dir_thread(get_directories, std::cref(home_dir), std::ref(dirs));


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
    std::string search = "";


    render(dirs, search, entries, choice);
    printw("Choice Index: %d\n", choice);
    printw("Search: %s ", search.c_str());
    refresh();

    while (c != '\n') {

        c = getch();
        switch(c) {
            case KEY_BACKSPACE:
                if (search.size() > 0) search.pop_back();
                break;
            case KEY_UP:
                choice--;
                break;
            case KEY_DOWN:
                choice++;
                break;
            case KEY_ENTER:
                break;
            default:
                search += c;
                choice = 0;
                break;
        }

        clear();
        render(dirs, search, entries, choice);
        printw("Choice Index: %d\n", choice);
        printw("Search: %s", search.c_str());
        refresh();
    }
    endwin();
    return 0;
}
