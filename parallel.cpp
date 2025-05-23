#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>

using namespace std;

struct Item {
    int weight;
    int value;
    double valuePerWeight;
};

bool compareItems(const Item& a, const Item& b) {
    return a.valuePerWeight > b.valuePerWeight;
}

int main() {
    int capacity;
    int numItems;

    cout << "backpack capacity: ";
    cin >> capacity;

    cout << "number of items: ";
    cin >> numItems;

    vector<Item> items(numItems);

    cout << "Enter the weight and value of each item.:" << endl;
    for (int i = 0; i < numItems; ++i) {
        cout << "Item " << i + 1 << ": ";
        cin >> items[i].weight >> items[i].value;
        items[i].valuePerWeight = (double)items[i].value / items[i].weight;
    }

    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) {
        numThreads = 2;
    }
    numThreads = min((unsigned int)numItems, numThreads);

    vector<vector<Item>> itemPartitions(numThreads);
    int partitionSize = numItems / numThreads;
    int remainder = numItems % numThreads;

    int startIndex = 0;
    for (unsigned int i = 0; i < numThreads; ++i) {
        int currentPartitionSize = partitionSize + (i < remainder ? 1 : 0);
        itemPartitions[i].resize(currentPartitionSize);
        for (int j = 0; j < currentPartitionSize; ++j) {
            itemPartitions[i][j] = items[startIndex + j];
        }
        startIndex += currentPartitionSize;
    }

    vector<thread> threads;
    for (unsigned int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&itemPartitions, i]() {
            sort(itemPartitions[i].begin(), itemPartitions[i].end(), compareItems);
        });
    }

    for (thread& thread : threads) {
        thread.join();
    }

    items.clear();
    for (const vector<Item>& partition : itemPartitions) {
        items.insert(items.end(), partition.begin(), partition.end());
    }

    int currentWeight = 0;
    int totalValue = 0;

    cout << "Items added to backpack:" << endl;
    for (const Item& item : items) {
        if (currentWeight + item.weight <= capacity) {
            currentWeight += item.weight;
            totalValue += item.value;
            cout << "  Weight: " << item.weight << ", Value: " << item.value << endl;
        } else {
            int remainingCapacity = capacity - currentWeight;
            if (remainingCapacity > 0) {
                double fraction = (double)remainingCapacity / item.weight;
                totalValue += (int)(item.value * fraction);

                cout << "  Weight: " << remainingCapacity << ", Value: " << (int)(item.value * fraction) << endl;
                currentWeight = capacity;
                break;
            }
        }

        if (currentWeight == capacity) {
            break;
        }
    }

    cout << "Total backpack value: " << totalValue << endl;
    cout << "Total backpack weight: " << currentWeight << endl;

    return 0;
}