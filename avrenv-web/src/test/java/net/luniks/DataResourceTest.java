package net.luniks;

import static io.restassured.RestAssured.given;
import static org.hamcrest.CoreMatchers.is;

import org.junit.jupiter.api.Test;

import io.quarkus.test.junit.QuarkusTest;

@QuarkusTest
class DataResourceTest {
    @Test
    void testHelloEndpoint() {
        given()
          .when().get("/data")
          .then()
             .statusCode(200)
             .body(is("{}"));
    }

}