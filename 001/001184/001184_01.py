# Using graphillion
from graphillion import GraphSet
import graphillion.tutorial as tl

def A(n):
    if n == 1: return 1
    universe = tl.grid(n - 1, n - 1)
    GraphSet.set_universe(universe)
    start, goal = 1, n * n
    paths = GraphSet.paths(start, goal, is_hamilton=True)
    return paths.len()


for i in range(1, 10, 2):
    print(A(i))
