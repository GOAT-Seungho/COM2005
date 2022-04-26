# Compile methods

- Environments : Mac OS Monterey 12.3.1 (Arm64)
- GCC : Apple clang version 13.1.6

## How to compile OpenMP

- Install llvm, libomp
```
brew install llvm
brew install libomp
```

- Set PATH
```
echo 'export PATH="/opt/homebrew/opt/llvm/bin:$PATH"' >> ~/.zshrc
export LDFLAGS="-L/opt/homebrew/opt/llvm/lib"
export CPPFLAGS="-I/opt/homebrew/opt/llvm/include"
```

- Compile
```
/opt/homebrew/opt/llvm/bin/clang $LDFLAGS -Xpreprocessor -fopenmp -lomp [filename].c
```

## How to compile POSIX Threads (PThread)
- Compile
```
gcc [filename].c -lpthread
```
