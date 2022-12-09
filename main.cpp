#include <fstream>
#include <iostream>
#include <pthread.h>
#include <queue>
#include <random>
#include <semaphore.h>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

#define SECTIONS_NUM 3
#define BUYERS_NUM 3
#define COUNT 1

string ANSI_RESET = "\033[0m";
string ANSI_RED = "\033[31m";
string ANSI_GREEN = "\033[32m";
string ANSI_YELLOW = "\033[33m";
string ANSI_BLUE = "\033[34m";
string ANSI_PURPLE = "\033[35m";
string ANSI_CYAN = "\033[36m";

/**
 * Класс продавца
 */
class Seller {
public:
    int id = 0;

    Seller() {
    }

    // Раскрасить вывод Seller
    static string getColor(const string &str, int id) {
        if (id == 0) {
            return ANSI_YELLOW + str + ANSI_RESET;
        } else if (id == 1) {
            return ANSI_GREEN + str + ANSI_RESET;
        }
        return ANSI_CYAN + str + ANSI_RESET;
    }
};

/**
 * Класс покупателя
 */
class Buyer {
public:
    int id = 0;

    queue<int> plan;
    Buyer() {
    }
};

// Буфер
vector<Buyer *> buf(SECTIONS_NUM);

bool flag = true;

unsigned int seed = 101;// инициализатор генератора случайных чисел

pthread_mutex_t mutex;// мьютекс для условных переменных

// поток-читатель блокируется этой условной переменной,
// когда количество занятых ячеек становится равно 0
pthread_cond_t section_empty;

//стартовая функция потоков – продавцов (писателей)
void *SellerFunc(void *param) {
    Seller seller = *((Seller *) param);
    while (flag) {
        if (buf[seller.id] == nullptr) {
            continue;
        }

        cout << Seller::getColor("\nSeller: " + to_string(seller.id) + " IS serving for Buyer: " + to_string(buf[seller.id]->id) + " \t\tclock: " + to_string((clock())), seller.id);

        sleep(2);

        cout << Seller::getColor("\nSeller: " + to_string(seller.id) + " ENDED serving for Buyer: " + to_string(buf[seller.id]->id) + " \t\tclock: " + to_string((clock())), seller.id);

        buf[seller.id] = nullptr;

        // Разбудить потоки-читатели после обновления элемента буфера
        //        pthread_cond_broadcast(&section_empty);
    }
    cout << Seller::getColor("\nSeller: " + to_string(seller.id) + " ended successfully... " + " \t\tclock: " + to_string((clock())), seller.id);
    return nullptr;
}

// Стартовая функция потоков – покупателей (читателей)
void *BuyerFunc(void *param) {
    Buyer *buyer = ((Buyer *) param);
    while (!buyer->plan.empty()) {
        // Извлечь элемент из буфера

        cout << Seller::getColor("\nBuyer: " + to_string(buyer->id) + " moved in line to the Seller: " + to_string(buyer->plan.front()) + " \tclock: " + to_string(clock()), buyer->plan.front());

        sleep(1);

        // Заснуть, если нужный сектор в магазине занят
        while (buf[buyer->plan.front()] != nullptr) {
        }

        // Защита операции чтения
        pthread_mutex_lock(&mutex);

        buf[buyer->plan.front()] = buyer;

        // Заснуть, если нужный сектор в магазине занят
        while (buf[buyer->plan.front()] != nullptr) {
            //            pthread_cond_wait(&section_empty, &mutex);
        }

        buyer->plan.pop();

        // Конец критической секции
        pthread_mutex_unlock(&mutex);
    }
    cout << "\nBuyer: " + to_string(buyer->id) + " went away..." + " \t\t\t\tclock: " + to_string(clock());
    return nullptr;
}

int main() {
    /*
     * Условие:
     * 10. Задача о магазине - 1.
     * В магазине работают три отдела, каждый отдел обслуживает один продавец.
     * Покупатель, зайдя в магазин, делает покупки в одном или нескольких произвольных отделах,
     * обходя их в произвольном порядке.
     * Если в выбранном отделе продавец не свободен,
     * покупатель становится в очередь и засыпает, пока продавец не освободится.
     * Создать многопоточное приложение, моделирующее рабочий день магазина.
     */

    // Список продавцов
    std::vector<Seller> sellers(SECTIONS_NUM);
    std::vector<pthread_t> threads_sellers(sellers.size());

    // Список покупателей
    std::vector<Buyer> buyers(BUYERS_NUM);
    std::vector<pthread_t> threads_buyers(buyers.size());

    srand(seed);
    // Инициализация мутексов и семафоров
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&section_empty, NULL);

    system("color 00");
    std::cout << ANSI_PURPLE + "\nThe store is opening!\n\n" + ANSI_RESET;

    // Запуск продовцов (по одному в каждом отделе)
    for (int i = 0; i < sellers.size(); i++) {
        // Зададим id для продавца
        sellers[i].id = i;
        // Запускаем поток
        pthread_create(&threads_sellers[i], nullptr,
                       SellerFunc,
                       &sellers[i]);
        cout << getColor("\nSeller: " + to_string(sellers[i].id) + " came to work", i);
    }

    // Запуск покупателей
    for (int i = 0; i < buyers.size(); ++i) {
        buyers[i].id = i;
        // int num = rand() % 5;
        // Создаём случайный план для покупателя
        queue<int> plan;
        string str = "\nBuyer: " + to_string(buyers[i].id) + " come with plan:";
        for (int j = 0; j < COUNT; ++j) {
            int val = rand() % SECTIONS_NUM;
            plan.push(val);
            str += " " + to_string(val);
        }
        cout << str;
        buyers[i].plan = plan;
    }

    for (int i = 0; i < buyers.size(); ++i) {
        pthread_create(&threads_buyers[i], nullptr,
                       BuyerFunc,
                       &buyers[i]);
    }

    // Ждём пока кончатся все покупатели
    for (unsigned long long threads_buyer : threads_buyers) {
        pthread_join(threads_buyer, nullptr);
    }

    flag = false;

    // Ждём пока закроются все отделы
    for (unsigned long long threads_seller : threads_sellers) {
        pthread_join(threads_seller, nullptr);
    }

    std::cout << ANSI_PURPLE + "\n\nThe store is closed!" + ANSI_RESET;
    return 0;
}