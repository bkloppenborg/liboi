
# Compiling liboi with beignet 0.9.1

# Operating system

Have your machine on Ubuntu 14.04 or newer

# Check installation requirements for Beignet. 

I had to install the following packages beyond what liboi requires

    sudo apt-get install clang libclang-3.4-dev libclang-dev libclang1

# Switch Beignet compiliation to specific version of clang

Open up the `beignet/backend/src/CMakeLists.txt` file and 

    replace llvm-as with llvm-as-3.4
    replace llvm-link with llvm-link-3.4

where llvm-as-X.Y matches the version you installed.

# Compile Beignet

    mkdir build
    cd build
    cmake ..
    make
    sudo make install

(you can do make -jN with N > 1 to do a parallel build)
