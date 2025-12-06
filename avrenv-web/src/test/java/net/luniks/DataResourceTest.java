package net.luniks;

import static io.restassured.RestAssured.given;
import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.MatcherAssert.assertThat;

import java.io.StringReader;
import java.io.StringWriter;

import org.junit.jupiter.api.Test;

import io.quarkus.test.junit.QuarkusTest;
import io.quarkus.test.oidc.client.OidcTestClient;
import io.restassured.http.ContentType;
import jakarta.json.Json;
import jakarta.json.JsonObject;
import jakarta.json.JsonReader;

@QuarkusTest
class DataResourceTest {

    private static final String OIDC_USERNAME = "envtest";
    private static final String OIDC_PASSWORD = "password";
    private static final String OIDC_CLIENT = "quarkus-app";
    private static final String OIDC_SECRET = "secret";

    private final OidcTestClient testClient = new OidcTestClient();

    @Test
    void testPostData_unsupportedMediaType() {
        given()
            .when()
                .contentType(ContentType.TEXT)
                .body("text")
                .post("/data")
            .then()
                .statusCode(415)
                .body(is(""));
    }

    @Test
    void testPostData_unauthorized() {
        given()
            .when()
                .contentType(ContentType.JSON)
                .body("{}")
                .post("/data")
            .then()
                .statusCode(401)
                .body(is(""));
    }

    @Test
    void testPostData_empty() {
        given()
            .when()
                .auth()
                .oauth2(getToken())
                .contentType(ContentType.JSON)
                .body("{}")
                .post("/data")
            .then()
                .statusCode(200)
                .body(is(""));

        final String response = given()
            .when()
                .get("/data")
            .then()
                .statusCode(200)
            .extract()
                .asString();

        final JsonObject data = toJsonObject(response);

        assertThat(true, is(data.getBoolean("online")));
        assertThat("00:00:00", is(data.getString("time")));
        assertThat(0, is(data.getInt("rssi")));
        assertThat(0, is(data.getInt("crc")));
        assertThat(0, is(data.getInt("voltage")));
        assertThat(0, is(data.getInt("power")));
        assertThat("0.0", is(data.getString("temperature")));
        assertThat(0, is(data.getInt("humidity")));
        assertThat(0, is(data.getInt("pressure")));
        assertThat(0, is(data.getInt("gasres")));
        assertThat(0, is(data.getInt("fix")));
        assertThat(0, is(data.getInt("sat")));
        assertThat("0.0", is(data.getString("lat")));
        assertThat("0.0", is(data.getString("lon")));
        assertThat(0, is(data.getInt("alt")));
        assertThat("0.0", is(data.getString("speed")));
    }

    @Test
    void testPostData_ok() {
        given()
            .when()
                .auth()
                .oauth2(getToken())
                .contentType(ContentType.JSON)
                .body(getTestData())
                .post("/data")
            .then()
                .statusCode(200)
                .body(is(""));

        final String response = given()
            .when()
                .get("/data")
            .then()
                .statusCode(200)
            .extract()
                .asString();

        final JsonObject data = toJsonObject(response);

        assertThat(true, is(data.getBoolean("online")));
        assertThat("00:50:10", is(data.getString("time")));
        assertThat(4, is(data.getInt("dur")));
        assertThat(-43, is(data.getInt("rssi")));
        assertThat(1, is(data.getInt("crc")));
        assertThat(3257, is(data.getInt("voltage")));
        assertThat(2, is(data.getInt("power")));
        assertThat("19.6", is(data.getString("temperature")));
        assertThat(44, is(data.getInt("humidity")));
        assertThat(1003, is(data.getInt("pressure")));
        assertThat(143, is(data.getInt("gasres")));
        assertThat(1, is(data.getInt("fix")));
        assertThat(8, is(data.getInt("sat")));
        assertThat("50.70761", is(data.getString("lat")));
        assertThat("3.45205", is(data.getString("lon")));
        assertThat(22, is(data.getInt("alt")));
        assertThat("0.0", is(data.getString("speed")));
    }

    @Test
    void testPostData_invalidJson() {
        given()
            .when()
                .auth()
                .oauth2(getToken())
                .contentType(ContentType.JSON)
                .body("{")
                .post("/data")
            .then()
                .statusCode(500);
                // .body(is("")); TODO should not expose so many details to caller
    }

    @Test
    void testPostData_invalidValue() {
        given()
            .when()
                .auth()
                .oauth2(getToken())
                .contentType(ContentType.JSON)
                .body("{\"online\":notaboolean}")
                .post("/data")
            .then()
                .statusCode(500);
                // .body(is("")); TODO should not expose so many details to caller
    }

    @Test
    void testPostData_unexpectedValue() {
        given()
            .when()
                .auth()
                .oauth2(getToken())
                .contentType(ContentType.JSON)
                .body("{\"time\":\"notanint\"}")
                .post("/data")
            .then()
                .statusCode(200)
                .body(is(""));

        final String response = given()
            .when()
                .get("/data")
            .then()
                .statusCode(200)
            .extract()
                .asString();

        final JsonObject data = toJsonObject(response);

        assertThat(true, is(data.getBoolean("online")));
        assertThat("00:00:00", is(data.getString("time")));
        assertThat(0, is(data.getInt("rssi")));
        assertThat(0, is(data.getInt("crc")));
        assertThat(0, is(data.getInt("voltage")));
        assertThat(0, is(data.getInt("power")));
        assertThat("0.0", is(data.getString("temperature")));
        assertThat(0, is(data.getInt("humidity")));
        assertThat(0, is(data.getInt("pressure")));
        assertThat(0, is(data.getInt("gasres")));
        assertThat(0, is(data.getInt("fix")));
        assertThat(0, is(data.getInt("sat")));
        assertThat("0.0", is(data.getString("lat")));
        assertThat("0.0", is(data.getString("lon")));
        assertThat(0, is(data.getInt("alt")));
        assertThat("0.0", is(data.getString("speed")));
    }

    private String getTestData() {
        final JsonObject data = Json.createObjectBuilder()
                .add("time", 3010)
                .add("dur", 4)
                .add("rssi", 43)
                .add("crc", 1)
                .add("voltage", 3257)
                .add("power", 2)
                .add("temperature", 1965)
                .add("humidity", 44)
                .add("pressure", 1003)
                .add("gasres", 143)
                .add("fix", 1)
                .add("sat", 8)
                .add("lat", 50424567)
                .add("lon", 3271234)
                .add("alt", 22)
                .add("speed", 3)
                .build();

        final StringWriter writer = new StringWriter();
        Json.createWriter(writer).writeObject(data);

        return writer.toString();
    }

    private String getToken() {
        return testClient.getAccessToken(OIDC_USERNAME, OIDC_PASSWORD, OIDC_CLIENT, OIDC_SECRET);
    }

    private JsonObject toJsonObject(final String response) {
        try (final JsonReader reader = Json.createReader(new StringReader(response))) {
            return reader.readObject();
        }
    }

}
