package net.luniks;

import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.time.LocalTime;
import java.time.format.DateTimeFormatter;
import java.time.format.DateTimeFormatterBuilder;
import java.util.Locale;
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
        current.set(convert(json));
        System.out.println(json.toString());

        return Response.ok().build();
    }
    
    private JsonObject convert(final JsonObject raw) {
        final DecimalFormat df = new DecimalFormat("0.0");
        df.setDecimalFormatSymbols(new DecimalFormatSymbols(Locale.ROOT));
        final DateTimeFormatter dtf = new DateTimeFormatterBuilder().appendPattern("hh:mm:ss").toFormatter();
        
        final String time = dtf.format(LocalTime.ofSecondOfDay(raw.getInt("time", 0)));
        final int dur = raw.getInt("dur", 0);
        final int rssi = -raw.getInt("rssi", 0);
        final String crc = raw.getInt("crc", 0) == 1 ? "OK" : "NOK";
        final int voltage = raw.getInt("voltage", 0);
        final int power = raw.getInt("power", 0);
        final String temperature = df.format(raw.getInt("temperature", 0) / 100f);
        final int humidity = raw.getInt("humidity", 0);
        final int pressure = raw.getInt("pressure", 0);
        final int gasres = raw.getInt("gasres", 0);
        final String fix = raw.getInt("fix", 0) > 0 ? "Yes" : "No";
        final int sat = raw.getInt("sat", 0);
        final String lat = toDecimalDegrees(raw.getInt("lat", 0));
        final String lon = toDecimalDegrees(raw.getInt("lon", 0));
        final int alt = raw.getInt("alt", 0);
        final String speed = df.format(raw.getInt("speed", 0) / 100f);
        
        JsonObject converted = Json.createObjectBuilder()
                .add("time", time)
                .add("dur", dur)
                .add("rssi", rssi)
                .add("crc", crc)
                .add("voltage", voltage)
                .add("power", power)
                .add("temperature", temperature)
                .add("humidity", humidity)
                .add("pressure", pressure)
                .add("gasres", gasres)
                .add("fix", fix)
                .add("sat", sat)
                .add("lat", lat)
                .add("lon", lon)
                .add("alt", alt)
                .add("speed", speed)
                .build();

        return converted;
    }
    
    final String toDecimalDegrees(final int degreesMinutes) {
        final int degrees = degreesMinutes / 1000000;
        final int minutes = degreesMinutes % 1000000;
        final int decimal = minutes / 6;
        
        return String.format("%d.%d", degrees, decimal);
    }
}
