# Group 16 - CS3307
## bundl.

**bundl.** is a simple arcade hub built using **C++** and **wxWidgets**. 

It provides a graphical interface that allows users to launch multiple games from a single application. The launcher serves as the central interface for the arcade and allows users to select and run games through a GUI menu.

---

## Included Games

The arcade currently includes the following games:

- Snake
- Connect Four
- Anagrams

Each game exists as its own program in the repository and can also be compiled independently.

The **bundl.** launcher automatically compiles and launches a game if it has not yet been built. Each game directory contains its own source files and Makefile.

---

## Repository Structure

```
group16/             # Repository root folder downloaded/cloned from GitLab
├── bundlApp/        # Main arcade launcher
│   └── Makefile
│
├── Snake/           # Snake game
│   └── Makefile
│
├── ConnectFour/     # Connect Four game
│   └── Makefile
│
├── Anagrams/        # Anagrams game
│   └── Makefile
│
└── README.md
```

---

## Requirements

The project requires the following software to be installed.

### C++ Compiler

A compiler supporting **C++17** is required.

Verify installation:
```bash
g++ --version
```

### wxWidgets
The graphical user interface is built using wxWidgets.

Verify installation:
```bash
wx-config --version
```
If wxWidgets is not installed, install it using one of the following methods.

macOS:
```bash
brew install wxwidgets
```
Ubuntu / Linux:
```bash
sudo apt install libwxgtk3.2-dev
```

---

## Building & Running the Launcher
Navigate to the launcher directory:
```bash
cd bundlApp
```
Compile the launcher:
```bash
make
```
This produces the executable `bundl`.\
Run the launcher from **inside the** `bundlApp` **directory**:
```bash
./bundl
```
The GUI menu will appear, allowing users to:
1. Launch **Snake**
2. Launch **Connect Four**
3. Launch **Anagrams**
4. Open **Settings**
5. **Exit** the application

### Building Games Individually (Optional)

Each game can also be built independently.

#### If you are currently in the `group16` directory:
Run **Snake**:
```
cd Snake
make
./SnakeApp
```

Run **Connect Four**:
```
cd ConnectFour
make
./ConnectFourApp
```

Run **Anagrams**:
```
cd Anagrams
make
./AnagramsApp
```

#### If you are currently in the `bundlApp` directory:
Run **Snake**:
```
cd ../Snake
make
./SnakeApp
```

Run **Connect Four**:
```
cd ../ConnectFour
make
./ConnectFourApp
```

Run **Anagrams**:
```
cd ../Anagrams
make
./AnagramsApp
```
