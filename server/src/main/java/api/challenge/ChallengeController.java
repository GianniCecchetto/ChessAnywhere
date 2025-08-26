package api.challenge;

import io.javalin.http.Context;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class ChallengeController {
    Map<String, Challenge> activeChallenges = new ConcurrentHashMap<>();

    public void getAll(Context ctx) {
        try {
            List<Challenge> challenges = new ArrayList<>(activeChallenges.values());
            ctx.status(200).json(challenges);
        } catch (Exception e) {
            ctx.status(500).result("Error retrieving challenges");
        }
    }

    public void getOne(Context ctx) {
        Integer id = ctx.pathParamAsClass("id", Integer.class).get();
    }

    public void create(Context ctx) {
        String userId = ctx.pathParamAsClass("userId", String.class).get();

        if (activeChallenges.containsKey(userId)) {
            ctx.status(409);
            return;
        }

        String color = ctx.pathParamAsClass("color", String.class).get();

        Challenge newChallenge = new Challenge(userId);
        activeChallenges.put(userId, newChallenge);

        ctx.json(newChallenge.Url());
    }

    public void createRandom(Context ctx) {
        String userId = ctx.pathParamAsClass("userId", String.class).get();

        if (activeChallenges.containsKey(userId)) {
            ctx.status(409);
            return;
        }

        Challenge newChallenge = new Challenge(userId);
        activeChallenges.put(userId, newChallenge);

        ctx.json(newChallenge.Url());
    }

    public void delete(Context ctx) {
        String userId = ctx.pathParamAsClass("userId", String.class).get();

        activeChallenges.remove(userId);

        ctx.status(204);
    }
}