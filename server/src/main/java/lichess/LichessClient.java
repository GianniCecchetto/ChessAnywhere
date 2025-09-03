package lichess;

import api.game.Game;
import api.game.GameController;
import api.game.GameWrapper;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import io.javalin.http.Context;

import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.time.Duration;
import java.util.*;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class LichessClient {
    private static HttpClient client = HttpClient.newHttpClient();
    private final ObjectMapper mapper = new ObjectMapper();
    private final ScheduledExecutorService scheduler = Executors.newScheduledThreadPool(1);

    private final String lichessUrl = "https://lichess.org";
    private final GameController gameController;
    private final String lichessToken = System.getenv("LICHESS_TOKEN");
    private final Duration cancelGameTimeout = Duration.ofSeconds(30);


    public LichessClient(GameController gameManager) {
        this.gameController = gameManager;
    }

    public void updateGames(Context ctx, Set<String> gameIds) {
        HttpRequest challengeReq = HttpRequest.newBuilder()
                .uri(URI.create(lichessUrl + "/api/challenge"))
                .header("Authorization", "Bearer " + lichessToken)
                .header("Accept", "application/json")
                .GET()
                .build();

        ctx.future(() -> client.sendAsync(challengeReq, HttpResponse.BodyHandlers.ofString())
                .thenCompose(challengeResp -> {
                    try {
                        System.out.println("Challenge JSON: " + challengeResp.body());
                        GameWrapper wrapper = mapper.readValue(challengeResp.body(), GameWrapper.class);
                        List<Map<String, String>> result = Collections.synchronizedList(new ArrayList<>());

                        // Combine both incoming and outgoing challenges
                        List<Game> challenges = new ArrayList<>();
                        if (wrapper.in != null) challenges.addAll(wrapper.in);
                        if (wrapper.out != null) challenges.addAll(wrapper.out);

                        for (Game g : challenges) {
                            if (gameIds.contains(g.id)) {
                                Map<String, String> info = new HashMap<>();
                                info.put("id", g.id);
                                info.put("white", g.challenger != null ? g.challenger.name : "Unknown");
                                info.put("black", g.destUser != null ? g.destUser.name : "Unknown");
                                result.add(info);
                            }
                        }
                        System.out.println("Challenge results: " + result);

                        // Prepare async fetches for full games
                        List<CompletableFuture<Void>> futures = gameIds.stream()
                                .map(id -> {
                                    HttpRequest gameReq = HttpRequest.newBuilder()
                                            .uri(URI.create(lichessUrl + "/game/export/" + id))
                                            .header("Authorization", "Bearer " + lichessToken)
                                            .header("Accept", "application/json")
                                            .GET()
                                            .build();

                                    return client.sendAsync(gameReq, HttpResponse.BodyHandlers.ofString())
                                            .thenAccept(resp -> {
                                                try {
                                                    JsonNode node = mapper.readTree(resp.body());
                                                    String status = node.path("status").asText("");

                                                    // Skip finished/aborted games
                                                    if ("aborted".equalsIgnoreCase(status)
                                                            || "mate".equalsIgnoreCase(status)
                                                            || "resign".equalsIgnoreCase(status)
                                                            || "stalemate".equalsIgnoreCase(status)
                                                            || "timeout".equalsIgnoreCase(status)) {
                                                        return;
                                                    }

                                                    String white = node.path("players").path("white").path("user").path("name").asText("Unknown");
                                                    String black = node.path("players").path("black").path("user").path("name").asText("Unknown");

                                                    Map<String, String> info = new HashMap<>();
                                                    info.put("id", id);
                                                    info.put("white", white);
                                                    info.put("black", black);
                                                    System.out.println("Game JSON for " + id + ": " + resp.body());
                                                    result.add(info);
                                                } catch (Exception e) {
                                                    System.err.println("Failed to parse game " + id + ": " + e.getMessage());
                                                }
                                            });
                                })
                                .toList();

                        // Wait for all futures, then return combined result
                        return CompletableFuture.allOf(futures.toArray(new CompletableFuture[0]))
                                .thenApply(v -> {
                                    ctx.status(200).json(result);
                                    return result;
                                });

                    } catch (Exception e) {
                        throw new RuntimeException("Failed to parse challenge/game list", e);
                    }
                })
                .exceptionally(e -> {
                    ctx.status(500).result("Failed to update games: " + e.getMessage());
                    return null;
                })
        );
    }

    public void updateGame(Context ctx, String gameId) {
        HttpRequest check = HttpRequest.newBuilder()
                .uri(URI.create(lichessUrl + "/api/challenge/" + gameId + "/show"))
                .header("Authorization", "Bearer " + lichessToken)
                .header("Accept", "application/json")
                .GET()
                .build();

        ctx.future(() -> client.sendAsync(check, HttpResponse.BodyHandlers.ofString())
                .thenApply(HttpResponse::body)
                .thenApply(json -> {
                    try {
                        Game game = mapper.readValue(json, Game.class);

                        gameController.addGame(game);

                        ctx.status(200).json(game);
                        return game;
                    } catch (Exception e) {
                        throw new RuntimeException("Failed to parse game", e);
                    }
                })
                .exceptionally(e -> {
                    ctx.status(500).result("Failed to update game: " + e.getMessage());
                    return null;
                })
        );
    }

    public void createGame(Context ctx) {
            // Build the HTTP request and send it immediately
            HttpRequest req = HttpRequest.newBuilder()
                    .uri(URI.create(lichessUrl + "/api/challenge/open"))
                    .header("Authorization", "Bearer " + lichessToken)
                    .header("Content-Type", "application/x-www-form-urlencoded")
                    .POST(HttpRequest.BodyPublishers.ofString("rated=false"))
                    .build();

        ctx.future(() -> {
            return client.sendAsync(req, HttpResponse.BodyHandlers.ofString())
                    .thenApply(HttpResponse::body)
                    .thenApply(json -> {
                        try {
                            if (json.contains("\"error\"")) {
                                throw new RuntimeException("Lichess API error: " + json);
                            }

                            Game game = mapper.readValue(json, Game.class);
                            gameController.addGame(game); // store in hashmap
                            scheduleCancel(game.id, cancelGameTimeout);
                            return game;

                        } catch (Exception e) {
                            throw new RuntimeException(e);
                        }
                    })
                    .thenAccept(game -> {
                        System.out.println("Stored game " + game.id + " in hashmap");
                        ctx.status(201).result(game.id); // return URL as plain text
                    })
                    .exceptionally(e -> {
                        ctx.status(500).result("Failed to create game: " + e.getMessage());
                        return null;
                    });
        });
    }

    public void joinGame(Context ctx, String gameId) {
        HttpRequest check = HttpRequest.newBuilder()
                .uri(URI.create(lichessUrl + "/api/challenge/" + gameId + "/show"))
                .header("Authorization", "Bearer " + lichessToken)
                .header("Accept", "application/json")
                .GET()
                .build();

        ctx.future(() -> client.sendAsync(check, HttpResponse.BodyHandlers.ofString())
                .thenApply(HttpResponse::body)
                .thenApply(json -> {
                    try {
                        Game game = mapper.readValue(json, Game.class);

                        gameController.addGame(game);

                        System.out.println("Player joined game " + game.url);
                        ctx.status(200).json(game.url);
                        return game.url;
                    } catch (Exception e) {
                        throw new RuntimeException("Failed to parse game", e);
                    }
                })
                .exceptionally(e -> {
                    ctx.status(500).result("Failed to join game: " + e.getMessage());
                    return null;
                })
        );
    }

    private void scheduleCancel(String gameId, Duration timeout) {
        scheduler.schedule(() -> cancelIfUnjoined(gameId), timeout.toSeconds(), TimeUnit.SECONDS);
        System.out.println("Scheduled cancel check for game " + gameId);
    }

    private void cancelIfUnjoined(String gameId) {
        HttpRequest check = HttpRequest.newBuilder()
                .uri(URI.create(lichessUrl + "/api/challenge/" + gameId + "/show"))
                .header("Authorization", "Bearer " + lichessToken)
                .header("Accept", "application/json")
                .GET()
                .build();

        client.sendAsync(check, HttpResponse.BodyHandlers.ofString())
                .thenApply(HttpResponse::body)
                .thenAccept(json -> {
                    try {
                        Game game = mapper.readValue(json, Game.class);

                        if (game.challenger == null && game.destUser == null) {
                            cancelGame(gameId);
                            gameController.removeGame(gameId);
                        } else {
                            System.out.println("Not cancelling " + gameId +
                                    " because players are waiting");
                        }
                    } catch (Exception e) {
                        throw new RuntimeException("Failed to parse game", e);
                    }
                });
    }

    private void cancelGame(String gameId) {
        HttpRequest cancelReq = HttpRequest.newBuilder()
                .uri(URI.create(lichessUrl + "/api/challenge/" + gameId + "/cancel"))
                .header("Authorization", "Bearer " + lichessToken)
                .header("Content-Type", "application/x-www-form-urlencoded")
                .POST(HttpRequest.BodyPublishers.noBody())
                .build();

        client.sendAsync(cancelReq, HttpResponse.BodyHandlers.ofString())
                .thenAccept(body -> System.out.println("Cancelled game " + gameId +
                        " with response: " + body.statusCode()));
    }
}
