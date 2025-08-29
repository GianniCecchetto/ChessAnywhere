package api.game;

import com.fasterxml.jackson.annotation.JsonProperty;

public class Variant {
    public String key;
    public String name;
    @JsonProperty("short")
    public String _short; // maps JSON "short" → Java _short
}
