#include <bits/stdc++.h>

using namespace std;

typedef vector<pair<int, int>> AttackSequence;
typedef vector<pair<int, int>> SelectionSequence;

struct Box {
  public:
    int idx;
    int hardness;
    int current_hardness;

    bool is_destroyed() { return current_hardness <= 0; }

    void attack(int damage) { current_hardness -= damage; }

    void reset() { current_hardness = hardness; }
};

struct Weapon {
  public:
    int idx;
    int durability;
    int current_durability;
    int durability_loss = 1;
    bool unlocked = false;
    vector<int> powers;
    double _value = -1;

    double value() {
        if (_value != -1) {
            return _value;
        }
        double sum = 0.0;
        for (int power : powers) {
            sum += double(power);
        }
        _value = sum * double(durability);
        return _value;
    }

    bool can_use() {
        if (!unlocked) {
            return false;
        }
        if (current_durability <= 0) {
            return false;
        }
        return true;
    }

    void use() { current_durability -= durability_loss; }

    void reset() {
        current_durability = durability;
        unlocked = false;
    }
};

class Environment {
  public:
    int N;
    map<int, Box> boxes;
    map<int, Weapon> weapons;

    int unlocked_weapons = 0;
    unordered_set<int> available_weapons;
    unordered_set<int> remaining_boxes;

    // (weapon_idx, box_idx) -> time taken by weapon to destroy box
    map<pair<int, int>, int> no_of_attacks;

    Environment(int N, vector<int> hardnesses, vector<int> durabilities,
                vector<vector<int>> powers) {
        this->N = N;

        for (int i = 0; i < N; i++) {
            boxes[i].idx = i;
            boxes[i].hardness = hardnesses[i];
            boxes[i].current_hardness = hardnesses[i];
        }

        // Bare Hands
        weapons[-1].durability = 1;
        weapons[-1].current_durability = 1;
        weapons[-1].powers.assign(N, 1);
        weapons[-1].idx = -1;
        weapons[-1].durability_loss = 0;
        weapons[-1].unlocked = true;

        for (int i = 0; i < N; i++) {
            weapons[i].idx = i;
            weapons[i].durability = durabilities[i];
            weapons[i].current_durability = durabilities[i];
            weapons[i].powers = powers[i];
        }

        available_weapons.insert(-1);
        for (int i = 0; i < N; i++) {
            remaining_boxes.insert(i);
        }

        for (int weapon_idx = 0; weapon_idx < N; weapon_idx++) {
            no_of_attacks[{-1, weapon_idx}] = boxes[weapon_idx].hardness;
            for (int box_idx = 0; box_idx < N; box_idx++) {
                if (box_idx == weapon_idx) {
                    continue;
                }

                int _no_of_attacks = 0;
                double hardness = double(boxes[box_idx].hardness);
                double power = double(weapons[weapon_idx].powers[box_idx]);
                double max_damage = weapons[weapon_idx].durability * power;

                if (hardness <= max_damage) {
                    _no_of_attacks = ceil(hardness / power);
                } else {
                    double remaining_hardness = hardness - double(max_damage);
                    _no_of_attacks =
                        weapons[weapon_idx].durability + remaining_hardness;
                }

                no_of_attacks[{weapon_idx, box_idx}] = _no_of_attacks;
            }
        }
    }

    /* Attack box with weapon */
    void attack(int weapon_idx, int box_idx) {
        Box &box = boxes[box_idx];
        Weapon &weapon = weapons[weapon_idx];

        if (!weapon.can_use()) {
            cerr << "Weapon " << weapon_idx << " is already broken or locked." << endl;
            return;
        }
        if (box.is_destroyed()) {
            cerr << "Box " << box_idx << " is already destroyed." << endl;
            return;
        }

        weapon.use();
        box.attack(weapon.powers[box_idx]);

        if (!weapon.can_use()) {
            available_weapons.erase(weapon_idx);
        }
        if (box.is_destroyed()) {
            weapons[box_idx].unlocked = true;
            unlocked_weapons++;
            remaining_boxes.erase(box_idx);
            available_weapons.insert(box_idx);
        }
    }

    void deplete(int weapon_idx, int box_idx) {
        Box &box = boxes[box_idx];
        Weapon &weapon = weapons[weapon_idx];

        while (true) {
            if (box.is_destroyed() || !weapon.can_use()) {
                break;
            }
            attack(weapon_idx, box_idx);
        }
    }

    void deplete(int weapon_idx, int box_idx, AttackSequence &attack_sequence) {
        Box &box = boxes[box_idx];
        Weapon &weapon = weapons[weapon_idx];

        while (true) {
            if (box.is_destroyed() || !weapon.can_use()) {
                break;
            }
            attack(weapon_idx, box_idx);
            attack_sequence.push_back({weapon_idx, box_idx});
        }
    }

    void reset() {
        for (int i = 0; i < N; i++) {
            boxes[i].reset();
            weapons[i].reset();
        }
        unlocked_weapons = 0;

        available_weapons.clear();
        available_weapons.insert(-1);

        remaining_boxes.clear();
        for (int i = 0; i < N; i++) {
            remaining_boxes.insert(i);
        }
    }

    pair<int, int> best_weapon_box_pair();
    double heuristic(int weapon_idx, int box_idx);
};

AttackSequence selection_to_attack_sequence(SelectionSequence &selection_sequence,
                                            Environment &env) {
    AttackSequence attack_sequence;

    for (auto &[weapon_idx, box_idx] : selection_sequence) {
        env.deplete(weapon_idx, box_idx, attack_sequence);
    }

    env.reset();
    return attack_sequence;
}

AttackSequence greedy(Environment &env, int first_box = -1) {
    AttackSequence attack_sequence;

    while (env.unlocked_weapons < env.N) {
        double best_ratio = numeric_limits<double>::max();
        int best_weapon_idx;
        int best_box_idx;

        if (first_box != -1) {
            best_box_idx = first_box;
            best_weapon_idx = -1;
            first_box = -1; // Only use first_box for the first iteration
        } else {
            best_box_idx = -1;
            best_weapon_idx = -1;
            for (int box_idx : env.remaining_boxes) {
                Box &box = env.boxes[box_idx];

                for (int weapon_idx : env.available_weapons) {
                    Weapon &weapon = env.weapons[weapon_idx];

                    if (!weapon.can_use()) {
                        continue;
                    }

                    double ratio = env.heuristic(weapon_idx, box_idx);
                    if (ratio <= best_ratio) {
                        best_ratio = ratio;
                        best_weapon_idx = weapon_idx;
                        best_box_idx = box_idx;
                    }
                }
            }
        }

        // selection_sequence.push_back({best_weapon_idx, best_box_idx});
        env.deplete(best_weapon_idx, best_box_idx, attack_sequence);
    }

    env.reset();
    return attack_sequence;
}

AttackSequence solve_future_value(Environment &env, int first_box = -1) {
    AttackSequence attack_sequence;

    while (env.unlocked_weapons < env.N) {
        if (first_box != -1) {
            env.deplete(-1, first_box, attack_sequence);
            first_box = -1;
            continue;
        }
        auto [best_weapon_idx, best_box_idx] = env.best_weapon_box_pair();

        // Perform the attack
        env.deplete(best_weapon_idx, best_box_idx, attack_sequence);
    }

    env.reset();
    return attack_sequence;
}

AttackSequence kruskal(Environment &env) {
    AttackSequence attack_sequence;

    // Create edges
    vector<tuple<double, int, int, int>> edges; // (score, hardness, weapon_idx,
                                                // box_idx)
    for (int weapon_idx = -1; weapon_idx < env.N; weapon_idx++) {
        for (int box_idx = 0; box_idx < env.N; box_idx++) {
            if (env.no_of_attacks.find({weapon_idx, box_idx}) ==
                env.no_of_attacks.end()) {
                continue;
            }
            double score = env.no_of_attacks[{weapon_idx, box_idx}];
            edges.push_back(
                {score, -env.weapons[weapon_idx].value(), weapon_idx, box_idx});
        }
    }
    // Sort edges by score
    sort(edges.begin(), edges.end());

    // Kruskal's algorithm
    // At each point select the edge with the least no_of_attacks
    // If that box is already destroyed, skip it
    // If the weapon is not unlocked yet, unlock it using bare hands
    // If the weapon is unlocked, use it to attack the box

    for (auto &[score, hardness, weapon_idx, box_idx] : edges) {
        if (env.unlocked_weapons >= env.N) {
            break;
        }
        Box &box = env.boxes[box_idx];
        Weapon &weapon = env.weapons[weapon_idx];

        if (box.is_destroyed()) {
            continue;
        }

        if (!weapon.unlocked) {
            // Break box[weapon_idx] using best weapon to break this box
            int best_score = numeric_limits<int>::max();
            int best_weapon_idx = -1;

            int no_of_available_weapons = env.available_weapons.size();
            int cnt = 0;
            for (int avail_weapon_idx : env.available_weapons) {
                if (cnt >= no_of_available_weapons) {
                    break;
                }
                Weapon &avail_weapon = env.weapons[avail_weapon_idx];
                int curr_score = env.no_of_attacks[{avail_weapon_idx, weapon_idx}];
                if (curr_score < best_score) {
                    best_score = curr_score;
                    best_weapon_idx = avail_weapon_idx;
                }
                cnt++;
            }
            env.deplete(best_weapon_idx, weapon_idx, attack_sequence);
        }

        // Attack box using weapon
        env.deplete(weapon_idx, box_idx, attack_sequence);
    }
    env.reset();
    return attack_sequence;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

#ifdef LOCAL
    freopen("/home/r3kste/code/heuristic-guild-2025/atcoder/contest/055/input.txt", "r",
            stdin);
    freopen("/home/r3kste/code/heuristic-guild-2025/atcoder/contest/055/output.txt",
            "w", stdout);
#endif

    int N;
    cin >> N;

    vector<int> hardnesses(N);
    for (int i = 0; i < N; i++) {
        cin >> hardnesses[i];
    }

    vector<int> durabilities(N);
    for (int i = 0; i < N; i++) {
        cin >> durabilities[i];
    }

    vector<vector<int>> powers(N, vector<int>(N));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cin >> powers[i][j];
        }
    }

    Environment env(N, hardnesses, durabilities, powers);

    int best_first_box = -1;
    int best_score = numeric_limits<int>::min();
    AttackSequence best_attack_sequence;

    vector<pair<int, int>> box_indices;
    for (int box_idx = 0; box_idx < N; box_idx++) {
        double ratio = -env.weapons[box_idx].value() / hardnesses[box_idx];
        box_indices.push_back({ratio, box_idx});
    }

    // Sort boxes in ascending order
    sort(box_indices.begin(), box_indices.end());
    vector<int> sorted_order;
    for (auto &[hardness, idx] : box_indices) {
        sorted_order.push_back(idx);
    }

    auto start_time = chrono::steady_clock::now();

    int idx = 0;
    while (chrono::duration<double>(chrono::steady_clock::now() - start_time).count() <
           1.97) {

        int first_box = sorted_order[idx];
        // AttackSequence attack_sequence1 = greedy(env, first_box);
        AttackSequence attack_sequence = solve_future_value(env, first_box);

        // AttackSequence attack_sequence;
        // if (attack_sequence1.size() < attack_sequence2.size()) {
        //     attack_sequence = attack_sequence1;
        // } else {
        //     attack_sequence = attack_sequence2;
        // }

        int score = 0;
        for (int hardness : hardnesses) {
            score += hardness;
        }
        score -= attack_sequence.size();
        score += 1;

        cerr << "First Box: " << first_box << ", Score: " << score << endl;
        if (score > best_score) {
            best_score = score;
            best_first_box = first_box;
            best_attack_sequence = attack_sequence;
        }

        idx++;
    }

    best_attack_sequence = solve_future_value(env, best_first_box);
    // best_attack_sequence = kruskal(env);

    for (auto &[weapon_idx, box_idx] : best_attack_sequence) {
        cout << weapon_idx << " " << box_idx << "\n";
    }

    int score = 1;
    for (int hardness : hardnesses) {
        score += hardness;
    }
    score -= best_attack_sequence.size();

    cerr << "Score: " << score << endl;

    return 0;
}

/*The function to minimize*/
double Environment::heuristic(int weapon_idx, int box_idx) {
    Box &box = boxes[box_idx];
    Weapon &weapon = weapons[weapon_idx];

    double box_hardness = double(box.current_hardness);
    double weapon_power = double(weapon.powers[box_idx]);

    return box_hardness / (weapon_power * weapon_power * weapon_power);
}

pair<int, int> Environment::best_weapon_box_pair() {
    double best_score = -1;
    int best_weapon_idx = -1;
    int best_box_idx = -1;

    // 1. Evaluate best weapon attack
    for (int weapon_idx : available_weapons) {
        Weapon &weapon = weapons[weapon_idx];

        for (int box_idx : remaining_boxes) {
            Box &box = boxes[box_idx];

            double score = weapon.powers[box_idx];
            if (score > best_score) {
                best_score = score;
                best_weapon_idx = weapon_idx;
                best_box_idx = box_idx;
            }
        }
    }

    // 2. Evaluate best bare-hand attack
    double best_bh_score = -1;
    int best_bh_target = -1;
    for (int box_idx : remaining_boxes) {
        Box &box = boxes[box_idx];
        double score = weapons[box_idx].value() / box.current_hardness;
        if (score > best_bh_score) {
            best_bh_score = score;
            best_bh_target = box_idx;
        }
    }

    // 3. Decide: Use weapon or bare hands?
    if (best_score < 4 && best_bh_target != -1) {
        best_weapon_idx = -1;
        best_box_idx = best_bh_target;
    }

    return {best_weapon_idx, best_box_idx};
}