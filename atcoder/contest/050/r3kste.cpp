#include <bits/stdc++.h>

using namespace std;

using Sequence = vector<pair<int, int>>;
using Grid = vector<vector<char>>;
const vector<pair<int, int>> dirs = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};

class Environment {
  private:
    Grid grid;
    vector<vector<double>> prob;
    double life;
    double total_score;

    void update_probabilities() {
        vector<vector<double>> next_prob(n, vector<double>(n, 0.0));

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (prob[i][j] <= 1e-5 || is_rock(i, j)) {
                    continue;
                }

                double new_prob = prob[i][j] / 4.0;

                for (auto [di, dj] : dirs) {
                    int ni = i, nj = j;
                    while (is_valid(ni + di, nj + dj) && is_empty(ni + di, nj + dj)) {
                        ni += di;
                        nj += dj;
                    }
                    next_prob[ni][nj] += new_prob;
                }
            }
        }

        prob = next_prob;
    }

  public:
    int n;
    int m;
    int initial_empty_cells;

    Environment(const Grid &input_grid) {
        grid = input_grid;
        n = grid.size();
        m = 0;

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (grid[i][j] == '#') {
                    m++;
                }
            }
        }

        prob.assign(n, vector<double>(n, 0.0));
        initial_empty_cells = n * n - m;
        double probability_per_empty_cell =
            (initial_empty_cells > 0) ? 1.0 / initial_empty_cells : 0.0;

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (grid[i][j] == '.') {
                    prob[i][j] = probability_per_empty_cell;
                }
            }
        }

        life = 1.0;
        total_score = 0.0;
    }

    bool is_rock(int r, int c) { return grid[r][c] == '#'; }
    bool is_empty(int r, int c) { return grid[r][c] == '.'; }
    bool is_valid(int r, int c) { return r >= 0 && r < n && c >= 0 && c < n; }

    double place_rock(int r, int c) {
        update_probabilities();
        life -= prob[r][c];
        prob[r][c] = 0.0;
        grid[r][c] = '#';
        total_score += life;
        return life;
    }

    int current_score() {
        int ub = n * n - m - 1;
        return round(total_score / ub * 1e6);
    }

    double get_prob(int i, int j) const { return prob[i][j]; }
    vector<vector<double>> &get_prob_grid() { return prob; }
    Grid &get_grid() { return grid; }
};

double calculate_expected_value(Environment &env, int max_steps);

Sequence greedy_lookahead(Environment &env, int max_lookahead_steps = 8,
                          int max_candidates = 15, double min_prob_threshold = 5e-4) {
    Sequence sequence;
    int n = env.n;

    auto &grid = env.get_grid();
    auto &prob = env.get_prob_grid();

    while (true) {
        vector<pair<double, pair<int, int>>> candidates;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (env.is_empty(i, j)) {
                    candidates.push_back({prob[i][j], {i, j}});
                }
            }
        }

        if (candidates.empty()) {
            break;
        }

        sort(candidates.begin(), candidates.end());

        // If we have only one candidate or if the first candidate is significantly
        // better than the second one, we place it immediately
        if (candidates.size() == 1 ||
            candidates[1].first - candidates[0].first > min_prob_threshold) {
            int r = candidates[0].second.first;
            int c = candidates[0].second.second;

            env.place_rock(r, c);
            sequence.push_back({r, c});
            continue;
        }

        double best_expected_value = -1e9;
        pair<int, int> best_move = candidates[0].second;

        int candidates_to_check = min(max_candidates, (int)candidates.size());

        for (int idx = 0; idx < candidates_to_check; idx++) {
            auto [prob_loss, pos] = candidates[idx];
            int r = pos.first, c = pos.second;

            if (idx > 0 && prob_loss > candidates[0].first + min_prob_threshold) {
                break;
            }

            Environment new_env = env;
            new_env.place_rock(r, c);

            double expected_value =
                calculate_expected_value(new_env, max_lookahead_steps);

            if (expected_value > best_expected_value) {
                best_expected_value = expected_value;
                best_move = {r, c};
            }
        }

        env.place_rock(best_move.first, best_move.second);
        sequence.push_back(best_move);
    }

    return sequence;
}

double calculate_expected_value(Environment &env, int max_steps) {
    int n = env.n;
    auto &prob = env.get_prob_grid();
    auto &grid = env.get_grid();

    // Calculate total probability mass
    double total_prob = 0.0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (env.is_empty(i, j)) {
                total_prob += prob[i][j];
            }
        }
    }

    if (total_prob < 1e-9) {
        return 0.0;
    }

    vector<vector<int>> distance(n, vector<int>(n, max_steps + 1));
    vector<vector<bool>> visited(n, vector<bool>(n, false));

    queue<pair<int, int>> q;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (env.is_rock(i, j) || i == 0 || i == n - 1 || j == 0 || j == n - 1) {
                distance[i][j] = 0;
                q.push({i, j});
                visited[i][j] = true;
            }
        }
    }

    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();

        if (distance[x][y] >= max_steps)
            continue;

        for (auto [dx, dy] : dirs) {
            int nx = x + dx, ny = y + dy;
            if (env.is_valid(nx, ny) && env.is_empty(nx, ny) && !visited[nx][ny]) {
                distance[nx][ny] = distance[x][y] + 1;
                visited[nx][ny] = true;
                q.push({nx, ny});
            }
        }
    }

    double expected_value = 0.0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (env.is_empty(i, j)) {
                double distance_factor = distance[i][j] / (double)max_steps;
                expected_value += prob[i][j] * distance_factor;
            }
        }
    }

    return expected_value;
}

Sequence greedy(Environment &env) {
    Sequence sequence;

    while (true) {
        double min_prob = numeric_limits<double>::infinity();
        pair<int, int> min_cell = {-1, -1};

        for (int i = 0; i < env.n; i++) {
            for (int j = 0; j < env.n; j++) {
                if (env.is_empty(i, j) && env.get_prob(i, j) < min_prob) {
                    min_prob = env.get_prob(i, j);
                    min_cell = {i, j};
                }
            }
        }

        if (min_cell.first == -1) {
            break;
        }

        int r = min_cell.first;
        int c = min_cell.second;
        env.place_rock(r, c);
        sequence.push_back({r, c});
    }

    return sequence;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    Grid grid(n, vector<char>(n));
    for (int i = 0; i < n; i++) {
        string line;
        cin >> line;
        for (int j = 0; j < n; j++) {
            grid[i][j] = line[j];
        }
    }

    Environment env(grid);
    Sequence sequence = greedy_lookahead(env);
    // Sequence sequence = greedy(env);

    for (auto [r, c] : sequence) {
        cout << r << " " << c << "\n";
    }

    cerr << "Score: " << env.current_score() << "\n";
    return 0;
}
