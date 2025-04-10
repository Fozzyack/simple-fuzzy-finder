#include <cctype>
#include <ncurses.h>
#include <set>
#include <string>
#include <algorithm>
#include <curses.h>
#include <vector>

#include <thread>
#include <mutex>
#include <filesystem>


namespace fs = std::filesystem;



int scoring(const std::string &dir, const std::string &search) {
    int n = dir.size();
    int m = search.size();
    int si = 0;
    std::vector<int> matches;
    if (search.length() >= 3 && dir.find(search) != std::string::npos) return 10000 - dir.length();

    for(int i = 0; si < m && i < n; i++) {
        if (tolower(search[si]) == ' ') si++;
        if ( tolower(dir[i]) == tolower(search[si])) {
            matches.push_back(i);
            si++;
        }
    }
    if (si < m) return 0;

    int score = 0;
    int consecutive_bonus = 50;
    int start_bonus = 20;
    int segment_bonus = 90;


    for (int i = 0; i < matches.size(); i++) {
        int idx = matches[i];
        score += 100 - idx;


        if (i > 0 && matches[i] == matches[i - 1] + 1) score += consecutive_bonus;
        if(idx == 0 || dir[idx - 1] == '/' ||  dir[idx - 1] == '_' || dir[idx - 1] == '-') score += segment_bonus;
        if(idx == 0) score += start_bonus;
    }
    score -= dir.length() / 6;
    return score;
}


void thread_score(int start, int end, const std::vector<std::string> &dirs, const std::string &search, std::vector<std::pair<int, std::string>> &out, std::mutex &mut_lk) {

    std::vector<std::pair<int, std::string>> local;

    for (int i = start; i < end; i++) {
        int score = scoring(dirs[i], search);
        if (score > 0) local.push_back({score, dirs[i]});
    }
    std::lock_guard<std::mutex> lock(mut_lk);
    out.insert(out.end(), local.begin(), local.end());
}

void search_paths (const std::vector<std::string> dirs, const std::string &search, std::vector<std::string> &result) {

    if (search == "") result = dirs;
    else {
        result.clear();
        size_t num_threads = std::thread::hardware_concurrency();
        size_t total_size = dirs.size();
        std::vector<std::thread> threads;
        size_t chunk_size = (total_size + num_threads - 1) / num_threads;
        std::vector<std::pair<int, std::string>> scored_out;
        std::mutex mtx;

        for (size_t i = 0; i < num_threads; i++) {
            int start = i * chunk_size;
            int end = std::min(start + chunk_size, total_size);
            threads.push_back(std::thread(thread_score, start, end, std::cref(dirs), std::cref(search), std::ref(scored_out), std::ref(mtx)));
        }

        for ( std::thread &th: threads) th.join();

        sort(scored_out.begin(), scored_out.end(),
                [](const std::pair<int, std::string> &a, const std::pair<int, std::string> &b) {
                return a.first > b.first;
                });

        for (int i = 0; i < scored_out.size(); i++) {
            result.push_back(scored_out[i].second);
        }
    }
}

std::string render(const std::vector<std::string> &result, int &choice, int entries, std::set<char> char_set) {
 
    size_t total_size = result.size() < entries ? result.size() : entries;

    if (choice < 0) choice = total_size - 1;
    else if (choice >= total_size) choice = 0;

    for(size_t i = 0; i < total_size; i++) {

        fs::path path(result[i]);
        std::string filename = path.filename();
        std::string ext = path.extension();
        printw(" ");
        if (choice == i) printw("[*] ");
        if(fs::is_directory(path)) printw("📁 ");
        else if(ext == ".cpp" || ext == ".ts" || ext == ".tsx" || ext == ".js" || ext == ".jsx" || ext == ".py" ) printw("📜 ");
        else if(ext == ".csv" || ext == ".json") printw("📜 ");
        else if(ext == ".md") printw("📝 ");
        else if(ext == ".h" || ext == ".hpp") printw("🧩 ");
        else if(filename == "") printw("🔒 ");
        else if(ext == "" || ext == ".out" || ext == ".bin" || ext == ".exe" || ext == ".bat" || ext == ".app") printw("💾 ");
        else if(filename == "CMakeLists") printw("🧱 ");
        else if(fs::is_regular_file(path)) printw("📄 ");
            
        for(int j = 0; j < result[i].size(); j++) {
            if (char_set.count(tolower(result[i][j]))) {
                
                attron(COLOR_PAIR(1));
                printw("%c", result[i][j]);
                attroff(COLOR_PAIR(1));
            }
            else {
                attron(COLOR_PAIR(2));
                printw("%c", result[i][j]);
                attroff(COLOR_PAIR(2));
            }
        }
        printw("\n");

    }

    std::string res;
    res = total_size > 0 ? result[choice] : "No Directory Chosen";
    return res;


}

