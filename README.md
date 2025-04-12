# CLI Fuzzy Finder

![Poor quality gif](./demo.gif)

(Apologies this Gif is very low quality... will upload a better one in the future)

## The Problem

Where are my files? why do I have to ```cd``` into everything? Wouldn't it be nice to just fuzzy find my files?

This small projects aims to solve this problem by creating a fuzzy finder for the easy navigation of the Linux
directory.

## How it Works

- Make a scoring algorithm for files
- The highest scores should be the files we are looking for
- Split the directory files into batches for multithreading
- Make it look pretty

## Scoring Algorithm

- Levenshtein did not work as intended and took too long to run.
- Moved to a scoring algorithm in which directory path names get reward based on different qualities.
    - Is the char included in the directory path
    - Detection of concurrent chars
    - Does the char exist after a separator/delimeter
    - Penalties apply for longer directories to ensure the smallest directory is returned first.

## Installation

To test this clone the git repository, and run the shell script. 

``` bash
git clone https://github.com/Fozzyack/simple-fuzzy-finder
cd simple-fuzzy-finder/build
cmake ..
source ff.sh -s 
# If using the fish shell please use ff.fish -s
```

To make it permanent add this to your bashrc

```bash
ff() {
    source "/path/to/ff.sh $1
}
```

## Tech 

Will be using C++. For the speed. Might try to add multithreading for the memes.
- Turns out multithreading was necessary.

###Implementation

- [x] Implement __ncurses__
- [x] Create new terminal Screen instead of using stdout
- [x] Get starting path using the C++ __FILESYSTEM__
- [x] Recursively get all other paths
- [x] Use multithreading to set up terminal and get directories simultaneously
- [x] Set up loop which runs on keystroke 
- [x] Enable function keys, arrows and scroll 
- [x] Split directories into chunks for __MULTITHREADING__
- [x] Use __MUTEX__ and locks to deal with raceconditions when fuzzy finding
- [x] Design Fuzzy Finding Scoring Algorithm
- [x] Implement Algorithm 
- [x] Set up Display 
- [x] Add Colour to show user search
- [x] Add icons to search

## Further Potential Optimisation

- Thread scores need to get sorted from highest to lowest (Might be able to do further thread optimisation)
- There is no point in showing thousands of results for a search. Might be able to lower the resulting thread item
  count.
