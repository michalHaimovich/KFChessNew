#include "kung_fu_bot.hpp"

KungFuBot::KungFuBot(PieceColor color, int elo, GameEngine& engine, RuleEngine& ruleEngine)
    : m_color(color), 
      m_config(BotConfig::fromElo(elo)), 
      m_engine(engine), 
      m_ruleEngine(ruleEngine),
      m_lastActionTimeMs(0) {}

int KungFuBot::getDistance(Position a, Position b) const {
    return std::abs(a.row - b.row) + std::abs(a.col - b.col);
}

bool KungFuBot::isUnderThreat(const GameSnapshot& snap, Position pos, long currentTimeMs, long& outThreatArrivalTime) const {
    for (const auto& motion : snap.activeMotions) {
        // Ignore dead motions[cite: 3] or motions from our own pieces
        if (motion.isDead || motion.piece.color == m_color) continue;
        
        // If the enemy is moving to this position
        if (motion.destination == pos) {
            // Apply APM/Reaction Time limit: We only "see" the threat if enough time passed since it started
            long timeSinceStart = currentTimeMs - motion.startTime;
            if (timeSinceStart >= m_config.reactionTimeMs) {
                outThreatArrivalTime = motion.arrivalTime;
                return true;
            }
        }
    }
    return false;
}

int KungFuBot::evaluateMove(const GameSnapshot& snap, const Piece& movingPiece, Position dest) const {
    int score = 0;

    // 1. Check if destination captures an enemy piece
    for (const auto& p : snap.stationaryPieces) {
        if (p.cell == dest && p.color != m_color) {
            score += 100; // Base capture score
            if (m_ruleEngine.isFatalDeath(p.kind)) { // Uses rule engine for abstraction[cite: 14]
                score += 10000; // Winning move!
            }
            break;
        }
    }

    // 2. Encourage moving towards the enemy side (Basic positioning)
    int forwardDirection = (m_color == PieceColor::White) ? -1 : 1;
    if ((dest.row - movingPiece.cell.row) == forwardDirection) {
        score += 10;
    }

    return score;
}

void KungFuBot::update(long currentTimeMs) {
    // APM Check: Enforce minimum time between actions
    if (currentTimeMs - m_lastActionTimeMs < m_config.actionDelayMs) {
        return; 
    }

    GameSnapshot snap = m_engine.getSnapshot();
    if (snap.isGameOver) return;

    // Temporary board object needed to ask RuleEngine for legal moves
    Board tempBoard(snap.boardWidth, snap.boardHeight);
    for (const auto& p : snap.stationaryPieces) {
        tempBoard.addPiece(p);
    }

    // Process each stationary piece owned by the bot
    for (const auto& piece : snap.stationaryPieces) {
        if (piece.color != m_color || piece.state != PieceState::Idle) continue; // Only process ready pieces[cite: 4, 5]

        long threatArrivalTime = 0;
        bool isThreatened = isUnderThreat(snap, piece.cell, currentTimeMs, threatArrivalTime);

        if (isThreatened) {
            // Dodge Logic
            if (m_config.canJump) {
                // If the threat is arriving soon (e.g., within next 400ms), jump to squash them!
                if (threatArrivalTime - currentTimeMs < 400) {
                    if (m_engine.requestJump(piece.cell)) { //[cite: 5, 6]
                        m_lastActionTimeMs = currentTimeMs;
                        return; // Action taken, exit update loop
                    }
                }
            }
            
            // If can't jump or decided to run away
            std::set<Position> legalMoves = m_ruleEngine.getLegalDestinations(tempBoard, piece); //[cite: 14]
            for (const auto& dest : legalMoves) {
                long dummyTime;
                // Move to a square that is NOT currently under threat
                if (!isUnderThreat(snap, dest, currentTimeMs, dummyTime)) {
                    if (m_engine.requestMove(piece.cell, dest)) {
                        m_lastActionTimeMs = currentTimeMs;
                        return;
                    }
                }
            }
        } else {
            // Offensive / Positioning Logic
            std::set<Position> legalMoves = m_ruleEngine.getLegalDestinations(tempBoard, piece);
            Position bestMove = piece.cell;
            int bestScore = -1;

            for (const auto& dest : legalMoves) {
                // Respect Tunnel Vision (Lookahead radius)
                if (getDistance(piece.cell, dest) > m_config.lookaheadRadius) continue;

                int score = evaluateMove(snap, piece, dest);
                if (score > bestScore) {
                    bestScore = score;
                    bestMove = dest;
                }
            }

            // If a highly valuable move was found (like a capture)
            if (bestScore > 0 && bestMove != piece.cell) {
                if (m_engine.requestMove(piece.cell, bestMove)) {
                    m_lastActionTimeMs = currentTimeMs;
                    return; 
                }
            }
        }
    }
}