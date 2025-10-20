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
    vector<vector<int>> powers;

    int unlocked_weapons = 0;
    set<int> available_weapons;
    set<int> remaining_boxes;

    Environment(int N, vector<int> hardnesses, vector<int> durabilities,
                vector<vector<int>> powers) {
        this->N = N;
        // this->powers = powers;

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
        weapons[-1].unlocked = true; // Bare Hands

        unlocked_weapons = 0;

        available_weapons.clear();
        available_weapons.insert(-1);

        remaining_boxes.clear();
        for (int i = 0; i < N; i++) {
            remaining_boxes.insert(i);
        }
    }
};

AttackSequence selection_to_attack_sequence(SelectionSequence &selection_sequence,
                                            Environment &env) {
    AttackSequence attack_sequence;

    for (auto &[weapon_idx, box_idx] : selection_sequence) {
        env.deplete(weapon_idx, box_idx, attack_sequence);
    }

    return attack_sequence;
}

double ratio_score(int box_hardness, int weapon_power) {
    double hardness = double(box_hardness);
    double power = double(weapon_power);

    return (hardness) / (power * power * power);
}

SelectionSequence greedy(Environment &env, int first_box = -1) {
    SelectionSequence selection_sequence;
    AttackSequence attack_sequence;

    while (env.unlocked_weapons < env.N) {
        double best_ratio = numeric_limits<double>::max();
        int best_weapon_idx = -1;

        int best_box_idx;
        if (first_box != -1) {
            best_box_idx = first_box;
            first_box = -1; // Only use first_box for the first iteration
        } else {
            best_box_idx = -1;
            for (int box_idx : env.remaining_boxes) {
                Box &box = env.boxes[box_idx];

                for (int weapon_idx : env.available_weapons) {
                    Weapon &weapon = env.weapons[weapon_idx];

                    if (!weapon.can_use()) {
                        continue;
                    }

                    double ratio =
                        ratio_score(box.current_hardness, weapon.powers[box_idx]);
                    if (ratio <= best_ratio) {
                        best_ratio = ratio;
                        best_weapon_idx = weapon_idx;
                        best_box_idx = box_idx;
                    }
                }
            }
        }

        selection_sequence.push_back({best_weapon_idx, best_box_idx});
        env.deplete(best_weapon_idx, best_box_idx, attack_sequence);
    }

    env.reset();
    return selection_sequence;
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

    vector<pair<int, int>> box_indices;
    for (int box_idx = 0; box_idx < N; box_idx++) {
        double highest_damage_outgoing = 0;
        for (int other_box_idx = 0; other_box_idx < N; other_box_idx++) {
            highest_damage_outgoing =
                max(highest_damage_outgoing, double(powers[box_idx][other_box_idx]));
        }

        double normalized_hardness = (double(hardnesses[box_idx]) - 100.0) * 2.5;
        double normalized_inverse_damage =
            ((500.0 / double(highest_damage_outgoing)) - 1.0) * 1000.0 / 499.0;

        double ratio = normalized_hardness * normalized_inverse_damage;
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
        SelectionSequence selection_sequence = greedy(env, first_box);
        AttackSequence attack_sequence =
            selection_to_attack_sequence(selection_sequence, env);

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
        }

        env.reset();
        idx++;
    }

    SelectionSequence selection_sequence = greedy(env, best_first_box);
    env.reset();

    AttackSequence attack_sequence =
        selection_to_attack_sequence(selection_sequence, env);
    for (auto &[weapon_idx, box_idx] : attack_sequence) {
        cout << weapon_idx << " " << box_idx << "\n";
    }

    int score = 0;
    for (int hardness : hardnesses) {
        score += hardness;
    }
    score -= attack_sequence.size();
    score += 1;

    cerr << "Score: " << score << endl;

    return 0;
}
