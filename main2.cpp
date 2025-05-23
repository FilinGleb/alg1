#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <limits>

using namespace std;
struct Item {
    unsigned weight = 0;  
    unsigned cost = 0;    
    static Item end_of_list() {
        return {numeric_limits<unsigned>::max(), numeric_limits<unsigned>::max()};
    }

    bool is_end_of_list() const {
        return weight == numeric_limits<unsigned>::max();
    }
    Item create_partial(unsigned partial_weight) const {
        return {partial_weight, (cost * partial_weight + weight - 1) / weight};
    }
    Item& operator+=(const Item& rhs) {
        weight += rhs.weight;
        cost += rhs.cost;
        return *this;
    }
    Item& operator-=(const Item& rhs) {
        weight -= rhs.weight;
        cost -= rhs.cost;
        return *this;
    }
    double unit_cost() const {
        return (double)cost / weight;
    }

    bool is_more_valuable(const Item& rhs) const {
        return cost > rhs.cost;
    }
    friend ostream& operator<<(ostream& strm, const Item& item) {
        strm << "{ weight = " << item.weight << ", cost = " << item.cost << " }";
        return strm;
    }
};
class Backpack {
public:
    using ItemsContainer = list<Item>;
    Backpack(unsigned max_capacity) : max_capacity(max_capacity) {}
    Item find_optimal(ItemsContainer available_items) {
        available_items.sort([](const Item& lhs, const Item& rhs) {
            return lhs.unit_cost() > rhs.unit_cost();
        });
        available_items.push_back(Item::end_of_list());
        items_in_backpack.clear();

        Backpack current_state(max_capacity);
        find_optimal_recursive(current_state, available_items.begin());
        return total_value;
    }
    friend ostream& operator<<(ostream& strm, const Backpack& backpack) {
        strm << backpack.total_value << ": ";
        for (const Item& item : backpack.items_in_backpack)
            strm << item << " ";
        return strm;
    }

private:
    ItemsContainer items_in_backpack;
    Item total_value;
    unsigned max_capacity;

    void remove_last_item() {
        Item item = std::move(items_in_backpack.back());
        items_in_backpack.pop_back();
        total_value -= item;
    }


    bool try_add_item(Item item) {
        if (total_value.weight + item.weight > max_capacity)
            return false;

        total_value += item;
        items_in_backpack.push_back(std::move(item));
        return true;
    }

    void find_optimal_recursive(Backpack& current_state, ItemsContainer::const_iterator it_available) {
        if (it_available->is_end_of_list()) {
            if (current_state.total_value.is_more_valuable(total_value)) {
                *this = current_state;
            }
            return;
        }
        Item greedy_estimate_exclude = calculate_greedy_upper_bound(current_state, next(it_available));
        if (greedy_estimate_exclude.is_more_valuable(total_value)) {
            find_optimal_recursive(current_state, next(it_available));
        }
        if (current_state.try_add_item(*it_available)) {
            Item greedy_estimate_include = calculate_greedy_upper_bound(current_state, next(it_available));
            if (greedy_estimate_include.is_more_valuable(total_value)) {
                find_optimal_recursive(current_state, next(it_available));
            }

            current_state.remove_last_item();
        }
    }
    Item calculate_greedy_upper_bound(const Backpack& current_state, ItemsContainer::const_iterator it_start) {
        Item total = current_state.total_value; 

        for (auto it = it_start; !it->is_end_of_list(); ++it) {
            if (total.weight + it->weight > max_capacity) {
                unsigned remaining_capacity = max_capacity - total.weight;
                total += it->create_partial(remaining_capacity);
                break; 
            } else {
                total += *it;
            }
        }
        return total;
    }
};

int main() {
    unsigned num_items, max_capacity;
    cin >> num_items >> max_capacity;

    
    vector<Item> items(num_items);
    for (unsigned i = 0; i < num_items; ++i) {
        cin >> items[i].cost >> items[i].weight;
    }
    Backpack backpack(max_capacity);
    Item best_solution = backpack.find_optimal(list<Item>(items.begin(), items.end())); 
    cout << best_solution.cost << endl;

    return 0;
}
