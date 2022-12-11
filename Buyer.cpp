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