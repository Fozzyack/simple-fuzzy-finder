#include <vector>
#include <iostream>
#include <filesystem>
#include <string>

using namespace std;
namespace fs = std::filesystem ;

int main(int argc, char * argv[]) {
    
    if (argc > 2) {
        cout << "Incorrect amount of arguments given" << endl;
        exit(0);
    }
    string initial_path = ".";

    if (argc == 2) {
        initial_path = argv[1];
    }

    std::vector<std::string> all_files;
    fs::path path(initial_path);
        for( fs::directory_entry dir_entry : fs::recursive_directory_iterator (path, fs::directory_options::skip_permission_denied)) {
            all_files.push_back(dir_entry.path());
    }

    for(size_t i = 0; i < all_files.size(); i++) {
        cout << "[" << i << "] " << all_files[i] << endl;
    }

    return 0;
}
