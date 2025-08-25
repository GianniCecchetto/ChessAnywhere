import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

class HelloServiceTest {
    @Test
    void shouldReturnGreeting() {
        assertEquals("Hello, World!", "Hello, World!");
    }
}
