## Dependnencies

* Drogon
* C++20 capable compiler (>= GCC11 or MSVC 19.16)
* libfmt

I decide to use Drogon because I'm familare with it and easier to use compared to `libcurl`. Furthermore, it provides C++ coroutine support
so I don't have to live with callback hell or blocking API

(Install dep on Arch Linux)

```
yay -S drogon-git --noconfirm
```

## How to build

```
mkdir build
cd build
cmake ..
make
```

### Windows
```
vcpkg install openssl:x64-windows
vcpkg install drogon:x64-windows
vcpkg integrate install

md build
cd build
cmake ..
cmake --build .
```
