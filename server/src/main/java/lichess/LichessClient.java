package lichess;

import api.game.Game;

import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.util.concurrent.CompletableFuture;

public class LichessClient {
    private static HttpClient client;
    private static final String lichessUrl = "https://lichess.org";
    private static final String openChallengeUrl = "/api/challenge/open";
    private static final String lichessToken = System.getenv("LICHESS_TOKEN");

    public static CompletableFuture<String> createGame() {
        client = HttpClient.newHttpClient();

        HttpRequest request = HttpRequest.newBuilder()
                .uri(URI.create(lichessUrl + openChallengeUrl))
                .header("Authorization", "Bearer " + lichessToken)
                .header("Accept", "application/json")
                .POST(HttpRequest.BodyPublishers.noBody())
                .build();

        return client.sendAsync(request, HttpResponse.BodyHandlers.ofString())
                .thenApply(HttpResponse::body);
    }
}
