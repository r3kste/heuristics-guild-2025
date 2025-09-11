#include <bits/stdc++.h>

using namespace std;

typedef array<int, 2> Point;
typedef vector<Point> Path;
typedef vector<vector<int>> Matrix2D;
typedef pair<int, int> Step;

const array<Step, 4> DIRS = {{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};
const map<char, Step> DIR_MAP = {
    {'U', {-1, 0}}, {'D', {1, 0}}, {'L', {0, -1}}, {'R', {0, 1}}, {'S', {0, 0}}};
const map<Step, char> RID_MAP = {
    {{-1, 0}, 'U'}, {{1, 0}, 'D'}, {{0, -1}, 'L'}, {{0, 1}, 'R'}};

const map<char, int> CMD_MAP = {{'U', 0}, {'D', 1}, {'L', 2}, {'R', 3}, {'S', 4}};
const map<int, char> DMC_MAP = {{0, 'U'}, {1, 'D'}, {2, 'L'}, {3, 'R'}, {4, 'S'}};

class Environment {
  public:
    int n, no_robots, no_buttons;
    vector<Point> pos;
    Matrix2D vertical_walls;
    Matrix2D horizontal_walls;

    int no_waxed;
    int no_steps_taken;
    vector<vector<Step>> steps;
    vector<vector<bool>> waxed;

    Environment(int n, int no_robots, int no_buttons, vector<Point> pos,
                Matrix2D vertical_walls, Matrix2D horizontal_walls)
        : n(n), no_robots(no_robots), no_buttons(no_buttons), pos(pos),
          vertical_walls(vertical_walls), horizontal_walls(horizontal_walls) {
        no_waxed = 0;
        no_steps_taken = 0;
        steps.assign(no_buttons, vector<Step>(no_robots, {0, 0}));
        waxed.assign(n, vector<bool>(n, false));

        for (const Point &p : pos) {
            waxed[p[0]][p[1]] = true;
            no_waxed++;
        }
    }

    bool can_move(Point from_pos, Point to_pos) {
        if (to_pos[0] < 0 || to_pos[0] >= n || to_pos[1] < 0 || to_pos[1] >= n) {
            return false;
        }

        if (from_pos[0] == to_pos[0]) {
            int row = from_pos[0];
            int left_col = min(from_pos[1], to_pos[1]);
            if (vertical_walls[row][left_col] == 1) {
                return false;
            }
        } else if (from_pos[1] == to_pos[1]) {
            int col = from_pos[1];
            int top_row = min(from_pos[0], to_pos[0]);
            if (horizontal_walls[top_row][col] == 1) {
                return false;
            }
        }

        return true;
    }

    void step(int button_idx) {
        no_steps_taken++;
        for (int robot_idx = 0; robot_idx < no_robots; ++robot_idx) {
            Step move = steps[button_idx][robot_idx];
            if (move.first == 0 && move.second == 0) {
                continue;
            }

            Point new_pos = {pos[robot_idx][0] + move.first,
                             pos[robot_idx][1] + move.second};
            if (can_move(pos[robot_idx], new_pos)) {
                pos[robot_idx] = new_pos;
                if (!waxed[new_pos[0]][new_pos[1]]) {
                    waxed[new_pos[0]][new_pos[1]] = true;
                    no_waxed++;
                }
            }
        }
    }

    int score() {
        if (no_waxed == n * n) {
            return 3 * n * n - no_steps_taken;
        } else {
            return no_waxed;
        }
    }
};

Path dfs(Environment &env, int robot_idx, array<Step, 4> &directions) {
    set<Point> visited;
    Path path;

    function<void(Point)> _dfs = [&](Point pos) {
        visited.insert(pos);
        path.push_back(pos);

        for (auto [di, dj] : directions) {
            int ni = pos[0] + di, nj = pos[1] + dj;
            Point neighbor = {ni, nj};

            if (ni >= 0 && ni < env.n && nj >= 0 && nj < env.n &&
                visited.find(neighbor) == visited.end() &&
                env.can_move(pos, neighbor)) {
                _dfs(neighbor);
            }
        }
    };

    Point start = env.pos[robot_idx];
    _dfs(start);
    return path;
}

Path rectify_path(Environment &env, Path path) {
    /* Rectify the path to ensure all consecutive points are adjacent
     */

    Path rectified_path = {path[0]};

    for (size_t idx = 1; idx < path.size(); ++idx) {
        Point prev = path[idx - 1];
        Point curr = path[idx];

        // The points are adjacent
        if (abs(prev[0] - curr[0]) + abs(prev[1] - curr[1]) == 1) {
            rectified_path.push_back(curr);
            continue;
        }

        // Use BFS to find the shortest path from prev to curr
        queue<Point> queue;
        map<Point, Point> came_from;
        queue.push(prev);
        came_from[prev] = {-1, -1};

        while (!queue.empty()) {
            Point node = queue.front();
            queue.pop();

            if (node == curr) {
                break;
            }

            for (auto [di, dj] : DIRS) {
                int ni = node[0] + di, nj = node[1] + dj;
                Point neighbor = {ni, nj};
                if (ni >= 0 && ni < env.n && nj >= 0 && nj < env.n &&
                    came_from.find(neighbor) == came_from.end() &&
                    env.can_move(node, neighbor)) {
                    came_from[neighbor] = node;
                    queue.push(neighbor);
                }
            }
        }

        Path sub_path;
        Point step = curr;
        while (step != Point{-1, -1}) {
            sub_path.push_back(step);
            step = came_from[step];
        }
        reverse(sub_path.begin(), sub_path.end());

        rectified_path.insert(rectified_path.end(), sub_path.begin() + 1,
                              sub_path.end());
    }

    return rectified_path;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, no_robots, no_buttons;
    cin >> n >> no_robots >> no_buttons;

    vector<Point> pos(no_robots);
    for (int i = 0; i < no_robots; ++i) {
        cin >> pos[i][0] >> pos[i][1];
    }

    Matrix2D vertical_walls(n, vector<int>(n - 1));
    for (int i = 0; i < n; ++i) {
        string tmp;
        cin >> tmp;

        for (int j = 0; j < n - 1; ++j) {
            vertical_walls[i][j] = (tmp[j] == '1' ? 1 : 0);
        }
    }

    Matrix2D horizontal_walls(n - 1, vector<int>(n));
    for (int i = 0; i < n - 1; ++i) {
        string tmp;
        cin >> tmp;

        for (int j = 0; j < n; ++j) {
            horizontal_walls[i][j] = (tmp[j] == '1' ? 1 : 0);
        }
    }

    vector<string> commands(no_buttons, string(no_robots, 'S'));
    for (int robot_idx = 0; robot_idx < no_robots; ++robot_idx) {
        for (int cmd_idx = 0; cmd_idx < 4; ++cmd_idx) {
            commands[cmd_idx][robot_idx] = DMC_MAP.at(cmd_idx);
        }
    }

    for (int button_idx = 0; button_idx < no_buttons; ++button_idx) {
        for (int robot_idx = 0; robot_idx < no_robots; ++robot_idx) {
            cout << commands[button_idx][robot_idx] << " ";
        }
        cout << "\n";
    }

    int best_robot_idx = 0;
    vector<int> best_operations;
    int best_score = -1;

    // try all permutations of directions
    array<Step, 4> directions = DIRS;

    do {
        for (int primary_robot_idx = 0; primary_robot_idx < no_robots;
             ++primary_robot_idx) {
            Environment env(n, no_robots, no_buttons, pos, vertical_walls,
                            horizontal_walls);

            Path path = dfs(env, primary_robot_idx, directions);
            path = rectify_path(env, path);

            for (int button_idx = 0; button_idx < no_buttons; ++button_idx) {
                for (int robot_idx = 0; robot_idx < no_robots; ++robot_idx) {
                    env.steps[button_idx][robot_idx] =
                        DIR_MAP.at(commands[button_idx][robot_idx]);
                }
            }

            vector<int> operations;
            Point curr_pos = env.pos[primary_robot_idx];
            for (size_t idx = 1; idx < path.size(); ++idx) {
                Point next_pos = path[idx];
                char move_char =
                    RID_MAP.at({next_pos[0] - curr_pos[0], next_pos[1] - curr_pos[1]});

                operations.push_back(CMD_MAP.at(move_char));
                curr_pos = next_pos;
            }

            int no_of_ops = 0;
            for (int op : operations) {
                env.step(op);
                no_of_ops++;

                if (env.no_waxed == n * n) {
                    break;
                }
            }

            int score = env.score();
            if (score > best_score) {
                best_score = score;
                best_robot_idx = primary_robot_idx;

                best_operations = operations;
                best_operations.resize(no_of_ops);
            }
        }
    } while (next_permutation(directions.begin(), directions.end()));

    for (int op : best_operations) {
        cout << op << "\n";
    }

    cerr << "Best robot: " << best_robot_idx << ", best score: " << best_score << "\n";
    return 0;
}
