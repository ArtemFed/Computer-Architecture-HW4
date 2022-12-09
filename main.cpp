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
#define COUNT 3

/**
 * Класс продавца
 */
class Seller {
public:
    int id = 0;

    Seller() {
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

        cout << "\nSeller: " + to_string(seller.id) + " IS serving for Buyer: " + to_string(buf[seller.id]->id) + " \tclock: " + to_string((clock()));

        sleep(2);

        cout << "\nSeller: " + to_string(seller.id) + " ENDED serving for Buyer: " + to_string(buf[seller.id]->id) + " \tclock: " + to_string((clock()));

        buf[seller.id] = nullptr;

        // Разбудить потоки-читатели после обновления элемента буфера
        pthread_cond_broadcast(&section_empty);
    }
    cout << "\nSeller: " + to_string(seller.id) + " ended successfully... " + " \tclock: " + to_string((clock()));
    return nullptr;
}

// Стартовая функция потоков – покупателей (читателей)
void *BuyerFunc(void *param) {
    Buyer buyer = *((Buyer *) param);
    while (!buyer.plan.empty()) {
        // Извлечь элемент из буфера

        cout << "\nBuyer: " + to_string(buyer.id) + " moved in line to the Seller: " + to_string(buyer.plan.front()) + " \tclock: " + to_string(clock());

        // Заснуть, если нужный сектор в магазине занят
        while (buf[buyer.plan.front()] != nullptr) {
            cout << "\n" << buyer.id << " ";
            pthread_cond_wait(&section_empty, &mutex);
        }

        // Защита операции чтения
        pthread_mutex_lock(&mutex);

        //sleep(1);

        //        cout << "\n" << buyer.id << " lock";
        buf[buyer.plan.front()] = new Buyer(buyer);

        // Заснуть, если нужный сектор в магазине занят
        while (buf[buyer.plan.front()] != nullptr) {
            pthread_cond_wait(&section_empty, &mutex);
        }

        buyer.plan.pop();

        // Конец критической секции
        pthread_mutex_unlock(&mutex);
        //        cout << "\n" << buyer.id << " unlock";
    }
    cout << "\nBuyer: " + to_string(buyer.id) + " went away..." + " \t\t\tclock: " + to_string(clock());
    return nullptr;
}

int main() {
    std::vector<Seller> sellers(SECTIONS_NUM);
    std::vector<pthread_t> threads_sellers(sellers.size());

    std::vector<Buyer> buyers(BUYERS_NUM);
    std::vector<pthread_t> threads_buyers(buyers.size());

    srand(seed);
    int i;
    // Инициализация мутексов и семафоров
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&section_empty, NULL);

    std::cout << "\nThe store is opening!\n\n";

    // Запуск продовцов (по одному в каждом отделе)
    for (i = 0; i < sellers.size(); i++) {
        // Зададим id для продавца
        sellers[i].id = i;
        // Запускаем поток
        pthread_create(&threads_sellers[i], nullptr,
                       SellerFunc,
                       &sellers[i]);
        cout << "\nSeller: " << sellers[i].id << " came to work";
    }

    // Запуск покупателей
    for (i = 0; i < buyers.size(); ++i) {
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

    std::cout << "\nThe store is closed!";
    return 0;
}