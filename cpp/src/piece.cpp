#include "piece.h"

Piece::Piece(int position, int size, int stride) :
    m_Position(position),
    m_Size(size),
    m_Stride(stride),
    m_Mask(0)
{
    int p = position;
    for (int i = 0; i < size; i++) {
        m_Mask |= (bb)1 << p;
        p += stride;
    }
}

int Piece::Position() const {
    return m_Position;
}

int Piece::Size() const {
    return m_Size;
}

int Piece::Stride() const {
    return m_Stride;
}

bb Piece::Mask() const {
    return m_Mask;
}

void Piece::Move(int steps) {
    const int d = m_Stride * steps;
    m_Position += d;
    if (steps > 0) {
        m_Mask <<= d;
    } else {
        m_Mask >>= -d;
    }
}
