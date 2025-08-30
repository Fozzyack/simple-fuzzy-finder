/**
 * @file fuzzy_find.h
 * @brief Header file for fuzzy finding functionality
 * 
 * This header defines the core functions for fuzzy finding and rendering
 * search results in a terminal interface using ncurses.
 */

#pragma once
#include <vector>
#include <string>
#include <set>

/**
 * @brief Searches through directory paths using fuzzy matching
 * 
 * Performs multithreaded fuzzy search on a collection of directory paths.
 * Uses a custom scoring algorithm to rank matches based on character
 * proximity, word boundaries, and path length.
 * 
 * @param dirs Vector of directory paths to search through
 * @param search Search string to match against paths
 * @param result Output vector containing matched paths sorted by relevance
 */
void search_paths(const std::vector<std::string> dirs, const std::string &search, std::vector<std::string> &result);

/**
 * @brief Renders search results in the terminal interface
 * 
 * Displays search results with syntax highlighting, file type icons,
 * and navigation indicators. Handles user selection and choice wrapping.
 * 
 * @param result Vector of search results to display
 * @param choice Reference to current selection index (modified by function)
 * @param entries Maximum number of entries to display
 * @param char_set Set of characters to highlight in the results
 * @return Selected path as string, or "No Directory Chosen" if empty
 */
std::string render(const std::vector<std::string> &result, int &choice, int entries, std::set<char> char_set);
