# Smart Pointers

Это проект на C++, в котором реализованы разные виды умных указателей: от `UniquePtr` до `IntrusivePtr` и механизма `enable_shared_from_this`. 

---

## Возможности

- **UniquePtr**  
  - Единоличное владение объектом  
  - Пользовательский `Deleter` и оптимизация через `CompressedPair`  
  - Специализация для массивов (`UniquePtr<T[]>`)

- **SharedPtr & MakeShared**  
  - Счётчик ссылок для общего владения  
  - Оптимизированная реализация `MakeShared` (единая аллокация под control block и данные)

- **WeakPtr**  
  - Слабое (non-owning) владение, предотвращающее циклические ссылки

- **EnableSharedFromThis**  
  - Механизм получения `SharedPtr` из `this` внутри класса

- **IntrusivePtr & MakeIntrusive**  
  - Встроенное управление счётчиком ссылок внутри пользовательского класса  
  - Удобный интерфейс `MakeIntrusive<T>(…)`

---

## Требования

- C++17 или новее  
- CMake ≥ 3.10  
- Компилятор с поддержкой C++17 (GCC, Clang, MSVC)

---

## Сборка и запуск

```bash
git clone https://github.com/FoggGhostt/Smart-Pointers.git
cd Smart-Pointers
mkdir build && cd build
cmake ..
cmake --build .
