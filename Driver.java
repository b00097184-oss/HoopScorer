import java.util.Scanner;

public class Driver {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        
        System.out.print("Enter COM port: ");
        String port = sc.nextLine().trim();
        SerialPortHandle sph = new SerialPortHandle(port);
        System.out.println("Opened port: " + port);

        Game game = new Game(sph);

        while (true) {
            System.out.println("\n==== MAIN MENU ====");
            System.out.println("1. Single Player");
            System.out.println("2. Multiplayer");
            System.out.println("3. Leaderboard");
            System.out.println("4. Exit");
            System.out.print("Choose an option: ");
            String choice = sc.nextLine().trim();

            switch (choice) {
                case "1":
                    game.singlePlayer(sc);
                    break;
                case "2":
                    game.multiplayer(sc);
                    break;
                case "3":
                    Leaderboard.showLeaderboard();
                    break;
                case "4":
                    System.out.println("Exiting...");
                    return;
                default:
                    System.out.println("Invalid choice, try again.");
            }
        }
    }
}
