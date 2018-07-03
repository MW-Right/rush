#include "enumerator.h"

#include <cmath>

#include "config.h"

PositionEntry::PositionEntry(const int group, const std::vector<Piece> &pieces) :
    m_Group(group),
    m_Pieces(pieces),
    m_Mask(0),
    m_Require(0)
{
    for (const auto &piece : pieces) {
        m_Mask |= piece.Mask();
    }
    if (!pieces.empty()) {
        const int stride = pieces[0].Stride();
        if (stride == H) {
            m_Require = (m_Mask >> stride) & ~m_Mask & ~RightColumn;
        } else {
            m_Require = (m_Mask >> stride) & ~m_Mask;
        }
    }
}

Enumerator::Enumerator() {
    std::vector<int> sizes;
    ComputeGroups(sizes, 0);
    ComputePositionEntries();
}

void Enumerator::Enumerate(EnumeratorFunc func) {
    Board board;
    uint64_t counter = 0;
    PopulatePrimaryRow(func, board, counter);
}

void Enumerator::EnumerateGroup(const int group, EnumeratorFunc func) {
    Board board;
    uint64_t counter = 0;
    PopulateGroupPrimaryRow(group, func, board, counter);
}

int Enumerator::NumGroups() const {
    const int n = BoardSize * 2 - 1;
    return std::pow(m_Groups.size(), n);
}

void Enumerator::PopulatePrimaryRow(
    EnumeratorFunc func, Board &board, uint64_t &counter) const
{
    for (const auto &pe : m_RowEntries[PrimaryRow]) {
        for (const auto &piece : pe.Pieces()) {
            board.AddPiece(piece);
        }
        PopulateRow(func, board, counter, 0, pe.Mask(), pe.Require(), 0);
        for (int i = 0; i < pe.Pieces().size(); i++) {
            board.PopPiece();
        }
    }
}

void Enumerator::PopulateRow(
    EnumeratorFunc func, Board &board, uint64_t &counter, int y,
    bb mask, bb require, int group) const
{
    if (y >= BoardSize) {
        PopulateCol(func, board, counter, 0, mask, require, group);
        return;
    }
    if (y == PrimaryRow) {
        PopulateRow(func, board, counter, y + 1, mask, require, group);
        return;
    }
    group *= m_Groups.size();
    for (const auto &pe : m_RowEntries[y]) {
        if ((mask & pe.Mask()) != 0) {
            continue;
        }
        for (const auto &piece : pe.Pieces()) {
            board.AddPiece(piece);
        }
        PopulateRow(
            func, board, counter, y + 1,
            mask | pe.Mask(), require | pe.Require(), group + pe.Group());
        for (int i = 0; i < pe.Pieces().size(); i++) {
            board.PopPiece();
        }
    }
}

void Enumerator::PopulateCol(
    EnumeratorFunc func, Board &board, uint64_t &counter, int x,
    bb mask, bb require, int group) const
{
    if (x >= BoardSize) {
        if ((mask & require) != require) {
            return;
        }
        func(counter, group, board);
        counter++;
        return;
    }
    group *= m_Groups.size();
    for (const auto &pe : m_ColEntries[x]) {
        if ((mask & pe.Mask()) != 0) {
            continue;
        }
        for (const auto &piece : pe.Pieces()) {
            board.AddPiece(piece);
        }
        PopulateCol(
            func, board, counter, x + 1,
            mask | pe.Mask(), require | pe.Require(), group + pe.Group());
        for (int i = 0; i < pe.Pieces().size(); i++) {
            board.PopPiece();
        }
    }
}

void Enumerator::PopulateGroupPrimaryRow(
    const int group, EnumeratorFunc func, Board &board, uint64_t &counter) const
{
    const int digit = BoardSize * 2 - 2;
    for (const auto &pe : m_RowEntries[PrimaryRow]) {
        for (const auto &piece : pe.Pieces()) {
            board.AddPiece(piece);
        }
        PopulateGroupRow(
            group, digit, func, board, counter, 0, pe.Mask(), pe.Require());
        for (int i = 0; i < pe.Pieces().size(); i++) {
            board.PopPiece();
        }
    }
}

void Enumerator::PopulateGroupRow(
    const int group, const int digit,
    EnumeratorFunc func, Board &board, uint64_t &counter,
    int y, bb mask, bb require) const
{
    if (y >= BoardSize) {
        PopulateGroupCol(group, digit, func, board, counter, 0, mask, require);
        return;
    }
    if (y == PrimaryRow) {
        PopulateGroupRow(group, digit, func, board, counter, y + 1, mask, require);
        return;
    }
    const int g = (int)(group / std::pow(m_Groups.size(), digit)) % m_Groups.size();
    for (const auto &pe : m_RowEntries[y]) {
        if (pe.Group() != g) {
            continue;
        }
        if ((mask & pe.Mask()) != 0) {
            continue;
        }
        for (const auto &piece : pe.Pieces()) {
            board.AddPiece(piece);
        }
        PopulateGroupRow(
            group, digit - 1, func, board, counter, y + 1,
            mask | pe.Mask(), require | pe.Require());
        for (int i = 0; i < pe.Pieces().size(); i++) {
            board.PopPiece();
        }
    }
}

void Enumerator::PopulateGroupCol(
    const int group, const int digit,
    EnumeratorFunc func, Board &board, uint64_t &counter,
    int x, bb mask, bb require) const
{
    if (x >= BoardSize) {
        if ((mask & require) != require) {
            return;
        }
        func(counter, group, board);
        counter++;
        return;
    }
    const int g = (int)(group / std::pow(m_Groups.size(), digit)) % m_Groups.size();
    for (const auto &pe : m_ColEntries[x]) {
        if (pe.Group() != g) {
            continue;
        }
        if ((mask & pe.Mask()) != 0) {
            continue;
        }
        for (const auto &piece : pe.Pieces()) {
            board.AddPiece(piece);
        }
        PopulateGroupCol(
            group, digit - 1, func, board, counter, x + 1,
            mask | pe.Mask(), require | pe.Require());
        for (int i = 0; i < pe.Pieces().size(); i++) {
            board.PopPiece();
        }
    }
}

void Enumerator::ComputeGroups(std::vector<int> &sizes, int sum) {
    if (sum >= BoardSize) {
        return;
    }
    m_Groups.push_back(sizes);
    for (int s = MinPieceSize; s <= MaxPieceSize; s++) {
        sizes.push_back(s);
        ComputeGroups(sizes, sum + s);
        sizes.pop_back();
    }
}

int Enumerator::GroupForPieces(const std::vector<Piece> &pieces) {
    for (int i = 0; i < m_Groups.size(); i++) {
        const auto &group = m_Groups[i];
        if (group.size() != pieces.size()) {
            continue;
        }
        bool ok = true;
        for (int j = 0; j < group.size(); j++) {
            if (group[j] != pieces[j].Size()) {
                ok = false;
                break;
            }
        }
        if (ok) {
            return i;
        }
    }
    throw "GroupForPieces failed";
}

void Enumerator::ComputeRow(int y, int x, std::vector<Piece> &pieces) {
    if (x >= BoardSize) {
        if (y == PrimaryRow) {
            if (pieces.size() != 1) {
                return;
            }
            if (pieces[0].Size() != PrimarySize) {
                return;
            }
        }
        int n = 0;
        for (const auto &piece : pieces) {
            n += piece.Size();
        }
        if (n >= BoardSize) {
            return;
        }
        const int group = GroupForPieces(pieces);
        m_RowEntries[y].emplace_back(PositionEntry(group, pieces));
        return;
    }
    for (int s = MinPieceSize; s <= MaxPieceSize; s++) {
        if (x + s > BoardSize) {
            continue;
        }
        const int p = y * BoardSize + x;
        pieces.emplace_back(Piece(p, s, H));
        ComputeRow(y, x + s, pieces);
        pieces.pop_back();
    }
    ComputeRow(y, x + 1, pieces);
}

void Enumerator::ComputeCol(int x, int y, std::vector<Piece> &pieces) {
    if (y >= BoardSize) {
        int n = 0;
        for (const auto &piece : pieces) {
            n += piece.Size();
        }
        if (n >= BoardSize) {
            return;
        }
        const int group = GroupForPieces(pieces);
        m_ColEntries[x].emplace_back(PositionEntry(group, pieces));
        return;
    }
    for (int s = MinPieceSize; s <= MaxPieceSize; s++) {
        if (y + s > BoardSize) {
            continue;
        }
        const int p = y * BoardSize + x;
        pieces.emplace_back(Piece(p, s, V));
        ComputeCol(x, y + s, pieces);
        pieces.pop_back();
    }
    ComputeCol(x, y + 1, pieces);
}

void Enumerator::ComputePositionEntries() {
    m_RowEntries.resize(BoardSize);
    m_ColEntries.resize(BoardSize);
    std::vector<Piece> pieces;
    for (int i = 0; i < BoardSize; i++) {
        ComputeRow(i, 0, pieces);
        ComputeCol(i, 0, pieces);
    }
    for (int i = 0; i < BoardSize; i++) {
        std::stable_sort(m_RowEntries[i].begin(), m_RowEntries[i].end(),
            [](const PositionEntry &a, const PositionEntry &b)
        {
            return a.Group() < b.Group();
        });
        std::stable_sort(m_ColEntries[i].begin(), m_ColEntries[i].end(),
            [](const PositionEntry &a, const PositionEntry &b)
        {
            return a.Group() < b.Group();
        });
    }
}
