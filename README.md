**ДЗ_3 "Симулятор жидкости, ускорение"**

Для запуска использовать команду ```./run1``` из корня

1) Без ускорения(код из ДЗ_2): примерно 120 сек на 1000 тиков
2) Ускорение за счет switch case в методе get класса VectorField: примерно 112 сек на 1000 тиков
3) Ускорение за счет ограничения скорости (переменная vp < 0.001, метод propagate_flow, 515 строка Fluid.hpp) и применение параллельных вычислений в отдельных функциях симулятора(вычисление сил и p): 3 сек на 1000 тиков

Задача по своему смыслу выполнена - ускорение значительное(со 120 сек до 3 сек на 1000 тиков). Однако все попытки распараллелить dfs обернулись провалом, программа вела себя некорректно(вода шла вверх или подобное). Тем не менее, количество потоков можно выбрать, ускорение будет, но значительность покажется только на больших тестах. Так же можно выбрать количество тиков. Все настраивается в файле run1.

Итог:

Возможность параллельного запуска и выбора числа потоков - есть

Программа корректно работает для любого числа потоков - есть

Существенное ускорение без использования потоков - есть

Ускорение за счет параллелизма на большом тесте - есть

Отчет - есть

Используемые статьи: 

https://habr.com/en/articles/656515/

https://www.cyberforum.ru/cpp-beginners/thread2925036.html

https://habr.com/en/articles/738250/ 

https://habr.com/ru/articles/182626/