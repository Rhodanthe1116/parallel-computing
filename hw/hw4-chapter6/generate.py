import random
# Python code to implement Conway's Game Of Life
import argparse
import numpy as np

ON = 1
OFF = 0
vals = [ON, OFF]


def randomGrid(N):
    """returns a grid of NxN random values"""
    return np.random.choice(vals, N*N, p=[0.5, 0.5]).reshape(N, N)


def is_safe(i,  j,  n,  m):
    if (0 <= i and i < n and 0 <= j and j < m):
        return True

    return False


def cal_neighber(i,  j,  n,  m, arr):

    total = 0
    dirs = [
        {'x': -1, 'y': -1},
        {'x': -1, 'y': 0},
        {'x': -1, 'y': 1},
        {'x': 0,  'y': -1},
        {'x': 0,  'y': 1},
        {'x': 1,  'y': -1},
        {'x': 1,  'y': 0},
        {'x': 1,  'y': 1}
    ]
    for d in dirs:
        x = i + d['x']
        y = j + d['y']
        if (is_safe(x, y, n, m)):
            total += arr[x][y]

    return total


def update(grid, N):

    # copy grid since we require 8 neighbors
    # for calculation and we go line by line
    newGrid = grid.copy()
    for i in range(N):
        for j in range(N):

            # compute 8-neghbor sum
            # using toroidal boundary conditions - x and y wrap around
            # so that the simulaton takes place on a toroidal surface.
            total = cal_neighber(i, j, N, N, grid)

            if (grid[i][j] == 1 and (total == 2 or total == 3)):
                newGrid[i][j] = 1
            elif (grid[i][j] == 1 and total > 3):
                newGrid[i][j] = 0
            elif (grid[i][j] == 1 and total < 1):
                newGrid[i][j] = 0
            elif (grid[i][j] == 0 and total == 3):
                newGrid[i][j] = 1
            else:
                newGrid[i][j] = 0

    # update data
    grid[:] = newGrid[:]
    return grid

# main() function


def main():

    # set grid size
    N = 500
    t = 3

    # declare grid
    grid = np.array([])

    grid = randomGrid(N)

    print(f'{N} {N}', file=open("life_input.txt", "w"))
    for r in grid:
        print(*r, file=open("life_input.txt", "a"))

    # for _ in range(t):
    #     grid = update(grid, N)
    #     for r in grid:
    #         pass
    #         # print(*r)
    #     # print()

    # print(f'{N} {N}', file=open("life_answer.txt", "w"))
    # ans = grid
    # for r in ans:
    #     print(*r, file=open("life_answer.txt", "a"))


# call main
if __name__ == '__main__':
    main()
