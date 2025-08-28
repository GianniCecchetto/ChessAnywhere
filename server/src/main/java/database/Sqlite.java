package database;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.sql.*;

public class Sqlite implements AutoCloseable {

    private Connection conn;
    private Statement stmt;

    public Sqlite()  {
        try {
            conn = DriverManager.getConnection("jdbc:sqlite:chess_anywhere.db");
            stmt = conn.createStatement();
        } catch (SQLException e) {
            System.err.println(e.getMessage());
        }
    }

    @Override
    public void close() throws SQLException {
        if (conn != null) conn.close();
        if (stmt != null) stmt.close();
    }

    private void executeSql(String sqlFilePath) throws IOException, SQLException {
        try (BufferedReader br = new BufferedReader(new FileReader(sqlFilePath))) {
            StringBuilder query = new StringBuilder();
            String line;
            boolean insideBlock = false;

            while ((line = br.readLine()) != null) {
                line = line.trim();
                if (line.startsWith("--") || line.isEmpty()) continue;

                if (line.startsWith("BEGIN")) {
                    insideBlock = true;
                }

                query.append(line).append(" ");

                if (insideBlock && line.endsWith("END;")) {
                    insideBlock = false;
                }

                if (!insideBlock && line.endsWith(";")) {
                    stmt.execute(query.toString());
                    query = new StringBuilder();
                }
            }
        }
    }

    public PreparedStatement prepare(String query, Object[] args) throws SQLException {
        return prepareWithKeys(query, args, new String[]{});
    }

    public PreparedStatement prepareWithKeys(String query, Object[] args, String[] keys) throws SQLException {
        PreparedStatement pstmt = conn.prepareStatement(query, keys);
        for (int index = 0; index < args.length; index++) {
            if (args[index].getClass() == Integer.class) {
                pstmt.setInt(index + 1, (int) args[index]);
            } else if (args[index].getClass() == String.class) {
                pstmt.setString(index + 1, (String) args[index]);
            }
        }

        return pstmt;
    }
}