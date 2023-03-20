# Tic Tac Toe AI with bitboard implementation 
This is a little program to play the classic game tic tac toe but with efficiency in mind. Bitboards are a data structure that represent the game board using binary digits, which allows us to perform bitwise operations in order to quickly and efficiently make moves. This approach significantly reduces the amount of memory required to store the game state, which ultimately leads to faster processing times. This implementation uses minimax with alpha beta in order to make the best move. It also takes into account the depth at which it will win/lose. 

# Performance 
I borrowed George Seif's implementation from [here](https://github.com/GeorgeSeif/Tic-Tac-Toe-AI#algorithm-details) and is under the file name `other.cpp`. I made a few modifications so that the AI could go first so that we could see the performance difference between the 2 implementations.  

Seif's performance on my computer takes about `0.5` seconds with no compiler optimization and about `0.122` seconds with the `-Ofast` flag. 

My program on my computer takes about `0.0077` seconds with no compiler optimization and about `0.0027` seconds with the `-Ofast` flag.

The performance difference is about 45 times greater!

Diving a little deeper, we can implement negamax with transpositions tables. These tables allow us to store different game positions with their heuristic value so we don't have to recalculate every search call. This speeds up our our searching about 3 times, taking just `.001` seconds on the first state.

Switching over to linux gave another performance boost. I have a newer g++ version on windows than on linux, so I am not sure if that contributes in any way, but we can search in about `.0003` seconds. This is almost 10 times faster without using the transposition tables! Comparing against Seif, it is about 407 times faster as well!

Another improvement we can make is using int_min and int_max instead of arbitrary low/high values for alpha/beta. After swapping to use the min and max of integer, our performance is nearly 3x, coming in at a blazing `.00013` seconds. 