
# Постановка задачи
Необходимо реализовать многопточное приложение генерации и обработки событий. Создается две группы потоков: группа потоков генерации событий и группа обработчиков.

# Решение
Реализованы два решения задачи: тривиальное и модификация.

## Тривиальное
В тривиальном решении инициализируется n потоков генераторов событий, в котором генерируются события и записываются в очередь, и m событий обработчиков, которые вычисляют простое ли число.
## Модификация
В модификации используется два списка: список с которым работают генераторы событий и список с которым работают обработчики. Создается дополнительный поток manager, в котором происходит распределение для обработчиков.

# Запуск программы
1. make
2. ./main argv

В main передается один аргумент: "modify" или "trivial". В зависимости от аргумента будет запущено тривиальное решение или с модификацией.
Если аргументы не передаются или они не валидные, то запускается тривиальное решение.
