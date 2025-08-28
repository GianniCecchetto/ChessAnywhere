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
            ctx.status(200).json(activeChallenges.values());
        } catch (Exception e) {
            e.printStackTrace();
            ctx.status(500).result("Internal error: " + e.getMessage());
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

        ctx.status(201).json(newChallenge);
    }

    public void createRandom(Context ctx) {
        String userId = ctx.pathParamAsClass("userId", String.class).get();

        if (activeChallenges.containsKey(userId)) {
            ctx.status(409);
            return;
        }

        Challenge newChallenge = new Challenge(userId);
        activeChallenges.put(userId, newChallenge);

        ctx.status(201).json(newChallenge);
    }

    public void delete(Context ctx) {
        String userId = ctx.pathParamAsClass("userId", String.class).get();

        Challenge removed = activeChallenges.remove(userId);

        if (removed != null) {
            ctx.status(204);
        } else {
            ctx.status(404).result("Challenge not found"); // ✅ more accurate
        }
    }
}