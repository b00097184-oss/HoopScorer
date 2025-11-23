import java.util.*;

public class Game {
    private SerialPortHandle sph;

    public Game(SerialPortHandle sph) {
        this.sph = sph;
    }

    // -------------------- SINGLE PLAYER --------------------
    public void singlePlayer(Scanner sc) {
        System.out.println("=== SINGLE PLAYER ===");
        System.out.print("Enter your name: ");
        String playerName = sc.nextLine().trim();

        // Get the time limit choice
        System.out.print("Choose time limit (0 for 15 seconds, 1 for 30 seconds): ");
        int timeLimit = Integer.parseInt(sc.nextLine().trim());

        // Get number of rounds
        System.out.print("Enter number of rounds (1, 3, 5): ");
        int numRounds = Integer.parseInt(sc.nextLine().trim());
        if (numRounds != 1 && numRounds != 3 && numRounds != 5) {
            numRounds = 1;  // default to 1 round
        }

        // Send setup to Arduino
        String setupMessage = "SETUP:" + timeLimit + ":" + numRounds + ":SINGLE:" + playerName;
        sph.send(setupMessage);
        System.out.println("Sent to Arduino: " + setupMessage);

        System.out.println("Waiting for Arduino to send scores...");
        String line = sph.readLine();
        if (line != null && line.contains(",")) {
            String[] parts = line.split(",");
            int playerScore = processStatsLine(parts);

            System.out.println(playerName + "'s Total Score: " + playerScore);
            Leaderboard.updateLeaderboard(playerName, playerScore); // Update leaderboard for single player
        }
        System.out.println("Game Over!");
    }

    // -------------------- MULTIPLAYER --------------------
    public void multiplayer(Scanner sc) {
        System.out.println("=== MULTIPLAYER ===");
        System.out.print("Enter Player 1 name: ");
        String player1 = sc.nextLine().trim();
        System.out.print("Enter Player 2 name: ");
        String player2 = sc.nextLine().trim();

        // Get the time limit choice
        System.out.print("Choose time limit (0 for 15 seconds, 1 for 30 seconds): ");
        int timeLimit = Integer.parseInt(sc.nextLine().trim());

        // Get number of rounds
        System.out.print("Enter number of rounds (1, 3, 5): ");
        int numRounds = Integer.parseInt(sc.nextLine().trim());
        if (numRounds != 1 && numRounds != 3 && numRounds != 5) {
            numRounds = 1;  // default to 1 round
        }

        // Send setup message to Arduino
        String setupMessage = "SETUP:" + timeLimit + ":" + numRounds + ":MULTI:" + player1 + "," + player2;
        sph.send(setupMessage);
        System.out.println("Sent to Arduino: " + setupMessage);

        // Round tracking
        int player1Wins = 0, player2Wins = 0;
        Map<Integer, Integer> roundScores = new HashMap<>(); // temporary storage for round scores

        for (int round = 1; round <= numRounds; ) {
            System.out.println("\nWaiting for round " + round + " scores...");

            String line = sph.readLine();
            if (line == null || !line.contains(",")) continue;

            String[] parts = line.split(",");
            if (parts.length < 4) continue;

            int playerNum = Integer.parseInt(parts[0].trim()); // First index: Player number
            int[] hoops = new int[3];
            for (int i = 0; i < 3; i++) {
                hoops[i] = Integer.parseInt(parts[i + 1].trim()); // Remaining indices: Hoops
            }

            int score = hoops[0] * 100 + hoops[1] * 200 + hoops[2] * 400;
            System.out.println((playerNum == 1 ? player1 : player2) + "'s score for round " + round + ":");
            System.out.println("Hoop 1 (" + hoops[0] + ") -> " + hoops[0] * 100 + " pts");
            System.out.println("Hoop 2 (" + hoops[1] + ") -> " + hoops[1] * 200 + " pts");
            System.out.println("Hoop 3 (" + hoops[2] + ") -> " + hoops[2] * 400 + " pts");
            System.out.println("Round Total = " + score + " pts\n");

            roundScores.put(playerNum, score);

            // When both players have sent scores for the round
            if (roundScores.size() == 2) {
                int score1 = roundScores.getOrDefault(1, 0);
                int score2 = roundScores.getOrDefault(2, 0);

                if (score1 > score2) {
                    System.out.println(player1 + " wins round " + round + "!");
                    player1Wins++;
                } else if (score2 > score1) {
                    System.out.println(player2 + " wins round " + round + "!");
                    player2Wins++;
                } else {
                    System.out.println("Round " + round + " is a tie!");
                }

                roundScores.clear();
                round++;
            }
        }

        // Final winner
        System.out.println("\n=== GAME OVER ===");
        System.out.println(player1 + " won " + player1Wins + " rounds");
        System.out.println(player2 + " won " + player2Wins + " rounds");

        if (player1Wins > player2Wins) {
            System.out.println(player1 + " wins the game!");
        } else if (player2Wins > player1Wins) {
            System.out.println(player2 + " wins the game!");
        } else {
            System.out.println("The game is a tie!");
        }
    }

    // -------------------- PROCESS SCORES --------------------
    private int processStatsLine(String[] parts) {
        int totalScore = 0;
        try {
            int hoop1 = Integer.parseInt(parts[1].trim());
            int hoop2 = Integer.parseInt(parts[2].trim());
            int hoop3 = Integer.parseInt(parts[3].trim());

            int score1 = hoop1 * 100;
            int score2 = hoop2 * 200;
            int score3 = hoop3 * 400;

            totalScore = score1 + score2 + score3;

            System.out.println("---- SCORE UPDATE ----");
            System.out.println("Hoop 1 (" + hoop1 + ") -> " + score1 + " pts");
            System.out.println("Hoop 2 (" + hoop2 + ") -> " + score2 + " pts");
            System.out.println("Hoop 3 (" + hoop3 + ") -> " + score3 + " pts");
            System.out.println("TOTAL SCORE = " + totalScore + " pts");
            System.out.println("----------------------");

        } catch (Exception e) {
            System.out.println("Error processing STATS line.");
        }
        return totalScore;
    }
}
