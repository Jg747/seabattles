package org.seabattles.src;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.text.ParseException;
import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.function.Predicate;
import java.util.stream.Collectors;

import org.json.JSONException;
import org.seabattles.interfaces.GUI;
import org.seabattles.interfaces.GUI.GuiAction;
import org.seabattles.main.Main;
import org.seabattles.net.BooleanSemaphore;
import org.seabattles.net.Client;
import org.seabattles.net.ClientWorker;
import org.seabattles.net.Protocol;
import org.seabattles.net.Server;
import org.seabattles.net.Server.ConnStatus;
import org.seabattles.src.Board.AttackStatus;
import org.seabattles.src.GameConfig.GameDifficulty;
import org.seabattles.src.Player.PlayerStatus;

public class Game implements Runnable {
	
	public enum GameStatus {
		CONFIG,						// Game not started yet
		PLACING,					// placing ships
		RUNNING,					// players playing
		END,						// winner chosen
		QUIT						// Player has quit
	};
	
	public enum GenericStatus {
		UNDEFINED,	// nothing is happening
		CHANGING,
		WAITING_MSG // something happened, waiting for a message from the thread
	};
	
	private Thread serverThread;
	private Server server;
	
	private Thread recvThread;
	private Client client;
	
	private GameConfig config;
	private LinkedHashMap<UUID, Player> players;
	
	private GUI gui;
	private final UUID gameID;
	
	public static final int MAX_PLAYERS = 4;
	private static final String DEFAULT_USERNAME = "You";
	
	private UUID ownID;
	private UUID turn;				// Contains ID of the player who has the turn
	private GameStatus status;
	private Instant start;
	private Instant end;
	
	private GenericStatus genericStatus;
	private String errMsg;
	private Object resource;
	
	private Set<UUID> attacked;
	private Set<UUID> availablePlayers;
	
	public static final String[] sems_names = {"server", "generic", "host", "board"};
	private List<BooleanSemaphore> sems;
	
	private boolean isMulti;
	
 	public Game() {
		config = new GameConfig();
		status = GameStatus.CONFIG;
		genericStatus = GenericStatus.UNDEFINED;
		resource = null;
		gameID = UUID.randomUUID();
		
		client = null;
		recvThread = null;
		server = null;
		serverThread = null;
		
		players = new LinkedHashMap<>();
		
		sems = new ArrayList<>();
		for (String s : sems_names) {
			sems.add(new BooleanSemaphore(s));
		}
		
		isMulti = false;
	}
 	
 	public UUID getGameID() {
 		return gameID;
 	}
 	
 	public boolean isServer() {
 		return server != null || serverThread != null;
 	}
 	
 	public void destroyServer() {
 		server.destroy();
 	}
 	
 	public void setGUI(GUI g) {
 		this.gui = g;
 	}
 	
 	public void setResource(Object obj) {
 		resource = obj;
 	}
 	
 	public Object getResource() {
 		return resource;
 	}
 	
 	public void setTurn(UUID id) {
 		turn = id;
 	}
 	
 	public UUID getTurn() {
 		return turn;
 	}
 	
 	public Set<UUID> getAttacked() {
 		return attacked;
 	}
 	
 	public Set<UUID> getAvailablePlayers() {
 		return availablePlayers;
 	}
 	
 	// GUI INTERFACE
 	public void flipGenericStatus() {
 		if (genericStatus == GenericStatus.WAITING_MSG) {
 			genericStatus = GenericStatus.UNDEFINED;
 		} else {
 			genericStatus = GenericStatus.WAITING_MSG;
 		}
 	}
 	
 	public GenericStatus getGenericStatus() {
 		return genericStatus;
 	}
 	
 	public void setErrMsg(String msg) {
 		errMsg = msg;
 	}
 	
 	public void clearErr() {
 		errMsg = null;
 	}
 	
 	// GUI INTERFACE
 	public String getErrMsg() {
 		return errMsg;
 	}
 	
 	// GUI INTERFACE
 	public Player getOwnPlayer() {
 		return players.get(ownID);
 	}

 	public void setOwnID(UUID id) {
 		ownID = id;
 	}
 	
 	public UUID getOwnID() {
 		return ownID;
 	}
 	
 	// GUI INTERFACE
	public void setNumberOfPlayers(short numberOfPlayers) {
		config.setNumberOfPlayers(numberOfPlayers);
	}
	
	// GUI INTERFACE
	public void setNumberOfBots(short numberOfBots) {
		config.setNumberOfBots(numberOfBots);
	}
	
	// GUI INTERFACE
	public Client getClient() {
		return client;
	}
	
	// GUI INTERFACE
	public void setConfigFilePath(String path) throws FileNotFoundException, ParseException {
		config.setConfigFile(new File(path));
	}
	
	
	// GUI INTERFACE
	public void setDifficulty(GameDifficulty diff) {
		config.setBotsDifficulty(diff);
	}
	
	// GUI INTERFACE
	public GameDifficulty getDifficulty() {
		return config.getBotsDifficulty();
	}
	
	public int getNumberOfPlayers() {
		return config.getNumberOfPlayers();
	}
	
	// GUI INTERFACE
	public int getTotalNumberOfPlayers() {
		return config.getTotalNumberOfPlayers();
	}
	
	// GUI INTERFACE
	public GameStatus getStatus() {
		return status;
	}
	
	public void setStatus(GameStatus status) {
		this.status = status;
	}
	
	public void setPlayerList(HashMap<UUID, String> users) {
		players = new LinkedHashMap<>();
		for (UUID key : users.keySet()) {
			players.put(key, new Player(key, users.get(key)));
		}
	}
	
	// GUI INTERFACE
	public void applyConfig() throws RuntimeException, ParseException, JSONException {
		Bot.setBotDifficulty(config.getBotsDifficulty());
		
		Board.setWidth(config.getBoardWidth());
		Board.setHeigth(config.getBoardHeigth());
	}
	
	public UUID addPlayer() {
		Player p = new Player();
		players.put(p.getID(), p);
		return p.getID();
	}
	
	public void addBot() {
		Bot b = new Bot(config.getShipsConfig());
		b.generateBoard();
		players.put(b.getID(), b);
	}
	
	public void addBots() {
		for (int i = 0; i < config.getNumberOfBots(); i++) {
			addBot();
		}
	}
	
	public void removePlayer(UUID id) {
		players.remove(id);
	}
	
	// GUI INTERFACE
	public Optional<Player> getPlayer(UUID id) {
		return Optional.ofNullable(players.get(id));
	}
	
	public Set<UUID> getPlayers() {
		return players.keySet();
	}
	
	public boolean doesExist(UUID id) {
		return players.containsKey(id);
	}
	
	
	public void setGameConfig(GameConfig cfg) throws IOException, ParseException {
		config = cfg;
	}
	
	public GameConfig getGameConfig() {
		return config;
	}
	
	private void createPlayersFromConfig(boolean bots) {
		if (players.size() == 0) {
			for (int i = 0; i < config.getNumberOfPlayers(); i++) {
				addPlayer();
			}
		}
		
		if (bots) {
			addBots();
		}
		generateBoards();
	}
	
	private void generateBoards() {
		for (Player p : players.values()) {
			p.setConfig(config);
			p.generateBoard();
		}
	}
	
	private boolean checkIfWinner() {
		return checkIfWinner(turn);
	}
	
	private boolean checkIfWinner(UUID winner) {
		Set<UUID> alive = players.entrySet().stream().filter(e -> !e.getValue().isDead() && !e.getValue().didQuit()).map(Map.Entry::getKey).collect(Collectors.toSet());
		return alive.size() == 1 && alive.contains(winner);
	}
	
	private Set<UUID> getPlayersPred(Predicate<Map.Entry<UUID, Player>> pred) {
		return players.entrySet().stream()
			.filter(pred)
			.map(Map.Entry::getKey)
			.collect(Collectors.toSet());
	}
	
	private Set<UUID> getPlayersToNotIgnore() {
		return getPlayersPred(e -> !e.getValue().isDead() && !e.getValue().didQuit() && !e.getKey().equals(ownID));
	}
	
	private Set<UUID> getPlayersToIgnore() {
		return getPlayersPred(e -> e.getValue().isDead() ||  e.getValue().didQuit() || e.getKey().equals(ownID));
	}
	
	private void playerAttack() {
		Set<UUID> ignored = getPlayersToIgnore();
		ignored.addAll(attacked);
		
		showPlayerOwnField();
		Optional<Player> who = gui.getWhoPlayer("Who do you want to attack? ", players, ignored);
		
		do {
			Object[] ret = gui.attack(who);	// ret[0] = UUID, ret[1] = Integer x, ret[2] = Integer y
			if (ret == null || ret[0] == null) {
				break;
			}
			
			UUID u = (UUID) ret[0];
			Player selected = players.get(u);
			int x = (Integer) ret[1];
			int y = (Integer) ret[2];
			
			if (u.equals(turn) || ignored.contains(u) || selected.getStatus() != PlayerStatus.READY) {
				if (attacked.contains(u)) {
					gui.invalidAttack("Player already attacked!");
				} else {
					gui.invalidAttack("Not possible to attack selected player!");
				}
			} else {
				AttackStatus as = players.get(turn).attack(selected, x, y);
				if (as == AttackStatus.HIT || as == AttackStatus.MISS || as == AttackStatus.SUNK) {
					attacked.add(u);
					
					if (selected.getBoard().allShipsGone()) {
						selected.setStatus(PlayerStatus.LOSER);
						
						if (checkIfWinner()) {
							players.get(turn).setStatus(PlayerStatus.WINNER);
							status = GameStatus.END;
						}
					}
					
					break;
				} else {
					gui.invalidAttack("Invalid attack coordinates!");
				}
			}
		} while (true);
	}
	
	private void playerTurn(Bot current) {
		Set<UUID> availablePlayers = getPlayersPred(e -> !e.getValue().isDead() && !e.getValue().didQuit() && !e.getKey().equals(turn) && !e.getValue().didQuit());
		availablePlayers.forEach(u -> {
			Player selected = players.get(u);
			while (status == GameStatus.RUNNING) {
				AttackStatus as = current.attack(selected);
				if (selected.getID().equals(ownID)) {
					gui.raiseAsyncEvent("gotAttacked");
				}
				if (as == AttackStatus.HIT || as == AttackStatus.MISS || as == AttackStatus.SUNK) {
					if (selected.getBoard().allShipsGone()) {
						selected.setStatus(PlayerStatus.LOSER);
						if (selected.getID().equals(ownID) || checkIfWinner()) {
							players.get(turn).setStatus(PlayerStatus.WINNER);
							status = GameStatus.END;
						}
					}
					
					break;
				}
			}
		});
	}
	
	public boolean isEndGameQuit() {
		Set<UUID> availablePlayers = getPlayersPred(e -> !e.getValue().isDead() && !e.getValue().didQuit() && !e.getValue().didQuit());
		return availablePlayers.size() == 1;
	}
	
	public void setWinner() {
		Set<UUID> availablePlayers = getPlayersPred(e -> !e.getValue().isDead() && !e.getValue().didQuit() && !e.getValue().didQuit());
		Iterator<UUID> it = availablePlayers.iterator();
		if (it.hasNext()) {
			players.get(availablePlayers.iterator().next()).setStatus(PlayerStatus.WINNER);
		}
	}
	
	private void playerTurn(Player current) {
		if (current instanceof Bot) {
			playerTurn((Bot) current);
			return;
		}
		
		attacked = new HashSet<>();
		availablePlayers = getPlayersToNotIgnore();
		
		while (status == GameStatus.RUNNING) {
			GuiAction act = gui.getAction(false);
			switch (act) {
				case SHOW_FIELD:
					showPlayerOwnField();
					Optional<Player> who = gui.getWhoPlayer("Who do you want to see (-1 go back)? ", players, getPlayersToIgnore());
					gui.showField(who, false, true);
					break;
				case ATTACK:
					playerAttack();
					if (attacked.containsAll(availablePlayers)) {
						return;
					}
					break;
				case QUIT:
					status = GameStatus.QUIT;
					break;
				default:
					break;
			}
		}
	}
	
	private Map<UUID, Object[]> getPlayerEndObj() {
		Map<UUID, Object[]> ret = new HashMap<>();
		for (Map.Entry<UUID, Player> e : players.entrySet()) {
			Object[] obj = new Object[2];
			obj[0] = e.getValue().getUsername();
			obj[1] = e.getValue().getStats();
			ret.put(e.getKey(), obj);
		}
		return ret;
	}
	
	private void gameStart() throws Exception {
		applyConfig();
		createPlayersFromConfig(true);
		setOwnID(players.values().stream().filter(e -> e instanceof Player).findFirst().get().getID());
		getOwnPlayer().setUsername(DEFAULT_USERNAME);
		gui.raiseAsyncEvent("userList");

		status = GameStatus.PLACING;
		Board board = gui.placeShips();
		if (board == null) {
			return;
		}
		
		getOwnPlayer().setBoard(board);
		players.values().stream().filter(e -> e instanceof Bot).forEach(b -> ((Bot) b).placeShips());
		
		status = GameStatus.RUNNING;
		start = Instant.now();
		
		while (status == GameStatus.RUNNING) {
			players.values().stream().filter(p -> !p.isDead() && !p.didQuit()).forEachOrdered(p -> {
				if (status != GameStatus.RUNNING || p.isDead() || p.didQuit()) {
					return;
				}
				
				turn = p.getID();
				p.setStatus(PlayerStatus.HAS_TURN);
				playerTurn(p);
				if (p.getStatus() == PlayerStatus.HAS_TURN) {
					p.setStatus(PlayerStatus.READY);
				}
			});
		}
		
		if (status != GameStatus.QUIT) {
			end = Instant.now();
			Map<UUID, Object[]> ret = getPlayerEndObj();
			gui.endScreen(ownID, getOwnPlayer().getStatus(), Duration.between(start, end), ret);
		}
	}
	
	public void destroyAll() throws IOException {
		try {
			if (serverThread != null) {
				serverThread.interrupt();
				server.destroy();
			}
			
			if (recvThread != null) {
				recvThread.interrupt();
				client.destroy();
			}
			
			File localSpritesFolder = new File(Main.CLIENT_SPRITE_PATH);
			if (localSpritesFolder.exists()) {
				File[] list = localSpritesFolder.listFiles();
				if (list != null) {
					for (File f : list) {
						f.delete();
					}
				}
				localSpritesFolder.delete();
			}
		} catch (Exception e) {
			client.sendQuit();
			throw e;
		}
	}
	
	public GUI getGUI() {
		return gui;
	}
	
	private boolean configGame() {
		GameConfig conf;
		try {
			conf = gui.configGame();
			if (conf == null) {
				return false;
			}
			setGameConfig(conf);
			conf.applyConfigFile();
		} catch (Exception e) {
			System.err.println("File default di configurazione non trovato, terminazione");
			return false;
		}
		return true;
	}
	
	public boolean start() throws Exception {
		int mode;
		String[] sessionInfo = null;
		
		if (Main.DEBUG_TESTING_DATA) {
			Object[] startData = Main.getStartTestData(false);
			
			mode = (int) startData[0];
			GameConfig conf = (GameConfig) startData[1];
			
			setGameConfig(conf);
			conf.applyConfigFile();
			applyConfig();
			
			if (mode > 1) {
				isMulti = true;
				sessionInfo = gui.getMultiplayerMode();
			}
		} else {
			mode = gui.getMode();
			sessionInfo = null;
			switch (mode) {
				case 1:
					isMulti = false;
					if (!configGame()) {
						return true;
					}
					break;
				case 2:
					isMulti = true;
					sessionInfo = gui.getMultiplayerMode();
					if (sessionInfo == null) {
						return true;
					}
					
					if (sessionInfo[1] == null) {
						if (!configGame()) {
							return true;
						}
					}
					break;
				case 3:
					return false;
				default: 
					return false;
			}
		}
		
		switch (mode) {
			case 1:
				gameStart();
				break;
			case 2:
				gameStartMulti(sessionInfo);
				destroyAll();
				break;
			default:
				break;
		}
		
		return true;
	}

	
	
	/*********************************/
	/*			SERVER SIDE			 */
	/*********************************/
	
	private void createAsHost() throws Exception {
		Game g = new Game();
		server = new Server(g);
		serverThread = new Thread(server);
		serverThread.start();
		
		createAsClient("127.0.0.1");

		client.sendConfiguration(config);
		client.acquire(Game.sems_names[2]);
		if ((Boolean) getResource() == false) {
			throw new Exception("Not host");
		}
		setResource(null);
		
		client.sendSprites(config.getShipsConfig());
		client.acquire(Game.sems_names[2]);
		if ((Boolean) getResource() == false) {
			throw new Exception("Not host");
		}
		setResource(null);
	}
	
	private boolean createAsClient(String ipAddress) {
		try {
			client = new Client(ipAddress, this);
		} catch (IOException e) {
			gui.errorScreen("Can't connect to server: invalid IP");
			return false;
		}
		
		if (client.getConnStatus() != ConnStatus.STATUS_OK) {
			if (serverThread != null) {
				serverThread.interrupt();
			}
			gui.errorScreen("Can't connect to server: " + client.getConnStatus());
			return false;
		}
		client.startRecvThread();
		return true;
	}
	
	public void createRecvThread() {
		if (recvThread == null) {
			recvThread = new Thread(client);
			recvThread.start();
		}
	}
	
	public boolean startProcedure() {
		client.sendStart();
		client.acquire(Game.sems_names[2]);
		
		while (resource == null) {
			Main.sleep(10);
		}
		int error = (Integer) getResource();
		setResource(null);
		
		switch (error) {
			case 1:
				gui.errorScreen("You must play with at least one adversary!");
				return false;
			case 2:
				gui.errorScreen("Not host");
				return false;
			default:
				return true;
		}
	}
	
	// GUI INTERFACE
	public boolean createSession(String[] sessionInfo) throws Exception {
		if (sessionInfo[1] == null) {
			createAsHost();
		} else if (!createAsClient(sessionInfo[1])) {
			return false;
		}
		
		client.sendUsername(sessionInfo[0]);
		client.acquire(Game.sems_names[1]);

		return gui.waitStartingGame(sessionInfo[1] == null);
	}
	
	public void checkAllBoardsOk() {
		for (Player p : players.values()) {
			if (!(p instanceof Bot) && !p.didQuit()) {
				for (Ship s : p.getBoard().getShips().values()) {
					if (!s.isPlaced()) {
						return;
					}
				}
			}
		}
		
		release(Game.sems_names[0]);
	}
	
	public void release(String name) {
		sems.stream().filter(e -> e.getName().equals(name)).findFirst().ifPresent(e -> e.release());
	}
	
	public void acquire(String name) {
		sems.stream().filter(e -> e.getName().equals(name)).findFirst().ifPresent(e -> e.acquire());
	}
	
	public void setServer(Server server) {
		this.server = server;
	}
	
	public void queryBoard(Optional<Player> player) {
		if (player.isEmpty()) {
			return;
		}
		
		client.sendBoardRequest(player.get().getID(), Main.DEBUG_STRING);
		client.acquire(Game.sems_names[1]);	// Wait BOARD message
		
		try {
			ByteArrayInputStream in = new ByteArrayInputStream((byte[]) getResource());
		    ObjectInputStream stream = new ObjectInputStream(in);
		    byte[][] data = (byte[][]) stream.readObject();
		    stream.close();
			players.get(player.get().getID()).getBoard().setBoardMatrix(data);
			setResource(null);
		} catch (IOException | ClassNotFoundException e) {}
	}
	
	public boolean isMultiplayer() {
		return isMulti;
	}
	
	private void showField() {
		showPlayerOwnField();
		Optional<Player> who = gui.getWhoPlayer("Who do you want to see (-1 go back)? ", players, getPlayersToIgnore());
		gui.showField(who, false, true);
	}
	
	private boolean waitTurn() {
		flipGenericStatus();
		while (getGenericStatus() == GenericStatus.WAITING_MSG && status == GameStatus.RUNNING) {
			GuiAction act = gui.getAction(true);
			if (act == null) {
				continue;
			}
			
			switch (act) {
				case SHOW_FIELD:
					showField();
					break;
				case CHAT:
					String msg = gui.sendChat();
					client.sendChatMessage(msg);
					break;
				case QUIT:
					quitGame();
					return false;
				default:
					break;
			}
		}
		return true;
	}
	
	private void showPlayerOwnField() {
		gui.showField(Optional.of(getOwnPlayer()), true, false);
	}
	
	private void playerAttackMulti() {
		Set<UUID> ignored = getPlayersToIgnore();
		ignored.addAll(attacked);
		
		showPlayerOwnField();
		Optional<Player> who = gui.getWhoPlayer("Who do you want to attack? ", players, ignored);
		
		do {
			Object[] ret = gui.attack(who);	// ret[0] = UUID, ret[1] = Integer x, ret[2] = Integer y
			if (ret == null || ret[0] == null) {
				break;
			}
			
			UUID u = (UUID) ret[0];
			Player selected = players.get(u);
			int x = (Integer) ret[1];
			int y = (Integer) ret[2];
			
			client.sendAttack(u, x, y);
			client.acquire(Game.sems_names[1]);
			AttackStatus as = (AttackStatus) getResource();
			setResource(null);
			
			if (as == AttackStatus.HIT || as == AttackStatus.MISS || as == AttackStatus.SUNK) {
				attacked.add(u);
				break;
			} else {
				gui.invalidAttack("Invalid attack coordinates!");
			}
		} while (true);
	}
	
	private void queryBoards() {
		players.entrySet().stream().filter(e -> !e.getKey().equals(ownID) && !e.getValue().isDead() && !e.getValue().didQuit()).forEach(e -> {
			queryBoard(Optional.of(e.getValue()));
		});
	}
	
	private void quitGame() {
		client.sendQuit();
		status = GameStatus.QUIT;
	}
	
	private boolean playerTurnMulti() {
		if (getOwnPlayer().getStatus() == PlayerStatus.LOSER) {
			return true;
		}
		
		queryBoards();

		attacked = new HashSet<>();
		availablePlayers = getPlayersToNotIgnore();
		
		while (status == GameStatus.RUNNING) {
			GuiAction act = gui.getAction(false);
			if (act == null) {
				continue;
			}
			
			try {
				switch (act) {
					case SHOW_FIELD:
						showField();
						break;
					case ATTACK:
						playerAttackMulti();
						if (attacked.containsAll(availablePlayers)) {
							return true;
						}
						break;
					case CHAT:
						client.sendChatMessage(gui.sendChat());
						break;
					case QUIT:
						quitGame();
						return false;
					default:
						break;
				}
			} catch (Exception e) {
				Logger.logException(e);
			}
		}
		return true;
	}
	
	private void gameStartMulti(String[] sessionInfo) throws Exception {
		try {
			if (sessionInfo == null) {
				return;
			}
			
			if (!createSession(sessionInfo)) {
				if (client != null) {
					client.sendQuit();
				}
				return;
			}
			
			if (!client.isConnected() || getErrMsg() != null) {
				gui.errorScreen(getErrMsg());
				return;
			}
			
			localGame();
		} catch (IOException e) {
			client.sendQuit();
			throw e;
		}
	}
	
	private void endGame() {
		Object[] ret = (Object[]) getResource();
		
		Duration d = Duration.ofSeconds((Long) ret[0]);
		UUID winner = (UUID) ret[1];
		UUID[] ids = (UUID[]) ret[2];
		String[] names = (String[]) ret[3];
		Stats[] stats = (Stats[]) ret[4];
		
		PlayerStatus s = PlayerStatus.LOSER;
		if (winner.equals(ownID)) {
			s = PlayerStatus.WINNER;
		}
		
		Map<UUID, Object[]> endObj = new HashMap<>();
		for (int i = 0; i < ids.length; i++) {
			Object[] arr = new Object[2];
			arr[0] = names[i];
			arr[1] = stats[i];
			endObj.put(ids[i], arr);
		}
		
		gui.endScreen(ownID, s, d, endObj);
	}
	
	private void playerTurnServer(Bot current) {
		Set<UUID> availablePlayers = getPlayersPred(e -> !e.getValue().isDead() && !e.getValue().didQuit() && !e.getKey().equals(turn));
		availablePlayers.forEach(u -> {
			Player selected = players.get(u);
			while (status == GameStatus.RUNNING) {
				AttackStatus as = current.attack(selected);
				if (as == AttackStatus.HIT || as == AttackStatus.MISS || as == AttackStatus.SUNK) {
					current.updateStats(selected, as);
					if (!(selected instanceof Bot)) {
						server.getWorker(selected.getID()).sendMsg(Protocol.getGotAttackedMessage(current.getID(), selected.getBoard().getHits()));
					}
					
					if (selected.getBoard().allShipsGone()) {
						selected.setStatus(PlayerStatus.LOSER);
						if (!(selected instanceof Bot)) {
							server.getWorker(selected.getID()).sendMsg(Protocol.getEliminatedMessage(current.getID()));
						}
						server.broadcastPlayerElimination(selected.getID());
						
						if (checkIfWinner()) {
							current.setStatus(PlayerStatus.WINNER);
							status = GameStatus.END;
						}
					}
					
					break;
				}
			}
		});
	}
	
	public boolean playerAttackServer(ClientWorker attacker, ClientWorker defender, Object[] attack) {
		UUID selected = (UUID) attack[0];
		Set<UUID> ignored = getPlayersPred(e -> e.getValue().isDead() || e.getValue().didQuit() || e.getValue().getID().equals(turn));
		ignored.addAll(attacked);
		
		if (turn.equals(selected) || players.get(turn).getStatus() != PlayerStatus.HAS_TURN || ignored.contains(selected) || players.get(selected).getStatus() != PlayerStatus.READY) {
			return false;
		} else {
			AttackStatus as = players.get(turn).attack(players.get(selected), (Integer) attack[1], (Integer) attack[2]);
			if (as == AttackStatus.HIT || as == AttackStatus.MISS || as == AttackStatus.SUNK) {
				attacked.add(selected);
				
				attacker.sendMsg(Protocol.getAttackStatusMessage(as));
				if (defender != null) {
					defender.sendMsg(Protocol.getGotAttackedMessage(turn, players.get(selected).getBoard().getHits()));
				}
				
				if (as == AttackStatus.SUNK) {
					server.broadcastShipSunk(attack);
				}
				
				if (players.get(selected).getBoard().allShipsGone()) {
					players.get(selected).setStatus(PlayerStatus.LOSER);
					if (defender != null) {
						defender.sendMsg(Protocol.getEliminatedMessage(turn));
					}
					server.broadcastPlayerElimination(selected);
					
					if (checkIfWinner()) {
						players.get(turn).setStatus(PlayerStatus.WINNER);
						status = GameStatus.END;
					}
				}
				
				return true;
			}
			attacker.sendMsg(Protocol.getAttackStatusMessage(AttackStatus.INVALID));
			return false;
		}
	}
	
	private void playerTurnServer(Player current) {
		server.broadcastTurn(current.getID());
		
		if (current instanceof Bot) {
			playerTurnServer((Bot) current);
			return;
		}
		
		attacked = new HashSet<>();
		availablePlayers = getPlayersPred(e -> !e.getValue().isDead() && !e.getValue().didQuit() && !e.getKey().equals(turn));
		
		acquire(Game.sems_names[0]);	// waiting current player attacks to all available players | current player quit
	}

	
	private void localGame() throws Exception {
		// GAME SECONDO IL CLIENT LOCALE
		acquire(Game.sems_names[1]);	// wait MATCH_PLCM_STARTED

		generateBoards();
		
		boolean go = false;
		Board b;
		do {
			b = gui.placeShips();
			if (b == null) {
				return;
			}
			
			client.sendBoard(b);
			client.acquire(Game.sems_names[3]);
			if (resource instanceof Boolean) {
				go = (Boolean) resource;
				setResource(null);
			}
		} while (!go);
					
		getOwnPlayer().setBoard(b);

		gui.waitGameStartAfterPlacement();
		client.write("Received game start");
		client.write("Local ID: " + ownID);

		status = GameStatus.RUNNING;
		while (status == GameStatus.RUNNING) {
			if (status == GameStatus.RUNNING && waitTurn()) {
				if (status == GameStatus.RUNNING) {
					playerTurnMulti();
				}
			}
		}
		
		client.write("Local game end msg");
		if (isServer()) {
			this.destroyAll();
		}
		
		if (status != GameStatus.QUIT) {
			endGame();
		}
	}
	
	@Override
	public void run() {
		// GAME SECONDO IL SERVER
		createPlayersFromConfig(false);
		
		status = GameStatus.PLACING;
		players.values().stream().filter(e -> e instanceof Bot).forEach(b -> ((Bot) b).placeShips());
		players.values().forEach(e -> e.setStatus(PlayerStatus.READY));
		acquire(Game.sems_names[0]);
		
		server.broadcastMatchStart();
		server.write("Game started");
		
		status = GameStatus.RUNNING;
		start = Instant.now();
		
		while (status == GameStatus.RUNNING) {
			players.values().stream().filter(p -> !p.isDead() && !p.didQuit()).forEachOrdered(p -> {
				if (status != GameStatus.RUNNING || p.isDead() || p.didQuit()) {
					return;
				}
				
				turn = p.getID();
				p.setStatus(PlayerStatus.HAS_TURN);
				playerTurnServer(p);
				if (p.getStatus() == PlayerStatus.HAS_TURN) {
					p.setStatus(PlayerStatus.READY);
				}
			});
			
			Set<UUID> quit = new HashSet<>();
			players.values().forEach(e -> {
				if (e.didQuit()) {
					quit.add(e.getID());
				}
			});
			
			quit.forEach(e -> players.remove(e));
		}
		
		if (status == GameStatus.END) {
			end = Instant.now();
			server.broadcastMatchEnd(Duration.between(start, end), turn, players);
		}
	}
	
	public void test() {
		sems.forEach(e -> System.out.println(e));
	}
	
}
