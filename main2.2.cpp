#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <limits>
#include <random>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <string>

using namespace std;
using namespace chrono;

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

class GeneticKnapsackSolver {
public:
    struct Chromosome {
        vector<bool> genes;
        unsigned total_cost = 0;
        unsigned total_weight = 0;

        bool operator<(const Chromosome& other) const {
            return total_cost > other.total_cost;
        }
    };

    GeneticKnapsackSolver(const vector<Item>& items, unsigned capacity, 
                        size_t population_size = 100, double mutation_rate = 0.01,
                        unsigned generations = 100)
        : items(items), capacity(capacity), population_size(population_size),
          mutation_rate(mutation_rate), generations(generations),
          engine(random_device{}()) {}

    Chromosome solve() {
        initialize_population();

        for (unsigned gen = 0; gen < generations; ++gen) {
            evolve_population();
            sort(population.begin(), population.end());
        }

        return population[0];
    }

private:
    const vector<Item>& items;
    unsigned capacity;
    size_t population_size;
    double mutation_rate;
    unsigned generations;
    vector<Chromosome> population;
    mt19937 engine;

    void initialize_population() {
        population.resize(population_size);
        uniform_int_distribution<int> dist(0, 1);

        for (auto& chrom : population) {
            chrom.genes.resize(items.size());
            for (size_t i = 0; i < items.size(); ++i) {
                if (dist(engine)) {
                    chrom.genes[i] = true;
                    chrom.total_cost += items[i].cost;
                    chrom.total_weight += items[i].weight;
                }
            }
            repair(chrom);
        }
        sort(population.begin(), population.end());
    }

    void repair(Chromosome& chrom) {
        while (chrom.total_weight > capacity) {
            vector<size_t> present_items;
            for (size_t i = 0; i < items.size(); ++i)
                if (chrom.genes[i]) present_items.push_back(i);

            if (present_items.empty()) break;

            uniform_int_distribution<size_t> dist(0, present_items.size()-1);
            size_t idx = present_items[dist(engine)];
            chrom.genes[idx] = false;
            chrom.total_cost -= items[idx].cost;
            chrom.total_weight -= items[idx].weight;
        }
    }

    void evolve_population() {
        vector<Chromosome> new_population;
        new_population.push_back(population[0]); 

        while (new_population.size() < population_size) {
            const auto& parent1 = select_parent();
            const auto& parent2 = select_parent();
            auto offspring = crossover(parent1, parent2);
            mutate(offspring);
            repair(offspring);
            new_population.push_back(offspring);
        }

        population = std::move(new_population);
    }

    const Chromosome& select_parent() {
        uniform_int_distribution<size_t> dist(0, population.size()-1);
        size_t a = dist(engine);
        size_t b = dist(engine);
        return population[a] < population[b] ? population[a] : population[b];
    }

    Chromosome crossover(const Chromosome& parent1, const Chromosome& parent2) {
        uniform_int_distribution<size_t> point_dist(0, items.size()-1);
        size_t crossover_point = point_dist(engine);

        Chromosome child;
        child.genes.resize(items.size());

        for (size_t i = 0; i < crossover_point; ++i)
            child.genes[i] = parent1.genes[i];

        for (size_t i = crossover_point; i < items.size(); ++i)
            child.genes[i] = parent2.genes[i];

        calculate_fitness(child);
        return child;
    }

    void mutate(Chromosome& chrom) {
        bernoulli_distribution dist(mutation_rate);
        for (size_t i = 0; i < items.size(); ++i) {
            if (dist(engine)) {
                chrom.genes[i] = !chrom.genes[i];
                if (chrom.genes[i]) {
                    chrom.total_cost += items[i].cost;
                    chrom.total_weight += items[i].weight;
                } else {
                    chrom.total_cost -= items[i].cost;
                    chrom.total_weight -= items[i].weight;
                }
            }
        }
    }

    void calculate_fitness(Chromosome& chrom) {
        chrom.total_cost = 0;
        chrom.total_weight = 0;
        for (size_t i = 0; i < items.size(); ++i) {
            if (chrom.genes[i]) {
                chrom.total_cost += items[i].cost;
                chrom.total_weight += items[i].weight;
            }
        }
    }
};

    int main() {
        string filename;
        cout << "Enter the test case name from data folder (e.g. ks_4_0): ";
        cin >> filename;

        ifstream file("data/" + filename);
        if (!file) {
            cerr << "Cannot open file: data/" << filename << endl;
            return 1;
        }

        unsigned num_items, max_capacity;
        file >> num_items >> max_capacity;

        vector<Item> items(num_items);
        for (unsigned i = 0; i < num_items; ++i) {
            file >> items[i].cost >> items[i].weight;
        }

    Backpack backpack(max_capacity);
    auto start = high_resolution_clock::now();
    Item bb_solution = backpack.find_optimal(list<Item>(items.begin(), items.end()));
    auto bb_duration = duration_cast<milliseconds>(high_resolution_clock::now() - start);

    start = high_resolution_clock::now();
    GeneticKnapsackSolver ga_solver(items, max_capacity);
    auto ga_solution = ga_solver.solve();
    auto ga_duration = duration_cast<milliseconds>(high_resolution_clock::now() - start);

    cout << "Method\t\tCost\tTime (ms)\n";
    cout << "Branch & Bound\t" << bb_solution.cost << "\t" << bb_duration.count() << endl;
    cout << "Genetic Algo\t" << ga_solution.total_cost << "\t" << ga_duration.count() << endl;

    return 0;
}