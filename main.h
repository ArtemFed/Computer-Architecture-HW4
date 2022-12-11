#ifndef COMPUTER_ARCHITECTURE_HW4_MAIN_H
#define COMPUTER_ARCHITECTURE_HW4_MAIN_H

// Отключаем некоторые warning'и, потому что бесят
#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc50-cpp"
#pragma ide diagnostic ignored "cert-err58-cpp"

#include <fstream>
#include <iostream>
#include <pthread.h>
#include <queue>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

// COLORS == 1, то "Разные цвета есть", иначе "Цвета стандартные".
// ЕСЛИ ЦВЕТА НЕ РАБОТАЮТ И ВЫВОДЯТСЯ ТОЛЬКО КОДЫ => поменять COLORS == 0
#define COLORS 1
#define SECTIONS_NUM 3

#if COLORS == 1
// Набор цветов для консоли.
string ANSI_RESET = "\033[0m";
string ANSI_RED = "\033[31m";
string ANSI_GREEN = "\033[32m";
string ANSI_YELLOW = "\033[33m";
string ANSI_PURPLE = "\033[35m";
string ANSI_CYAN = "\033[36m";
#endif

// Пока покупатели не кончились, работодатели пашут
bool flag = true;

// Режим ввода
string answer;

// Строка для вывода
string str;

// Накопительная переменная для вывода в файл
string cumulative;

// Мьютекс для защиты операции чтения
pthread_mutex_t mutex_0;
pthread_mutex_t mutex_1;
pthread_mutex_t mutex_2;


#endif //COMPUTER_ARCHITECTURE_HW4_MAIN_H
