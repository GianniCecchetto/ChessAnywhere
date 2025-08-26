package api.challenge;

import io.javalin.http.Context;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class ChallengeController {
    Map<String, Challenge> activeChallenges = new ConcurrentHashMap<>();
    private Integer key;

    public void getAll(Context ctx) {
        try
        {
            ctx.status(200).json(activeChallenges);
        } catch(Exception e) {
            ctx.status(500);
        }
    }

    public void getOne(Context ctx) {
        Integer id = ctx.pathParamAsClass("id", Integer.class).get();
    }

    public void create(Context ctx) {
        Challenge newChallenge = new Challenge();
        activeChallenges.put(key.toString(), newChallenge);
        key = key.intValue() + 1;

        ctx.json(newChallenge.Url());
    }

    public void createRandom(Context ctx) {

    }

    public void delete(Context ctx) {

    }
}