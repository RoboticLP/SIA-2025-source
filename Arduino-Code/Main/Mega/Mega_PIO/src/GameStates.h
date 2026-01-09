// ───────────────────── Game States ─────────────────────
enum GameState {
    WAIT_FOR_BALL,   // Kugel noch nicht im Spiel
    IN_GAME,         // Spiel läuft
    GAME_OVER,       // Kugel verloren
    RESET,            // Reset des Spiels
    DEBUG             // Nur für Debug-Zwecke
};