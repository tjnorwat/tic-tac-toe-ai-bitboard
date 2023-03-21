# Tic Tac Toe AI with Bitboard Implementation

In this project, I implemented an efficient version of the classic Tic Tac Toe game using bitboards. Bitboards are a data structure that represents the game board with binary digits, enabling the use of bitwise operations for quick and efficient move execution. This approach substantially decreases the memory needed to store the game state, leading to faster processing times. My implementation employs the Minimax algorithm with Alpha-Beta pruning to make the best move while also considering the depth at which it will win/lose.

## Performance

I adapted George Seif's implementation found [here](https://github.com/GeorgeSeif/Tic-Tac-Toe-AI#algorithm-details) and saved it under the filename `other.cpp`. To measure the performance difference between the two implementations, I modified Seif's implementation to allow the AI to make the first move.

On my computer, Seif's implementation took about `0.5` seconds without compiler optimization and approximately `0.122` seconds with the `-Ofast` flag.

My implementation took about `0.0077` seconds without compiler optimization and roughly `0.0027` seconds with the `-Ofast` flag.

This results in a performance difference of about 45 times!

To further improve performance, I implemented the Negamax algorithm with transposition tables. These tables store different game positions along with their heuristic values, eliminating the need to recalculate each search call. This enhancement speeds up my search by about `3` times, taking just `.001` seconds on the first state.

Switching to Linux provided an additional performance boost. I observed a search time of about `.0003` seconds, which is almost `10` times faster than without using transposition tables! Compared to Seif's implementation, my approach is about `407` times faster!

By replacing arbitrary low/high values for Alpha/Beta with `INT_MIN` and `INT_MAX`, my performance improved by almost 3 times, clocking in at an impressive `.00013` seconds.

Finally, I eliminated the `possible_moves()` function and inlined the algorithm, resulting in another 3 times performance increase. This optimization removed the need for vectors and dynamic element addition. After inlining the function in the `find_best_move()` and `negamax()` functions, my implementation achieved a search time of about `.000042` seconds.

Comparing Seif's `.5` seconds and my best time so far `.000042`, it is about `11,905` times faster. Comparing his `.122` time with the fast flag, we get about `2,905` times faster which is probably a more accurate comparison, because both programs are compiled with the fast flag. 