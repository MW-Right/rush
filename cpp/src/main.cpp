#include <cstdlib>
#include <iostream>
#include <unordered_set>

#include "board.h"
#include "config.h"
#include "enumerator.h"
#include "search.h"

using namespace std;

void print(const Board &board) {
    cout << board << endl;
    // cout << BitboardString(board.Mask()) << endl;
    // cout << BitboardString(board.HorzMask()) << endl;
    // cout << BitboardString(board.VertMask()) << endl;
    // cout << endl;
}

int main() {
    // Enumerator enumerator;
    // enumerator.Enumerate([&](uint64_t counter, int group, const Board &board) {
    //     ReachableStates(board);
    // });
    // return 0;

    // Board board("BCDDE.BCF.EGB.FAAGHHHI.G..JIKKLLJMM."); // 51 moves
    // Board board("BB.C...D.CEE.DAAFGH.IIFGH.JKK.LLJ..."); // 541934 states

    // 24 43 13 B..CDDBEEC.F.G.AAF.GHHIJKKL.IJ..L.MM 278666
    Board board("B..CDDBEEC.F.G.AAF.GHHIJKKL.IJ..L.MM");

    // while (1) {
        const int count = ReachableStates(board);
        cout << count << endl;
    // }

    // vector<Move> moves;
    // unordered_set<BoardKey> seen;

    // for (int i = 0; i < 5000000; i++) {
    //     // if (board.Solved()) {
    //     //     cout << i << endl;
    //     //     break;
    //     // }
    //     seen.emplace(board.Key());

    //     board.Moves(moves);
    //     const int index = rand() % moves.size();
    //     board.DoMove(moves[index]);
    // }

    // print(board);
    // cout << seen.size() << endl;

    return 0;
}
