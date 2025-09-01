package api.game;

import io.javalin.http.Context;
import lichess.LichessClient;

import java.util.ArrayList;
import java.util.List;
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

    public List<Game> getGames() {
        return new ArrayList<>(activeGames.values());
    }

    public void addGame(Game game) {
        activeGames.put(game.id, game);
    }

    public void removeGame(String gameId) {
        activeGames.remove(gameId);
    }

    public void getAll(Context ctx) {
        lichessClient.updateGames(ctx, activeGames.keySet());
    }

    public void getOne(Context ctx) {
        String gameId = ctx.pathParamAsClass("gameId", String.class).get();

        lichessClient.updateGame(ctx, gameId);
    }

    public void create(Context ctx) {
        String color = ctx.pathParamAsClass("color", String.class).get();
    }

    public void createRandom(Context ctx) {
        lichessClient.createGame(ctx);
    }

    public void join(Context ctx) {
        String gameId = ctx.pathParamAsClass("gameId", String.class).get();

        lichessClient.joinGame(ctx, gameId);
    }
}