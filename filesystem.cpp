#include "filesystem.h"
#include <limits>
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <map>
#include <algorithm>
#include <ctime>
#include <string>
#include <vector>
#include <cctype>          // for tolower
using namespace std;

// Helper: current local timestamp “YYYY‑MM‑DD HH:MM:SS”
static string getTimestamp() {
    time_t now = time(nullptr);
    char buf[32]{};
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buf);
}

/*────────────────────────────  File  ───────────────────────────*/
File::File(const string& filename)
    : name(filename), content(""),
      createdAt(getTimestamp()), modifiedAt(createdAt) {}

/*──────────────────────────  Directory  ────────────────────────*/
Directory::Directory(const string& dirName, Directory* par)
    : name(dirName), parent(par) {}

Directory::~Directory() {
    for (auto& d : subDirs) delete d.second;
    for (auto& f : files)   delete f.second;
}

/*──────────────────────────  FileSystem  ───────────────────────*/
FileSystem::FileSystem() {
    root = new Directory("root");
    curr = root;
    loadFromDisk("fs_data.txt");
}

FileSystem::~FileSystem() {
    saveToDisk("fs_data.txt");
    delete root;
}

/*───────────── internal helper used by loadFromDisk ───────────*/
namespace {
    Directory* ensureDir(Directory* root, const string& relPath) {
        Directory* cur = root;
        stringstream ss(relPath);
        string token;
        while (getline(ss, token, '/')) {
            if (token.empty()) continue;
            auto it = cur->subDirs.find(token);
            if (it == cur->subDirs.end()) {
                Directory* neo = new Directory(token, cur);
                cur->subDirs[token] = neo;
                cur = neo;
            } else
                cur = it->second;
        }
        return cur;
    }
}

/*──────────────────────  Persistence  ─────────────────────────*/
void FileSystem::saveToDisk(const string& filename) {
    ofstream out(filename, ios::binary);
    if (!out) return;

    queue<pair<Directory*, string>> q;
    q.push({root, ""});

    while (!q.empty()) {
        auto it = q.front();
        Directory* dir = it.first;
        string path = it.second;
        q.pop();

        for (const auto& d : dir->subDirs) {                   // sub‑dirs
            string child = path + "/" + d.first;
            out << "D|" << child << '\n';
            q.push({d.second, child});
        }
        for (const auto& fp : dir->files) {                    // files
            File* f = fp.second;
            string filePath = path + "/" + f->name;
            out << "F|" << filePath << '|'
                << f->createdAt << '|'
                << f->modifiedAt << '|'
                << f->content.size() << '\n';
            if (!f->content.empty())
                out.write(f->content.data(),
                          static_cast<streamsize>(f->content.size()));
            out << '\n';
        }
    }
}

void FileSystem::loadFromDisk(const string& filename) {
    ifstream in(filename, ios::binary);
    if (!in) return;                                           // first run

    string line;
    while (getline(in, line)) {
        if (line.rfind("D|", 0) == 0) {                        // dir line
            ensureDir(root, line.substr(2));
        } else if (line.rfind("F|", 0) == 0) {                 // file line
            size_t p1 = line.find('|', 2);
            size_t p2 = line.find('|', p1 + 1);
            size_t p3 = line.find('|', p2 + 1);
            string filePath  = line.substr(2, p1 - 2);
            string createdAt = line.substr(p1 + 1, p2 - p1 - 1);
            string modifiedAt= line.substr(p2 + 1, p3 - p2 - 1);
            size_t len       = stoul(line.substr(p3 + 1));

            string content(len, '\0');
            if (len) in.read(&content[0], static_cast<streamsize>(len));
            in.get();                                          // eat '\n'

            size_t lastSlash = filePath.find_last_of('/');
            string dirPart = (lastSlash == string::npos) ? "" : filePath.substr(0, lastSlash);
            string base = filePath.substr(lastSlash + 1);

            Directory* parent = ensureDir(root, dirPart);
            if (parent->files.count(base)) continue;           // already exists
            File* f = new File(base);
            f->createdAt  = createdAt;
            f->modifiedAt = modifiedAt;
            f->content    = move(content);
            parent->files[base] = f;
        }
    }
    curr = root;
}

/*───────────────  Path navigation helper  ─────────────────────*/
Directory* FileSystem::navigateToPath(const string& relPath) {
    if (relPath.empty() || relPath == "/") return root;

    Directory* dir = root;
    stringstream ss(relPath);
    string token;
    while (getline(ss, token, '/')) {
        if (token.empty()) continue;
        auto it = dir->subDirs.find(token);
        if (it == dir->subDirs.end()) return nullptr;          // bad segment
        dir = it->second;
    }
    return dir;
}

vector<string> FileSystem::listAndNumber(const map<string, Directory*> &dirs, const map<string, File*> &fls,bool showDirs) {
    vector<string> names;
    int idx = 1;
    if (showDirs) {
        cout << "DIRECTORIES:" << endl;
        for (auto &d : dirs) {
            cout << "  " << idx++ << ". " << d.first << endl;
            names.push_back(d.first);
        }
        if (names.empty()) cout << "  (NO DIRECTORIES FOUND)" << endl;
    } else {
        cout << "FILES:" << endl;
        for (auto &f : fls) {
            cout << "  " << idx++ << ". " << f.first << endl;
            names.push_back(f.first);
        }
        if (names.empty()) cout << "  (NO FILES FOUND)" << endl;
    }
    return names;
}

string FileSystem::chooseFromList(const vector<string>& names, const string& prompt) {
    if (names.empty()) return "";
    int choice; 
    cout << prompt; 
    cin >> choice; 
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (choice < 1 || choice > (int)names.size()) {
        cout << "INVALID CHOICE." << endl;
        return "";
    }
    return names[choice - 1];
}

void FileSystem::makeDirectory(const string &name) {
    if (curr->subDirs.count(name) || curr->files.count(name)) {
        cout << "NAME ALREADY IN USE." << endl;
    } else {
        Directory* newDir = new Directory(name, curr);
        curr->subDirs[name] = newDir;
        cout << "DIRECTORY CREATED." << endl;
    }
}

void FileSystem::deleteFileByName(const string& name) {
    if(!curr->files.count(name)) {
        cout << "File not found!" << endl;
        return;
    }
    delete curr->files[name];
    curr->files.erase(name);
    cout << "File deleted." << endl;
}

void FileSystem::deleteDirectoryByName(const string& name) {
    if(!curr->subDirs.count(name)) {
        cout << "Directory not found!" << endl;
        return;
    }
    delete curr->subDirs[name];
    curr->subDirs.erase(name);
    cout << "Directory deleted." << endl;
}

void FileSystem::renameDirectory(const string &oldN, const string &newN) {
    if (!curr->subDirs.count(oldN)) { 
        cout << "DIRECTORY NOT FOUND." << endl; 
        return; 
    }
    if (curr->subDirs.count(newN) || curr->files.count(newN)) { 
        cout << "NAME ALREADY EXISTS." << endl; 
        return; 
    }
    curr->subDirs[newN] = curr->subDirs[oldN];
    curr->subDirs[newN]->name = newN;
    curr->subDirs.erase(oldN);
    cout << "DIRECTORY RENAMED." << endl;
}

void FileSystem::changeDirectory() {
    vector<string> names;
    int idx = 1;
    
    cout << "AVAILABLE DIRECTORIES:" << endl;
    for (auto& d : curr->subDirs) {
        cout << "  " << idx++ << ". " << d.first << endl;
        names.push_back(d.first);
    }
    
    if (curr->parent) {
        cout << "  " << idx << ". .. (Parent Directory)" << endl;
        names.push_back("..");
    }

    if (names.empty()) {
        cout << "  (NO DIRECTORIES AVAILABLE)" << endl;
        return;
    }

    cout << "SELECT DIRECTORY NUMBER: ";
    int choice; 
    cin >> choice; 
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (choice < 1 || choice > (int)names.size()) {
        cout << "INVALID SELECTION." << endl;
        return;
    }

    string selected = names[choice-1];
    if (selected == "..") {
        curr = curr->parent;
    } else {
        curr = curr->subDirs[selected];
    }
    cout << "NOW IN: "; printPath();
}

void FileSystem::createFile(const string &name) {
    if (curr->files.count(name) || curr->subDirs.count(name)) { 
        cout << "NAME ALREADY IN USE." << endl; 
        return; 
    }
    File* newFile = new File(name);
    curr->files[name] = newFile;
    cout << "FILE CREATED." << endl;
}

void FileSystem::renameFile(const string &oldN, const string &newN) {
    if (!curr->files.count(oldN)) { 
        cout << "FILE NOT FOUND." << endl; 
        return; 
    }
    if (curr->files.count(newN) || curr->subDirs.count(newN)) { 
        cout << "NAME ALREADY EXISTS." << endl; 
        return; 
    }
    curr->files[newN] = curr->files[oldN];
    curr->files[newN]->name = newN;
    curr->files.erase(oldN);
    cout << "FILE RENAMED." << endl;
}

void FileSystem::writeFile(const string &name, bool append) {
    if (!curr->files.count(name)) {
        cout << "FILE NOT FOUND." << endl;
        return;
    }

    cout << "ENTER CONTENT (END WITH 'EOF' ON NEW LINE):\n";
    string content, line;
    while (getline(cin, line)) {
        if (line == "EOF") break;
        content += line + "\n";
    }

    if (append) {
        curr->files[name]->content += content;
    } else {
        curr->files[name]->content = content;
    }
    curr->files[name]->modifiedAt = getTimestamp();
    cout << "WRITE SUCCESSFUL." << endl;
}

void FileSystem::readFile(const string &name) {
    if (!curr->files.count(name)) { 
        cout << "FILE NOT FOUND." << endl; 
        return; 
    }
    cout << "\n----- FILE CONTENT -----\n" << curr->files[name]->content 
            << "\n------------------------" << endl;
}

void FileSystem::fileMetadata(const string &name) {
    if (!curr->files.count(name)) { 
        cout << "FILE NOT FOUND." << endl; 
        return; 
    }
    File *f = curr->files[name];
    cout << "NAME: " << f->name << "\nCREATED: " << f->createdAt << "\nMODIFIED: " << f->modifiedAt << endl;
}

void FileSystem::directoryMetadata() {
    cout << "\n----- DIRECTORY INFO -----" << endl;
    cout << "NAME: " << curr->name << "\nPATH: "; printPath();
    cout << "SUBDIRECTORIES: " << curr->subDirs.size() << endl;
    cout << "FILES: " << curr->files.size() << endl;
    cout << "-------------------------" << endl;
}

void FileSystem::searchFiles(const string &pattern) {
    cout << "SEARCH RESULTS:" << endl;
    bool found = false;
    
    for (auto& f : curr->files) {
        if (f.first.find(pattern) != string::npos) {
            cout << "  " << f.first << endl;
            found = true;
        }
    }
    
    if (!found) cout << "  (NO MATCHING FILES)" << endl;
}

void FileSystem::batchCreateFiles() {
    cout << "ENTER FILENAMES (SEPARATED BY COMMAS): ";
    string input;
    getline(cin, input);
    
    size_t pos = 0;
    while ((pos = input.find(',')) != string::npos) {
        string filename = input.substr(0, pos);
        createFile(filename);
        input.erase(0, pos + 1);
    }
    if (!input.empty()) createFile(input);
}

void FileSystem::deleteAll() {
    // Confirm with user
    cout << "WARNING: This will delete ALL files and directories. Continue? (y/n): ";
    char confirm;
    cin >> confirm;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
    if (tolower(confirm) != 'y') {
        cout << "Operation cancelled.\n";
        return;
    }

    // Delete everything recursively starting from root
    for (auto& dir : root->subDirs) {
        delete dir.second;
    }
    for (auto& file : root->files) {
        delete file.second;
    }
    root->subDirs.clear();
    root->files.clear();
    
    cout << "All files and directories deleted.\n";
}

void FileSystem::printPath() {
    vector<string> v; 
    Directory *t = curr;
    while (t) { v.push_back(t->name); t = t->parent; }
    for (int i = v.size() - 1; i >= 0; --i) cout << '/' << v[i];
    cout << endl;
}

void FileSystem::showHelp() {
    cout << "\n===== HELP =====\n"
            << "1. PATHS: Use numbers to navigate\n"
            << "2. FILES: Create before writing\n"
            << "3. CONTENT: Use 'EOF' to end input\n"
            << "4. SEARCH: Partial names work\n"
            << "5. BATCH: Create multiple files\n"
            << "===============\n";
}

void FileSystem::printTreeHelper(Directory* dir, int depth) {
    for (int i = 0; i < depth; ++i) cout << "  ";
    cout << "+ " << dir->name << "/" << endl;
    for (const auto& filePair : dir->files) {
        for (int i = 0; i < depth + 1; ++i) cout << "  ";
        cout << "- " << filePair.first << endl;
    }
    for (const auto& subdirPair : dir->subDirs) {
        printTreeHelper(subdirPair.second, depth + 1);
    }
}

void FileSystem::printTree() {
    cout << "\n===== FILE SYSTEM TREE =====" << endl;
    printTreeHelper(root, 0);
    cout << "===========================\n" << endl;
}

void FileSystem::moveFile(const string& name, Directory* target) {
    if (!curr->files.count(name)) {
        cout << "FILE NOT FOUND." << endl;
        return;
    }
    if (target->files.count(name) || target->subDirs.count(name)) {
        cout << "TARGET ALREADY HAS AN ITEM WITH THIS NAME." << endl;
        return;
    }
    target->files[name] = curr->files[name];
    curr->files.erase(name);
    cout << "FILE MOVED." << endl;
}

void FileSystem::moveDirectory(const string& name, Directory* target) {
    if (!curr->subDirs.count(name)) {
        cout << "DIRECTORY NOT FOUND." << endl;
        return;
    }
    if (target->subDirs.count(name) || target->files.count(name)) {
        cout << "TARGET ALREADY HAS AN ITEM WITH THIS NAME." << endl;
        return;
    }
    target->subDirs[name] = curr->subDirs[name];
    curr->subDirs[name]->parent = target;
    curr->subDirs.erase(name);
    cout << "DIRECTORY MOVED." << endl;
}

void FileSystem::copyFile(const string& name, Directory* target) {
    if (!curr->files.count(name)) {
        cout << "FILE NOT FOUND." << endl;
        return;
    }
    if (target->files.count(name) || target->subDirs.count(name)) {
        cout << "TARGET ALREADY HAS AN ITEM WITH THIS NAME." << endl;
        return;
    }
    File* orig = curr->files[name];
    File* copy = new File(orig->name);
    copy->content = orig->content;
    copy->createdAt = getTimestamp();
    copy->modifiedAt = copy->createdAt;
    target->files[name] = copy;
    cout << "FILE COPIED." << endl;
}

void FileSystem::copyDirectoryHelper(Directory* orig, Directory* target) {
    Directory* copy = new Directory(orig->name, target);
    for (auto& f : orig->files) {
        File* fcopy = new File(f.second->name);
        fcopy->content = f.second->content;
        fcopy->createdAt = getTimestamp();
        fcopy->modifiedAt = fcopy->createdAt;
        copy->files[f.first] = fcopy;
    }
    for (auto& d : orig->subDirs) {
        copyDirectoryHelper(d.second, copy);
    }
    target->subDirs[orig->name] = copy;
}

void FileSystem::copyDirectory(const string& name, Directory* target) {
    if (!curr->subDirs.count(name)) {
        cout << "DIRECTORY NOT FOUND." << endl;
        return;
    }
    if (target->subDirs.count(name) || target->files.count(name)) {
        cout << "TARGET ALREADY HAS AN ITEM WITH THIS NAME." << endl;
        return;
    }
    Directory* orig = curr->subDirs[name];
    copyDirectoryHelper(orig, target);
    cout << "DIRECTORY COPIED." << endl;
}

void FileSystem::mainMenu() {
    cout << "\n============= MAIN MENU =============" << endl
            << "1. CONTENT OPERATIONS" << endl
            << "2. SEARCH TOOLS" << endl
            << "3. BATCH OPERATIONS" << endl
            << "4. HELP" << endl
            << "5. DELETE EVERYTHING" << endl
            << "6. SHOW TREE VIEW" << endl
            << "7. EXIT" << endl
            << "=====================================" << endl;
}

void FileSystem::showContentMenu() {
    cout << "\nCONTENT MENU:" << endl
        << " 1. CHANGE DIRECTORY" << endl
        << " 2. LIST CONTENTS (WITH INFO)" << endl
        << " 3. CREATE (DIRECTORY/FILE)" << endl
        << " 4. DELETE (DIRECTORY/FILE)" << endl
        << " 5. RENAME (DIRECTORY/FILE)" << endl
        << " 6. EDIT FILE (OVERWRITE/APPEND)" << endl
        << " 7. READ FILE" << endl
        << " 8. MOVE (DIRECTORY/FILE)" << endl
        << " 9. COPY (DIRECTORY/FILE)" << endl
        << "10. RETURN" << endl;
}

void FileSystem::contentOps() {
    int c;
    while (true) {
        showContentMenu();
        cout << "CONTENT CHOICE: ";
        if (!(cin >> c)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "INVALID INPUT. Please enter 1-10.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (c == 1) changeDirectory();
        else if (c == 2) {
            // List directories and files with info
            cout << "\nDIRECTORIES:" << endl;
            int idx = 1;
            for (auto& d : curr->subDirs) {
                cout << "  " << idx++ << ". " << d.first
                     << " [Subdirs: " << d.second->subDirs.size()
                     << ", Files: " << d.second->files.size() << "]" << endl;
            }
            if (curr->subDirs.empty()) cout << "  (NO DIRECTORIES FOUND)" << endl;

            cout << "\nFILES:" << endl;
            idx = 1;
            for (auto& f : curr->files) {
                cout << "  " << idx++ << ". " << f.first
                     << " [Created: " << f.second->createdAt
                     << ", Modified: " << f.second->modifiedAt << "]" << endl;
            }
            if (curr->files.empty()) cout << "  (NO FILES FOUND)" << endl;
        }
        else if (c == 3) {
            cout << "CREATE (1) Directory or (2) File? ";
            int t; cin >> t; cin.ignore();
            string n;
            cout << "ENTER NAME: ";
            getline(cin, n);
            if (t == 1) makeDirectory(n);
            else if (t == 2) createFile(n);
            else cout << "INVALID TYPE.\n";
        }
        else if (c == 4) {
            cout << "DELETE (1) Directory or (2) File? ";
            int t; cin >> t; cin.ignore();
            if (t == 1) {
                auto names = listAndNumber(curr->subDirs, curr->files, true);
                string sel = chooseFromList(names, "SELECT DIR NUMBER TO DELETE: ");
                if (!sel.empty()) deleteDirectoryByName(sel);
            } else if (t == 2) {
                auto names = listAndNumber(curr->subDirs, curr->files, false);
                string sel = chooseFromList(names, "SELECT FILE NUMBER TO DELETE: ");
                if (!sel.empty()) deleteFileByName(sel);
            } else cout << "INVALID TYPE.\n";
        }
        else if (c == 5) {
            cout << "RENAME (1) Directory or (2) File? ";
            int t; cin >> t; cin.ignore();
            if (t == 1) {
                auto names = listAndNumber(curr->subDirs, curr->files, true);
                string oldN = chooseFromList(names, "SELECT DIR NUMBER TO RENAME: ");
                if (oldN.empty()) continue;
                string newN;
                cout << "ENTER NEW NAME: ";
                getline(cin, newN);
                renameDirectory(oldN, newN);
            } else if (t == 2) {
                auto names = listAndNumber(curr->subDirs, curr->files, false);
                string oldN = chooseFromList(names, "SELECT FILE NUMBER TO RENAME: ");
                if (oldN.empty()) continue;
                string newN;
                cout << "ENTER NEW NAME: ";
                getline(cin, newN);
                renameFile(oldN, newN);
            } else cout << "INVALID TYPE.\n";
        }
        else if (c == 6) {
            auto names = listAndNumber(curr->subDirs, curr->files, false);
            string sel = chooseFromList(names, "SELECT FILE TO EDIT: ");
            if (sel.empty()) continue;
            cout << "EDIT MODE: (1) Overwrite, (2) Append? ";
            int mode; cin >> mode; cin.ignore();
            writeFile(sel, mode == 2);
        }
        else if (c == 7) {
            auto names = listAndNumber(curr->subDirs, curr->files, false);
            string sel = chooseFromList(names, "SELECT FILE TO READ: ");
            if (!sel.empty()) readFile(sel);
        }
        else if (c == 8) {
            cout << "MOVE (1) Directory or (2) File? ";
            int t; cin >> t; cin.ignore();
            if (t == 1) {
                auto names = listAndNumber(curr->subDirs, curr->files, true);
                string sel = chooseFromList(names, "SELECT DIR NUMBER TO MOVE: ");
                if (sel.empty()) continue;
                cout << "ENTER TARGET PATH (e.g. path after root/): ";
                string path; getline(cin, path);
                Directory* target = navigateToPath(path);
                if (!target) { cout << "INVALID PATH.\n"; continue; }
                moveDirectory(sel, target);
            } else if (t == 2) {
                auto names = listAndNumber(curr->subDirs, curr->files, false);
                string sel = chooseFromList(names, "SELECT FILE NUMBER TO MOVE: ");
                if (sel.empty()) continue;
                cout << "ENTER TARGET PATH (e.g. path after root/): ";
                string path; getline(cin, path);
                Directory* target = navigateToPath(path);
                if (!target) { cout << "INVALID PATH.\n"; continue; }
                moveFile(sel, target);
            } else cout << "INVALID TYPE.\n";
        }
        else if (c == 9) {
            cout << "COPY (1) Directory or (2) File? ";
            int t; cin >> t; cin.ignore();
            if (t == 1) {
                auto names = listAndNumber(curr->subDirs, curr->files, true);
                string sel = chooseFromList(names, "SELECT DIR NUMBER TO COPY: ");
                if (sel.empty()) continue;
                cout << "ENTER TARGET PATH (e.g. path after root/): ";
                string path; getline(cin, path);
                Directory* target = navigateToPath(path);
                if (!target) { cout << "INVALID PATH.\n"; continue; }
                copyDirectory(sel, target);
            } else if (t == 2) {
                auto names = listAndNumber(curr->subDirs, curr->files, false);
                string sel = chooseFromList(names, "SELECT FILE NUMBER TO COPY: ");
                if (sel.empty()) continue;
                cout << "ENTER TARGET PATH (path after root/): ";
                string path; getline(cin, path);
                Directory* target = navigateToPath(path);
                if (!target) { cout << "INVALID PATH.\n"; continue; }
                copyFile(sel, target);
            } else cout << "INVALID TYPE.\n";
        }
        else if (c == 10) break;
        else cout << "INVALID." << endl;
    }
}

void FileSystem::searchMenu() {
    cout << "\nSEARCH MENU:" << endl
            << " 1. SEARCH FILES" << endl
            << " 2. RETURN" << endl;
}

void FileSystem::batchMenu() {
    cout << "\nBATCH MENU:" << endl
            << " 1. CREATE MULTIPLE FILES" << endl
            << " 2. RETURN" << endl;
}

void FileSystem::start() {
    int ch;
    while (true) {
        mainMenu(); 
        cout << "ENTER CHOICE: "; 
        if (!(cin >> ch)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "INVALID INPUT. Please enter a number 1-7.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (ch == 1) contentOps();
        else if (ch == 2) searchOps();
        else if (ch == 3) batchOps();
        else if (ch == 4) showHelp();
        else if (ch == 5) deleteAll();
        else if (ch == 6) printTree();
        else if (ch == 7) {
            saveToDisk("fs_data.txt");
            cout << "GOODBYE!" << endl; break; 
        }
        else cout << "INVALID." << endl;
    }
}

void FileSystem::searchOps() {
    int s;
    while (true) {
        searchMenu();
        cout << "SEARCH CHOICE: ";
        if (!(cin >> s)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "INVALID INPUT. Please enter 1-2.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        if (s == 1) {
            string pattern;
            cout << "ENTER SEARCH PATTERN: ";
            getline(cin, pattern);
            searchFiles(pattern);
        }
        else if (s == 2) break;
        else cout << "INVALID." << endl;
    }
}

void FileSystem::batchOps() {
    int b;
    while (true) {
        batchMenu();
        cout << "BATCH CHOICE: ";
        if (!(cin >> b)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "INVALID INPUT. Please enter 1-2.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        if (b == 1) batchCreateFiles();
        else if (b == 2) break;
        else cout << "INVALID." << endl;
    }
}

void FileSystem::listContents(bool showDirectories) {
    if (showDirectories) {
        listAndNumber(curr->subDirs, curr->files, true);
    }
    listAndNumber(curr->subDirs, curr->files, false);
}
