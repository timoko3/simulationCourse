# SimCourse

Учебный функциональный симулятор собственной **TIM ISA**, написанный на C++17.
Он моделирует выполнение инструкций, состояние регистров и память, загружает
программы из ELF и исполняет их до завершающего syscall.

Инструкции декодируются в базовые блоки и сохраняются в LIRS-кэше. Для исполнения
реализованы два варианта диспетчеризации:

- **Switch** — цикл по инструкциям блока с выбором операции через `switch`.
- **Threaded** — цепочка обработчиков с хвостовыми переходами `clang::musttail`.

Оба варианта используют общие реализации операций. Программы для TIM создаются
через Ruby DSL: инструкции, метки и данные описываются в Ruby, а генератор
собирает готовый ELF. Пример — [genFibonacci.rb](genFibonacci.rb).

## Сборка через CMake

Команды выполняются из корня проекта в Linux или WSL. Нужны **CMake 3.24+**,
**Ninja**, компилятор **C++17** и **Ruby** для генерации программ.
Для threaded нужен Clang с поддержкой `clang::musttail`; проверен Clang 18.
При первой конфигурации CMake скачивает недостающие зависимости.

В Ubuntu инструменты можно установить так:

```bash
sudo apt update
sudo apt install clang cmake ninja-build ruby
```

Сборка Release с тестами:

```bash
cmake --preset release
cmake --build --preset release --parallel
ctest --preset release
```

Результаты сборки находятся в `build/release/`. Для Debug используйте preset
`debug` вместо `release`; его каталог — `build/debug/`.
Все presets используют `clang++` по умолчанию.

## Запуск программы

```bash
# Сгенерировать ELF с вычислением Fibonacci.
ruby genFibonacci.rb

# Исполнить через switch (режим по умолчанию) или threaded.
./build/release/simcourse tests/fibonacci.elf
./build/release/simcourse tests/fibonacci.elf --threaded
```

Симулятор принимает ELF для TIM ISA.

## Сборка через Nix

Проект содержит `flake.nix` и `flake.lock`: версии окружения и зависимостей
зафиксированы. Нужен Nix с включённой поддержкой flakes. Поддерживаются
`x86_64-linux` и `aarch64-linux`.

```bash
# Собрать Release; сборка включает запуск CTest.
nix build

# Запустить собранный симулятор.
./result/bin/simcourse tests/fibonacci.elf --threaded

# Войти в окружение разработки с Clang, CMake, Ninja и Ruby.
nix develop
```

Внутри `nix develop` доступны команды CMake, генератора и бенчмарков из этого
README. `nix build` собирает пакет симулятора; бенчмарки собирает Ruby-раннер.

## Бенчмарки

Сравнивается switch и threaded на арифметике, переходах, обращениях
к памяти, CRC32 и сортировке вставками.

```bash
ruby benchmarks/run.rb comparison
```

Доступны также другие опции запуска подробности: `ruby benchmarks/run.rb --help`.

Артефакты сохраняются в `benchmark-results/<UTC-время>/`.

**[Отчёт: результаты и применённые оптимизации](benchmarks/threaded_vs_switch.md).**
