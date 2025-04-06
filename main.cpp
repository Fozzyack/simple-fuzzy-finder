#include <unistd.h>
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

    fs::path path("./");
    if (argc > 1) {
        path = argv[1];
    }


    // Running this in a new terminal
    SCREEN *s = NULL;
    FILE* out = stdout;
    if(!isatty(fileno(stdout))) {
        out = fopen("/dev/tty", "w");
        setbuf(out, NULL);
    }
    s = newterm(NULL, out, stdin);

    cbreak();
    noecho();
    keypad(stdscr, true);
    scrollok(stdscr, true);

    vector<string> dirs = create_file_list(path);

    string search;
    string res_path;
    int ch;
    int index = -1;

    while(ch != '\n') {
        clear();
        res_path = search_screen(dirs, search, index);
        if (res_path != "") {
            printw("Current: %s\n", res_path.c_str());
            printw("Current: %s\n", fs::absolute(res_path).c_str());
        }
        printw("Search: %s", search.c_str());
        refresh();
        ch = getch();
        switch(ch) {
            case KEY_BACKSPACE:
            case KEY_DC:
                if (search.size() != 0) search.pop_back();
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
            default:
                search += ch;
                index = -1;
                break;
        }
    }
    endwin();
    fs::path final(res_path);
    if(!fs::is_directory(final)) {
        final.remove_filename();
    }
    if(chdir(fs::absolute(final).c_str()) == 0) {
        cout << "Directory Changed to: " << final.string() << endl;
    } else {
        cout << "Failed";
    }

    return 0;
}
