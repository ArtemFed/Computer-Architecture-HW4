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

/*
 *Класс продавца
 */
class Seller {
public:
    int id{};

    Seller() = default;

    // Раскрасить вывод Seller
    /**
   * 0 - Yellow
   * 1 - Green
   * 2 - Cyan
   * 3 - Purple
   * @param line Строка
   * @param id Код цвета
   * @return Раскрашенный Цвет
   */
    static string getColor(const string &line, int id) {
#if COLORS == 1
        if (answer == "2") {
            return line;
        }
        switch (id) {
            case 0:
                return ANSI_YELLOW + line + ANSI_RESET;
            case 1:
                return ANSI_GREEN + line + ANSI_RESET;
            case 2:
                return ANSI_CYAN + line + ANSI_RESET;
            case 3:
                return ANSI_PURPLE + line + ANSI_RESET;
            case 4:
                return ANSI_RED + line + ANSI_RESET;
            default:
                return line;
        }
#else
        return line;
#endif
    }
};

/*
 * Класс покупателя
 */
class Buyer {
public:
    int id{};
    int time_start{};

    queue<int> plan;

    Buyer() = default;

    [[nodiscard]] string getPlanToString() const {
        string line;
        queue<int> plan_copy(plan);
        for (int j = 0; j < plan.size(); ++j) {
            line += " " + to_string(plan_copy.front() + 1);
            plan_copy.pop();
        }
        return line;
    }
};

// Вектор активных к обслуживанию покупателей (индекс = отдел)
vector<Buyer *> active_buyers(SECTIONS_NUM);

// Стартовая функция потоков – продавцов (писателей)
void *SellerFunc(void *param) {
    string str_thread;
    Seller seller = *((Seller *) param);
    // Пока покупатели не кончились, работодатели пашут
    while (flag) {
        // Ждём пока не появится покупатель, чтобы не было null reference (не зависит от других потоков-продавцов)
        if (active_buyers[seller.id] == nullptr) {
            continue;
        }

        str_thread = Seller::getColor("\nSeller: " + to_string(seller.id + 1) +
                                      " IS serving for Buyer: " +
                                      to_string(active_buyers[seller.id]->id + 1) +
                                      " \t\tclock: " + to_string((clock())),
                                      seller.id);
        cumulative += str_thread;
        cout << str_thread;
        // Обслуживает покупателя
        sleep(2);

        str_thread = Seller::getColor("\nSeller: " + to_string(seller.id + 1) +
                                      " ENDED serving for Buyer: " +
                                      to_string(active_buyers[seller.id]->id + 1) +
                                      " \t\tclock: " + to_string((clock())),
                                      seller.id);
        cumulative += str_thread;
        cout << str_thread;

        active_buyers[seller.id] = nullptr;
    }
    str_thread = Seller::getColor("\nSeller: " + to_string(seller.id + 1) +
                                  " ended successfully... " +
                                  " \t\tclock: " + to_string((clock())),
                                  seller.id);
    cumulative += str_thread;
    cout << str_thread;
    return nullptr;
}

// Стартовая функция потоков – покупателей (читателей)
void *BuyerFunc(void *param) {
    Buyer *buyer = ((Buyer *) param);
    string str_thread;
    // Покупатель засыпает на случайное время (не все же приходят к открытию)
    sleep(buyer->time_start);
    str_thread = "\nBuyer: " + to_string(buyer->id + 1) + " come to the Store." +
                 " \t\t\tclock: " + to_string(clock());
    cumulative += str_thread;
    cout << str_thread;
    while (!buyer->plan.empty()) {
        str_thread = Seller::getColor(
                "\nBuyer: " + to_string(buyer->id + 1) + " moved in line to the Seller: " +
                to_string(buyer->plan.front() + 1) + " \tclock: " + to_string(clock()),
                buyer->plan.front());
        cumulative += str_thread;
        cout << str_thread;

        // Покупатель идёт в отдел
        sleep(1);

        // Формирование очереди + защита операции чтения
        switch (buyer->plan.front()) {
            case 0:
                pthread_mutex_lock(&mutex_0);
                break;
            case 1:
                pthread_mutex_lock(&mutex_1);
                break;
            case 2:
                pthread_mutex_lock(&mutex_2);
                break;
            default:
                cout << Seller::getColor("\nPROBLEM on locking!", 4);
        }

        // Занимаем место у кассы, тем самым подаём "сигнал" продавцу
        active_buyers[buyer->plan.front()] = buyer;

        // Заснуть, если нужный сектор в магазине занят
        while (active_buyers[buyer->plan.front()] != nullptr) {
            usleep(200 * 1000);
        }

        // Пропускаем следующего из очереди + конец критической секции
        switch (buyer->plan.front()) {
            case 0:
                pthread_mutex_unlock(&mutex_0);
                break;
            case 1:
                pthread_mutex_unlock(&mutex_1);
                break;
            case 2:
                pthread_mutex_unlock(&mutex_2);
                break;
            default:
                cout << Seller::getColor("\nPROBLEM on unlocking!", 4);
        }
        // Отойдя от кассы, удаляем пункт из плана
        buyer->plan.pop();
    }
    str_thread = "\nBuyer: " + to_string(buyer->id + 1) + " went away..." +
                 " \t\t\t\tclock: " + to_string(clock());
    cumulative += str_thread;
    cout << str_thread;
    return nullptr;
}

int startTheStore(int argc, char **argv) {
    // Сид генератора случайных чисел и просто числа
    int seed, num, val, max_count;

    // Работа с файлами
    string input, output;

    answer = "0";
    // argc == 3   =>   input.txt output.txt
    // argc == 4   =>   number_of_Buyers MAX_number_of_tasks seed
    if (argc == 3) {
        answer = "2";
        input = argv[1];
        output = argv[2];
    } else if (argc == 4) {
        answer = "3";
        num = stoi(argv[1]);
        max_count = stoi(argv[2]);
        seed = stoi(argv[3]);
    }

    if (answer == "0") {
        str = "Select the input/output format:\n"
              " 1. Input from the console.\n"
              " 2. Input from a file.\n"
              " 3. Random input.\n"
              "Input:";
        cout << str;
        cin >> answer;
    }

    // Список продавцов
    std::vector<Seller> sellers(SECTIONS_NUM);
    std::vector<pthread_t> threads_sellers(sellers.size());

    // Список покупателей
    std::vector<Buyer> buyers;
    std::vector<pthread_t> threads_buyers;

    // Инициализация mutex
    pthread_mutex_init(&mutex_0, nullptr);
    pthread_mutex_init(&mutex_1, nullptr);
    pthread_mutex_init(&mutex_2, nullptr);

    if (answer == "1") {
        // Ручной ввод в консоль.
        cout << "Enter count of Buyers:";
        cin >> num;
        buyers = vector<Buyer>(num);
        threads_buyers = vector<pthread_t>(buyers.size());
        // Создаём покупателей (дома готовят план покупок)
        for (int i = 0; i < buyers.size(); ++i) {
            buyers[i].id = i;
            // Создаём план для покупателя по полученным данным из консоли
            cout << "Buyer: " + to_string(i + 1);
            cout << "\nEnter the number of tasks:";
            cin >> num;
            if (num < 1) {
                continue;
            }
            queue<int> plan;
            str = "Enter " + to_string(num) + " numbers separated by a space (from " +
                  to_string(1) + " to " + to_string(SECTIONS_NUM) + "):";
            cout << str;
            for (int j = 0; j < num; ++j) {
                cin >> val;
                plan.push(max(0, val - 1) % SECTIONS_NUM);
            }
            buyers[i].plan = plan;
        }
    } else if (answer == "2") {
        if (argc != 3) {
            // Ввод с помощью файла.
            cout << "Enter the name of INPUT file:";
            cin >> input;
            cout << "Enter the name of OUTPUT file:";
            cin >> output;
        }
        // Поток файла ввода
        ifstream in(input);
        if (!in.is_open()) {
            cout << "Error on opening" << endl;
            return 1;
        }
        in >> num;
        buyers = vector<Buyer>(num);
        threads_buyers = vector<pthread_t>(buyers.size());
        for (int i = 0; i < buyers.size(); ++i) {
            buyers[i].id = i;
            // Количество деталей плана
            in >> num;
            queue<int> plan;
            for (int j = 0; j < num; ++j) {
                in >> val;
                plan.push(max(0, val - 1) % SECTIONS_NUM);
            }
            buyers[i].plan = plan;
        }
        in.close();
    } else {
        // Случайная генерация.
        if (argc != 4) {
            cout << "Enter the number of Buyers:";
            cin >> num;
            cout << "Enter the MAX number of tasks for each Buyer (>0):";
            cin >> max_count;
            cout << "Enter a seed for random generation:";
            cin >> seed;
        }
        buyers = vector<Buyer>(num);
        threads_buyers = vector<pthread_t>(buyers.size());
        srand(seed);
        // Создаём покупателей (дома готовят план покупок)
        for (int i = 0; i < buyers.size(); ++i) {
            buyers[i].id = i;
            // Создаём случайный план для покупателя
            num = 1 + rand() % max(1, max_count);
            queue<int> plan;
            for (int j = 0; j < num; ++j) {
                val = rand() % SECTIONS_NUM;
                plan.push(val);
            }
            buyers[i].plan = plan;
        }
    }

#if COLORS == 1
    system("color 00");
#endif
    str = Seller::getColor("\nThe store is opening!\n\n", 3);
    cumulative += str;
    cout << str;

    // Запуск продавцов (по одному в каждом отделе) По условию 3 отдела
    for (int i = 0; i < sellers.size(); i++) {
        // Зададим id для продавца
        sellers[i].id = i;
        // Запускаем поток
        pthread_create(&threads_sellers[i], nullptr, SellerFunc, &sellers[i]);
        str = Seller::getColor(
                "\nSeller: " + to_string(sellers[i].id + 1) + " " + "came to" + " work", i);
        cumulative += str;
        cout << str;
    }

    // Информация о покупателях
    for (int i = 0; i < buyers.size(); ++i) {
        buyers[i].id = i;
        // Случайное время, когда покупатель придёт в магазин
        buyers[i].time_start = rand() % 14;
        str = "\nBuyer: " + to_string(buyers[i].id + 1) +
              " has prepared a plan:" + buyers[i].getPlanToString();
        cumulative += str;
        cout << str;
    }
    cout << "\n";

    // Запускаем потоки покупателей
    for (int i = 0; i < buyers.size(); ++i) {
        pthread_create(&threads_buyers[i], nullptr, BuyerFunc, &buyers[i]);
    }

    // Тут параллельно происходит взаимодействие продавцов и покупателей

    // Ждём пока кончатся все покупатели
    for (unsigned long long threads_buyer: threads_buyers) {
        pthread_join(threads_buyer, nullptr);
    }

    // Завершаем работу продавцов
    flag = false;

    cout << "\n";

    // Ждём пока закроются все отделы
    for (unsigned long long threads_seller: threads_sellers) {
        pthread_join(threads_seller, nullptr);
    }

    str = Seller::getColor("\n\nThe store is closed!", 3);
    cumulative += str;
    cout << str;

    if (answer == "2") {
        ofstream out(output);
        if (!out.is_open()) {
            cout << "Error on opening" << endl;
            return 0;
        }
        out << cumulative;
    }

    pthread_mutex_destroy(&mutex_0);
    pthread_mutex_destroy(&mutex_1);
    pthread_mutex_destroy(&mutex_2);

    return 0;
}

int main(int argc, char **argv) {
    /*
   * Условие:
     *
   * Вариант 10. Задача о магазине - 1. Федоров Артём
     *
   * В магазине работают три отдела, каждый отдел обслуживает один продавец.
   * Покупатель, зайдя в магазин, делает покупки в одном или нескольких
   * произвольных отделах, обходя их в произвольном порядке. Если в выбранном
   * отделе продавец не свободен, покупатель становится в очередь и засыпает,
   * пока продавец не освободится. Создать многопоточное приложение,
   * моделирующее рабочий день магазина.
     *
   */
    try {
        int res = startTheStore(argc, argv);
        if (res == 1) {
            return 1;
        }
        cout << "\nEnter anything to end program:";
        cin >> str;
        return 0;
    } catch (const std::exception &exception) {
        cout << "\nProblem... The store burned down and had to close... "
                "Sorry for the inconvenience provided." << endl;
        return 1;
    }
}

#pragma clang diagnostic pop