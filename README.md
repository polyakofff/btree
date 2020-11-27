# btree
Б-дерево - дерево во вторичной памяти

Писал это три дня, так что грех не выложить. Не претендую на лучшую реализацию, но считаю, что эта неплоха.

Теория: https://neerc.ifmo.ru/wiki/index.php?title=B-%D0%B4%D0%B5%D1%80%D0%B5%D0%B2%D0%BE

Для справки:

Все операции за O(t*logt(n))

По моим наблюдениям наиболее оптимальный параметр дерева t лежит в диапазоне 40-100

По скорости работы btree меня приятно удивило. При n=1e5 основных операциях (вставка, удаление, поиск) btree во внешней памяти проигрывает std::map в оперативной памяти примерно в 10-20 раз.
Однако конечно же самое долгое тут это постоянные операции ввода/вывода в файл и аллокации/деаллокации памяти. 

Эта же реализация в оперативной памяти (при n=1e6) в среднем быстрее std::map аж в 2 раза.

В любой момент времени в оперативную память загружено не более чем t*const узлов.

По памяти на единицу полезной нагрузки в худшем случае тратится: 2 ссылки (тут они int, при больших деревьях надо менять на long long) и еще одна такая единица.
