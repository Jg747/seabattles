package org.seabattles.gui.ascii;

import java.io.File;
import java.io.FileNotFoundException;
import java.time.Duration;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Optional;
import java.util.Scanner;

import org.seabattles.interfaces.GUI;
import org.seabattles.src.Board;
import org.seabattles.src.Game;
import org.seabattles.src.GameConfig;
import org.seabattles.src.GameConfig.GameDifficulty;
import org.seabattles.src.Main;
import org.seabattles.src.Player;
import org.seabattles.src.Player.PlayerStatus;
import org.seabattles.src.Ship;
import org.seabattles.src.Stats;

public class AsciiGUI implements GUI {
	
	private static final String title = Main.name + " - v" + Main.version;
	
	private Scanner scan;
	private Game game;
	
	public AsciiGUI(String[] args, Game g) {
		game = g;
		scan = new Scanner(System.in);
	}
	
	private void placeShip(Board b, int choice) {
		Ship[] ships = b.getShips();
		for (Ship s : ships) {
			if (s.getID() == choice) {
				boolean done;
				boolean move = false;
				if (s.isPlaced()) {
					b.unplaceShip(s);
					move = true;
				}
				do {
					done = false;
					try {
						System.out.print("What X do you want to place the ship in? ");
						int x = Integer.parseInt(scan.nextLine());
						System.out.print("What Y do you want to place the ship in? ");
						int y = Integer.parseInt(scan.nextLine());
						System.out.print("What rotation do you want the ship to have (0 = right, 1 = down, 2 = left, 3 = up)? ");
						String c = scan.nextLine();
						int r;
						if (c.equals("r") || c.equals("right")) {
							r = 0;
						} else if (c.equals("d") || c.equals("down")) {
							r = 1;
						} else if (c.equals("l") || c.equals("left")) {
							r = 2;
						} else if (c.equals("u") || c.equals("up")) {
							r = 3;
						} else {
							r = Integer.parseInt(c);
						}
						
						s.setX(x);
						s.setY(y);
						s.rotate(r);
						
						if (!b.placeShip(s)) {
							System.err.print("Incorrect ship placement: ");
							throw new IllegalArgumentException();
						}
						
						done = true;
					} catch (IllegalArgumentException e) {
						System.err.println("Invalid values provided");
						sleep(200);
					}
				} while (!done);
			}
		}
	}
	
	private boolean yesNoQuestion(String msg) {
		do {
			System.out.print(msg);
			String rsp = scan.nextLine();
			if (rsp.equals("1") || rsp.equalsIgnoreCase("y") || rsp.equalsIgnoreCase("s") || rsp.equalsIgnoreCase("si") || rsp.equalsIgnoreCase("yes")) {
				return true;
			} else if (rsp.equals("0") || rsp.equalsIgnoreCase("n") || rsp.equalsIgnoreCase("no")) {
				return false;
			}
		} while (true);
	}
	
	private static void showBoard(byte[][] b) {
		for (int i = 0; i < b.length; i++) {
			System.out.println(Arrays.toString(b[i]));
		}
	}
	
	private static void showBoard(String msg, byte[][] b) {
		System.out.println(msg);
		showBoard(b);
	}
	
	private static void showBoard(String msg, Board b, boolean own) {
		System.out.println(msg);
		showBoard(b, own);
	}
	
	private static void showBoard(Board b, boolean own) {
		byte[][] hits = !own && !Game.DEBUG_MODE ? b.getObfuscatedHits() : b.getHits();
		
		System.out.print("\t");
		for (int i = 0; i < hits.length; i++) {
			System.out.print("[" + i + "]\t");
		}
		System.out.println();
		
		for (int i = 0; i < hits.length; i++) {
			System.out.print("[" + i + "]\t");
			for (int j = 0; j < hits[i].length; j++) {
				/*if (hits[i][j] != Board.NOTHING_FLAG) {
					System.err.print(" " + hits[i][j] + "\t");
					System.err.flush();
				} else {*/
					System.out.print(" " + hits[i][j] + "\t");
					/*System.out.flush();
				}*/
			}
			System.out.println();
		}
	}

	@Override
	public Board placeShips() {
		Board b = new Board(game.getGameConfig().getShipsConfig());
		boolean done = false;
		Ship[] ships = b.getShips();
		
		ships[0].setX(5);
		ships[0].setY(5);
		ships[0].rotate(0);
		/*ships[1].setX(1);
		ships[1].setY(4);
		ships[1].rotate(0);
		ships[2].setX(2);
		ships[2].setX(7);
		ships[2].rotate(2);
		ships[3].setX(1);
		ships[3].setY(6);
		ships[3].rotate(3);
		ships[4].setX(2);
		ships[4].setY(1);
		ships[4].rotate(3);*/
		for (Ship s : ships) {
			b.placeShip(s);
		}
		return b;
		
		/*do {
			printTitle();
			showBoard(b);
			if (done) {
				if (yesNoQuestion("All ships placed, do you want to proceed? ")) {
					break;
				}
			}
			
			System.out.print("Ships available to place: ");
			for (int i = 0; i < ships.length; i++) {
				if (!ships[i].isPlaced()) {
					System.out.print("[id: " + ships[i].getID() + " l: " + ships[i].getLength() + " w: " + ships[i].getWidth() + "] ");
				}
			}
			System.out.print("\nShips already placed: ");
			for (int i = 0; i < ships.length; i++) {
				if (ships[i].isPlaced()) {
					System.out.print("[id: " + ships[i].getID() + " l: " + ships[i].getLength() + " w: " + ships[i].getWidth() + "] ");
				}
			}
			System.out.print("\nWhat ship do you want to place/move (-1 to skip)? ");
			int choice;
			try {
				choice = Integer.parseInt(scan.nextLine());
				if (choice != -1) {
					placeShip(b, choice);
					System.out.println("Ship status changed!");
					sleep(200);
				}
			} catch (NumberFormatException e) {}
			
			done = true;
			for (Ship s : ships) {
				if (!s.isPlaced()) {
					done = false;
					break;
				}
			}
		} while (true);
		return b.getHits();*/
	}
	
	private void sleep(long millis) {
		try {
			Thread.sleep(millis);
		} catch (InterruptedException e) {}
	}

	@Override
	public int[] attack(Optional<Player> who) {
		int[] ret = new int[3];
		
		if (who.isEmpty()) {
			ret[0] = -1;
			return ret;
		}
		
		Player p = who.get();
		showField(p, false);
		
		ret[0] = p.getID();
		
		boolean done = false;
		int x = numberChoice("x (-1 to go back): ", -1, Board.getWidth());
		if (x < 0) {
			ret[0] = -1;
			return ret;
		}
		
		int y = numberChoice("y (-1 to go back): ", -1, Board.getHeigth());
		if (y < 0) {
			ret[0] = -1;
			return ret;
		}
		
		ret[1] = x;
		ret[2] = y;
		
		return ret;
	}

	@Override
	public void showField(Player who, boolean waitInput) {
		printTitle();
		showBoard("\t\t\t\t" + who.getUsername() + "\'s field", who.getBoard(), false);
		if (waitInput) {
			System.out.print("Type anything to go back");
			scan.nextLine();
		}
	}

	@Override
	public void endScreen(int myID, PlayerStatus myStatus, Duration duration, Short[] ids, String[] names, Stats[] stats) {
		System.err.println("GAY FINITO");
	}

	@Override
	public int getMode() {
		printTitle();
		
		String choice;
		int value;
		
		do {
			value = -1;
			System.out.print("Selezionare la modalità di gioco (0 = offline, 1 = crea un match, 2 = partecipa ad un match): ");
			choice = scan.nextLine();
			try {
				value = Integer.parseInt(choice);
			} catch (NumberFormatException e) {}
		} while (value < 0 || value > 2);
		
		return value;
	}
	
	@Override
	public GameConfig configGame() throws Exception {
		printTitle();
		
		GameConfig ret = new GameConfig();
		boolean ok;
		boolean file = false;
		String rsp;
		
		do {
			ok = false;
			System.out.print("Hai un file personalizzato di configurazione? ");
			rsp = scan.nextLine();
			if (rsp.equalsIgnoreCase("s") || rsp.equalsIgnoreCase("y") || rsp.equalsIgnoreCase("n") || rsp.equals("1") || rsp.equals("0")) {
				ok = true;
				if (!rsp.equalsIgnoreCase("n") && !rsp.equalsIgnoreCase("0")) {
					file = true;
				}
			}
		} while (!ok);
		
		if (file) {
			do {
				ok = false;
				System.out.print("Inserire il percorso del file di configurazione (lasciare vuoto per non inserire alcun file): ");
				rsp = scan.nextLine();
				if (rsp.isEmpty()) {
					break;
				}
				
				try {
					ret.setConfigFile(new File(rsp));
				} catch (FileNotFoundException e) {
					System.err.println("File non trovato, riprovare");
				}
			} while (!ok);
		}
		
		ret.applyConfigFile();
		
		do {
			ok = false;
			System.out.println("1) Normale\n2) Difficile\n3) Impossibile\n\nA quale difficoltà vuoi giocare?");
			rsp = scan.nextLine();
			try {
				int value = Integer.parseInt(rsp);
				switch (value) {
					case 1:
						ret.setBotsDifficulty(GameDifficulty.NORMAL);
						break;
					case 2:
						ret.setBotsDifficulty(GameDifficulty.HARD);
						break;
					case 3:
						ret.setBotsDifficulty(GameDifficulty.IMPOSSIBLE);
						break;
					default:
						throw new NumberFormatException();
				}
				
				ok = true;
			} catch (NumberFormatException e) {
				System.err.println("Devi inserire un numero tra 1 e 3");
			}
		} while (!ok);
		
		do {
			ok = false;
			System.out.println("Con quanti bot vuoi giocare? ");
			rsp = scan.nextLine();
			try {
				int value = Integer.parseInt(rsp);
				if (value < 1 || value >= Game.MAX_PLAYERS - 1) {
					throw new NumberFormatException();
				}
				ret.setNumberOfBots(value);
				ok = true;
			} catch (NumberFormatException e) {
				System.err.println("Devi inserire un numero tra 1 e " + (Game.MAX_PLAYERS - 1));
			}
		} while (!ok);
		
		return ret;
	}
	
	private void printTitle() {
		for (int i = 0; i < 10; i++) {
			System.out.println();
		}
		System.out.println(title);
	}

	@Override
	public void showPlayerField() {
		printTitle();
		showBoard("\t\t\t\t\tYour board", game.getOwnPlayer().getBoard(), true);
	}
	
	private int numberChoice(String msg, int min, int max) {
		int ret;
		do {
			System.out.print(msg);
			try {
				ret = Integer.parseInt(scan.nextLine());
				if (ret >= min && ret <= max) {
					return ret;
				}
			} catch (NumberFormatException e) {}
		} while (true);
	}

	@Override
	public GuiAction getAction() {
		showPlayerField();
		
		System.out.println("1. Show field\n2. Attack\n3. Leave");
		int choice = numberChoice("What do you want to do? ", 1, 3);
		
		switch (choice) {
			case 1:
				return GuiAction.SHOW_FIELD;
			case 2:
				return GuiAction.ATTACK;
			case 3:
				if (yesNoQuestion("Are you sure you want to leave? ")) {
					return GuiAction.QUIT;
				}
			default:
				return GuiAction.NONE;
		}
	}
	
	private int getAvailablePlayerCount(Player[] players) {
		int count = 0;
		for (Player p : players) {
			if (p.getStatus() == PlayerStatus.READY || p.getStatus() == PlayerStatus.HAS_TURN) {
				count++;
			}
		}
		return count;
	}

	private HashMap<Short, Short> printPlayerList(Player me, Player[] players, Short[] noPrint) {
		short index = 1;
		HashMap<Short, Short> ret = new HashMap<>();
		for (Player p : players) {
			if (p.getID() != me.getID()) {
				if (noPrint != null && !Arrays.asList(noPrint).contains(p.getID())) {
					System.out.println(index + ". " + p.getUsername());
					ret.put(index, p.getID());
					index++;
				} else if (noPrint == null) {
					System.out.println(index + ". " + p.getUsername());
					ret.put(index, p.getID());
					index++;
				}
			}
		}
		return ret;
	}
	
	@Override
	public Optional<Player> getWhoPlayer(String msg, Player me, Player[] players, Short[] noPrint) {
		showPlayerField();
		if (getAvailablePlayerCount(players) > 2) {
			boolean hideDead = true;
			do {
				HashMap<Short, Short> ids = printPlayerList(me, players, noPrint);
				int choice = numberChoice(msg, -1, ids.size());
				if (choice == -1) {
					return Optional.empty();
				}
				if (choice >= 1 && ids.get((short) choice) != me.getID()) {
					return Optional.of(players[ids.get((short) choice)]);
				}
			} while (true);
		} else {
			for (Player p : players) {
				if (p.getID() != me.getID() && (p.getStatus() == PlayerStatus.READY || p.getStatus() == PlayerStatus.HAS_TURN)) {
					return Optional.of(p);
				}
			}
		}
		
		return Optional.empty();
	}

	@Override
	public void invalidAttack(String msg) {
		System.err.println(msg);
	}
}
