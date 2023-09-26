# Advanced Vector
Это экспериментальный контейнер, созданный для изучения концепций языка связанных с копированием, перемещением, обработкой исключений, RAII, SFINAE и схожий по функционалу с std::vector. Для работы с памятью создан вспомогательный класс RawMemory использующий идиому RAII. При перевыделении памяти доступный объём контейнера увеличивается в два раза. Контейнер не уступает std::vector в количестве вызовов операторов присваивания, конструкторов копирования и перемещения хранимых типов данных, а также реализует строгую гарантию безопасности исключений.
## Реализованные методы:
- Метод Emplace принимает позицию вставки и параметры конструктора хранимого типа. Создаёт элемент сразу в месте его размещения.
- Метод Insert вставляет элемент в указанную позицию вектора используя копирование или перемещение в зависимости от свойств хранимого типа.
- Метод EmplaceBack конструирует новый элемент в конце вектора.
- Метод PushBack копирует или перемещает элемент в конец вектора.
- Метод Erase удаляет элемент в переданной позиции.
- Метод PopBack удаляет последний элемент вектора. Вызывается деструктор элемента, размер вектора уменьшается на единицу.
- Оператор [] обеспечивает доступ к произвольному элементу.
- Метод Back обеспечивает доступ к последнему элементу.
- Метод Reserve резервирует память под заданное количество элементов.
- Метод Resize меняет текущий размер вектора на заданный.
- Метод Swap обменивает содержимое двух векторов.
## Использование:
Добавьте файл vector.h в ваш проект. Подключите директивой include.
## Требования:
- C++17 (STL)
- GCC, Clang
## Планы по доработке:
Добавить методы ShrinkToFit, Clear и Data.
## Стек технологий:
- RAII
- SFINAE
- forwarding references
- STL