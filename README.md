# CLI Fuzzy Finder

![Demo](./demo.gif)

A fast, multithreaded fuzzy finder for navigating directories and files in the terminal.

## Features

- **Fast fuzzy search** with intelligent scoring algorithm
- **Multithreaded** directory scanning and search
- **Interactive UI** with ncurses
- **File type icons** for better visual navigation
- **Keyboard shortcuts** for efficient navigation
- **Cross-platform** support (Linux, macOS, Windows with WSL)

## How It Works

1. **Directory Scanning**: Recursively scans directories using C++17 filesystem
2. **Fuzzy Matching**: Custom scoring algorithm ranks files based on:
   - Character matches in the path
   - Consecutive character bonuses
   - Word boundary detection
   - Path length penalties
3. **Multithreading**: Splits search workload across CPU cores for performance
4. **Interactive Display**: Real-time results with syntax highlighting

## Usage

```bash
# Search from current directory
./ffcli

# Search from specific directory
./ffcli /path/to/directory

# Search from home directory
./ffcli -s
./ffcli --start

# Show help
./ffcli -h
./ffcli --help
```

### Controls
- **Type** to search
- **↑/↓** or **Tab** to navigate results
- **←/→** to adjust number of displayed entries
- **Backspace** to delete search characters
- **Enter** to select and output path

## Installation

### Prerequisites
- C++17 compatible compiler (GCC 7+, Clang 5+)
- CMake 3.16+
- ncurses development library

```bash
# Ubuntu/Debian
sudo apt install build-essential cmake libncurses5-dev

# macOS
brew install cmake ncurses

# Arch Linux
sudo pacman -S base-devel cmake ncurses
```

### Build

```bash
git clone https://github.com/Fozzyack/simple-fuzzy-finder
cd simple-fuzzy-finder
mkdir build && cd build
cmake ..
make
```

### Shell Integration

Add to your shell configuration:

**Bash (~/.bashrc):**
```bash
ff() {
    local result=$(~/simple-fuzzy-finder/build/ffcli "$1")
    if [[ "$result" != "No Directory Chosen" ]]; then
        cd "$result"
    fi
}
```

**Fish (~/.config/fish/config.fish):**
```fish
function ff
    set result (~/simple-fuzzy-finder/build/ffcli $argv)
    if test "$result" != "No Directory Chosen"
        cd "$result"
    end
end
```

## Technical Details

### Built With
- **C++17** for performance and modern features
- **ncurses** for terminal user interface
- **std::filesystem** for cross-platform file operations
- **std::thread** for parallel processing
- **CMake** for cross-platform building

### Architecture
- **Multithreaded scanning**: Directory traversal runs in background
- **Parallel search**: Search workload distributed across CPU cores
- **Thread-safe operations**: Mutex protection for shared data structures
- **Memory efficient**: Streaming results without loading entire directory tree

### Performance
- Handles thousands of files efficiently
- Sub-second search times on typical directory structures
- Scales with available CPU cores
- Minimal memory footprint

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

### Development

```bash
# Build in debug mode
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Run with debugging symbols
gdb ./ffcli
```

## Known Issues

- Permission errors may occur when scanning system directories
- Very large directory trees (>100k files) may impact performance
- Unicode filename support varies by terminal
