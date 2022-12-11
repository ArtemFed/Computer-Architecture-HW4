# Computer-Architecture-HW4. <br/> Выполнил Федоров Артём БПИ217 (на 8 баллов). <br/> Вариант 10

## Арифметика с плавающей точкой <br/>

__Условие. Вариант 10:__
__Задача о магазине - 1.__
   В магазине работают три отдела, каждый отдел обслуживает один продавец.
   Покупатель, зайдя в магазин, делает покупки в одном или нескольких
   произвольных отделах, обходя их в произвольном порядке. Если в выбранном
   отделе продавец не свободен, покупатель становится в очередь и засыпает,
   пока продавец не освободится. Создать многопоточное приложение,
   моделирующее рабочий день магазина.

---- 
<br/>

## Как устроена программа:
##### Начало.
```
Select the input/output format:
  1. Input from the console.
  2. Input from a file.
  3. Random input.
```
##### 1. Ввод: 1 - __Работа с консолью__
Необходим ввод:
1) Количество покупателей
Далее для каждого покупателя
2) Количесво отделов, которые он хотел посетить (с повторениями)
3) Через пробел номера от 1 до 3 (включительно) номера отделов в порядке очереди для посещения
   
##### 2. Ввод: 2 - __Работа с файлами__
Необходим ввод имени файлов ввода и вывода (Название, если он находится рядом с .exe файлом или полный путь к файлу)
__Формат данных в файле для чтения:__
> __*Пример:*__
> 2   (Количество покупателей)
> 3 1 2 3   (Первое число n = кол-во последующих цифр; n чисел  с номерами отделов от 1 до 3 (включительно))
> 2 1 3   (2 => будет прочитано два числа 1 и 3 - номера отделов в плане посещения)

__Заметка 1:__ 
В режиме работы с файлами работа с цветом в консоли будет отключена

##### 3. Ввод: (!= 1 и != 2) - __Случайный генератор__
__Необходим ввод:__
1) Количество покупателей
2) Максимальное количесво отделов для посещения в планах (отделы могут повторяться)
3) Сид для генерации (от не случайный, чтобы можно было сравнивать работы одинаковых генераций)

__Заметка 2:__ 
Во всех режимах, кроме работы с файлами, вывод будет сопровождаться цветом в консоли, индивидуальным для каждого отдела (продавца) (1 - Жёлтый, 2 - Зелёный, 3 - Голубой)


# Отчёт 4 балла:
### 1. Приведено решение задачи на C++:
> [main.cpp](https://github.com/ArtemFed/Computer-Architecture-HW4/blob/main/main.cpp)


### 2.  Модель параллельных вычислений:
> ![diagram](https://user-images.githubusercontent.com/57373162/206895172-99d7d1cf-4154-4ce1-9efb-fffadf112cbd.png)


### 3. Входные данные программы + вариативные диапазоны:
> 1. Потоков продавцов (Seller'ов) всегда три по условию
> 2. Количество потоков покупателей (Buyer'ов) задаваемое вручную значение
> 3. Количество дел в плане каждого покупателя задаваемое или случайное значение. Чем больше номеров отделов в плане, тем больше очередей и "конфликтов" между покупателями, то есть дольше работает программа, пока все не будут обслужены.
> 4. Если покупатели встали в очерь к одному продовцу одновременно, то порядок их обслуживания решается рыцарским поединком... Шутка! Просто какой-то поток случайно победит.


---- 
<br/>


# Отчёт 5 баллов:
### Сценарий (пример):
> 1. Магазин открывается - день начинается
> 2. Три продавца-потока открывают кассы в своих отделах и ждут пока все покупатели не закончатся, обрабатывая подходящих к ним клиентов
> 3. Приходит n покупателей-потоков со своими планами (к открытию магазина)
> 4. Покупатели, смотря на свои планы, по очереди проходят к нужным отделам, вставая в очередь только к нужному продавцу, не мешая другим очередям.
> 5. Будучи принятыми продавцом, который закончил работу с предыдущим клиентов, не дают подойти другим покупателям к этой же кассе.
> 6. Как только последний пункт из плана покупателя вычеркнут - он покидает магазин.
> 7. Как только все покупатели покинули магазин - продавцы перестают работать и тоже уходят.
> 8. Магазин закрыт - день закончен

Продавцы работают отдельно и никак не мешают друг-другу (вообще не пересекаются)
Покупатели занимают очередь к конкретному продавцу и принимаются по готовности

---- 
<br/>

# Отчёт 6 баллов:
### 1. Реализован ввод данных из командной строки. <br/>

### 2. Обобщенный алгоритм.:
> _Смотреть рисунок в пункте 3 на 4 балла_
> 
---- 
<br/>


# Отчёт 7 баллов:
### 1. В программу добавлены ввод данных из файла и вывод результатов в файл + Результаты дублируются на экран: <br/>


### 2. Приведены входные и выходные файлы с различными результатами выполнения программы.
> [files] ()

---- 
<br/>

# Отчёт 8 баллов:
### 1. В программу добавлен генератор случайных данных в допустимых диапазонах.

### 2. Приведены входные и выходные файлы с различными результатами выполнения программы.
> [files] ()
