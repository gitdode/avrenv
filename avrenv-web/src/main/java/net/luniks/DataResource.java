package net.luniks;

import java.util.concurrent.atomic.AtomicReference;

import jakarta.annotation.security.RolesAllowed;
import jakarta.json.Json;
import jakarta.json.JsonObject;
import jakarta.ws.rs.Consumes;
import jakarta.ws.rs.GET;
import jakarta.ws.rs.POST;
import jakarta.ws.rs.Path;
import jakarta.ws.rs.Produces;
import jakarta.ws.rs.core.MediaType;
import jakarta.ws.rs.core.Response;

@Path("/data")
public class DataResource {
    
    private static final JsonObject EMPTY_JSON = Json.createObjectBuilder().build();
    
    private final AtomicReference<JsonObject> current = new AtomicReference<>(EMPTY_JSON);

    @GET
    @Produces(MediaType.APPLICATION_JSON)
    public JsonObject get() {
        return current.get();
    }

    @POST
    @RolesAllowed("baloon")
    @Consumes(MediaType.APPLICATION_JSON)
    @Produces(MediaType.TEXT_PLAIN)
    public Response post(final JsonObject json) {
        current.set(json);
        System.out.println(json.toString());

        return Response.ok().build();
    }
}
