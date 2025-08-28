package api.game;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import io.javalin.http.Context;

import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Supplier;

import static lichess.LichessClient.*;

public class GameController {
    private Map<String, Game> activeGames = new ConcurrentHashMap<>();

    public void getAll(Context ctx) {
        try {
            ctx.status(200).json(activeGames.values());
        } catch (Exception e) {
            e.printStackTrace();
            ctx.status(500).result("Internal error: " + e.getMessage());
        }
    }

    public void getOne(Context ctx) {
        String gameId = ctx.pathParamAsClass("gameId", String.class).get();

        try {
            ctx.status(200).json(activeGames.get(gameId));
        } catch (Exception e) {
            e.printStackTrace();
            ctx.status(500).result("Internal error: " + e.getMessage());
        }
    }

    public void create(Context ctx) {
        String userId = ctx.pathParamAsClass("userId", String.class).get();

        if (activeGames.containsKey(userId)) {
            ctx.status(409);
            return;
        }

        String color = ctx.pathParamAsClass("color", String.class).get();

        Game newGame = new Game(userId);
        activeGames.put(userId, newGame);

        ctx.status(201).json(newGame);
    }

    public void createRandom(Context ctx) {
        String userId = ctx.pathParamAsClass("userId", String.class).get();

        if (activeGames.containsKey(userId)) {
            ctx.status(409);
            return;
        }

        ctx.future(() -> createGame() // returns CompletableFuture<String> (the JSON body)
                .thenApply(json -> {
                    try {
                        if (json.contains("\"error\"")) {
                            System.err.println("Lichess API error: " + json);
                            throw new RuntimeException();
                        }
                        System.out.println(json);
                        ObjectMapper mapper = new ObjectMapper();
                        return mapper.readValue(json, Game.class); // transform JSON -> Game
                    } catch (JsonProcessingException e) {
                        throw new RuntimeException(e);
                    }
                })
                .thenAccept(game -> {
                    // Now you can store the Game object in your map
                    activeGames.put(game.id, game);
                    System.out.println("Stored game " + game.id + " in hashmap");

                    ctx.status(201).json(game.url);
                })
                .exceptionally(e -> {
                    ctx.status(500).result("Failed to create game: " + e.getMessage());
                    return null;
                })
        );
    }

    public void delete(Context ctx) {
        String userId = ctx.pathParamAsClass("userId", String.class).get();

        Game removed = activeGames.remove(userId);

        if (removed != null) {
            ctx.status(204);
        } else {
            ctx.status(404).result("Game not found"); // ✅ more accurate
        }
    }
}