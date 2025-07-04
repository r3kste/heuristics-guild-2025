#include <bits/stdc++.h>
using namespace std;

double speed;
double t_realize, t_empty;

struct Point {
    double x, y;

    Point(double x = 0, double y = 0) : x(x), y(y) {}

    double distance(const Point &other) const {
        return sqrt(pow(x - other.x, 2) + pow(y - other.y, 2));
    }
};

class Bastion {
  public:
    int id;
    Point position;
    double value;

    friend ostream &operator<<(ostream &os, const Bastion &bastion) {
        os << "Bastion " << bastion.id << " at (" << bastion.position.x << ", "
           << bastion.position.y << ") with value " << bastion.value;
        return os;
    }

    double distance(const Point &point) const { return position.distance(point); }

    double actual_value(double time) const {
        if (time < t_realize) {
            return value;
        } else if (time >= t_empty) {
            return 0.0;
        } else {
            double fraction = (time - t_realize) / (t_empty - t_realize);
            return value * (1.0 - fraction);
        }
    }

    // Preference function from a point that does NOT consider the decay over time
    double preference(const Point &from = Point(0, 0)) const {
        double nr = value;
        nr = pow(nr, 2);
        double dr = distance(from);
        dr = pow(dr, 3);

        dr += 1e-9;
        return nr / dr;
    }

    // Preference function from a point and time that considers the decay over time
    double actual_preference(const Point &from, double current_time) const {
        double time = current_time + distance(from) / speed;
        double nr = actual_value(time);
        nr = pow(nr, 2);
        double dr = distance(from);
        dr = pow(dr, 3);

        dr += 1e-9;
        return nr / dr;
    }

    bool operator==(const Bastion &other) const { return id == other.id; }
};

double gold_collected(const vector<Bastion> &tour) {
    Point current_position(0, 0);
    double time_taken = 0.0;
    double total_gold = 0.0;

    for (int i = 0; i < tour.size(); i++) {
        double distance = tour[i].distance(current_position);
        current_position = tour[i].position;
        time_taken += distance / speed;
        if (time_taken >= t_empty) {
            return total_gold;
        }
        total_gold += tour[i].actual_value(time_taken);
    }
    return total_gold;
}

int effective_tour_length(vector<Bastion> &bastions) {
    double total_distance = 0.0;
    double time_taken = 0.0;
    Point current_position(0, 0);
    int i = 0;
    for (const Bastion &bastion : bastions) {
        double distance = bastion.distance(current_position);
        current_position = bastion.position;
        time_taken += distance / speed;
        if (time_taken >= t_empty) {
            break;
        }
        i++;
        total_distance += distance;
    }
    return i;
}

vector<Bastion> brute_force(vector<Bastion> &bastions) {
    vector<Bastion> best_tour;
    double best_value = 0.0;

    auto sorted_bastions = bastions;
    sort(sorted_bastions.begin(), sorted_bastions.end(),
         [](const Bastion &a, const Bastion &b) { return a.id < b.id; });

    do {
        double current_value = gold_collected(sorted_bastions);
        if (current_value > best_value) {
            best_value = current_value;
            best_tour = sorted_bastions;
        }
    } while (next_permutation(
        sorted_bastions.begin(), sorted_bastions.end(),
        [](const Bastion &a, const Bastion &b) { return a.id < b.id; }));

    return best_tour;
}

vector<Bastion> greedy(vector<Bastion> bastions, string preference = "normal") {
    vector<Bastion> tour;
    double current_time = 0.0;
    Point current_position(0, 0);

    while (!bastions.empty()) {
        auto best_neighbor = max_element(
            bastions.begin(), bastions.end(), [&](const Bastion &a, const Bastion &b) {
                if (preference == "normal") {
                    return a.preference(current_position) <
                           b.preference(current_position);
                } else {
                    return a.actual_preference(current_position, current_time) <
                           b.actual_preference(current_position, current_time);
                }
            });

        if (best_neighbor == bastions.end()) {
            break;
        }

        Bastion best_bastion = *best_neighbor;
        bastions.erase(best_neighbor);

        double distance = best_bastion.distance(current_position);
        current_time += distance / speed;
        current_position = best_bastion.position;

        tour.push_back(best_bastion);
    }

    return tour;
}

vector<Bastion> simulated_annealing(vector<Bastion> &bastions, int time_limit = 3000,
                                    double initial_temp = 1.0,
                                    double cooling_rate = 0.995) {
    // mt19937 gen(chrono::high_resolution_clock::now().time_since_epoch().count());
    mt19937 gen(12);
    uniform_real_distribution<> prob_dis(0.0, 1.0);
    uniform_int_distribution<> dis(0, bastions.size() - 1);

    vector<Bastion> current_tour = bastions;
    double current_value = gold_collected(current_tour);

    vector<Bastion> best_tour = current_tour;
    double best_value = current_value;

    double temperature = initial_temp;
    auto end_time =
        chrono::high_resolution_clock::now() + chrono::milliseconds(time_limit);
    int iterations = 0;
    while (chrono::high_resolution_clock::now() < end_time) {
        iterations++;
        vector<Bastion> new_tour = current_tour;

        if (new_tour.size() >= 2) {
            int i = dis(gen), j = dis(gen);
            int op = gen() % 3;
            if (op == 0) {
                if (i != j) {
                    swap(new_tour[i], new_tour[j]);
                }
            } else if (op == 1) {
                if (i > j) {
                    swap(i, j);
                }
                reverse(new_tour.begin() + i, new_tour.begin() + j + 1);
            } else {
                if (i != j) {
                    Bastion temp = new_tour[i];
                    new_tour.erase(new_tour.begin() + i);
                    new_tour.insert(new_tour.begin() + j, temp);
                }
            }
        }

        double new_value = gold_collected(new_tour);

        if (new_value >= current_value) {
            current_tour = new_tour;
            current_value = new_value;
            if (current_value > best_value) {
                best_tour = current_tour;
                best_value = current_value;
            }
        } else {
            temperature = max(temperature, 1e-9);
            double acceptance_probability =
                exp((new_value - current_value) / temperature);
            double probability = prob_dis(gen);
            if (probability < acceptance_probability) {
                current_tour = new_tour;
                current_value = new_value;
            }
        }

        temperature *= cooling_rate;
    }

    cerr << "Iterations: " << iterations << "\n";

    return best_tour;
}

vector<Bastion> expand_path(vector<Bastion> &bastions) {
    int effective_length = effective_tour_length(bastions);
    vector<Bastion> actual_tour =
        vector<Bastion>(bastions.begin(), bastions.begin() + effective_length);
    vector<Bastion> remainining_bastions(bastions.begin() + effective_length,
                                         bastions.end());
    Point current_position(0, 0);
    double current_time = 0.0;

    vector<Bastion> tour;
    int ptr_actual = 0;
    int ptr_remaining = 0;

    while (ptr_actual < actual_tour.size()) {
        Bastion &next_bastion = actual_tour[ptr_actual];

        ptr_remaining = 0;
        while (ptr_remaining < remainining_bastions.size()) {
            Bastion &new_bastion = remainining_bastions[ptr_remaining];

            double old_distance = next_bastion.distance(current_position);
            double new_distance = new_bastion.distance(current_position) +
                                  new_bastion.distance(next_bastion.position);

            if (new_distance <= old_distance * 1.1) {
                double distance = new_bastion.distance(current_position);
                current_time += distance / speed;
                current_position = new_bastion.position;
                tour.push_back(new_bastion);
                remainining_bastions.erase(remainining_bastions.begin() +
                                           ptr_remaining);
                ptr_remaining = 0;
            } else {
                ptr_remaining++;
            }
        }

        double distance = next_bastion.distance(current_position);
        current_time += distance / speed;
        current_position = next_bastion.position;
        tour.push_back(next_bastion);

        ptr_actual++;
    }

    return tour;
}

int main() {
#ifndef ONLINE_JUDGE
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
#endif

    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    long long int no_of_total_bastions;
    cin >> no_of_total_bastions;

    vector<Bastion> total_bastions(no_of_total_bastions);
    for (int i = 0; i < no_of_total_bastions; ++i) {
        double x, y, value;
        cin >> x >> y >> value;
        total_bastions[i] = {i, Point(x, y), value};
    }

    cin >> speed;
    cin >> t_realize >> t_empty;

    auto start_time = chrono::high_resolution_clock::now();

    vector<Bastion> reachable_bastions;
    const Point origin(0, 0);
    for (const Bastion &bastion : total_bastions) {
        if (bastion.distance(origin) < speed * t_empty) {
            reachable_bastions.push_back(bastion);
        }
    }

    vector<Bastion> tour;
    if (reachable_bastions.size() < 10) {
        tour = brute_force(reachable_bastions);
    } else {
        tour = greedy(reachable_bastions, "normal");
        tour = simulated_annealing(tour, 2990);
        tour = expand_path(tour);

        if (effective_tour_length(tour) < 10) {
            tour.resize(effective_tour_length(tour));
            brute_force(tour);
        }
    }

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);

    vector<Bastion> result;
    Point current_position(0, 0);
    double time_taken = 0.0;
    double gold_collected = 0.0;
    for (const Bastion &bastion : tour) {
        double distance = bastion.distance(current_position);
        current_position = bastion.position;
        time_taken += distance / speed;
        if (time_taken >= t_empty) {
            break;
        }
        gold_collected += bastion.actual_value(time_taken);
        result.push_back(bastion);
    }

    cout << result.size() << "\n";
    for (const Bastion &bastion : result) {
        cout << bastion.id << " ";
    }
    cout << "\n";

#ifndef ONLINE_JUDGE
    double total_gold = 0.0;
    for (const Bastion &bastion : total_bastions) {
        total_gold += bastion.value;
    }
    cerr << "Gold collected: " << gold_collected << "\n";
    cerr << "Total gold value: " << total_gold << "\n";
    cerr << "Score: " << 200 * gold_collected / total_gold << "\n";
    cerr << "Time taken: " << duration.count() << " ms\n";
#endif
}
