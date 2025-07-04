# This file is used to generate test cases
import random

with open("application/contest/input.txt", "w") as f:
    n = 500
    f.write(f"{n}\n")
    for i in range(n):
        x = random.randint(-1000, 1000)
        y = random.randint(-1000, 1000)
        v = random.randint(1, 1000)
        f.write(f"{x} {y} {v}\n")

    speed = random.randint(40, 100)
    T_realize = random.randint(1, 100)
    T_empty = T_realize + random.randint(1, 100)
    f.write(f"{speed}\n{T_realize} {T_empty}\n")
