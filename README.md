# SecInfer
Implementation for the protocols described in IEEE ICC 2025: `SecInfer: Secure and Efficient Model Inference on Vertically Partitioned Data`.

Our work is based on the preceding frameworks [MOTION](https://github.com/encryptogroup/MOTION) and [ScalableMixedModeMPC](https://github.com/radhika1601/ScalableMixedModeMPC/tree/master).

### Requirements

------

- A **Linux distribution** of your choice (developed and tested with recent versions of [Ubuntu](http://www.ubuntu.com/), [Manjaro](https://manjaro.org/) and [Arch Linux](https://www.archlinux.org/)).
- Required packages:
  - `g++` (version >=10) or another compiler and standard library implementing C++20 including the filesystem library
  - `make`
  - `cmake`
  - [`boost`](https://www.boost.org/) (version >=1.75.0)
  - `OpenMP`
  - [`OpenSSL`](https://www.openssl.org/) (version >=1.1.0)

#### Further Dependencies

1. ```
   wget https://raw.githubusercontent.com/emp-toolkit/emp-readme/master/scripts/install.py
   ```

2. ```
   python install.py -install -tool -ot
   ```

   1. By default it will build for Release. `-DCMAKE_BUILD_TYPE=[Release|Debug]` option is also available.
   2. No sudo? Change [`CMAKE_INSTALL_PREFIX`](https://cmake.org/cmake/help/v2.8.8/cmake.html#variable%3aCMAKE_INSTALL_PREFIX).
   3. On Mac [homebrew](https://brew.sh/) is needed for installation.

3. Install openFHE.

   ```ssh
   git clone https://github.com/openfheorg/openfhe-development.git --branch v1.0.4
   cd openfhe-development && mkdir build && cd build
   cmake .. && make -j8 && sudo make install
   ```

### Building SecInfer

1. Clone the base framework MOTION git repository by running:

   ```ssh
   git clone https://github.com/encryptogroup/MOTION.git
   ```

2. Enter the Framework directory: `cd MOTION/`, then replace `./CMakeLists.txt` with `SecInfer/CMakeLists.txt`.

3. Enter the `extern/` directory and install ScalableMixedModeMPC.

   ```ssh
   git clone https://github.com/radhika1601/ScalableMixedModeMPC.git
   ```

4. Install the dependencies of ScalableMixedModeMPC.

   1. ```
      wget https://raw.githubusercontent.com/emp-toolkit/emp-readme/master/scripts/install.py
      ```

   2. ```
      python install.py -install -tool -ot
      ```

   3. Install openFHE

      ```
      git clone https://github.com/openfheorg/openfhe-development.git --branch v1.0.4
      cd openfhe-development && mkdir build && cd build
      cmake .. && make -j8 && sudo make install
      ```

5. Replace `extern/ScalableMixedModeMPC/CMakeLists.txt` with `SecInfer/ScalableMixedModeMPC/CMakeLists.txt`.

6. Enter the ScalableMixedModeMPC Framework directory: `cd extern/ScalableMixedModeMPC/`, and build ScalableMixedModeMPC using cmake.

   ```
   cmake .
   make
   ```

7. Replace `src/` with the files in the directory `SecInfer/src/`.

8. Enter the root Framework directory and build the whole file using cmake.

   ```
   cmake .
   make
   ```

### Configurations

You can run the following commands before you run the tests.

```
chmod +x throttle.sh
./throttle.sh <option>
```

\<options\> can be 'del', 'wan' or 'lan'. 

### Running tests

```
chmod +x throttle.sh
./run_tests.sh
```

