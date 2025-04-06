#include <vector>
#include <iostream>
#include <filesystem>
#include <string>
#include <ncurses.h>


using namespace std;
namespace fs = std::filesystem ;


vector<string> create_search_list(const string &path) {
    vector<string> res;
    for (fs::directory_entry dir_entry: fs::recursive_directory_iterator(path, fs::directory_options::skip_permission_denied)) {
        res.push_back(dir_entry.path());
    }
    return res;
}

int main(int argc, char * argv[]) {
    
    if (argc > 2) {
        cout << "Incorrect amount of arguments given" << endl;
        exit(0);
    }
    string initial_path = ".";

    if (argc == 2) {
        initial_path = argv[1];
    }

    vector<string> dirs = create_search_list(initial_path);
    for (size_t i = 0; i < dirs.size(); i++) cout << dirs[i] << endl;

    char ch;
    string input;
    initscr();              // Start curses mode
    cbreak();               // Disable line buffering
    noecho();               // Don't echo input
    refresh();
    while(true) {
        ch = getch();
        if(ch == 27) break;
        input += (char)ch;
        refresh();
    }
    cout << input << endl;

    return 0;
}
