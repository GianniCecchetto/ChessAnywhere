import api.Api;
import api.challenge.Challenge;
import io.restassured.RestAssured;
import org.apache.http.util.Asserts;
import org.junit.jupiter.api.*;

import java.util.List;

import static io.restassured.RestAssured.*;
import static org.hamcrest.Matchers.*;
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
                .queryParam("userId", "1")
                .when()
                .post("/api/challenge")
                .then()
                .statusCode(201);
    }

    @Test
    void testCreateTwoChallenge() {
        given()
                .queryParam("userId", "1")
                .when()
                .post("/api/challenge")
                .then()
                .statusCode(201);
        given()
                .queryParam("userId", "1")
                .when()
                .post("/api/challenge")
                .then()
                .statusCode(409);
    }

    @Test
    void testListChallenges() {
        given()
                .queryParam("userId", "1")
                .when()
                .post("/api/challenge")
                .then()
                .statusCode(201);
        given()
                .queryParam("userId", "2")
                .when()
                .post("/api/challenge")
                .then()
                .statusCode(201);

        List challenges =
                given()
                        .queryParam("userId", "123")
                        .when()
                        .post("/api/challenges")
                        .then()
                        .extract().body().as(List.class);

        assertEquals(challenges.size(), 2);
    }

    @Test
    void testDeleteChallenge() {
        given()
            .queryParam("userId", "123")
                .when()
                .delete("/api/challenge/")
                .then()
                .statusCode(204);
    }
}
