#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <numeric>

using namespace std;
namespace fs = filesystem;

struct Point {
    double x, y;
};

double distance(const Point& a, const Point& b) {
    return hypot(a.x - b.x, a.y - b.y);
}

double totalDistance(const vector<Point>& points, const vector<int>& path) {
    double dist = 0;
    for (size_t i = 0; i < path.size() - 1; i++) {
        dist += distance(points[path[i]], points[path[i + 1]]);
    }
    dist += distance(points[path.back()], points[path[0]]);
    return dist;
}

vector<Point> readFile(const string& filename) {
    ifstream file(filename);
    vector<Point> points;
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return points;
    }

    int n;
    file >> n;
    if (n <= 0) {
        cerr << "Invalid number of points in file: " << filename << endl;
        return points;
    }

    points.resize(n);
    for (int i = 0; i < n; ++i) {
        file >> points[i].x >> points[i].y;
    }
    return points;
}

vector<int> generateInitialPath(int size) {
    vector<int> path(size);
    iota(path.begin(), path.end(), 0);
    return path;
}

vector<int> solve2Opt(const vector<Point>& points, vector<int> path) {
    bool improved = true;
    while (improved) {
        improved = false;
        double bestDistance = totalDistance(points, path);

        for (size_t i = 1; i < path.size() - 1; i++) {
            for (size_t j = i + 1; j < path.size(); j++) {
                vector<int> newPath = path;
                reverse(newPath.begin() + i, newPath.begin() + j);

                double newDistance = totalDistance(points, newPath);
                if (newDistance < bestDistance) {
                    path = std::move(newPath);
                    bestDistance = newDistance;
                    improved = true;
                }
            }
        }
    }
    return path;
}

vector<int> solve3Opt(const vector<Point>& points, vector<int> path) {
    bool improved = true;
    while (improved) {
        improved = false;
        double bestDistance = totalDistance(points, path);

        for (size_t i = 0; i < path.size() - 2; i++) {
            for (size_t j = i + 1; j < path.size() - 1; j++) {
                for (size_t k = j + 1; k < path.size(); k++) {
                    vector<int> bestPath = path;

                    vector<int> newPath1 = path;
                    reverse(newPath1.begin() + i, newPath1.begin() + j);
                    reverse(newPath1.begin() + j, newPath1.begin() + k);

                    vector<int> newPath2 = path;
                    reverse(newPath2.begin() + i, newPath2.begin() + k);

                    double dist1 = totalDistance(points, newPath1);
                    double dist2 = totalDistance(points, newPath2);

                    if (dist1 < bestDistance) {
                        bestPath = std::move(newPath1);
                        bestDistance = dist1;
                        improved = true;
                    }
                    if (dist2 < bestDistance) {
                        bestPath = std::move(newPath2);
                        bestDistance = dist2;
                        improved = true;
                    }

                    if (improved) path = bestPath;
                }
            }
        }
    }
    return path;
}

void processFile(const string& filename, bool tableMode, int fileIndex) {
    auto points = readFile(filename);
    if (points.empty()) return;

    vector<int> initialPath = generateInitialPath(points.size());

    auto start2 = chrono::high_resolution_clock::now();
    auto path2 = solve2Opt(points, initialPath);
    auto end2 = chrono::high_resolution_clock::now();
    double time2 = chrono::duration<double>(end2 - start2).count();
    double len2 = totalDistance(points, path2);

    double len3 = 0.0, time3 = 0.0;
    vector<int> path3;

    if (fileIndex < 30) {
        auto start3 = chrono::high_resolution_clock::now();
        path3 = solve3Opt(points, path2);
        auto end3 = chrono::high_resolution_clock::now();
        time3 = chrono::duration<double>(end3 - start3).count();
        len3 = totalDistance(points, path3);
    }

    if (tableMode) {
        cout << filename << "\t" << len2 << "\t" << time2;
        if (fileIndex < 30) {
            cout << "\t" << len3 << "\t" << time3;
        }
        cout << endl;
    } else {
        cout << "File: " << filename << endl;
        cout << "2-opt: Length = " << len2 << ", Time = " << time2 << " sec, Loop: 1, Path: ";
        for (int v : path2) cout << v << " ";
        cout << endl;

        if (fileIndex < 30) {
            cout << "3-opt: Length = " << len3 << ", Time = " << time3 << " sec, Loop: 1, Path: ";
            for (int v : path3) cout << v << " ";
            cout << endl;
        }
    }
    }

    void processDirectory(const string& dir) {
    cout << "File\tLength op2\tTime op2\tLength op3\tTime op3" << endl;
    vector<pair<string, uintmax_t>> files;
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            files.emplace_back(entry.path().string(), fs::file_size(entry.path()));
        }
    }

    sort(files.begin(), files.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    int fileIndex = 0;
    for (const auto& file : files) {
        processFile(file.first, true, fileIndex);
        fileIndex++;
    }
    }

    int main() {
    cout << "Type 1 for processing a single file, 2 for processing a full directory: ";
    int mode;
    cin >> mode;
    cin.ignore();

    cout << "Enter path: ";
    string path;
    getline(cin, path);

    if (mode == 1) {
        processFile(path, false, 0);
    } else if (mode == 2) {
        processDirectory(path);
    } else {
        cout << "Invalid mode" << endl;
    }

    return 0;
    }