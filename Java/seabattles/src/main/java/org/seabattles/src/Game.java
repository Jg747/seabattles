package org.seabattles.src;

import java.io.File;
import java.io.FileNotFoundException;
import java.text.ParseException;
import java.time.Duration;
import java.time.Instant;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.function.Predicate;
import java.util.stream.Collectors;

import org.json.JSONException;
import org.seabattles.interfaces.GUI;
import org.seabattles.interfaces.GUI.GuiAction;
import org.seabattles.net.Client;
import org.seabattles.net.Server;
import org.seabattles.src.Board.AttackStatus;
import org.seabattles.src.GameConfig.GameDifficulty;
import org.seabattles.src.Player.PlayerStatus;

public class Game implements Runnable {
	
	public enum GameStatus {
		CONFIG,						// Game not started yet
		PLACING,					// placing ships
		RUNNING,					// players playing
		END							// winner chosen
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
	
	public static final int MAX_PLAYERS = 4;
	public static final String DEBUG_STRING = "debug_string";
	public static boolean DEBUG_MODE = true;
	private static final String DEFAULT_USERNAME = "You";
	
	private UUID ownID;
	private UUID turn;				// Contains ID of the player who has the turn
	private GameStatus status;
	private Instant start;
	private Instant end;
	
	private GenericStatus genericStatus;
	private String errMsg;
	private Object resource;
	
 	public Game() {
		config = new GameConfig();
		status = GameStatus.CONFIG;
		genericStatus = GenericStatus.UNDEFINED;
		resource = null;
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
 		// TODO notify GUI that turn changed
 	}
 	
 	// GUI INTERFACE
 	public void waitStatusChange() {
 		genericStatus = GenericStatus.CHANGING;
 		while (genericStatus == GenericStatus.WAITING_MSG) {
 			try {
 				Thread.sleep(500);
 			} catch (InterruptedException e) {
 				genericStatus = GenericStatus.UNDEFINED;
 				break;
 			}
 		}
 	}
 	
 	public GenericStatus getGenericStatus() {
 		return genericStatus;
 	}
 	
 	public void changeGenericStatus() {
 		genericStatus = GenericStatus.UNDEFINED;
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
		
		players = new LinkedHashMap<>();
	}
	
	public UUID addPlayer() {
		Player p = new Player(config.getShipsConfig());
		p.generateBoard();
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
	
	private void createAsHost() {
		server = new Server(this);
		serverThread = new Thread(server);
		serverThread.start();
		
		createAsClient("127.0.0.1");
		
		client.sendConfiguration(config);
	}
	
	// GUI INTERFACE
	public Optional<Player> getPlayer(UUID id) {
		return Optional.ofNullable(players.get(id));
	}
	
	public Set<UUID> getPlayers() {
		return players.keySet();
	}
	
	private void createAsClient(String ipAddress) {
		client = new Client(ipAddress, this);
		recvThread = new Thread(client);
		recvThread.start();
	}
	
	// GUI INTERFACE
	public void create(Optional<String> ipAddress) {
		if (!ipAddress.isPresent()) {
			createAsHost();
		} else {
			createAsClient(ipAddress.get());
		}
		client.sendUsername();
	}
	
	
	public boolean doesExist(UUID id) {
		return players.containsKey(id);
	}
	
	
	public void setGameConfig(GameConfig cfg) {
		config = cfg;
	}
	
	public GameConfig getGameConfig() {
		return config;
	}

	@Override
	public void run() {
		gameStartServer();
	}
	
	private void createPlayersFromConfig() {
		for (int i = 0; i < config.getNumberOfPlayers(); i++) {
			addPlayer();
		}
		
		addBots();
	}
	
	private boolean checkIfWinner() {
		return checkIfWinner(turn);
	}
	
	private boolean checkIfWinner(UUID winner) {
		Set<UUID> alive = players.entrySet().stream().filter(e -> !e.getValue().isDead()).map(Map.Entry::getKey).collect(Collectors.toSet());
		return alive.size() == 1 && alive.contains(winner);
	}
	
	private void gameStartServer() {
		// TODO online game management
	}
	
	private Set<UUID> getPlayersPred(Predicate<Map.Entry<UUID, Player>> pred) {
		return players.entrySet().stream()
			.filter(pred)
			.map(Map.Entry::getKey)
			.collect(Collectors.toSet());
	}
	
	private Set<UUID> getPlayersToNotIgnore() {
		return getPlayersPred(e -> !e.getValue().isDead() && !e.getKey().equals(ownID));
	}
	
	private Set<UUID> getPlayersToIgnore() {
		return getPlayersPred(e -> e.getValue().isDead() || e.getKey().equals(ownID));
	}
	
	private void playerAttack(Set<UUID> attacked) {
		Set<UUID> ignored = getPlayersToIgnore();
		ignored.addAll(attacked);
		Optional<Player> who = gui.getWhoPlayer("Who do you want to attack? ", players, ignored);
		
		do {
			Object[] ret = gui.attack(who);	// ret[0] = UUID, ret[1] = Integer x, ret[2] = Integer y
			if (ret[0] == null) {
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
					players.get(turn).updateStats(selected, as);
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
		Set<UUID> availablePlayers = getPlayersPred(e -> !e.getValue().isDead() && !e.getKey().equals(turn));
		availablePlayers.forEach(u -> {
			Player selected = players.get(u);
			while (status == GameStatus.RUNNING) {
				AttackStatus as = current.attack(selected);
				if (as == AttackStatus.HIT || as == AttackStatus.MISS || as == AttackStatus.SUNK) {
					current.updateStats(selected, as);
					
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
	
	private void playerTurn(Player current) {
		if (current instanceof Bot) {
			playerTurn((Bot) current);
			return;
		}
		
		Set<UUID> attacked = new HashSet<>();
		Set<UUID> availablePlayers = getPlayersToNotIgnore();
		
		while (status == GameStatus.RUNNING) {
			GuiAction act = gui.getAction();
			switch (act) {
				case SHOW_FIELD:
					Optional<Player> who = gui.getWhoPlayer("Who do you want to see (-1 go back)? ", players, getPlayersToIgnore());
					if (who.isPresent()) {
						gui.showField(who.get(), true);
					}
					break;
				case ATTACK:
					playerAttack(attacked);
					if (attacked.containsAll(availablePlayers)) {
						return;
					}
					break;
				case QUIT:
					status = GameStatus.END;
					break;
				default:
					break;
			}
		}
	}
	
	private Object[] getPlayerEndObj() {
		Object[] ret = new Object[3];
		
		ret[0] = new UUID[players.size()];
		ret[1] = new String[players.size()];
		ret[2] = new Stats[players.size()];
		
		int index = 0;
		for (Map.Entry<UUID, Player> e : players.entrySet()) {
			((UUID[]) ret[0])[index] = e.getKey();
			((String[]) ret[1])[index] = e.getValue().getUsername();
			((Stats[]) ret[2])[index] = e.getValue().getStats();
			
			index++;
		}
		
		return ret;
	}
	
	private void gameStart() throws Exception {
		applyConfig();
		createPlayersFromConfig();
		setOwnID(players.values().stream().filter(e -> e instanceof Player).findFirst().get().getID());
		getOwnPlayer().setUsername(DEFAULT_USERNAME);
		
		status = GameStatus.PLACING;
		getOwnPlayer().setBoard(gui.placeShips());
		players.values().stream().filter(e -> e instanceof Bot).forEach(b -> ((Bot) b).placeShips());
		
		status = GameStatus.RUNNING;
		start = Instant.now();
		
		while (status == GameStatus.RUNNING) {
			players.values().stream().filter(p -> !p.isDead()).forEachOrdered(p -> {
				if (status != GameStatus.RUNNING) {
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
		
		end = Instant.now();
		Object[] ret = getPlayerEndObj();
		gui.endScreen(ownID, getOwnPlayer().getStatus(), Duration.between(start, end), (UUID[]) ret[0], (String[]) ret[1], (Stats[]) ret[2]);
	}
	
	public void start() throws Exception {
		/*int mode = gui.getMode();
		if (mode < 2) {
			GameConfig conf;
			try {
				conf = gui.configGame();
			} catch (Exception e) {
				System.err.println("File default di configurazione non trovato, terminazione");
				return;
			}
			setGameConfig(conf);
		}*/
		
		int mode = 0;
		GameConfig conf = new GameConfig();
		conf.setNumberOfBots(2);
		conf.setBotsDifficulty(GameDifficulty.NORMAL);
		setGameConfig(conf);
		conf.applyConfigFile();
		applyConfig();
		
		if (mode == 0) {
			gameStart();
		} else {
			// crea server | get IP e connessione
		}
	}
	
}
