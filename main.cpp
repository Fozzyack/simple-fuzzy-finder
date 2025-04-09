#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iterator>
#include <mutex>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>


#include <ncurses.h>


using namespace std;
namespace fs = std::filesystem;

void get_dirs(const fs::path home_dir, vector<string> &dirs); 
void render_list(const vector<string> &dirs, string &search, int &entries_to_display);

int main(int argc, char *argv[]) {

    char *home = getenv("HOME");
    fs::path home_dir(home);
    vector<string> dirs;

    thread make_dir_thread(get_dirs, home_dir, ref(dirs));

    SCREEN *s = NULL; FILE *out = stdout;
    if(!isatty(fileno(stdout))) {
        out = fopen("/dev/tty", "w");
        setbuf(out, NULL);
    }
    s = newterm(NULL, out, stdin);


    initscr();
    cbreak();
    scrollok(stdscr, TRUE);
    keypad(stdscr, TRUE);


    int input;
    int entries_to_display = 20;
    string search = "";

    make_dir_thread.join();
    render_list(dirs, search, entries_to_display);
    printw("Search: %s", search.c_str());

    while(input != '\n') {
        input = getch();
        switch(input) {
            case KEY_BACKSPACE:
                if (search.size() > 0) search.pop_back();
                break;
            case '\n':
            case KEY_ENTER:
                break;
            case KEY_RIGHT:
                entries_to_display++;
                break;
            case KEY_LEFT:
                if (entries_to_display > 0) entries_to_display--;
                break;
            default:
                search += input;
                break;
        }

        clear();

        render_list(dirs, search, entries_to_display);
        printw("Search: %s", search.c_str());
        refresh();
    }



    return 0;
}

void get_dirs(const fs::path home_dir, vector<string> &dirs) {

    for (fs::directory_entry dir_entry : fs::recursive_directory_iterator(home_dir, fs::directory_options::skip_permission_denied)) {
        dirs.push_back(dir_entry.path());
    }

}


int char_bonus(char prev, char curr) {
    if (prev == '/' || prev == '-' || prev == '_' || isspace(prev))
        return 10;
    return 0;
}

void fuzzy_match_score(const string &dir_entry, const string &search, int &res_score) {
    size_t n = dir_entry.size(), m = search.size();
    size_t i = 0, j = 0;
    int score = 0;
    int consequtive_bonus = 10;
    int position_bonus = 5;
    int prev_match = -1;

    if (dir_entry == search) score += 1000;
    if( dir_entry.find(search) != string::npos) score += 500;

    while (i < n && j < m) {
        if (islower(dir_entry[i]) == islower(search[j])) {
            score += 100;
            score += max(0, static_cast<int>(50 - i));

            if(i > 0) {
                char_bonus(dir_entry[i - 1], dir_entry[i]);
            } else {
                score += 10;
            }
            if (prev_match == static_cast<int>(i) - 1) {
                score += consequtive_bonus;
            }
            prev_match = i;
            ++j;
        }
        ++i;
    }
    score -= dir_entry.length();
    res_score = score;

}

void thread_score(int start, int end, const vector<string> &dirs, const string &search, vector<pair<int, string>> &result, mutex &mu_check) {

    vector<pair<int, string>> local;

    for (size_t i = start; i < end; i++) {
        int score;
        fuzzy_match_score(dirs[i], search, score);
        local.push_back({score, dirs[i]});
    }

    lock_guard<mutex> lock(mu_check);
    result.insert(result.end(), local.begin(), local.end());
}

void render_list(const vector<string> &dirs, string &search, int &entries_to_display) {

    int nu_display = entries_to_display;

    if (search == "") {
        if (dirs.size() <= nu_display) nu_display = dirs.size();
        for (size_t i = 0; i < nu_display; i++) {
            printw("%s\n", dirs[i].c_str());
        }
    } else {

        size_t num_threads = thread::hardware_concurrency();
        size_t chunk_size = (dirs.size() + num_threads - 1) / num_threads;
        mutex mu_check;
        vector<pair<int, string>> result;
        vector<thread> threads;

        for(int i = 0; i < num_threads; i++) {
            size_t start = i * chunk_size;
            size_t end = min(start + chunk_size, dirs.size());
            threads.push_back(thread(thread_score, start, end, cref(dirs), cref(search), ref(result), ref(mu_check)));
        }

        for (thread &t: threads) t.join();

        sort(result.begin(), result.end(),
                [](const pair<int, string> a, const pair<int, string> b) {
                    return a.first > b.first;
                });

        if (result.size() <= nu_display) nu_display = result.size();
        for (size_t i = 0; i < nu_display; i++) {
            printw("%d: %s\n", result[i].first, result[i].second.c_str());
        }

    }
}

