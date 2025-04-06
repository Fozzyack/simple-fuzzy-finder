#include <cctype>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <filesystem>
#include <ncurses.h>

using namespace std;
namespace fs = std::filesystem;


vector<string> create_file_list(fs::path &path) {
    vector<string> res;
    for(fs::directory_entry dir_entry : fs::recursive_directory_iterator(path, fs::directory_options::skip_permission_denied)) {
        res.push_back(dir_entry.path());
    }
    return res;
}

string search_screen(vector<string> &dirs, string search, int &index) {

    vector<string> print_dir;
    string res;
    copy_if(dirs.begin(), dirs.end(), std::back_inserter(print_dir),
            [search](std::string &s) {
                return !s.empty() && s.find(search) != string::npos;
            });

    if (index < 0) index = print_dir.size() -1;
    if (index >= print_dir.size()) index = 0;
    
    for (size_t i = max((int)print_dir.size() - 30, 0); i < max((int)print_dir.size(), 0); i++) {
        if (index == i) {
            printw("[*] %s\n", print_dir[i].c_str());
            res = print_dir[i];
        } else {
            printw("%s\n", print_dir[i].c_str());
        }
    }
    printw("%d size of print_dir\n", (int)print_dir.size());
    printw("%d size of index\n", index);
    return res;
}


int main(int argc, char *argv[]) {

    fs::path path(".");
    if (argc > 1) {
        path = argv[1];
    }


    initscr();
    cbreak();
    noecho();
    keypad(stdscr, true);
    scrollok(stdscr, true);

    vector<string> dirs = create_file_list(path);

    string search;
    string res_path;
    int ch;
    int index = -1;

    while(true) {
        clear();
        res_path = search_screen(dirs, search, index);
        printw("Search: %s", search.c_str());
        refresh();
        ch = getch();
        switch(ch) {
            case KEY_BACKSPACE:
            case KEY_DC:
                search.pop_back();
                break;
            case KEY_UP:
                index--;
                break;
            case KEY_DOWN:
                index++;
                break;
            case KEY_LEFT:
                break;
            case KEY_RIGHT:
                break;
            case KEY_ENTER:
                {
                    string tmp;
                    path = res_path;
                    tmp = fs::absolute(path);
                    endwin();
                    cout << tmp << endl;
                    break;
                }
            default:
                search += ch;
                index = -1;
                break;
        }
    }
    endwin();

    return 0;
}
