
// Load Balancer Simulator - DSA Project

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <string>
#include <functional>
using namespace std;

struct Server { int id, maxLoad, load; };
struct Request { int id, cost, server; };

// Min Heap
typedef pair<double, int> HeapItem;
typedef priority_queue<HeapItem, vector<HeapItem>, greater<HeapItem>> MinHeap;

vector<Server> servers;
queue<Request> waiting;
vector<Request> running;
vector<string> history;
vector<int> usedIds;

int strategy = 1;
int nextRR = 0;
const int THRESHOLD = 80;

int readInt() {
    int x;
    while (!(cin >> x)) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid! Enter a number: ";
    }
    return x;
}

// Linear Search
int findServer(int id) {
    for (int i = 0; i < servers.size(); i++)
        if (servers[i].id == id) return i;
    return -1;
}

double percent(Server s) { return s.load * 100.0 / s.maxLoad; }

bool moreLoaded(Server a, Server b) { return percent(a) > percent(b); }

bool fits(int i, int cost) { return servers[i].load + cost <= servers[i].maxLoad; }

// Build Min Heap
MinHeap buildHeap() {
    MinHeap heap;
    for (int i = 0; i < servers.size(); i++)
        heap.push({percent(servers[i]), i});
    return heap;
}

// Select Server
int pickServer(int cost) {
    int n = servers.size();

    if (strategy == 1) {
        for (int k = 0; k < n; k++) {
            int i = (nextRR + k) % n;
            if (fits(i, cost)) {
                nextRR = (i + 1) % n;
                return i;
            }
        }
    }
    else {
        MinHeap heap = buildHeap();
        while (!heap.empty()) {
            int i = heap.top().second;
            heap.pop();
            if (fits(i, cost)) return i;
        }
    }
    return -1;
}

// Place Request
bool place(Request r) {
    int i = pickServer(r.cost);
    if (i == -1) return false;

    servers[i].load += r.cost;
    r.server = servers[i].id;
    running.push_back(r);

    history.push_back("R" + to_string(r.id) + " (load " +
        to_string(r.cost) + ") -> S" + to_string(r.server));

    return true;
}

// Check Waiting Requests
void tryWaiting() {
    int n = waiting.size();

    for (int k = 0; k < n; k++) {
        Request r = waiting.front();
        waiting.pop();

        if (place(r))
            cout << "Waiting request R" << r.id << " got a server.\n";
        else
            waiting.push(r);
    }
}

void addServer() {
    if (servers.size() >= 100) {
        cout << "Maximum 100 servers reached!\n";
        return;
    }

    cout << "Server ID: ";
    int id = readInt();

    cout << "Max capacity: ";
    int cap = readInt();

    cout << "Current load: ";
    int cur = readInt();

    if (id <= 0 || findServer(id) != -1) {
        cout << "Invalid or duplicate server ID!\n";
        return;
    }

    if (cap <= 0) {
        cout << "Capacity must be positive!\n";
        return;
    }

    if (cur < 0 || cur > cap) {
        cout << "Invalid current load!\n";
        return;
    }

    Server s = {id, cap, cur};
    servers.push_back(s);

    cout << "Server S" << id << " added.\n";
    tryWaiting();
}

void removeServer() {
    cout << "Server ID to remove: ";
    int id = readInt();

    int i = findServer(id);

    if (i == -1) {
        cout << "Server not found!\n";
        return;
    }

    servers.erase(servers.begin() + i);

    for (int k = 0; k < running.size(); ) {
        if (running[k].server == id) {
            waiting.push(running[k]);
            running.erase(running.begin() + k);
        }
        else k++;
    }

    cout << "Server S" << id << " removed.\n";
    tryWaiting();
}

void addRequest() {
    if (usedIds.size() >= 1000) {
        cout << "Maximum 1000 requests reached!\n";
        return;
    }

    cout << "Request ID: ";
    int id = readInt();

    cout << "Request load: ";
    int cost = readInt();

    bool duplicate = false;
    for (int x : usedIds)
        if (x == id) duplicate = true;

    if (id <= 0 || cost < 0 || duplicate) {
        cout << "Invalid or duplicate request!\n";
        return;
    }

    usedIds.push_back(id);

    Request r = {id, cost, 0};

    if (!place(r)) {
        waiting.push(r);
        history.push_back("R" + to_string(id) + " (load " +
            to_string(cost) + ") -> WAITING");
    }

    cout << history.back() << endl;
}

void completeRequest() {
    cout << "Request ID completed: ";
    int id = readInt();

    for (int k = 0; k < running.size(); k++) {
        if (running[k].id == id) {
            int i = findServer(running[k].server);

            if (i == -1) {
                cout << "Server not found for running request!\n";
                return;
            }

            servers[i].load -= running[k].cost;

            history.push_back("R" + to_string(id) +
                " completed on S" + to_string(running[k].server));

            running.erase(running.begin() + k);

            cout << "Request R" << id << " completed.\n";
            tryWaiting();
            return;
        }
    }

    cout << "No running request with that ID!\n";
}

// Show Server Status
void showServers() {
    vector<Server> list = servers;
    sort(list.begin(), list.end(), moreLoaded);

    cout << "\n===== SERVER STATUS =====\n";

    for (int i = 0; i < list.size(); i++) {
        double p = percent(list[i]);

        cout << "S" << list[i].id << " Load: "
             << list[i].load << "/" << list[i].maxLoad
             << " | " << p << "% | "
             << string((int)p / 10, '#') << endl;
    }

    if (list.empty()) cout << "No servers.\n";
}

void showQueue() {
    cout << "\n===== WAITING QUEUE =====\n";

    queue<Request> copy = waiting;

    if (copy.empty()) cout << "Empty\n";

    while (!copy.empty()) {
        cout << "R" << copy.front().id
             << " (load " << copy.front().cost << ")\n";
        copy.pop();
    }
}

void showHistory() {
    cout << "\n===== ASSIGNMENT HISTORY =====\n";

    for (int i = 0; i < history.size(); i++)
        cout << i + 1 << ". " << history[i] << "\n";

    if (history.empty()) cout << "No history yet.\n";
}

void showOverloaded() {
    cout << "\n===== OVERLOADED SERVERS (>= "
         << THRESHOLD << "%) =====\n";

    bool found = false;

    for (int i = 0; i < servers.size(); i++) {
        if (percent(servers[i]) >= THRESHOLD) {
            cout << "S" << servers[i].id
                 << " Load: " << servers[i].load
                 << " Max: " << servers[i].maxLoad << "\n";
            found = true;
        }
    }

    if (!found) cout << "None\n";
}

void showHeap() {
    cout << "\n===== MIN-HEAP (least utilized first) =====\n";

    MinHeap heap = buildHeap();
    int step = 1;

    while (!heap.empty()) {
        int i = heap.top().second;
        double p = heap.top().first;

        cout << "Pop #" << step++ << ": S" << servers[i].id
             << " (" << p << "%)\n";

        heap.pop();
    }
}

int main() {
    cout << fixed << setprecision(1) << left;

    int choice = -1;

    while (choice != 0) {
        cout << "\n====== LOAD BALANCER SIMULATOR ======\n";

        if (strategy == 1)
            cout << "Strategy: Round Robin\n";
        else
            cout << "Strategy: Least Loaded\n";

        cout << "1.Add Server  2.Remove Server  3.Add Request  4.Complete Request\n";
        cout << "5.Select Strategy  6.Server Status  7.Waiting Queue  8.History\n";
        cout << "9.Overloaded Servers  10.Show Min-Heap  0.Exit\nChoice: ";

        choice = readInt();

        if (choice == 1) addServer();
        else if (choice == 2) removeServer();
        else if (choice == 3) addRequest();
        else if (choice == 4) completeRequest();

        else if (choice == 5) {
            cout << "1.Round Robin  2.Least Loaded: ";
            int s = readInt();

            if (s == 1 || s == 2) strategy = s;
            else cout << "Invalid strategy!\n";
        }

        else if (choice == 6) showServers();
        else if (choice == 7) showQueue();
        else if (choice == 8) showHistory();
        else if (choice == 9) showOverloaded();
        else if (choice == 10) showHeap();
        else if (choice != 0) cout << "Invalid choice!\n";
    }

    return 0;
}
