import api.Api;

public class Main {
    public static void main(String[] args) {
        String port = System.getenv("PORT");
        int portNumber = (port != null) ? Integer.parseInt(port) : 7000;

        Api api = new Api();
        api.start(portNumber);
    }
}
