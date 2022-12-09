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

// COLORS == 1, то "Разные цвета есть", иначе "Цвета стандартные".
#define COLORS 1
#define SECTIONS_NUM 3
#define BUYERS_NUM 3
#define COUNT 3

#if COLORS == 1
// Набор цветов для консоли.
string ANSI_RESET = "\033[0m";
string ANSI_RED = "\033[31m";
string ANSI_GREEN = "\033[32m";
string ANSI_YELLOW = "\033[33m";
string ANSI_BLUE = "\033[34m";
string ANSI_PURPLE = "\033[35m";
string ANSI_CYAN = "\033[36m";
#endif

/*
 *Класс продавца
*/
class Seller {
public:
    int id;

    Seller() {}

    // Раскрасить вывод Seller
    /**
     * 1 - Yello
     * 2 - Green
     * 3 - Cyan
     * 4 - Purple
     * @param str Строка
     * @param id Код цвета
     * @return Раскришенный Цвет
     */
    static string getColor(const string &str, int id) {
#if COLORS == 1
        switch (id) {
            case 1:
                return ANSI_YELLOW + str + ANSI_RESET;
            case 2:
                return ANSI_GREEN + str + ANSI_RESET;
            case 3:
                return ANSI_CYAN + str + ANSI_RESET;
            case 4:
                return ANSI_PURPLE + str + ANSI_RESET;
            default:
                return str;
        }
#else
        return str;
#endif
    }
};

/*
 * Класс покупателя
*/
class Buyer {
public:
    int id{};

    queue<int> plan;
    Buyer() = default;

    string getPlanToString() const {
        queue<int> plan_copy(plan);
        string str;
        for (int j = 0; j < plan.size(); ++j) {
            str += " " + to_string(plan_copy.front());
            plan_copy.pop();
        }
        return str;
    }
};

// Вектор активных к обслуживанию покупателей (индекс = отдел)
vector<Buyer *> active_buyers(SECTIONS_NUM);

// Пока покупатели не кончились, работодатели пашут
bool flag = true;

// Мьютекс для защиты операции чтения
pthread_mutex_t mutex;

pthread_cond_t section_empty;
unsigned int microseconds;
// Стартовая функция потоков – продавцов (писателей)
void *SellerFunc(void *param) {
    Seller seller = *((Seller *) param);
    // Пока покупатели не кончились, работодатели пашут
    while (flag) {
        // Ждём пока не появится покупатель
        if (active_buyers[seller.id - 1] == nullptr) {
            usleep(200 * 1000);
            continue;
        }

        cout << Seller::getColor("\nSeller: " + to_string(seller.id) +
                                         " IS serving for Buyer: " +
                                         to_string(active_buyers[seller.id - 1]->id) +
                                         " \t\tclock: " + to_string((clock())),
                                 seller.id);

        // Обслуживает покупателя
        sleep(2);

        cout << Seller::getColor("\nSeller: " + to_string(seller.id) +
                                         " ENDED serving for Buyer: " +
                                         to_string(active_buyers[seller.id - 1]->id) +
                                         " \t\tclock: " + to_string((clock())),
                                 seller.id);

        active_buyers[seller.id - 1] = nullptr;

        // Разбудить потоки-читатели после обновления элемента буфера
        // pthread_cond_broadcast(&section_empty);
    }
    cout << Seller::getColor("\nSeller: " + to_string(seller.id) +
                                     " ended successfully... " +
                                     " \t\tclock: " + to_string((clock())),
                             seller.id);
    return nullptr;
}

// Стартовая функция потоков – покупателей (читателей)
void *BuyerFunc(void *param) {
    Buyer *buyer = ((Buyer *) param);
    while (!buyer->plan.empty()) {
        // Извлечь элемент из буфера

        cout << Seller::getColor(
                "\nBuyer: " + to_string(buyer->id) + " moved in line to the Seller: " +
                        to_string(buyer->plan.front()) + " \tclock: " + to_string(clock()),
                buyer->plan.front() + 1);

        // Покупатель идёт в отдел
        sleep(1);

        // Заснуть, если продавец в нужном отделе занят

        // Защита операции чтения
        pthread_mutex_lock(&mutex);

        active_buyers[buyer->plan.front()] = buyer;

        // Заснуть, если нужный сектор в магазине занят
        while (active_buyers[buyer->plan.front()] != nullptr) {
            usleep(200 * 1000);
        }

        buyer->plan.pop();
        // Конец критической секции
        pthread_mutex_unlock(&mutex);
    }
    cout << "\nBuyer: " + to_string(buyer->id) + " went away..." +
                    " \t\t\t\tclock: " + to_string(clock());
    return nullptr;
}

int main(int argc, char **argv) {
    /*
   * Условие:
   * 10. Задача о магазине - 1.
   * В магазине работают три отдела, каждый отдел обслуживает один продавец.
   * Покупатель, зайдя в магазин, делает покупки в одном или нескольких
   * произвольных отделах, обходя их в произвольном порядке. Если в выбранном
   * отделе продавец не свободен, покупатель становится в очередь и засыпает,
   * пока продавец не освободится. Создать многопоточное приложение,
   * моделирующее рабочий день магазина.
   */

    // Ввод с консоли на 6
    // Ввод.Вывод с/из файла на 7
    // Генератор на 8

    // Сид генератора случайных чисел
    int seed, num, val;
    string answer, str;

    cout << "Select the input/output format:\n"
            " 1. Input from the console.\n"
            " 2. Input from a file.\n"
            " 3. Random input.\n"
            "Input:";

    cin >> answer;

    // Список продавцов
    std::vector<Seller> sellers(SECTIONS_NUM);
    std::vector<pthread_t> threads_sellers(sellers.size());

    // Список покупателей
    std::vector<Buyer> buyers(BUYERS_NUM);
    std::vector<pthread_t> threads_buyers(buyers.size());

    // Инициализация мутексов и семафоров
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&section_empty, nullptr);

    if (answer == "1") {
        // Создаём покупателей (дома готовят план покупок)
        for (int i = 0; i < buyers.size(); ++i) {
            buyers[i].id = i + 1;
            // Создаём план для покупателя по полученным данным из консоли
            cout << "Buyer \" << i + 1";
            cout << "\nEnter the number of tasks for";
            cin >> num;
            queue<int> plan;
            str = "\nEnter " + to_string(num) + " numbers separated by a space (from " +
                  to_string(0) + " to " + to_string(SECTIONS_NUM) + "): ";
            cout << str;
            for (int j = 0; j < num; ++j) {
                cin >> val;
                plan.push(val % SECTIONS_NUM);
            }
            buyers[i].plan = plan;
        }
    } else if (answer == "3") {
        cout << "Enter a seed for random generation:";
        cin >> seed;
        srand(seed);
        // Создаём покупателей (дома готовят план покупок)
        for (int i = 0; i < buyers.size(); ++i) {
            buyers[i].id = i + 1;
            // Создаём случайный план для покупателя
            num = 1 + rand() % COUNT;
            queue<int> plan;
            for (int j = 0; j < num; ++j) {
                val = rand() % SECTIONS_NUM;
                plan.push(val);
            }
            buyers[i].plan = plan;
        }
    }

    system("color 00");
    std::cout << Seller::getColor("\nThe store is opening!\n\n", 3);

    // Запуск продавцов (по одному в каждом отделе) По условию 3 отдела
    for (int i = 0; i < sellers.size(); i++) {
        // Зададим id для продавца
        sellers[i].id = i + 1;
        // Запускаем поток
        pthread_create(&threads_sellers[i], nullptr, SellerFunc, &sellers[i]);
        str = Seller::getColor(
                "\nSeller: " + to_string(sellers[i].id) + " " + "came to" + " work", i);
        cout << str;
    }

    // Информация о покупателях
    for (auto &buyer : buyers) {
        str = "\nBuyer: " + to_string(buyer.id) +
              " has prepared a plan:" + buyer.getPlanToString();
        cout << str;
    }

    // Запускаем потоки покупателей
    for (int i = 0; i < buyers.size(); ++i) {
        pthread_create(&threads_buyers[i], nullptr, BuyerFunc, &buyers[i]);
    }

    // Тут параллельно происходит взаимодействие продавцов и покупателей

    // Ждём пока кончатся все покупатели
    for (unsigned long long threads_buyer : threads_buyers) {
        pthread_join(threads_buyer, nullptr);
    }

    // Завершаем работу продавцов
    flag = false;
    // Ждём пока закроются все отделы
    for (unsigned long long threads_seller : threads_sellers) {
        pthread_join(threads_seller, nullptr);
    }

    std::cout << Seller::getColor("\n\nThe store is closed!", 3);
    return 0;
}