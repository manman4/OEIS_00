# Using graphillion
from graphillion import GraphSet
import graphillion.tutorial as tl

def A064298(n, k):
    if n == 1 or k == 1: return 1
    universe = tl.grid(n - 1, k - 1)
    GraphSet.set_universe(universe)
    start, goal = 1, k * n
    paths = GraphSet.paths(start, goal)
    return paths.len()

print([A064298(j + 1, i - j + 1) for i in range(7) for j in range(i + 1)])
