package org.seabattles.gui.ascii;

import java.io.File;
import java.io.FileNotFoundException;
import java.time.Duration;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.Semaphore;

import org.seabattles.gui.ascii.AsciiController.InputType;
import org.seabattles.gui.jfx.FxGUI;
import org.seabattles.interfaces.GUI;
import org.seabattles.main.Main;
import org.seabattles.src.Board;
import org.seabattles.src.Game;
import org.seabattles.src.Game.GameStatus;
import org.seabattles.src.GameConfig;
import org.seabattles.src.GameConfig.GameDifficulty;
import org.seabattles.src.Player;
import org.seabattles.src.Player.PlayerStatus;
import org.seabattles.src.Ship;
import org.seabattles.src.Stats;

public class AsciiGUI implements GUI {
	private static final String YOUR_BOARD = "\t\t\t\t\tYour board";
	
	private AsciiController controller;
	private Object[] lastEvent;
	private Game game;
	private List<String> chat;
	private Semaphore threadWrite;
	private boolean waitingTurn;
	
	/************************* GUI *************************/
	public AsciiGUI() {
		initGUI();
	}
	
	@Override
	public void setGame(Game g) {
		game = g;
	}
	
	public void initGUI() {
		threadWrite = new Semaphore(1);
		controller = new AsciiController(this);
		lastEvent = null;
		chat = new LinkedList<>();
		lastEvent = null;
		waitingTurn = false;
	}
	
	public void setWaitingTurn(boolean state) {
		waitingTurn = state;
	}
	
	public void setLastEvent(Object[] lastEvent) {
		this.lastEvent = lastEvent;
	}
	
	public Object[] getLastEvent() {
		return lastEvent;
	}
	
	private int getIntObj() {
		int ret;
		try {
			ret = (Integer) controller.getReturnObject();
		} catch (NullPointerException e) {
			ret = Integer.MIN_VALUE;
		}
		return ret;
	}
	
	private boolean getBoolObj() {
		boolean ret;
		try {
			ret = (Boolean) controller.getReturnObject();
		} catch (NullPointerException e) {
			ret = false;
		}
		return ret;
	}
	
	private String getStringObj() {
		String ret = (String) controller.getReturnObject();
		if (ret == null) {
			ret = "";
		}
		return ret;
	}
	
	/************************* GUI FUNCTIONS *************************/
	private void printTitle() {
		controller.println(TITLE);
		printChat();
		controller.println();
	}
	
	private void printChat() {
		if (chat.size() > 0) {
			controller.println("[CHAT]");
			for (String m : chat) {
				controller.println(m);
			}
		}
	}
	
	private void showBoard(String header, Board b, boolean own) {
		controller.println(header);
		
		byte[][] hits = !own && !Main.DEBUG_STRING.equals(game.getGameConfig().getDebugString()) ? b.getObfuscatedHits() : b.getHits();
		
		controller.print("\t");
		for (int i = 0; i < hits.length; i++) {
			controller.print("[" + i + "]\t");
		}
		controller.println();
		
		for (int i = 0; i < hits.length; i++) {
			controller.print("[" + i + "]\t");
			for (int j = 0; j < hits[i].length; j++) {
				/*if (hits[i][j] != Board.NOTHING_FLAG) {
					System.err.print(" " + hits[i][j] + "\t");
					System.err.flush();
				} else {*/
				controller.print(" " + hits[i][j] + "\t");
					/*flush();
				}*/
			}
			controller.println();
		}
		
		controller.println();
	}
	
	private GuiAction askLeave() {
		controller.raiseEvent("askLeaveEvent");
		boolean answer = getBoolObj();
		if (answer) {
			return GuiAction.QUIT;
		}
		return GuiAction.NONE;
	}
	
	private GuiAction getActionSingle() {
		controller.raiseEvent("getActionSingleplayerEvent");
		int ret = getIntObj();
		
		switch (ret) {
			case 1:
				return GuiAction.SHOW_FIELD;
			case 2:
				return GuiAction.ATTACK;
			case 3:
				return askLeave();
			default:
				return GuiAction.NONE;
		}
	}
	
	private GuiAction getActionMulti(boolean waitingForTurn) {
		controller.raiseEvent("getActionMultiplayerEvent", waitingForTurn);
		int ret = getIntObj();
		
		if (waitingForTurn) {
			switch (ret) {
				case 1:
					return GuiAction.SHOW_FIELD;
				case 2:
					return GuiAction.CHAT;
				case 3:
					return askLeave();
				default:
					return GuiAction.NONE;
			}
		} else {
			switch (ret) {
				case 1:
					return GuiAction.SHOW_FIELD;
				case 2:
					return GuiAction.ATTACK;
				case 3:
					return GuiAction.CHAT;
				case 4:
					return askLeave();
				default:
					return GuiAction.NONE;
			}
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
	
	private void printPlayerList(Map<Integer, UUID> idmap, Map<UUID, Player> players) {
		idmap.forEach((index, uuid) -> controller.println(index + ". " + players.get(uuid).getUsername()));
	}
	
	private void printPlayerList() {
		game.getPlayers().forEach(e -> controller.println("> " + game.getPlayer(e).get().getUsername()));
	}
	
	private void printOwnBoard() {
		printTitle();
		showBoard(YOUR_BOARD, game.getOwnPlayer().getBoard(), true);
	}
	
	private void printPlayerBoard(Player p) {
		printTitle();
		showBoard(("\t\t\t\t" + p.getUsername() + "'s field"), p.getBoard(), false);
	}
	
	private void selectAndPrintBoard(Player p) {
		if (p.getID().equals(game.getOwnID())) {
			printOwnBoard();
		} else {
			printPlayerBoard(p);
		}
	}
	
	private void placeShip(Board b, int choice) {
		b.getShips().values().stream().filter(e -> e.getID() == choice).findFirst().ifPresent(s -> {
			if (s.isPlaced()) {
				b.unplaceShip(s);
			}
			
			boolean done;
			do {
				done = false;
				
				controller.raiseEvent("getXEvent", b);
				int x = getIntObj();
				if (x == -1) {
					return;
				}
					
				controller.raiseEvent("getYEvent", b);
				int y = getIntObj();
				if (y == -1) {
					return;
				}
				
				controller.raiseEvent("getREvent", b);
				String c = getStringObj();
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
					try {
						r = Integer.parseInt(c);
					} catch (NumberFormatException e) {
						r = -2;
					}
				}
				
				if (r == -1) {
					return;
				}
				
				
				try {
					s.setX(x);
					s.setY(y);
					s.rotate(r);
					
					if (!b.placeShip(s)) {
						controller.raiseEvent("invalidPlacementEvent", b);
						Main.sleep(500);
					} else {
						done = true;
					}
				} catch (IllegalArgumentException e) {
					controller.raiseEvent("invalidPlacementEvent", b);
					Main.sleep(500);
				}
			} while (!done);
		});
	}
	
	private boolean waitStartingGameHost() throws Exception {
		boolean hide = false;
		do {
			controller.raiseEvent("waitStartingGameHostEvent", hide);
			int choice = getIntObj();
			
			switch (choice) {
				case 1:
					String msg = sendChat();
					game.getClient().sendChatMessage(msg);
					break;
				case 2:
					executeMod(false);
					break;
				case 3:
					executeMod(true);
					break;
				case 4:
					controller.raiseEvent("askGameStartEvent");
					boolean rsp = getBoolObj();
					if (rsp) {
						hide = true;
						if (!game.startProcedure()) {
							hide = false;
						}
					}
					break;
				case 5:
					return false;
				default:
					break;
			}
		} while (game.getStatus() != GameStatus.PLACING);
		return true;
	}
	
	private boolean waitStartingGameNotHost() {
		while (game.getStatus() != GameStatus.PLACING) {
			controller.raiseEvent("waitStartingGameNotHostEvent");
			int choice = getIntObj();
			switch (choice) {
				case 1:
					String msg = sendChat();
					game.getClient().sendChatMessage(msg);
					break;
				case 2:
					GuiAction act = askLeave();
					if (act == GuiAction.QUIT) {
						return false;
					}
					break;
				default:
					break;
			}
		}
		return true;
	}
	
	/************************* GUI EVENTS *************************/
	public void errorEvent(String msg) {
		controller.asyncPrint("\n[ERROR] " + msg, false);
	}
	
	public void getModeEvent() {
		printTitle();
		controller.print("1. Singleplayer\n2. Multiplayer\n3. Exit\n\nChoice: ");
		controller.getInput(InputType.INPUT_RANGED_NUMBER, 1, 3);
	}
	
	public void getMultiplayerModeEvent() {
		printTitle();
		controller.print("1. Host match\n2. Connect to match\n3. Back\n\nChoice: ");
		controller.getInput(InputType.INPUT_RANGED_NUMBER, 1, 3);
	}
	
	public void getUsernameEvent() {
		printTitle();
		controller.print("Username: ");
		controller.getInput(InputType.INPUT_STRING);
	}
	
	public void getIPEvent(String username) {
		printTitle();
		controller.print("Username: " + username + "\nIP address to connect to [eg. 127.0.0.1]: ");
		controller.getInput(InputType.INPUT_STRING);
	}
	
	public void askFileEvent() {
		printTitle();
		controller.print("Do you have a custom config file? ");
		controller.getInput(InputType.INPUT_BOOLEAN);
	}
	
	public void getFilePathEvent() {
		printTitle();
		controller.print("Config file path (\"?\" back): ");
		controller.getInput(InputType.INPUT_STRING);
	}
	
	public void askBotNumberEvent() {
		printTitle();
		controller.print("Number of bots (1 - " + (Game.MAX_PLAYERS - 1) + ")? (-1 back) ");
		controller.getInput(InputType.INPUT_RANGED_NUMBER, -1, (Game.MAX_PLAYERS - 1));
	}
	
	public void askBotsDifficultyEvent() {
		printTitle();
		controller.print("1. Normal\n2. Hard\n3. Impossible\n4. Back\n\nDifficulty: ");
		controller.getInput(InputType.INPUT_RANGED_NUMBER, 1, 4);
	}
	
	public void showFieldEventAsync(Player who, boolean own, boolean waitInput) {
		if (own) {
			printOwnBoard();
		} else {
			printPlayerBoard(who);
		}
		
		if (waitInput) {
			controller.print("Type anything to go back");
		}
	}
	
	public void showFieldEvent(Player who, boolean own, boolean waitInput) {
		showFieldEventAsync(who, own, waitInput);
		if (waitInput) {
			controller.getInput(InputType.INPUT_STRING);
		} else {
			controller.releaseWaitingThread();
		}
	}
	
	public void askLeaveEvent() {
		printTitle();
		controller.print("Are you sure you want to leave? ");
		controller.getInput(InputType.INPUT_BOOLEAN);
	}
	
	public void getActionSingleplayerEvent() {
		showBoard(YOUR_BOARD, game.getOwnPlayer().getBoard(), true);
		
		controller.print("1. Show field\n2. Attack\n3. Leave\n\nChoice: ");
		controller.getInput(InputType.INPUT_RANGED_NUMBER, 1, 3);
	}
	
	public void getActionMultiplayerEventAsync(boolean waitingForTurn) {
		printOwnBoard();
		if (waitingForTurn) {
			controller.print("1. Show Field\n2. Chat\n3. Leave\n\nChoice: ");
		} else {
			controller.print("1. Show Field\n2. Attack\n3. Chat\n4. Leave\n\nChoice: ");
		}
	}
	
	public void getActionMultiplayerEvent(boolean waitingForTurn) {
		getActionMultiplayerEventAsync(waitingForTurn);
		if (waitingForTurn) {
			controller.getInput(InputType.INPUT_RANGED_NUMBER, 1, 3);
		} else {
			controller.getInput(InputType.INPUT_RANGED_NUMBER, 1, 4);
		}
	}
	
	public void printAndGetPlayerListEventAsync(String msg, Map<Integer, UUID> idmap, Map<UUID, Player> players) {
		// printOwnBoard();
		printPlayerList(idmap, players);
		controller.print(msg);
	}
	
	public void printAndGetPlayerListEvent(String msg, Map<Integer, UUID> idmap, Map<UUID, Player> players) {
		printAndGetPlayerListEventAsync(msg, idmap, players);
		controller.getInput(InputType.INPUT_RANGED_NUMBER, -1, idmap.size());
	}
	
	public void getXAttackEventAsync(Player who, int min, int max) {
		printPlayerBoard(who);
		controller.print("x (-1 back): ");
	}
	
	public void getXAttackEvent(Player who, int min, int max) {
		getXAttackEventAsync(who, min, max);
		controller.getInput(InputType.INPUT_RANGED_NUMBER, min, max);
	}
	
	public void getYAttackEventAsync(Player who, int min, int max) {
		printPlayerBoard(who);
		controller.print("y (-1 back): ");
	}
	
	public void getYAttackEvent(Player who, int min, int max) {
		getYAttackEventAsync(who, min, max);
		controller.getInput(InputType.INPUT_RANGED_NUMBER, min, max);
	}
	
	public void getXEventAsync(Board b) {
		printTitle();
		showBoard(YOUR_BOARD, b, true);
		controller.print("What X do you want to place the ship in (-1 back)? ");
	}
	
	public void getXEvent(Board b) {
		getXEventAsync(b);
		controller.getInput(InputType.INPUT_RANGED_NUMBER, -1, Board.getWidth() - 1);
	}
	
	public void getYEventAsync(Board b) {
		printTitle();
		showBoard(YOUR_BOARD, b, true);
		controller.print("What Y do you want to place the ship in (-1 back)? ");
	}
	
	public void getYEvent(Board b) {
		getYEventAsync(b);
		controller.getInput(InputType.INPUT_RANGED_NUMBER, -1, Board.getHeigth() - 1);
	}
	
	public void getREventAsync(Board b) {
		printTitle();
		showBoard(YOUR_BOARD, b, true);
		controller.print("What rotation do you want the ship to have (0 = right, 1 = down, 2 = left, 3 = up, -1 = back)? ");
	}
	
	public void getREvent(Board b) {
		getREventAsync(b);
		controller.getInput(InputType.INPUT_STRING);
	}

	public void invalidAttackEvent(String msg) {
		controller.print(msg);
		controller.releaseWaitingThread();
	}
	
	public void placeShipsDoneEvent(Board b) {
		printTitle();
		showBoard(YOUR_BOARD, b, true);
		controller.print("All ships placed, do you want to proceed? ");
		controller.getInput(InputType.INPUT_BOOLEAN);
	}
	
	public void placeShipsEvent(Board b) {
		Map<Integer, Ship> ships = b.getShips();
		
		printTitle();
		showBoard(YOUR_BOARD, b, true);
		
		controller.print("Ships available to place: ");
		ships.values().stream()
			.filter(e -> !e.isPlaced())
			.forEach(e -> controller.print("[id: " + e.getID() + " l: " + e.getLength() + " w: " + e.getWidth() + "] "));
		
		controller.print("\nShips already placed: ");
		ships.values().stream()
			.filter(e -> e.isPlaced())
			.forEach(e -> controller.print("[id: " + e.getID() + " l: " + e.getLength() + " w: " + e.getWidth() + "] "));
		controller.print("\nWhat ship do you want to place/move (-1 quit)? ");
		
		controller.getInput(InputType.INPUT_NUMBER);
	}
	
	public void invalidPlacementEvent(Board b) {
		printTitle();
		showBoard(YOUR_BOARD, b, true);
		controller.print("Incorrect ship placement!");
		controller.releaseWaitingThread();
	}
	
	public void endScreenEvent(UUID myID, PlayerStatus myStatus, Duration duration, Map<UUID, Object[]> playerStats) {
		printTitle();
		switch (myStatus) {
			case LOSER:
				controller.println("YOU LOST!");
				break;
			case WINNER:
				controller.println("YOU WIN!");
				break;
			default:
				break;
		}
		long time = Math.abs(duration.getSeconds());
		controller.println(String.format("Finish time: %dm %ds", time / 60, time % 60));
		
		// Object[0] = String userName, Object[1] = Stats playerStats
		
		Object[] mystats = playerStats.get(myID);
		controller.println(((Stats) mystats[1]).toString());
		controller.print("\n[Type anything to go back to main menu]");
		controller.getInput(InputType.INPUT_STRING);
	}
	
	public void waitStartingGameHostEventAsync(boolean hide) {
		printTitle();
		printPlayerList();
		controller.print("\n1. Chat");
		if (!hide) {
			controller.print("\n2. Kick player\n3. Ban player\n4. Start game\n5. Quit");
		}
		controller.print("\n\nChoice: ");
	}
	
	public void waitStartingGameHostEvent(boolean hide) {
		waitStartingGameHostEventAsync(hide);
		controller.getInput(InputType.INPUT_RANGED_NUMBER, 1, 1 + ((!hide ? 1 : 0) * 4));
	}
	
	public void askGameStartEvent() {
		controller.clearSyncBuffer();
		
		printTitle();
		controller.print("Are you sure you want to start? ");
		controller.getInput(InputType.INPUT_BOOLEAN);
	}
	
	public void waitStartingGameNotHostEventAsync() {
		printTitle();
		printPlayerList();
		controller.println();
		printChat();
		controller.print("\nWaiting host game start\n\n1. Chat\n2. Quit\n\nChoice: ");
	}
	
	public void waitStartingGameNotHostEvent() {
		waitStartingGameNotHostEventAsync();
		controller.getInput(InputType.INPUT_RANGED_NUMBER, 1, 2);
	}
	
	public void chatEvent() {
		printTitle();
		controller.print("\nChat send (empty to go back): ");
		controller.getInput(InputType.INPUT_STRING);
	}
	
	public void waitGameStartAfterPlacementEvent() {
		printOwnBoard();
		controller.releaseWaitingThread();
	}
	
	/************************* INTERFACE *************************/
	@Override
	public void raiseAsyncEvent(String event, Object... params) {
		if (lastEvent == null) {
			return;
		}
		
		if ("gameEnd".equals(event)) {
			releaseFromInput();
			return;
		}
		
		if ("playerTurn".equals(event)) {
			releaseFromInput();
			return;
		}
		
		// System.out.println("ASYNC: " + (String) lastEvent[0] + " - " + Arrays.toString(Arrays.copyOfRange(lastEvent, 1, lastEvent.length)));
		controller.raiseAsyncEvent((String) lastEvent[0], Arrays.copyOfRange(lastEvent, 1, lastEvent.length));
	}
	
	@Override
	public void releaseFromInput() {
		controller.release();
	}
	
	@Override
	public void startGUI() {
		controller.start();
	}
	
	@Override
	public void stopGUI() {
		System.out.println("[PRESS ANY KEY TO CLOSE THE PROGRAM]");
		controller.stop();
		Main.stopPgm();
	}

	@Override
	public int getMode() {
		controller.raiseEvent("getModeEvent");
		int ret = getIntObj();
		return ret;
	}
	
	@Override
	public String[] getMultiplayerMode() {
		controller.raiseEvent("getMultiplayerModeEvent");
		int choice = getIntObj();
		if (choice == 3) {
			return null;
		}
		
		String[] ret;
		if (Main.DEBUG_TESTING_DATA) {
			// TODO TESTING
			ret = Main.getMultiplayerTestData(choice);
		} else {
			ret = new String[2];
			controller.raiseEvent("getUsernameEvent");
			String username = getStringObj();
			username = username.trim().strip();
			if (username.contains(" ")) {
				username = username.substring(0, username.indexOf(' '));
			}
			ret[0] = username;
			
			if (choice == 1) {
				ret[1] = null;
			} else {
				controller.raiseEvent("getIPEvent", ret[0]);
				String ip = getStringObj();
				ip = ip.trim().strip();
				if (ip.contains(" ")) {
					ip = ip.substring(0, ip.indexOf(' '));
				}
				ret[1] = ip;
			}
		}
		
		return ret;
	}

	@Override
	public GameConfig configGame() throws Exception {
		GameConfig ret = new GameConfig();
		
		boolean ok;
		String rsp;
		
		controller.raiseEvent("askFileEvent");
		boolean file = getBoolObj();
		
		if (file) {
			do {
				ok = true;
				controller.raiseEvent("getFilePathEvent");
				rsp = getStringObj();
				
				if (rsp.contains("?")) {
					file = false;
					break;
				} else {
					try {
						ret.setConfigFile(new File(rsp));
					} catch (FileNotFoundException e) {
						errorScreen("File not found, retry");
						Main.sleep(500);
					}
				}
			} while (!ok);
		}
		ret.applyConfigFile();
		
		int number;
		do {
			controller.raiseEvent("askBotNumberEvent");
			number = getIntObj();
			if (number == -1) {
				return null;
			}
		} while (number == 0);
		ret.setNumberOfBots(number);
		
		do {
			ok = true;
			controller.raiseEvent("askBotsDifficultyEvent");
			number = getIntObj();
			switch (number) {
				case 1:
					ret.setBotsDifficulty(GameDifficulty.NORMAL);
					break;
				case 2:
					ret.setBotsDifficulty(GameDifficulty.HARD);
					break;
				case 3:
					ret.setBotsDifficulty(GameDifficulty.IMPOSSIBLE);
					break;
				case 4:
					return null;
				default:
					ok = false;
					break;
			}
		} while (!ok);
		
		return ret;
	}

	@Override
	public boolean waitStartingGame(boolean isHost) throws Exception {
		if (isHost) {
			return waitStartingGameHost();
		} else {
			return waitStartingGameNotHost();
		}
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
			game.getClient().acquire(Game.sems_names[2]);
		}
	}

	@Override
	public void waitGameStartAfterPlacement() {
		controller.raiseEvent("waitGameStartAfterPlacementEvent");
		game.getClient().acquire(Game.sems_names[1]);
	}

	@Override
	public String sendChat() {
		controller.raiseEvent("chatEvent");
		String msg = getStringObj();
		return msg;
	}
	
	@Override
	public void addChatMessage(UUID from, String msg) {
		if (chat.size() == MAX_CHAT_SIZE) {
			chat.remove(0);
		}
		chat.add(game.getPlayer(from).get().getUsername() + ": " + msg);
		if (((String) lastEvent[0]).equals("getActionMultiplayerEvent")) {
			lastEvent[1] = !waitingTurn;
		}
		raiseAsyncEvent("chatEvent");
	}

	@Override
	public Board placeShips() {
		Board b = new Board(game.getGameConfig().getShipsConfig());
		boolean done = false;
		Map<Integer, Ship> ships = b.getShips();
		
		if (Main.DEBUG_TESTING_DATA) {
			Main.placeTestShips(b);
		} else {
			boolean allPlaced = false;
			do {
				if (allPlaced) {
					controller.raiseEvent("placeShipsDoneEvent", b);
					done = getBoolObj();
					if (done) {
						break;
					}
				}
				
				controller.raiseEvent("placeShipsEvent", b);
				int choice = getIntObj();
				if (choice != -1) {
					placeShip(b, choice);
				} else {
					return null;
				}
				
				allPlaced = true;
				for (Ship s : ships.values()) {
					if (!s.isPlaced()) {
						allPlaced = false;
						break;
					}
				}
			} while (true);
		}
		
		return b;
	}

	@Override
	public GuiAction getAction(boolean waitingForTurn) {		
		if (!game.isMultiplayer()) {
			return getActionSingle();
		} else {
			return getActionMulti(waitingForTurn);
		}
	}

	@Override
	public Optional<Player> getWhoPlayer(String msg, Map<UUID, Player> players, Set<UUID> ignore) {
		Set<UUID> temp = new HashSet<>(players.keySet());
		temp.removeAll(ignore);
		
		if (temp.size() == 1) {
			return Optional.ofNullable(players.get(temp.stream().findFirst().get()));
		} else {
			Map<Integer, UUID> idmap = getIDMapping(temp);
			
			controller.raiseEvent("printAndGetPlayerListEvent", msg, idmap, players);
			int choice = getIntObj();
			if (choice == -1) {
				return Optional.empty();
			}
			
			return Optional.ofNullable(players.get(idmap.get(choice)));
		}
	}

	@Override
	public void showField(Optional<Player> who, boolean own, boolean waitInput) {
		if (who.isEmpty()) {
			return;
		}
		
		if (game.isMultiplayer()) {
			game.queryBoard(who);
		}
		
		controller.raiseEvent("showFieldEvent", who.get(), own, waitInput);
		controller.getReturnObject();
	}

	@Override
	public Object[] attack(Optional<Player> who) {
		Object[] ret = new Object[3];
		
		if (who.isEmpty()) {
			ret[0] = null;
			return ret;
		}
		
		Player p = who.get();
		ret[0] = p.getID();

		controller.raiseEvent("getXAttackEvent", p, -1, Board.getWidth());
		int x = getIntObj();
		if (x < 0) {
			ret[0] = null;
			return ret;
		}
		
		controller.raiseEvent("getYAttackEvent", p, -1, Board.getWidth());
		int y = getIntObj();
		if (y < 0) {
			ret[0] = null;
			return ret;
		}
		
		ret[1] = x;
		ret[2] = y;
		
		return ret;
	}

	@Override
	public void invalidAttack(String msg) {
		controller.raiseEvent("invalidAttackEvent", msg);
		Main.sleep(GUI.INVALID_ATTACK_ANIMATION_TIMER);
	}

	@Override
	public void endScreen(UUID myID, PlayerStatus myStatus, Duration duration, Map<UUID, Object[]> playerStats) {
		controller.raiseEvent("endScreenEvent", myID, myStatus, duration, playerStats);
		controller.getReturnObject();
	}

	@Override
	public void errorScreen(String msg) {
		controller.raiseAsyncEvent("errorEvent", msg);
	}

	@Override
	public void threadWriteDebug(String msg) {
		try {
			threadWrite.acquire();
			System.err.println(msg);
			threadWrite.release();
		} catch (Exception e) {}
	}
	
	@Override
	public void threadWrite(String msg) {
		try {
			threadWrite.acquire();
			System.out.print(msg);
			threadWrite.release();
		} catch (Exception e) {}
	}
}
