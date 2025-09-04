package api;

import api.game.GameController;

import io.javalin.Javalin;
import io.javalin.http.InternalServerErrorResponse;

import java.sql.SQLException;
import java.time.LocalDateTime;

public class Api {
    private final Javalin app;

    private final GameController gameController;

    public Api(/*Sqlite database*/) {
        this.app = Javalin.create(config -> config.validation.register(LocalDateTime.class, LocalDateTime::parse));

        this.gameController = new GameController();

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
        app.get("/api/games", gameController::getAll);
        app.get("/api/games/{gameId}", gameController::getOne);
        app.post("/api/games/create", gameController::createRandom);
        app.post("/api/games/create/{color}", gameController::create);
        app.post("/api/games/join/{gameId}/{userId}", gameController::join);

        app.start("0.0.0.0", port);
    }

    public void stop() {
        app.stop();
    }
}