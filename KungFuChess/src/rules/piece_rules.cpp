#include "rules/piece_rules.hpp"
#include <vector>
#include <utility>

static std::set<Position> getSlidingDestinations(const Board& board, const Piece& piece, const std::vector<std::pair<int, int>>& directions) {
    std::set<Position> destinations;
    
    for (const auto& dir : directions) {
        Position current = piece.cell;
        while (true) {
            current.row += dir.first;
            current.col += dir.second;

            if (!board.isInside(current)) break;

            auto targetPiece = board.getPiece(current);
            if (!targetPiece.has_value()) {
                destinations.insert(current); // empty square
            } else {
                if (targetPiece->color != piece.color) {
                    destinations.insert(current); // capture enemy
                }
                break; // blocked by piece, stop sliding in this direction
            }
        }
    }
    return destinations;
}

static std::set<Position> getSteppingDestinations(const Board& board, const Piece& piece, const std::vector<std::pair<int, int>>& offsets) {
    std::set<Position> destinations;
    
    for (const auto& offset : offsets) {
        Position target{piece.cell.row + offset.first, piece.cell.col + offset.second};

        if (board.isInside(target)) {
            auto targetPiece = board.getPiece(target);
            if (!targetPiece.has_value() || targetPiece->color != piece.color) {
                destinations.insert(target); // empty square or enemy
            }
        }
    }
    return destinations;
}

// ==========================================
// ROOK IMPLEMENTATION
// ==========================================
std::set<Position> RookRule::legalDestinations(const Board& board, const Piece& piece) const {
    return getSlidingDestinations(board, piece, {{0, 1}, {0, -1}, {1, 0}, {-1, 0}});
}

// ==========================================
// BISHOP IMPLEMENTATION
// ==========================================
std::set<Position> BishopRule::legalDestinations(const Board& board, const Piece& piece) const {
    return getSlidingDestinations(board, piece, {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}});
}

// ==========================================
// QUEEN IMPLEMENTATION
// ==========================================
std::set<Position> QueenRule::legalDestinations(const Board& board, const Piece& piece) const {
    return getSlidingDestinations(board, piece, {
        {0, 1}, {0, -1}, {1, 0}, {-1, 0},  // rook directions
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1} // bishop directions
    });
}

// ==========================================
// KNIGHT IMPLEMENTATION
// ==========================================
std::set<Position> KnightRule::legalDestinations(const Board& board, const Piece& piece) const {
    return getSteppingDestinations(board, piece, {
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
        {1, 2}, {1, -2}, {-1, 2}, {-1, -2}
    });
}

// ==========================================
// KING IMPLEMENTATION
// ==========================================
std::set<Position> KingRule::legalDestinations(const Board& board, const Piece& piece) const {
    return getSteppingDestinations(board, piece, {
        {0, 1}, {0, -1}, {1, 0}, {-1, 0},
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    });
}

// ==========================================
// PAWN IMPLEMENTATION
// ==========================================
std::set<Position> PawnRule::legalDestinations(const Board& board, const Piece& piece) const {
    std::set<Position> destinations;

    // determine direction and starting row
    int direction = (piece.color == PieceColor::White) ? -1 : 1;
    int startRow = (piece.color == PieceColor::White) ? (board.getHeight() - 2) : 1;

    Position current = piece.cell;

    // 1. one step forward
    Position forward1{current.row + direction, current.col};
    bool canMoveOneStep = false;

    if (board.isInside(forward1)) {
        auto targetPiece = board.getPiece(forward1);
        if (!targetPiece.has_value()) {
            destinations.insert(forward1);
            canMoveOneStep = true;
        }
    }

    // 2. double move from starting rank
    if (canMoveOneStep && current.row == startRow) {
        Position forward2{current.row + 2 * direction, current.col};
        if (board.isInside(forward2)) {
            auto targetPiece2 = board.getPiece(forward2);
            if (!targetPiece2.has_value()) {
                destinations.insert(forward2);
            }
        }
    }

    // 3. capture diagonally (left and right)
    int captureOffsets[2] = {-1, 1};
    for (int colOffset : captureOffsets) {
        Position capturePos{current.row + direction, current.col + colOffset};
        
        if (board.isInside(capturePos)) {
            auto targetPiece = board.getPiece(capturePos);
            if (targetPiece.has_value() && targetPiece->color != piece.color) {
                destinations.insert(capturePos);
            }
        }
    }

    return destinations;
}

// ==========================================
// KING SPECIFIC RULES
// ==========================================
bool KingRule::isFatalDeath() const {
    return true; 
}

// ==========================================
// PAWN SPECIFIC RULES
// ==========================================
void PawnRule::onArrival(const Board& board, Piece& piece) const {
    if (piece.cell.row == 0 || piece.cell.row == board.getHeight() - 1) {
        piece.kind = PieceKind::Queen; 
    }
}