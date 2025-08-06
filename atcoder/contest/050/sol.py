# import sys
# input_file = "atcoder/contest/050/input.txt"
# sys.stdin = open(input_file, "r")


DIRS = ((0, 1), (1, 0), (0, -1), (-1, 0))


class Environment:
    def __init__(self, grid):
        self.grid = [row[:] for row in grid]
        self.n = len(grid)
        self.m = sum(row.count("#") for row in grid)

        self.prob = [[0.0] * self.n for _ in range(self.n)]
        self.empty_cells = self.n * self.n - self.m
        probability_per_empty_cell = (
            1.0 / self.empty_cells if self.empty_cells > 0 else 0.0
        )
        for i in range(self.n):
            for j in range(self.n):
                if self.grid[i][j] == ".":
                    self.prob[i][j] = probability_per_empty_cell

        self.life = 1.0
        self.total_score = 0.0
        self.lives = [1.0]

    def is_rock(self, r, c):
        return self.grid[r][c] == "#"

    def is_empty(self, r, c):
        return self.grid[r][c] == "."

    def is_valid(self, r, c):
        return 0 <= r < self.n and 0 <= c < self.n

    def update_probabilities(self):
        next_prob = [[0.0] * self.n for _ in range(self.n)]

        for i in range(self.n):
            for j in range(self.n):
                if self.prob[i][j] <= 1e-5:
                    continue

                new_prob = self.prob[i][j] / 4.0

                for di, dj in DIRS:
                    ni, nj = i, j
                    while self.is_valid(ni + di, nj + dj) and self.is_empty(
                        ni + di, nj + dj
                    ):
                        ni += di
                        nj += dj

                    next_prob[ni][nj] += new_prob

        self.prob = next_prob

    def place_rock(self, r, c):
        self.update_probabilities()

        self.life -= self.prob[r][c]
        self.prob[r][c] = 0.0
        self.grid[r][c] = "#"

        self.total_score += self.life
        self.lives.append(self.life)

        return self.life

    @property
    def current_score(self):
        ub = self.n * self.n - self.m - 1
        return int(self.total_score / ub * 1e6)


def greedy(env):
    sequence = []
    while True:
        min_prob = float("inf")
        min_cell = None
        for i in range(env.n):
            for j in range(env.n):
                if env.is_empty(i, j) and env.prob[i][j] < min_prob:
                    min_prob = env.prob[i][j]
                    min_cell = (i, j)
        if min_cell is None:
            break

        r, c = min_cell
        _ = env.place_rock(r, c)
        sequence.append((r, c))
    return sequence


if __name__ == "__main__":
    n, m = map(int, input().split())
    grid = [list(input().strip()) for _ in range(n)]

    env = Environment(grid)
    solver = greedy

    sequence = solver(env)

    for r, c in sequence:
        print(r, c)
