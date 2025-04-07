#include <unistd.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <filesystem>
#include <ncurses.h>

using namespace std;
namespace fs = std::filesystem;


/*
 * Returns a vector of all possible files
 * Could potential remove (duplicates) and not add it in if the file is a directory
 */

vector<string> create_file_list(fs::path &path) {
    vector<string> res;
    for(fs::directory_entry dir_entry : fs::recursive_directory_iterator(path, fs::directory_options::skip_permission_denied)) {
        res.push_back(dir_entry.path());
    }
    return res;
}

/*
 * Provides the real time searching of data through the screen.
 * Displays the top LIST_SIZE results
 */

string search_screen(vector<string> &dirs, string search, int &index) {

    int LIST_SIZE = 35;

    vector<string> print_dir;
    string res;

    copy_if(dirs.begin(), dirs.end(), std::back_inserter(print_dir),
            [search](std::string &s) {
                return !s.empty() && s.find(search) != string::npos;
            });

    size_t entry_length = min((int)print_dir.size() - 1, LIST_SIZE);
    if(index < 0) index = entry_length;
    index = index % entry_length;
    
    for (size_t i = 0; i < entry_length; i++) {
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


    /*
     * Creates a new screen. There is a problem if you try to print to stdout
     */

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

    string search; //Holds the search string
    string res_path; // Holds the currently selected path
    int ch; // The character that the user enters
    int index = -1; //The index of the list which the user has selected

    /*
     * The main loop that runs
     */
    while(ch != '\n') {
        clear();
        res_path = search_screen(dirs, search, index);
        if (res_path != "") { // Print the users current selection
            printw("Current Selection: %s\n", res_path.c_str());
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
            case KEY_BTAB:
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
    cout << fs::absolute(final) << endl;

    return 0;
}
