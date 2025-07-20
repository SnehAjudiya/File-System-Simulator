
# 📁 File System Simulator in C++

A terminal-based File System Simulator written in C++ that mimics basic file system operations like creating/deleting files and folders, moving/copying items, viewing metadata, and persistent storage.

---

## 🛠 Features

- Hierarchical Directory and File structure
- File operations: Create, Read, Write, Rename, Delete, Append
- Directory operations: Create, Rename, Delete, Navigate
- Persistent storage in `fs_data.txt`
- Tree visualization of the file system
- Batch file creation
- Search files by name
- CLI-based menu navigation
- Interactive and beginner-friendly

---

## 📁 Project Structure

```
.
├── filesystem.cpp         # Core functionality
├── filesystem.h           # Class declarations
├── main.cpp               # Entry point
├── fs_data.txt            # Persistent storage (auto-generated)
```

---

## 🚀 Getting Started

### 🔧 Compile

```bash
g++ main.cpp filesystem.cpp -o filesystem
```

### ▶️ Run

```bash
./filesystem
```

---

## 🧠 How It Works

### 🧷 Entry Point

```cpp
int main() {
    FileSystem fs;
    fs.start();
    return 0;
}
```

### 🧠 Class Design

#### File
```cpp
struct File {
    std::string name;
    std::string content;
    std::string createdAt;
    std::string modifiedAt;
    File(const std::string& filename);
};
```

#### Directory
```cpp
class Directory {
public:
    std::string name;
    Directory* parent;
    std::map<std::string, Directory*> subDirs;
    std::map<std::string, File*> files;
    Directory(const std::string& dirName, Directory* par = nullptr);
    ~Directory();
};
```

#### FileSystem
- Wraps and manages the entire simulation
- Handles I/O, navigation, operations, and menus

---

## 📚 Core Functionalities

### 📁 Directory Management

#### Create Directory
```cpp
void FileSystem::makeDirectory(const std::string& name);
```

#### Change Directory
```cpp
void FileSystem::changeDirectory();
```

#### Delete Directory
```cpp
void FileSystem::deleteDirectoryByName(const std::string& name);
```

#### Rename Directory
```cpp
void FileSystem::renameDirectory(const std::string& oldN, const std::string& newN);
```

### 📄 File Management

#### Create File
```cpp
void FileSystem::createFile(const std::string& name);
```

#### Write / Append to File
```cpp
void FileSystem::writeFile(const std::string& name, bool append);
```

#### Read File
```cpp
void FileSystem::readFile(const std::string& name);
```

#### Rename File
```cpp
void FileSystem::renameFile(const std::string& oldN, const std::string& newN);
```

#### Delete File
```cpp
void FileSystem::deleteFileByName(const std::string& name);
```

### 🔍 Search & Batch

#### Search Files
```cpp
void FileSystem::searchFiles(const std::string& pattern);
```

#### Batch Create
```cpp
void FileSystem::batchCreateFiles();
```

### 📂 Move & Copy

#### Move File or Directory
```cpp
void FileSystem::moveFile(...);
void FileSystem::moveDirectory(...);
```

#### Copy File or Directory
```cpp
void FileSystem::copyFile(...);
void FileSystem::copyDirectory(...);
```

---

## 🌲 Tree View Display

```cpp
void FileSystem::printTree();
```

Example Output:

```
+ root/
  + DOCUMENTS/
    - tasks.txt
    - notes.txt
  + ARCHIVED/
    + LASTYEAR/
      - result.txt
```

---

## 💾 Data Persistence

- Automatically saves and restores from `fs_data.txt`.
- Used on start and before exit.

```cpp
void FileSystem::saveToDisk(const std::string& filename);
void FileSystem::loadFromDisk(const std::string& filename);
```

---

## 🧪 Sample CLI Output

```
============= MAIN MENU =============
1. CONTENT OPERATIONS
2. SEARCH TOOLS
3. BATCH OPERATIONS
4. HELP
5. DELETE EVERYTHING
6. SHOW TREE VIEW
7. EXIT
=====================================
ENTER CHOICE:
```

---

## ✅ Skills Demonstrated

- **Object-Oriented Programming (OOP)**: Strong design using classes like `File`, `Directory`, and `FileSystem` to manage relationships and encapsulate behavior.
- **C++ Standard Library (STL)**: Efficient use of `map`, `vector`, `queue`, `string`, and `sstream` to implement dynamic structures and CLI tools.
- **Dynamic Memory Management**: Manual allocation and cleanup of directory and file objects using pointers and destructors.
- **File I/O & Data Persistence**: Real-time saving and loading of the entire file system using binary and formatted file writing.
- **Interactive Command-Line Interface (CLI)**: Menu-driven system that handles navigation, selection, and input validation robustly.
- **Recursive Algorithms**: For directory tree traversal, copying, and printing, simulating a real file system structure.
- **Modular Software Design**: Clean separation of interface (`.h`) and implementation (`.cpp`), with logical grouping of operations.

---

## 🚀 Future Improvements

- **Trash & Restore System**: Soft-deletion with a `.trash` directory to allow file recovery.
- **Multi-User Environment**: Add login and user-based separation for personalized file systems.
- **File Versioning**: Automatically keep history of changes for each file with timestamps.
- **CLI Autocomplete or Command Mode**: Improve CLI experience with command suggestions and shortcuts.
- **Compression Support**: Allow archiving and extraction of folders/files into `.zip` or custom formats.
- **GUI Version**: Extend project using Qt or ImGui to provide a visual interface for the simulator.
- **Access Control**: Implement file and directory permissions (read/write/execute) for role-based access.

---

## 👨‍💻 Author

**Sneh Ajudiya**  
> IIT Hyderabad

---
