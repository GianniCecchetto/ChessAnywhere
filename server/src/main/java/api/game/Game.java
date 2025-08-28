package api.game;

import com.fasterxml.jackson.databind.ObjectMapper;

import java.io.IOException;
import java.net.http.HttpResponse;

// Root object
public class Game {
    public String id;
    public String url;
    public String status;
    public String challenger; // null in this case
    public String destUser;   // null in this case
    public Variant variant;
    public Boolean rated;
    public String speed;
    public TimeControl timeControl;
    public String color;
    public String finalColor;
    public Perf perf;
    public Open open;
    public String urlWhite;
    public String urlBlack;

    public Game() {}

    public Game(String challenger) {
        this.challenger = challenger;
    }
}
