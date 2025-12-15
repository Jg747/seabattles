package org.seabattles.gui.ascii;

import java.io.File;
import java.io.FileNotFoundException;
import java.time.Duration;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Optional;
import java.util.Scanner;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.Semaphore;

import org.seabattles.interfaces.GUI;
import org.seabattles.src.Board;
import org.seabattles.src.Game;
import org.seabattles.src.Game.GameStatus;
import org.seabattles.src.GameConfig;
import org.seabattles.src.Logger;
import org.seabattles.src.GameConfig.GameDifficulty;
import org.seabattles.src.Main;
import org.seabattles.src.Player;
import org.seabattles.src.Player.PlayerStatus;
import org.seabattles.src.Ship;
import org.seabattles.src.Stats;

public class AsciiGUI implements GUI {
	
	private static final String title = Main.name + " - v" + Main.version;
	
	private Semaphore s;
	private Scanner scan;
	private Game game;
	
	public AsciiGUI(String[] args, Game g) {
		game = g;
		scan = new Scanner(System.in);
		s = new Semaphore(1);
	}
	
	@SuppressWarnings("unused")
	private void placeShip(Board b, int choice) {
		Ship[] ships = b.getShips();
		for (Ship s : ships) {
			if (s.getID() == choice) {
				boolean done;
				if (s.isPlaced()) {
					b.unplaceShip(s);
				}
				do {
					done = false;
					try {
						print("What X do you want to place the ship in? ");
						int x = Integer.parseInt(scan.nextLine());
						print("What Y do you want to place the ship in? ");
						int y = Integer.parseInt(scan.nextLine());
						print("What rotation do you want the ship to have (0 = right, 1 = down, 2 = left, 3 = up)? ");
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
	
	private boolean boolQuestion(String msg) {
		do {
			print(msg);
			String rsp = scan.nextLine();
			if (rsp.equals("1") || rsp.equalsIgnoreCase("y") || rsp.equalsIgnoreCase("s") || rsp.equalsIgnoreCase("si") || rsp.equalsIgnoreCase("yes")) {
				return true;
			} else if (rsp.equals("0") || rsp.equalsIgnoreCase("n") || rsp.equalsIgnoreCase("no")) {
				return false;
			}
		} while (true);
	}
	
	private void showBoard(byte[][] b) {
		for (int i = 0; i < b.length; i++) {
			println(Arrays.toString(b[i]));
		}
	}
	
	@SuppressWarnings("unused")
	private void showBoard(String msg, byte[][] b) {
		println(msg);
		showBoard(b);
	}
	
	private void showBoard(String msg, Board b, boolean own) {
		println(msg);
		showBoard(b, own);
	}
	
	private void showBoard(Board b, boolean own) {
		byte[][] hits = !own && !Game.DEBUG_MODE ? b.getObfuscatedHits() : b.getHits();
		
		print("\t");
		for (int i = 0; i < hits.length; i++) {
			print("[" + i + "]\t");
		}
		println();
		
		for (int i = 0; i < hits.length; i++) {
			print("[" + i + "]\t");
			for (int j = 0; j < hits[i].length; j++) {
				/*if (hits[i][j] != Board.NOTHING_FLAG) {
					System.err.print(" " + hits[i][j] + "\t");
					System.err.flush();
				} else {*/
					print(" " + hits[i][j] + "\t");
					/*flush();
				}*/
			}
			println();
		}
	}

	@Override
	public Board placeShips() {
		Board b = new Board(game.getGameConfig().getShipsConfig());
		boolean done = false;
		Ship[] ships = b.getShips();
		
		/******* TODO TESTING *******/
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
		/******* TODO TESTING *******/
		
		/*do {
			printTitle();
			showBoard(b);
			if (done) {
				if (yesNoQuestion("All ships placed, do you want to proceed? ")) {
					break;
				}
			}
			
			print("Ships available to place: ");
			for (int i = 0; i < ships.length; i++) {
				if (!ships[i].isPlaced()) {
					print("[id: " + ships[i].getID() + " l: " + ships[i].getLength() + " w: " + ships[i].getWidth() + "] ");
				}
			}
			print("\nShips already placed: ");
			for (int i = 0; i < ships.length; i++) {
				if (ships[i].isPlaced()) {
					print("[id: " + ships[i].getID() + " l: " + ships[i].getLength() + " w: " + ships[i].getWidth() + "] ");
				}
			}
			print("\nWhat ship do you want to place/move (-1 to skip)? ");
			int choice;
			try {
				choice = Integer.parseInt(scan.nextLine());
				if (choice != -1) {
					placeShip(b, choice);
					println("Ship status changed!");
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
	public Object[] attack(Optional<Player> who) {
		Object[] ret = new Object[3];
		
		if (who.isEmpty()) {
			ret[0] = null;
			return ret;
		}
		
		Player p = who.get();
		showField(p, false);
		
		ret[0] = p.getID();
		
		int x = numberChoice("x (-1 to go back): ", -1, Board.getWidth());
		if (x < 0) {
			ret[0] = null;
			return ret;
		}
		
		int y = numberChoice("y (-1 to go back): ", -1, Board.getHeigth());
		if (y < 0) {
			ret[0] = null;
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
			print("Type anything to go back");
			scan.nextLine();
		}
	}

	@Override
	public void endScreen(UUID myID, PlayerStatus myStatus, Duration duration, Map<UUID, Object[]> playerStats) {
		printTitle();
		switch (myStatus) {
			case LOSER:
				println("YOU LOST!");
				break;
			case WINNER:
				println("YOU WIN!");
				break;
			default:
				break;
		}
		println("Finish time: " + duration.toMinutes() + "m " + duration.toSeconds() + "s");
		
		// Object[0] = String userName, Object[1] = Stats playerStats
		
		Object[] mystats = playerStats.get(myID);
		println(((Stats) mystats[1]).toString());
	}

	@Override
	public int getMode() {
		printTitle();
		
		String choice;
		int value;
		
		do {
			value = -1;
			print("Selezionare la modalità di gioco (0 = offline, 1 = crea un match, 2 = partecipa ad un match): ");
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
			print("Hai un file personalizzato di configurazione? ");
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
				print("Inserire il percorso del file di configurazione (lasciare vuoto per non inserire alcun file): ");
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
			println("1) Normale\n2) Difficile\n3) Impossibile\n\nA quale difficoltà vuoi giocare?");
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
			println("Con quanti bot vuoi giocare? ");
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
			println();
		}
		println(title);
	}

	@Override
	public void showPlayerField() {
		printTitle();
		showBoard("\t\t\t\t\tYour board", game.getOwnPlayer().getBoard(), true);
	}
	
	private int numberChoice(String msg, int min, int max) {
		int ret;
		do {
			print(msg);
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
		
		println("1. Show field\n2. Attack\n3. Leave");
		int choice = numberChoice("What do you want to do? ", 1, 3);
		
		switch (choice) {
			case 1:
				return GuiAction.SHOW_FIELD;
			case 2:
				return GuiAction.ATTACK;
			case 3:
				if (boolQuestion("Are you sure you want to leave? ")) {
					return GuiAction.QUIT;
				}
			default:
				return GuiAction.NONE;
		}
	}
	
	private Map<Integer, UUID> getIDMapping(Set<UUID> uuids) {
		HashMap<Integer, UUID> ret = new HashMap<>();
		int index = 1;
		
		for (UUID u : uuids) {
			ret.put(index, u);
			index++;
		}
		
		return ret;
	}
	
	private void printPlayerList(Map<Integer, UUID> list, Map<UUID, Player> players) {
		list.forEach((index, uuid) -> println(index + ". " + players.get(uuid).getUsername()));
	}
	
	@Override
	public Optional<Player> getWhoPlayer(String msg, Map<UUID, Player> players, Set<UUID> ignore) {
		//showPlayerField();
		
		Set<UUID> temp = new HashSet<>(players.keySet());
		temp.removeAll(ignore);
		
		if (temp.size() == 1) {
			return Optional.ofNullable(players.get(temp.stream().findFirst().get()));
		} else {
			Map<Integer, UUID> idmap = getIDMapping(temp);
			printPlayerList(idmap, players);
			int choice = numberChoice(msg, -1, idmap.size());
			if (choice == -1) {
				return Optional.empty();
			}
			return Optional.ofNullable(players.get(idmap.get(choice)));
		}
	}

	@Override
	public void threadWriteDebug(String msg) {
		try {
			s.acquire();
			System.err.println(msg);
			s.release();
		} catch (Exception e) {}
	}
	
	public void print(String msg) {
		try {
			s.acquire();
			System.out.print(msg);
			s.release();
		} catch (Exception e) {}
	}
	
	public void println() {
		print("\n");
	}
	
	public void println(String msg) {
		print(msg + "\n");
	}
	
	private void printPlayerList() {
		for (UUID id : game.getPlayers()) {
			println("> " + game.getPlayer(id).get().getUsername());
		}
	}

	@Override
	public String[] getMultiplayerMode() {
		String[] ret = new String[2];
		printTitle();
		println("1. Host match\n2. Connect to match");
		int choice = numberChoice("Choice: ", 1, 2);
		
		print("Username: ");
		String name = scan.nextLine();
		name = name.trim().strip();
		if (name.contains(" ")) {
			name = name.substring(0, name.indexOf(' '));
		}
		ret[0] = name;
		
		switch (choice) {
			case 1:
				ret[1] = null;
				break;
			case 2:
				print("IP address to connect to [eg. 127.0.0.1]: ");
				String ip = scan.nextLine();
				ip = ip.trim().strip();
				if (ip.contains(" ")) {
					ip = ip.substring(0, ip.indexOf(' '));
				}
				ret[2] = ip;
				break;
			default:
				break;
		}
		
		return ret;
	}

	@Override
	public void invalidAttack(String msg) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void errorScreen(String msg) {
		System.err.println(msg);
	}
	
	@Override
	public void executeMod(boolean ban) {
		Map<UUID, Player> temp = new LinkedHashMap<>();
		for (UUID u : game.getPlayers()) {
			temp.put(u, game.getPlayer(u).get());
		}
		Set<UUID> ignore = new HashSet<>();
		ignore.add(game.getOwnID());
		
		Optional<Player> ret = getWhoPlayer("Who you want to " + (ban ? "ban" : "kick") + " (-1 to go back)? ", temp, ignore);
		if (ret.isPresent()) {
			if (ban) {
				game.getClient().sendBan(ret.get().getID());
			} else {
				game.getClient().sendKick(ret.get().getID());
			}
			game.getClient().acquire();
		}
	}

	@Override
	public boolean waitStartingGame(boolean isHost) {
		if (isHost) {
			int hide = 0;
			do {
				printTitle();
				printPlayerList();
				println("\n1. Refresh");
				println("2. Kick player");
				println("3. Ban player");
				if (hide == 0) {
					println("4. Start game");
				}
				int choice = numberChoice("Choice (-1 go back): ", -1, 4 - hide);
				switch (choice) {
					case -1:
						return false;
					case 1:
						break;
					case 2:
						executeMod(false);
						break;
					case 3:
						executeMod(true);
						break;
					case 4:
						if (boolQuestion("Are you sure you want to start? ")) {
							hide++;
							game.getClient().sendStart();
							game.getClient().acquire();
						}
						break;
					default:
						break;
				}
			} while (game.getStatus() != GameStatus.PLACING);
		} else {
			while (game.getStatus() != GameStatus.PLACING) {
				try {
					printTitle();
					printPlayerList();
					
					println("\nWaiting game start");
					
					if (boolQuestion("0 refresh, 1 quit: ")) {
						return false;
					}
					
					Thread.sleep(1000);
				} catch (InterruptedException e) {}
			}
		}
		return true;
	}
	
	@Override
	public void waitGameStart() {
		showPlayerField();
		game.getClient().acquire();
	}
}
