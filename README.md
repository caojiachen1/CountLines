# CountLines

A high-performance CLI tool for counting lines of code in project directories, written in C with cross-platform CMake support. Now with a beautiful web visualization interface!

## Features

- **Fast Performance**: Optimized C implementation for rapid file processing
- **Cross-Platform**: Uses CMake for seamless builds on Windows, Linux, and macOS
- **Web Interface**: Modern, responsive web UI for visualizing code statistics
- **Smart File Detection**: Automatically detects text-based source files
- **Folder Exclusion**: Powerful pattern-based exclusion system
- **Detailed Analysis**: Breaks down lines into code, comments, and blank lines
- **Wide Language Support**: Supports 50+ programming languages and file types

## Supported File Types

C/C++, Java, JavaScript, TypeScript, Python, Ruby, PHP, Go, Rust, C#, Visual Basic, F#, Swift, Kotlin, Scala, Clojure, Haskell, HTML, CSS, SCSS, JSON, YAML, Shell scripts, and many more.

## Building

### Prerequisites

- CMake 3.10 or higher
- A C99-compatible compiler (GCC, Clang, MSVC)

### Build Instructions (Linux/macOS)

```bash
# Clone the repository
git clone https://github.com/caojiachen1/CountLines.git
cd CountLines

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make

# The executable will be in build/bin/countlines
```

### Windows (Using BAT Script)

For Windows users, a convenient batch script is provided:

```cmd
REM Simply run the build script
build.bat

REM The executable will be in build\bin\countlines.exe or build\bin\Release\countlines.exe
```

### Windows (Manual Build)

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
```

## Usage

### Web Interface Mode

Start the web server to use the visual interface:

```bash
# Start web server on default port (8080)
./countlines --web

# Start web server on custom port
./countlines --web 3000
```

Then open your browser to `http://localhost:8080` (or your custom port) to access the web interface.

![Web Interface](https://github.com/user-attachments/assets/1c09c487-5554-4174-a0e8-b7899917998c)
![Results Display](https://github.com/user-attachments/assets/d08e883f-d933-4b92-97a3-b4bf02a9b684)

### Command Line Mode

### Basic Usage

```bash
# Count lines in current directory
./countlines .

# Count lines in specific directory
./countlines /path/to/project
```

### Excluding Directories

```bash
# Exclude single directory
./countlines -e node_modules /path/to/project

# Exclude multiple directories
./countlines -e node_modules -e build -e dist /path/to/project

# Using long form
./countlines --exclude=node_modules --exclude=.git /path/to/project
```

### Default Exclusions

The tool automatically excludes common directories:
- `.git`, `.svn`, `.hg` (version control)
- `node_modules` (Node.js dependencies)
- `__pycache__` (Python cache)
- `.vs`, `.vscode` (IDE files)

### Command Line Options

- `-e, --exclude DIR`: Exclude directory or pattern (can be used multiple times)
- `-h, --help`: Show help message
- `-v, --version`: Show version information
- `-w, --web [PORT]`: Start web server mode (default port: 8080)

## Example Output (CLI Mode)

```
Counting lines in: /path/to/project
Excluding patterns: .git, node_modules, build
Processing...

=== Code Line Count Results ===
Target: /path/to/project
Files processed: 245
Total lines: 15,432
Code lines: 11,234
Comment lines: 2,156
Blank lines: 2,042

Breakdown:
Code:     72.8%
Comments: 14.0%
Blank:    13.2%

Processing completed in 0.045 seconds
```

## Web Interface Features

The web interface provides:

- **Visual Dashboard**: Beautiful cards showing key statistics
- **Interactive Charts**: Bar charts displaying code breakdown
- **Real-time Processing**: Instant results via AJAX API calls
- **Responsive Design**: Works on desktop, tablet, and mobile devices
- **Customizable Paths**: Analyze any directory accessible to the server
- **Exclude Patterns**: Support for excluding directories via the UI

## Performance Features

- **Efficient File Traversal**: Uses platform-native directory APIs
- **Smart File Filtering**: Only processes known text file types
- **Optimized Line Counting**: Fast character-by-character processing
- **Memory Efficient**: Processes files one at a time without loading entire contents
- **Compiler Optimizations**: Built with `-O3` optimization flags

## Algorithm Details

The tool uses several optimizations for maximum performance:

1. **Platform-specific directory traversal** using Windows FindFirstFile/FindNextFile or POSIX opendir/readdir
2. **File type detection** based on file extensions to avoid processing binary files
3. **Efficient line counting** with single-pass character processing
4. **Comment detection** for accurate code vs. comment line classification
5. **Pattern-based exclusion** using simple string matching for fast filtering

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test on multiple platforms
5. Submit a pull request

## Platform Support

- **Linux**: Tested on Ubuntu, CentOS, Alpine
- **macOS**: Tested on macOS 10.15+
- **Windows**: Tested on Windows 10/11 with MSVC and MinGW