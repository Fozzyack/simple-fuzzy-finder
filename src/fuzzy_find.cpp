/**
 * @file fuzzy_find.cpp
 * @brief Implementation of fuzzy finding algorithms and terminal rendering
 * 
 * This file contains the core fuzzy finding logic including:
 * - Custom scoring algorithm for path matching
 * - Multithreaded search implementation
 * - Terminal rendering with ncurses
 * - File type detection and icon display
 */

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



/**
 * @brief Calculates fuzzy match score for a directory path
 * 
 * Implements a custom scoring algorithm that rewards:
 * - Exact substring matches (highest score)
 * - Character matches at word boundaries
 * - Consecutive character matches
 * - Matches at the beginning of paths
 * 
 * Penalties are applied for longer paths to prefer shorter, more specific matches.
 * 
 * @param dir Directory path to score
 * @param search Search string to match against
 * @return Score value (higher is better, 0 means no match)
 */
int scoring(const std::string &dir, const std::string &search) {
    int n = dir.size();
    int m = search.size();
    int si = 0;
    std::vector<int> matches;
    
    // Exact substring match gets highest priority
    if (search.length() >= 3 && dir.find(search) != std::string::npos) return 10000 - dir.length();

    // Find all character matches in order
    for(int i = 0; si < m && i < n; i++) {
        if (tolower(search[si]) == ' ') si++;  // Skip spaces in search
        if ( tolower(dir[i]) == tolower(search[si])) {
            matches.push_back(i);
            si++;
        }
    }
    
    // No match if not all search characters found
    if (si < m) return 0;

    // Calculate score based on match positions
    int score = 0;
    int consecutive_bonus = 50;  // Bonus for consecutive matches
    int start_bonus = 20;        // Bonus for matches at start
    int segment_bonus = 90;      // Bonus for matches after separators

    // Score each match position
    for (int i = 0; i < matches.size(); i++) {
        int idx = matches[i];
        score += 100 - idx;  // Earlier positions get higher scores

        // Apply bonuses
        if (i > 0 && matches[i] == matches[i - 1] + 1) score += consecutive_bonus;
        if(idx == 0 || dir[idx - 1] == '/' ||  dir[idx - 1] == '_' || dir[idx - 1] == '-') score += segment_bonus;
        if(idx == 0) score += start_bonus;
    }
    
    // Penalty for longer paths to prefer specific matches
    score -= dir.length() / 6;
    return score;
}


/**
 * @brief Worker thread function for parallel scoring
 * 
 * Processes a chunk of directory paths in parallel, scoring each path
 * against the search string. Results are sorted locally before being
 * merged with the main result set.
 * 
 * @param num_threads Total number of worker threads
 * @param start Starting index in the directory vector
 * @param end Ending index in the directory vector
 * @param dirs Vector of all directory paths
 * @param search Search string to match against
 * @param out Shared output vector for results (protected by mutex)
 * @param mut_lk Mutex for thread-safe access to shared output
 */
void thread_score(size_t num_threads, int start, int end, const std::vector<std::string> &dirs, const std::string &search, std::vector<std::pair<int, std::string>> &out, std::mutex &mut_lk) {

    std::vector<std::pair<int, std::string>> local;

    // Score all paths in this thread's chunk
    for (int i = start; i < end; i++) {
        int score = scoring(dirs[i], search);
        if (score > 0) local.push_back({score, dirs[i]});
    }

    // Sort local results by score (highest first)
    sort(local.begin(), local.end(),
            [] (const std::pair<int, std::string> a, const std::pair<int, std::string> b) {
                return a.first > b.first;
            });

    // Limit results per thread to prevent overwhelming output
    int number_of_results = local.size() / num_threads == 0 ? local.size() : local.size() / num_threads;

    // Thread-safe merge with global results
    std::lock_guard<std::mutex> lock(mut_lk);
    out.insert(out.end(), local.begin(), local.begin() + number_of_results);
}

/**
 * @brief Main search function using multithreaded fuzzy matching
 * 
 * Distributes the search workload across multiple threads for performance.
 * If no search string is provided, returns all directories. Otherwise,
 * uses parallel scoring to find and rank the best matches.
 * 
 * @param dirs Vector of all directory paths to search
 * @param search Search string to match against
 * @param result Output vector containing sorted search results
 */
void search_paths (const std::vector<std::string> dirs, const std::string &search, std::vector<std::string> &result) {

    // Return all directories if no search string
    if (search == "") result = dirs;
    else {
        result.clear();
        
        // Set up multithreading
        size_t num_threads = std::thread::hardware_concurrency();
        size_t total_size = dirs.size();
        std::vector<std::thread> threads;
        size_t chunk_size = (total_size + num_threads - 1) / num_threads;
        std::vector<std::pair<int, std::string>> scored_out;
        std::mutex mtx;

        // Launch worker threads
        for (size_t i = 0; i < num_threads; i++) {
            int start = i * chunk_size;
            int end = std::min(start + chunk_size, total_size);
            threads.push_back(std::thread(thread_score, num_threads, start, end, std::cref(dirs), std::cref(search), std::ref(scored_out), std::ref(mtx)));
        }

        // Wait for all threads to complete
        for ( std::thread &th: threads) th.join();

        // Sort final results by score
        sort(scored_out.begin(), scored_out.end(),
                [](const std::pair<int, std::string> &a, const std::pair<int, std::string> &b) {
                return a.first > b.first;
                });

        // Extract paths from scored results
        for (int i = 0; i < scored_out.size(); i++) {
            result.push_back(scored_out[i].second);
        }
    }
}

/**
 * @brief Renders search results with syntax highlighting and file icons
 * 
 * Displays search results in the terminal using ncurses with:
 * - File type specific icons (folders, code files, documents, etc.)
 * - Syntax highlighting for matched characters
 * - Selection indicator for current choice
 * - Automatic choice wrapping for navigation
 * 
 * @param result Vector of search results to display
 * @param choice Reference to current selection index (modified for wrapping)
 * @param entries Maximum number of entries to display
 * @param char_set Set of characters to highlight in magenta
 * @return Selected path string or "No Directory Chosen" if no results
 */
std::string render(const std::vector<std::string> &result, int &choice, int entries, std::set<char> char_set) {
 
    size_t total_size = result.size() < entries ? result.size() : entries;

    // Handle choice wrapping
    if (choice < 0) choice = total_size - 1;
    else if (choice >= total_size) choice = 0;

    // Render each result with icons and highlighting
    for(size_t i = 0; i < total_size; i++) {

        fs::path path(result[i]);
        std::string filename = path.filename();
        std::string ext = path.extension();
        
        printw(" ");
        
        // Selection indicator
        if (choice == i) printw("[*] ");
        
        // File type icons
        if(fs::is_directory(path)) printw("📁 ");
        else if(ext == ".cpp" || ext == ".ts" || ext == ".tsx" || ext == ".js" || ext == ".jsx" || ext == ".py" ) printw("📜 ");
        else if(ext == ".csv" || ext == ".json") printw("📜 ");
        else if(ext == ".md") printw("📝 ");
        else if(ext == ".h" || ext == ".hpp") printw("🧩 ");
        else if(filename == "") printw("🔒 ");
        else if(ext == "" || ext == ".out" || ext == ".bin" || ext == ".exe" || ext == ".bat" || ext == ".app") printw("💾 ");
        else if(filename == "CMakeLists") printw("🧱 ");
        else if(fs::is_regular_file(path)) printw("📄 ");
            
        // Render path with character highlighting
        for(int j = 0; j < result[i].size(); j++) {
            if (char_set.count(tolower(result[i][j]))) {
                // Highlight matched characters in magenta
                attron(COLOR_PAIR(1));
                printw("%c", result[i][j]);
                attroff(COLOR_PAIR(1));
            }
            else {
                // Normal characters in white
                attron(COLOR_PAIR(2));
                printw("%c", result[i][j]);
                attroff(COLOR_PAIR(2));
            }
        }
        printw("\n");
    }

    printw("\n Total Items: %d", (int) result.size());

    // Return selected path or default message
    std::string res;
    res = total_size > 0 ? result[choice] : "No Directory Chosen";
    return res;


}

