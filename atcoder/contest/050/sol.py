# import sys
# input_file = "atcoder/contest/050/input.txt"
# sys.stdin = open(input_file, "r")

n, m = map(int, input().split())
grid = [list(input().strip()) for _ in range(n)]

dirs = ((0, 1), (1, 0), (0, -1), (-1, 0))


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
                if self.prob[i][j] == 0.0:
                    continue

                for di, dj in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
                    i2, j2 = i, j
                    while self.is_valid(i2 + di, j2 + dj) and self.is_empty(
                        i2 + di, j2 + dj
                    ):
                        i2 += di
                        j2 += dj

                    next_prob[i2][j2] += self.prob[i][j] * 0.25

        self.prob = next_prob

    def place_rock(self, r, c):
        if self.is_rock(r, c):
            raise ValueError("Rock already exists at this position.")

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


if __name__ == "__main__":
    env = Environment(grid)

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
        score = env.place_rock(r, c)
        sequence.append((r, c))
        print(r, c)
