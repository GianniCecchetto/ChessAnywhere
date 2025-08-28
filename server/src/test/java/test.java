import api.Api;
import io.restassured.RestAssured;
import org.junit.jupiter.api.*;

import java.util.List;

import static io.restassured.RestAssured.*;
import static org.junit.jupiter.api.Assertions.*;

class ChallengeApiTest {

    private static Api api;

    @BeforeAll
    static void setup() {
        api = new Api();
        api.start(7000);

        RestAssured.baseURI = "http://localhost";
        RestAssured.port = 7000;
    }

    @AfterAll
    static void tearDown() {
        api.stop();
    }

    @Test
    void testCreateChallenge() {
        given()
                .post("/api/game/1")
                .then()
                .statusCode(201);
    }

    @Test
    void testCreateWithSameUserIdChallenge() {
        given()
                .post("/api/game/1")
                .then()
                .statusCode(409);
    }

    @Test
    void testCreateChallengeWithAnotherUserId() {
        given()
                .post("/api/game/2")
                .then()
                .statusCode(201);
    }

    @Test
    void testListChallenges() {
        List challenges =
                given()
                        .get("/api/challenges")
                        .then()
                        .extract().body().as(List.class);

        assertEquals(challenges.size(), 2);
    }

    @Test
    void testDeleteChallenge() {
        given()
                .delete("/api/game/1")
                .then()
                .statusCode(204);
    }
}
