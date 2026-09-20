# SimCourse 
Все команды ниже выполняются из корня `simCourse`.

## Что нужно

- CMake 3.24 или новее и компилятор с поддержкой C++17.
- Make для Linux-команд ниже или Ninja для варианта с Windows.
- При первой настройке с тестами — интернет: CMake скачивает GoogleTest 1.17.0.

После клонирования загрузить подмодули:

```sh
git submodule update --init --recursive
```

## Linux: основная сборка

```sh
# Настроить сборку (первый раз или после изменения настроек).
cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON

# Собрать программу и тесты; повторять после изменения исходников.
cmake --build build --parallel

# Запустить все тесты.
ctest --test-dir build --output-on-failure

# Запустить симулятор с файлом машинных инструкций.
./build/simcourse ./program.bin
```
## Threaded сборка
```sh
# Настроить сборку (первый раз или после изменения настроек).
cmake -S . -B build     -DCMAKE_CXX_COMPILER=clang++     -DCMAKE_BUILD_TYPE=Release     -DTHREADED_DISPATCH=ON
