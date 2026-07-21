#include "engine/game_engine.hpp"

namespace {
    constexpr long PHYSICS_TICK_MS = 50;
}

GameEngine::GameEngine(int width, int height) : currentState(width, height) {} 

GameState& GameEngine::getGameState() {
    return currentState;
}

bool GameEngine::requestMove(Position source, Position destination, long durationMs) {
    std::lock_guard<std::mutex> lock(mtx);

    // 1. Application-level guard: Game Over, cooldown
    if (currentState.isGameOver) return false;

    auto pieceOpt = currentState.board.getPiece(source);
    if (!pieceOpt.has_value()) return false;
    Piece piece = pieceOpt.value();

    if (piece.state != PieceState::Idle) {
        return false;
    }
   
    // 3. Delegate to RuleEngine for read-only legality validation
    if (!ruleEngine.validateMove(currentState.board, piece, destination)) {
        return false;
    }

    // 4. Remove from logical Board (it is now "in the air")
    currentState.board.removePiece(source);

    // 5. Update state and start motion in Arbiter
    piece.state = PieceState::Move;
    arbiter.startMotion(piece, source, destination, durationMs);

    return true;
}

void GameEngine::wait(long ms) {
    std::lock_guard<std::mutex> lock(mtx);

    if (currentState.isGameOver) return;

    long elapsed = 0;

    while (elapsed < ms) {
        long step = std::min(PHYSICS_TICK_MS, ms - elapsed);
        
        arbiter.advanceTime(step);
        resolvePhysicsTick();

        for (const auto& motion : arbiter.getActiveMotionsConst()) {
            if (motion.isDead && ruleEngine.isFatalDeath(motion.piece.kind)) {
                currentState.isGameOver = true;
                // NEW: The piece that died in motion gives the win to the opposite color
                currentState.winner = (motion.piece.color == PieceColor::White) ? PieceColor::Black : PieceColor::White;
            }
        }
        
        arbiter.cleanupDeadMotions();

        std::vector<Motion> arrivals = arbiter.popArrivedMotions();
        processArrivals(arrivals);

        long currentTime = arbiter.getCurrentTime();
        for (int r = 0; r < currentState.board.getHeight(); ++r) {
            for (int c = 0; c < currentState.board.getWidth(); ++c) {
                Position pos{r, c};
                auto pieceOpt = currentState.board.getPiece(pos);
                
                if (pieceOpt.has_value()) {
                    Piece p = pieceOpt.value();
                    if ((p.state == PieceState::ShortRest || p.state == PieceState::LongRest) 
                        && currentTime >= p.readyTime) {
                        
                        p.state = PieceState::Idle; 
                        
                        currentState.board.removePiece(pos);
                        currentState.board.addPiece(p);
                    }
                }
            }
        }
        elapsed += step;
    }
}

void GameEngine::resolvePhysicsTick() {
    long now = arbiter.getCurrentTime();
    
    checkAirToGroundCollisions(now);
    checkMidAirCollisions(now);
}

void GameEngine::checkAirToGroundCollisions(long now) {
    auto& motions = arbiter.getActiveMotionsRef();

    for (size_t i = 0; i < motions.size(); ++i) {
        if (motions[i].isDead) continue;
        
        if (motions[i].type == MotionType::Jump || motions[i].piece.kind == PieceKind::Knight) continue;

        Position pos = motions[i].getCurrentCell(now);
        Position prevPos = motions[i].getCurrentCell(now - PHYSICS_TICK_MS);

        if (pos != motions[i].destination && pos != motions[i].source) {
            auto stationaryOpt = currentState.board.getPiece(pos);
            if (stationaryOpt.has_value()) {
                Piece stationaryPiece = stationaryOpt.value();
                
                if (stationaryPiece.color != motions[i].piece.color) {
                    notifyPieceCaptured(stationaryPiece);
                    currentState.board.removePiece(pos); 
                    
                    if (ruleEngine.isFatalDeath(stationaryPiece.kind)) {
                        currentState.isGameOver = true;
                        currentState.winner = motions[i].piece.color;
                    }
                    
                } else {
                    if (prevPos != motions[i].source) {
                        motions[i].destination = prevPos;
                    } else {
                        motions[i].destination = motions[i].source; 
                    }
                    motions[i].arrivalTime = now; 
                }
            }
        }
    }
}

void GameEngine::checkMidAirCollisions(long now) {
    auto& motions = arbiter.getActiveMotionsRef();

    for (size_t i = 0; i < motions.size(); ++i) {
        if (motions[i].isDead) continue; 

        for (size_t j = i + 1; j < motions.size(); ++j) {
            if (motions[j].isDead) continue;

            Position pos1 = motions[i].getCurrentCell(now);
            Position pos2 = motions[j].getCurrentCell(now);

            Position prevPos1 = motions[i].getCurrentCell(now - PHYSICS_TICK_MS); 
            Position prevPos2 = motions[j].getCurrentCell(now - PHYSICS_TICK_MS);

            if (pos1 == pos2 || (pos1 == prevPos2 && pos2 == prevPos1)) {
                Motion& m1 = motions[i];
                Motion& m2 = motions[j];

                if (m1.type == MotionType::Jump || m2.type == MotionType::Jump) {
                    Motion& jumper = (m1.type == MotionType::Jump) ? m1 : m2;
                    Motion& walker = (m1.type == MotionType::Jump) ? m2 : m1;
                    
                    if (walker.type == MotionType::Jump) continue;

                    if (jumper.piece.color != walker.piece.color) {
                        walker.isDead = true; 
                        notifyPieceCaptured(walker.piece);

                        if (ruleEngine.isFatalDeath(walker.piece.kind)) {
                            currentState.isGameOver = true;
                            currentState.winner = jumper.piece.color;
                        }

                    } else if (walker.piece.kind != PieceKind::Knight) {
                        walker.arrivalTime = now; 
                        walker.destination = walker.getCurrentCell(now - PHYSICS_TICK_MS);
                    }
                    continue; 
                }

                bool isM1Knight = (m1.piece.kind == PieceKind::Knight);
                bool isM2Knight = (m2.piece.kind == PieceKind::Knight);

                if (m1.piece.color != m2.piece.color) {
                    if (isM1Knight || isM2Knight) continue; 
                    
                    if (m1.startTime < m2.startTime) {
                        m2.isDead = true; 
                        notifyPieceCaptured(m2.piece);
                        if (ruleEngine.isFatalDeath(m2.piece.kind)) {
                            currentState.isGameOver = true;
                            currentState.winner = m1.piece.color;
                        }
                    } else if (m2.startTime < m1.startTime) {
                        m1.isDead = true;
                        notifyPieceCaptured(m1.piece);
                        if (ruleEngine.isFatalDeath(m1.piece.kind)) {
                            currentState.isGameOver = true;
                            currentState.winner = m2.piece.color;
                        }
                    } else {
                        m1.isDead = true;
                        m2.isDead = true;
                        notifyPieceCaptured(m1.piece); 
                        notifyPieceCaptured(m2.piece);
                        if (ruleEngine.isFatalDeath(m1.piece.kind) || ruleEngine.isFatalDeath(m2.piece.kind)) {
                            currentState.isGameOver = true;
                        }
                    }
                } else {
                    if (isM1Knight || isM2Knight) {
                        continue; 
                    } else {
                        if (m1.startTime < m2.startTime) {
                            m2.arrivalTime = now;
                            m2.destination = m2.getCurrentCell(now - PHYSICS_TICK_MS);
                        } else {
                            m1.arrivalTime = now;
                            m1.destination = m1.getCurrentCell(now - PHYSICS_TICK_MS);
                        }
                    }
                }
            }
        }
    }
}

void GameEngine::processArrivals(const std::vector<Motion>& arrivals) {
    long currentTime = arbiter.getCurrentTime();

    for (const auto& motion : arrivals) {
        if (motion.isDead) continue;

        Piece landingPiece = motion.piece;
        landingPiece.cell = motion.destination;
        
        if (motion.type == MotionType::Jump) {
            landingPiece.state = PieceState::ShortRest;
        } else {
            landingPiece.state = PieceState::LongRest;
        }
        
        landingPiece.readyTime = currentTime + ruleEngine.getCooldownMs(landingPiece.kind, landingPiece.state);

        ruleEngine.processArrival(currentState.board, landingPiece);

        bool destinationCapture = false;

        auto targetCellPiece = currentState.board.getPiece(landingPiece.cell);
        if (targetCellPiece.has_value()) {
            
            if (targetCellPiece->color == landingPiece.color) {
                int dr = (motion.destination.row > motion.source.row) ? 1 : (motion.destination.row < motion.source.row) ? -1 : 0;
                int dc = (motion.destination.col > motion.source.col) ? 1 : (motion.destination.col < motion.source.col) ? -1 : 0;
                
                landingPiece.cell = Position{motion.destination.row - dr, motion.destination.col - dc};
                currentState.board.addPiece(landingPiece);
                continue; 
            }

            notifyPieceCaptured(targetCellPiece.value());
            destinationCapture = true;
            
            currentState.board.removePiece(landingPiece.cell);
            
            if (ruleEngine.isFatalDeath(targetCellPiece->kind)) {
                currentState.isGameOver = true;
                // NEW: The landing piece captured a fatal piece
                currentState.winner = landingPiece.color; 
            }
        }

        notifyMoveCompleted(landingPiece, motion.source, motion.destination, destinationCapture, currentTime);

        currentState.board.addPiece(landingPiece);
    }
}

GameSnapshot GameEngine::getSnapshot(std::optional<Position> selectedPos) const {
    std::lock_guard<std::mutex> lock(mtx);

    GameSnapshot snap;
    snap.serverTime = arbiter.getCurrentTime();
    snap.boardWidth = currentState.board.getWidth();
    snap.boardHeight = currentState.board.getHeight();
    snap.isGameOver = currentState.isGameOver;
    snap.winner = currentState.winner; // Assuming you added this to GameState

    for (int r = 0; r < snap.boardHeight; ++r) {
        for (int c = 0; c < snap.boardWidth; ++c) {
            auto p = currentState.board.getPiece(Position{r, c});
            if (p.has_value()) {
                snap.stationaryPieces.push_back(p.value());
            }
        }
    }
    
    snap.activeMotions = arbiter.getActiveMotionsConst();
    
    // NEW: Handle Selection and Highlights
    if (selectedPos.has_value()) {
        auto pieceOpt = currentState.board.getPiece(selectedPos.value());
        
        // Only highlight if there is a piece there AND it's currently Idle (can be moved)
        if (pieceOpt.has_value() && pieceOpt.value().state == PieceState::Idle) {
            snap.selectedPiece = pieceOpt.value();
            snap.highlightedCells = ruleEngine.getLegalDestinations(currentState.board, pieceOpt.value());
        }
    }
    
    return snap;
}

bool GameEngine::requestJump(Position pos) {
    std::lock_guard<std::mutex> lock(mtx);

    // 1. guard: game is over
    if (currentState.isGameOver) return false;

    // 2. check: is there a piece at this position?
    auto pieceOpt = currentState.board.getPiece(pos);
    if (!pieceOpt.has_value()) return false;
    Piece piece = pieceOpt.value();

    // 3. invariant rule: Move or captured pieces cannot jump
    if (piece.state != PieceState::Idle) return false;

    // 4. remove from logical board (piece detaches from ground and goes airborne)
    currentState.board.removePiece(pos);
    piece.state = PieceState::Move;

    // 5. start Jump motion in arbiter (1000ms, same position)
    // note: ensure startMotion supports MotionType
    arbiter.startMotion(piece, pos, pos, 1000, MotionType::Jump);

    return true;
}

void GameEngine::setupStandardBoard() {
    std::lock_guard<std::mutex> lock(mtx);

    if (currentState.board.getWidth() != 8 || currentState.board.getHeight() != 8) return;

    int currentId = 1; 

    for (int col = 0; col < 8; ++col) {
        currentState.board.addPiece(Piece(currentId++, PieceColor::Black, PieceKind::Pawn, Position{1, col}));
        currentState.board.addPiece(Piece(currentId++, PieceColor::White, PieceKind::Pawn, Position{6, col}));
    }

    PieceKind backRank[] = {
        PieceKind::Rook, PieceKind::Knight, PieceKind::Bishop, PieceKind::Queen,
        PieceKind::King, PieceKind::Bishop, PieceKind::Knight, PieceKind::Rook
    };

    for (int col = 0; col < 8; ++col) {
        currentState.board.addPiece(Piece(currentId++, PieceColor::Black, backRank[col], Position{0, col}));
        currentState.board.addPiece(Piece(currentId++, PieceColor::White, backRank[col], Position{7, col}));
    }
}

void GameEngine::notifyMoveCompleted(const Piece& piece, Position source, Position dest, bool destinationCapture, long timeMs) {    
    if (!eventBus) return;
    
   MoveCompletedEvent event(piece, source, dest, destinationCapture, timeMs);
    eventBus->publish(event);
}

void GameEngine::notifyPieceCaptured(const Piece& capturedPiece) {
    if (!eventBus) return;
    
    PieceCapturedEvent event(capturedPiece);
    
    eventBus->publish(event);
}