import math
import matplotlib.pyplot as plt

# global variables
speed = t_realize = t_empty = 0.0


class Bastion:
    def __init__(self, id, x, y, value):
        self.id = id
        self.x = x
        self.y = y
        self.value = value

    def distance(self, other):
        return math.hypot(self.x - other.x, self.y - other.y)

    def actual_value(self, time):
        if time < t_realize:
            return self.value
        elif time >= t_empty:
            return 0.0
        else:
            fraction = (time - t_realize) / (t_empty - t_realize)
            return self.value * (1.0 - fraction)

    def __repr__(self):
        return f"Bastion({self.id}, {self.x}, {self.y}, {self.value})"


input_file = "application/contest/input.txt"
with open(input_file, "r") as f:
    lines = f.readlines()

n = int(lines[0].strip())
bastions = []
for i in range(n):
    x, y, v = map(int, lines[i + 1].strip().split())
    bastions.append(Bastion(i, x, y, v))
speed = float(lines[n + 1].strip())
t_realize, t_empty = map(float, lines[n + 2].strip().split())

output_file = "application/contest/output.txt"
with open(output_file, "r") as f:
    lines = f.readlines()
tour_size = int(lines[0].strip())
tour = [bastions[int(i)] for i in lines[1].strip().split()]

plt.figure(figsize=(10, 8))
ax = plt.gca()
ax.set_facecolor(("#2d0a0a", 0.6))

# Plot bastions with gold/yellow color
xs = [b.x for b in bastions]
ys = [b.y for b in bastions]
sc = plt.scatter(
    xs,
    ys,
    c=[0.001 * b.value * b.value for b in bastions],
    cmap="YlOrBr",
    label="Bastions",
    s=[0.0001 * b.value * b.value for b in bastions],
    edgecolors="black",
    alpha=0.85,
    zorder=2,
)
plt.colorbar(sc, label="Bastion Value")

plt.scatter(
    [0],
    [0],
    c="#a020f0",
    label="Nether Portal",
    s=300,
    marker="P",
    edgecolors="black",
    zorder=3,
)

# Plot tour path
tx = [0] + [b.x for b in tour]
ty = [0] + [b.y for b in tour]
plt.plot(tx, ty, color="#0003a8", linewidth=2.5, label="Tour", zorder=1)

plt.legend()
plt.grid(True, which="both", linestyle="--", linewidth=0.5, color="black")
plt.tight_layout()
plt.savefig("application/contest/bastion.png", dpi=300)
plt.close("all")
