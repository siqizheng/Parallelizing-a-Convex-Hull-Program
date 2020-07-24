# Parallelizing-a-Convex-Hull-Program

In this programming assignment, I will parallelize a convex hull program with multiple processes. The algorithm that I'll use is a combination of divide-and-conquer and Graham’s scan. Divide-and-conquer divides an entire problem into a tree of sub-spaces, each small enough to be solved with an independent process. I'll fork child processes in a tree as partitioning a problem with divide-and-conquer and let these processes work on their own sub-space in parallel.
