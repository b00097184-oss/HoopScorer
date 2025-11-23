import java.io.*;
import java.util.*;

public class Leaderboard {
    private static final String LEADERBOARD_FILE = "leaderboard.csv";

    public static void showLeaderboard() {
        System.out.println("\n=== LEADERBOARD (Top 3) ===");
        List<String[]> leaderboard = readLeaderboard();
        leaderboard.sort((a, b) -> Integer.parseInt(b[1]) - Integer.parseInt(a[1]));

        int max = Math.min(3, leaderboard.size());
        for (int i = 0; i < max; i++) {
            String[] entry = leaderboard.get(i);
            System.out.println((i + 1) + ". " + entry[0] + " - " + entry[1]);
        }
        if (leaderboard.isEmpty()) System.out.println("No records yet.");
    }

    public static void updateLeaderboard(String playerName, int score) {
        List<String[]> leaderboard = readLeaderboard();
        leaderboard.add(new String[]{playerName, String.valueOf(score)});
        leaderboard.sort((a, b) -> Integer.parseInt(b[1]) - Integer.parseInt(a[1]));

        if (leaderboard.size() > 3) leaderboard = leaderboard.subList(0, 3);

        try (PrintWriter pw = new PrintWriter(new FileWriter(LEADERBOARD_FILE))) {
            for (String[] entry : leaderboard) {
                pw.println(entry[0] + "," + entry[1]);
            }
        } catch (IOException e) {
            System.out.println("Failed to update leaderboard.");
        }
    }

    private static List<String[]> readLeaderboard() {
        List<String[]> list = new ArrayList<>();
        try (BufferedReader br = new BufferedReader(new FileReader(LEADERBOARD_FILE))) {
            String line;
            while ((line = br.readLine()) != null) {
                list.add(line.split(","));
            }
        } catch (IOException e) {
            System.out.println("Error reading leaderboard.");
        }
        return list;
    }
}
