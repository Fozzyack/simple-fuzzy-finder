# CLI Fuzzy Finder

## Problem

Where are my files, why do I have to ```cd``` into everything. Wouldn't it be nice to just fuzzy find my files. I need a
new project to complete

## Discovery

- Turns out you cannot get the user to directly cd into a directory through a C++ program. This needs to be installed
- You do need to tamper with your shell in order to run this. Might add to the dotman installer to help with the
  installation of tools.

## Solution

Create a simple fuzzy finder for your files.

## How it's going to work

Currently, it should read all the files in the current directory (or a directory provided). Then, it should allow the
user to search through all of the files and move into (```cd```) into that directory. Might make it look nice but idk
yet.

## Tech

Will be using C++. For the speed. Might try to add multithreading for no the memes.
