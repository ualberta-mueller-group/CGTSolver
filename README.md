# CGTSolver for Linear NoGo

CGTSolver is a game-solving program for the game of NoGo played on $1\times n$ boards. It is a minimax solver with enhancements inspired by combinatorial game theory (CGT).

This repo contains the source code of our paper: Haoyu Du and Martin Müller. Solving Linear NoGo with Combinatorial Game Theory. In *Computers and Games*, 2024.

CGTSolver features:
* A pre-computed database of all reduced game positions with 1~15 empty points.
* Complete simplest equal games are found up to 8 empty points in the database.
* Restricted simplest equal games are found up to 13 empty points in the database.
* Simplification & split.
* Static evaluation.
* Simplest equal game replacement.
* Play-in-the-middle heuristic.
* Zobrist Hashing for sum games.
