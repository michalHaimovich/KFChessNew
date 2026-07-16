#include "engine/game_engine.hpp"

GameEngine::GameEngine(int width, int height) : currentState(width, height) {} 

GameState& GameEngine::getGameState() {
    return currentState;}

bool GameEngine::requestMove(Position source, Position destination, long durationMs) {
    // 1. Application-level guard: Game Over , cooldown
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
    if (currentState.isGameOver) return;

    const long TICK_MS = 50; // physics refresh rate
    long elapsed = 0;

    while (elapsed < ms) {
        long step = std::min(TICK_MS, ms - elapsed);
        
        // advance the clock
        arbiter.advanceTime(step);
        
        // compute airborne collisions
        resolvePhysicsTick();

        for (const auto& motion : arbiter.getActiveMotionsConst()) {
            if (motion.isDead && ruleEngine.isFatalDeath(motion.piece.kind)) {
                currentState.isGameOver = true;
            }
        }
        
        // clean up motions marked as dead in the air
        arbiter.cleanupDeadMotions();

        // check which pieces arrived just now
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

    auto& motions = arbiter.getActiveMotionsRef();
    long now = arbiter.getCurrentTime();

    for (size_t i = 0; i < motions.size(); ++i) {
        if (motions[i].isDead) continue;
        
        if (motions[i].type == MotionType::Jump || motions[i].piece.kind == PieceKind::Knight) continue;

        Position pos = motions[i].getCurrentCell(now);
        Position prevPos = motions[i].getCurrentCell(now - 50);

        if (pos != motions[i].destination && pos != motions[i].source) {
            auto stationaryOpt = currentState.board.getPiece(pos);
            if (stationaryOpt.has_value()) {
                Piece stationaryPiece = stationaryOpt.value();
                
                if (stationaryPiece.color != motions[i].piece.color) {
                    currentState.board.removePiece(pos); 
                    
                    if (ruleEngine.isFatalDeath(stationaryPiece.kind)) {
                        currentState.isGameOver = true;
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
    for (size_t i = 0; i < motions.size(); ++i) {
        if (motions[i].isDead) continue; // skip dead motions

        for (size_t j = i + 1; j < motions.size(); ++j) {
            if (motions[j].isDead) continue;

            Position pos1 = motions[i].getCurrentCell(now);
            Position pos2 = motions[j].getCurrentCell(now);

            Position prevPos1 = motions[i].getCurrentCell(now - 50); // where it was before the tick
            Position prevPos2 = motions[j].getCurrentCell(now - 50);

            if (pos1 == pos2 || (pos1 == prevPos2 && pos2 == prevPos1)) {
                Motion& m1 = motions[i];
                Motion& m2 = motions[j];

                // 1. jumps
                if (m1.type == MotionType::Jump || m2.type == MotionType::Jump) {
                    Motion& jumper = (m1.type == MotionType::Jump) ? m1 : m2;
                    Motion& walker = (m1.type == MotionType::Jump) ? m2 : m1;
                    
                    if (walker.type == MotionType::Jump) continue;

                    if (jumper.piece.color != walker.piece.color) {
                        walker.isDead = true; // destroyed in air
                    } else if (walker.piece.kind != PieceKind::Knight) {
                        walker.arrivalTime = now; 
                        walker.destination = walker.getCurrentCell(now - 50);
                    }
                    continue; 
                }

                bool isM1Knight = (m1.piece.kind == PieceKind::Knight);
                bool isM2Knight = (m2.piece.kind == PieceKind::Knight);

                if (m1.piece.color != m2.piece.color) {
                    if (isM1Knight || isM2Knight) continue; 
                    
                    if (m1.startTime < m2.startTime) {
                        m2.isDead = true; 
                    } else if (m2.startTime < m1.startTime) {
                        m1.isDead = true;
                    } else {
                        m1.isDead = true;
                        m2.isDead = true;
                    }
                } else {
                    if (isM1Knight || isM2Knight) {
                        Motion& knight = isM1Knight ? m1 : m2;
                        Motion& victim = isM1Knight ? m2 : m1;
                        victim.isDead = true; // destroyed in air
                    } else {
                        if (m1.startTime < m2.startTime) {
                            m2.arrivalTime = now;
                            m2.destination = m2.getCurrentCell(now - 50);
                        } else {
                            m1.arrivalTime = now;
                            m1.destination = m1.getCurrentCell(now - 50);
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

        notifyMoveCompleted(landingPiece, motion.source, motion.destination, currentTime);

        auto targetCellPiece = currentState.board.getPiece(landingPiece.cell);
        if (targetCellPiece.has_value()) {
            currentState.board.removePiece(landingPiece.cell);
            
            notifyPieceCaptured(targetCellPiece.value());

            // generic end-of-game check
            if (ruleEngine.isFatalDeath(targetCellPiece->kind)) {
                currentState.isGameOver = true;
            }
        }

        currentState.board.addPiece(landingPiece);
    }
}

GameSnapshot GameEngine::getSnapshot() const {
    GameSnapshot snap;
    snap.boardWidth = currentState.board.getWidth();
    snap.boardHeight = currentState.board.getHeight();
    snap.isGameOver = currentState.isGameOver;

    // 1. collect pieces physically on the board (ground)
    for (int r = 0; r < snap.boardHeight; ++r) {
        for (int c = 0; c < snap.boardWidth; ++c) {
            auto p = currentState.board.getPiece(Position{r, c});
            if (p.has_value()) {
                snap.stationaryPieces.push_back(p.value());
            }
        }
    }
    
    // 2. clean architecture: engine adds airborne pieces into the snapshot.
    // the renderer does not know they are airborne; it only receives their start positions.
    snap.activeMotions = arbiter.getActiveMotionsConst();
    
    return snap;
}

bool GameEngine::requestJump(Position pos) {
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

void GameEngine::notifyMoveCompleted(const Piece& piece, Position source, Position dest, long timeMs) {
    // עוברים על כל המאזינים שנרשמו, ומעדכנים אותם!
    for (auto* observer : observers) {
        observer->onMoveCompleted(piece, source, dest, timeMs);
    }
}

void GameEngine::notifyPieceCaptured(const Piece& capturedPiece) {
    for (auto* observer : observers) {
        observer->onPieceCaptured(capturedPiece);
    }
}
