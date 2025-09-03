package lichess;

import api.game.Game;
import api.game.GameController;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ArrayNode;
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

/**
 * LichessClient est une classe responsable de l'interaction avec l'API Lichess.
 * Elle permet de récupérer les challenges et parties en cours, créer et rejoindre des parties,
 * et gérer l'annulation automatique des parties non rejointes après un délai.
 */
public class LichessClient {
    private static HttpClient client = HttpClient.newHttpClient(); // Client HTTP pour toutes les requêtes
    private final ObjectMapper mapper = new ObjectMapper(); // JSON mapper pour transformer JSON <-> objets
    private final ScheduledExecutorService scheduler = Executors.newScheduledThreadPool(1); // Scheduler pour annuler les parties non rejointes

    private final String lichessUrl = "https://lichess.org"; // URL de base de l'API Lichess
    private final GameController gameController; // Gestionnaire local des parties
    private final String lichessToken = System.getenv("LICHESS_TOKEN"); // Token pour authentification API
    private final Duration cancelGameTimeout = Duration.ofSeconds(30); // Timeout avant annulation automatique d'une partie

    /**
     * Constructeur.
     * @param gameManager L'instance de GameController qui gère les parties en mémoire.
     */
    public LichessClient(GameController gameManager) {
        this.gameController = gameManager;
    }

    /**
     * Récupère les challenges entrants, sortants et les parties en cours depuis Lichess
     * et renvoie un JSON simplifié à l'utilisateur.
     * @param ctx Contexte HTTP Javalin
     */
    public void updateGames(Context ctx) {
        // Récupère les challenges depuis Lichess
        HttpRequest challengeReq = HttpRequest.newBuilder()
                .uri(URI.create(lichessUrl + "/api/challenge"))
                .header("Authorization", "Bearer " + lichessToken)
                .header("Accept", "application/json")
                .GET()
                .build();

        ctx.future(() -> client.sendAsync(challengeReq, HttpResponse.BodyHandlers.ofString())
                .thenCompose(challengesResp -> {
                    List<Map<String, Object>> result = new ArrayList<>();

                    try {
                        // --- parse challenges ---
                        JsonNode challengesNode = mapper.readTree(challengesResp.body());

                        ArrayNode in = (ArrayNode) challengesNode.path("in");
                        ArrayNode out = (ArrayNode) challengesNode.path("out");

                        if (in != null) {
                            for (JsonNode c : in) {
                                Map<String, Object> info = new HashMap<>();
                                info.put("id", c.path("id").asText());
                                info.put("type", "challenge");
                                info.put("challenger", c.path("challenger").path("name").asText("Unknown"));
                                info.put("dest", c.path("destUser").path("name").asText("Unknown"));
                                result.add(info);
                            }
                        }
                        if (out != null) {
                            for (JsonNode c : out) {
                                Map<String, Object> info = new HashMap<>();
                                info.put("id", c.path("id").asText());
                                info.put("type", "challenge");
                                info.put("challenger", c.path("challenger").path("name").asText("Unknown"));
                                info.put("dest", c.path("destUser").path("name").asText("Unknown"));
                                result.add(info);
                            }
                        }
                    } catch (Exception e) {
                        ctx.status(500).result("Failed to parse challenges: " + e.getMessage());
                        return CompletableFuture.completedFuture(result);
                    }

                    // --- fetch each game from Lichess using its gameId ---
                    List<CompletableFuture<Void>> gameFutures = new ArrayList<>();

                    Set<String> finishedStatuses = Set.of(
                            "mate", "resign", "stalemate", "timeout", "draw", "outoftime", "aborted", "terminated"
                    );

                    for (Game g : gameController.getGames()) {
                        HttpRequest req = HttpRequest.newBuilder()
                                .uri(URI.create(lichessUrl + "/game/export/" + g.id + "?asJson=true"))
                                .header("Authorization", "Bearer " + lichessToken)
                                .header("Accept", "application/json")
                                .GET()
                                .build();

                        CompletableFuture<Void> future = client.sendAsync(req, HttpResponse.BodyHandlers.ofString())
                                .orTimeout(10, TimeUnit.SECONDS)
                                .thenApply(HttpResponse::body)
                                .thenAccept(json -> {
                                    try {
                                        JsonNode gameNode = mapper.readTree(json);

                                        String status = gameNode.path("status").asText("unknown");

                                        if (!finishedStatuses.contains(status)) {
                                            Map<String, Object> info = new HashMap<>();
                                            info.put("id", gameNode.path("id").asText());
                                            info.put("type", "game");

                                            // Players
                                            String white = gameNode.path("players").path("white").path("user").path("name").asText("Unknown");
                                            String black = gameNode.path("players").path("black").path("user").path("name").asText("Unknown");

                                            info.put("white", white);
                                            info.put("black", black);
                                            info.put("status", status);

                                            result.add(info);
                                        }
                                    } catch (Exception e) {
                                        System.err.println("Failed to parse game " + g.id + ": " + e.getMessage());
                                    }
                                });

                        gameFutures.add(future);
                    }

                    // Wait for all game fetches to complete
                    return CompletableFuture.allOf(gameFutures.toArray(new CompletableFuture[0]))
                            .thenApply(v -> {
                                ctx.status(200).json(result);
                                return result;
                            });
                }));
    }

    /**
     * Récupère une partie spécifique depuis Lichess et l'ajoute au GameController.
     * @param ctx Contexte HTTP Javalin
     * @param gameId ID de la partie Lichess
     */
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

    /**
     * Crée une partie ouverte sur Lichess (non classée) et l'ajoute au GameController.
     * Planifie l'annulation automatique si personne ne rejoint la partie.
     * @param ctx Contexte HTTP Javalin
     */
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

    /**
     * Permet de rejoindre une partie existante sur Lichess et de l'ajouter au GameController.
     * @param ctx Contexte HTTP Javalin
     * @param gameId ID de la partie à rejoindre
     */
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

    /**
     * Planifie l'annulation d'une partie si elle n'est pas encore jointe après un certain délai.
     * @param gameId ID de la partie
     * @param timeout Durée avant annulation
     */
    private void scheduleCancel(String gameId, Duration timeout) {
        scheduler.schedule(() -> cancelIfUnjoined(gameId), timeout.toSeconds(), TimeUnit.SECONDS);
        System.out.println("Scheduled cancel check for game " + gameId);
    }

    /**
     * Vérifie si une partie est toujours non jointe et l'annule si nécessaire.
     * @param gameId ID de la partie à vérifier
     */
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

    /**
     * Annule une partie Lichess spécifique.
     * @param gameId ID de la partie à annuler
     */
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
