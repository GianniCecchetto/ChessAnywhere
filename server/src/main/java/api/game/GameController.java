package api.game;

import io.javalin.http.Context;
import lichess.LichessClient;

import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

public class GameController {
    private Map<String, Game> activeGames;
    LichessClient lichessClient;

    public GameController() {
        activeGames = new ConcurrentHashMap<>();
        lichessClient = new LichessClient(this);
    }

    public Set<String> getGames() {
        return activeGames.keySet();
    }

    public void addGame(Game game) {
        activeGames.put(game.id, game);
    }

    public void removeGame(String gameId) {
        activeGames.remove(gameId);
    }

    public void getAll(Context ctx) {
        lichessClient.updateGames(ctx, activeGames.keySet());
        try {
            ctx.status(200).json(activeGames.values());
        } catch (Exception e) {
            e.printStackTrace();
            ctx.status(500).result("Internal error: " + e.getMessage());
        }
    }

    public void getOne(Context ctx) {
        String gameId = ctx.pathParamAsClass("gameId", String.class).get();

        lichessClient.updateGame(ctx, gameId);
        try {
            ctx.status(200).json(activeGames.get(gameId));
        } catch (Exception e) {
            e.printStackTrace();
            ctx.status(500).result("Internal error: " + e.getMessage());
        }
    }

    public void create(Context ctx) {
        String color = ctx.pathParamAsClass("color", String.class).get();
    }

    public void createRandom(Context ctx) {
        lichessClient.createGame(ctx);
    }
}