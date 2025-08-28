package api.challenge;

import api.user.User;

public class Challenge {
    private String id;
    private String url;
    private String status;
    private String challengerId;
    private String destUserId;
    private String variant;
    private boolean rated;
    private String speed;
    private String urlWhite;
    private String urlBlack;

    public Challenge(String challengerId) {
        this.challengerId = challengerId;
    }

    // Getters
    public String getId() { return id; }
    public String getUrl() { return url; }
    public String getStatus() { return status; }
    public String getChallengerId() { return challengerId; }
    public String getDestUserId() { return destUserId; }
    public String getVariant() { return variant; }
    public boolean isRated() { return rated; }
    public String getSpeed() { return speed; }
    public String getUrlWhite() { return urlWhite; }
    public String getUrlBlack() { return urlBlack; }

    // Setters
    public void setId(String id) { this.id = id; }
    public void setUrl(String url) { this.url = url; }
    public void setStatus(String status) { this.status = status; }
    public void setDestUserId(String destUserId) { this.destUserId = destUserId; }
    public void setVariant(String variant) { this.variant = variant; }
    public void setRated(boolean rated) { this.rated = rated; }
    public void setSpeed(String speed) { this.speed = speed; }
    public void setUrlWhite(String urlWhite) { this.urlWhite = urlWhite; }
    public void setUrlBlack(String urlBlack) { this.urlBlack = urlBlack; }
}
