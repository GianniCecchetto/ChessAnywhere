package api;

import api.challenge.ChallengeController;
import database.Sqlite;

import io.javalin.Javalin;
import io.javalin.http.InternalServerErrorResponse;

import java.io.IOException;
import java.security.NoSuchAlgorithmException;
import java.security.spec.InvalidKeySpecException;
import java.sql.SQLException;
import java.time.LocalDateTime;

public class Api {
    private final Javalin app;

    private final ChallengeController challengeController;

    public Api(/*Sqlite database*/) {
        this.app = Javalin.create(config -> config.validation.register(LocalDateTime.class, LocalDateTime::parse));

        this.challengeController = new ChallengeController();

        //this.auth = new Auth(database, cacherUser); maybe later
    }

    public void start(int port) {
        app.exception(SQLException.class, (e, ctx) -> {
            System.out.println(e.getMessage());
            throw new InternalServerErrorResponse();
        });

        //app.before(auth::protect); maybe later

        app.get("/ping", ctx -> ctx.status(200));

        // Challenges
        app.get("/api/challenges", challengeController::getAll);
        app.post("/api/challenge/{userId}", challengeController::createRandom);
        app.post("/api/challenge/{color}/{userId}", challengeController::create);
        app.delete("/api/challenge/{userId}", challengeController::delete);

        app.start("0.0.0.0", port);
    }

    public void stop() {
        app.stop();
    }
}