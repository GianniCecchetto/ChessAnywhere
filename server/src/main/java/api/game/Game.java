package api.game;

import com.fasterxml.jackson.annotation.JsonIgnore;

// Root object
public class Game {
    public String id;
    public String url;
    public String status;
    public Player challenger; // null in this case
    public Player destUser;   // null in this case
    public Variant variant;
    public Boolean rated;
    public String speed;
    public TimeControl timeControl;
    public String color;
    public String finalColor;
    public Perf perf;
    @JsonIgnore
    public Open open;
    public String urlWhite;
    public String urlBlack;
    public String direction;

    public Game() {}
}
