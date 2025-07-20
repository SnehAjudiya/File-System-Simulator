#pragma once
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <ctime>
#include <queue>
#include <fstream>
#include <sstream>

struct File {
    std::string name;
    std::string content;
    std::string createdAt;
    std::string modifiedAt;
    File(const std::string& filename);
};

class Directory {
public:
    std::string name;
    Directory* parent;
    std::map<std::string, Directory*> subDirs;
    std::map<std::string, File*> files;
    Directory(const std::string& dirName, Directory* par = nullptr);
    ~Directory();
};

class FileSystem {
private:
    Directory* root;
    Directory* curr;

    // ── Persistence ──────────────────────────────────────────────
    void saveToDisk(const std::string& filename);
    void loadFromDisk(const std::string& filename);

    // ── Path helper ───────────────────────────────────────────────
    Directory* navigateToPath(const std::string& relPath);

    // ── Core operations ───────────────────────────────────────────
    std::vector<std::string> listAndNumber(const std::map<std::string, Directory*>& dirs, const std::map<std::string, File*>& fls, bool showDirs = true);
    std::string chooseFromList(const std::vector<std::string>& names, const std::string& prompt);

    void makeDirectory(const std::string& name);
    void deleteDirectoryByName(const std::string& name);
    void renameDirectory(const std::string& oldN, const std::string& newN);
    void changeDirectory();
    void createFile(const std::string& name);
    void deleteFileByName(const std::string& name);
    void renameFile(const std::string& oldN, const std::string& newN);
    void writeFile(const std::string& name, bool append);
    void readFile(const std::string& name);
    void fileMetadata(const std::string& name);
    void directoryMetadata();
    void searchFiles(const std::string& pattern);
    void batchCreateFiles();
    void printPath();
    void showHelp();
    void listContents(bool showDirectories = true);
    void deleteAll();
    void printTree();
    void printTreeHelper(Directory* dir, int depth);

    // Menu handlers
    void searchOps();
    void batchOps();
    void mainMenu();
    void searchMenu();
    void batchMenu();
    void showContentMenu();
    void contentOps();

    // Move / copy helpers
    void moveFile(const std::string& name, Directory* target);
    void moveDirectory(const std::string& name, Directory* target);
    void copyFile(const std::string& name, Directory* target);
    void copyDirectory(const std::string& name, Directory* target);
    void copyDirectoryHelper(Directory* orig, Directory* target);

public:
    FileSystem();
    ~FileSystem();
    void start();
};
