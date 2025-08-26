package api.challenge;

import api.user.User;

public class Challenge {
    private String id;
    private String url;
    private String status;
    private User challenger;
    private User destUser;
    private String variant;
    private boolean rated;
    private String speed;
    private String urlWhite;
    private String urlBlack;

    public Challenge() {

    }

    public String Url() {
        return url;
    }

    public void Url(String url) {
        this.url = url;
    }
}
