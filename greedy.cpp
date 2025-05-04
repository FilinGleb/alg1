#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Item {
    int weight;
    int value;
    double valuePerWeight;
};

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

    sort(items.begin(), items.end(), [](const Item& a, const Item& b) {
        return a.valuePerWeight > b.valuePerWeight;
    });

    int currentWeight = 0;
    int totalValue = 0;

    cout << "Items added to backpack:" << endl;
    for (const auto& item : items) {
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
