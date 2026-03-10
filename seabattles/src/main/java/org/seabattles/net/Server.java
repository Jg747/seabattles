package org.seabattles.net;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.text.ParseException;
import java.time.Duration;
import java.util.ArrayList;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.json.JSONObject;
import org.seabattles.main.Main;
import org.seabattles.net.Protocol.MsgType;
import org.seabattles.src.Board;
import org.seabattles.src.Board.AttackStatus;
import org.seabattles.src.Bot;
import org.seabattles.src.Game;
import org.seabattles.src.Game.GameStatus;
import org.seabattles.src.GameConfig;
import org.seabattles.src.Logger;
import org.seabattles.src.Player;
import org.seabattles.src.Player.PlayerStatus;
import org.seabattles.src.Ship;
import org.seabattles.src.ShipConfig;
import org.seabattles.src.Stats;

public class Server implements Runnable {
	
	public enum ConnStatus {
		STATUS_OK,
		STATUS_FULL,
		STATUS_STARTED,
		STATUS_ERR
	};

	public static final int SERVER_PORT = 42069;
	public static final int BUF_SIZE = 64;
	
	private static final byte MAX_CLIENTS = Game.MAX_PLAYERS;
	
	private ExecutorService threadsPool;
	private ServerSocket socket;
	private Map<UUID, ClientWorker> clients;
	private ArrayList<InetAddress> banList;
	private UUID host;
	private Game game;
	private boolean stop;
	
	private File spritesFolder;
	
	public static String getMessage(BufferedReader in) throws IOException {
		return in.readLine();
	}
	
	public static void sendMsg(PrintWriter out, String msg) {
		out.println(msg);
		Main.sleep(1); // per qualche ragione il Kernel non sembra fare in tempo a caricare sul socket i dati quindi ho dovuto mettere questa riga, boh
		out.flush();
	}
	
	public Server(Game game) throws IOException {
		socket = new ServerSocket(SERVER_PORT);
		
		this.game = game;
		clients = new HashMap<>();
		stop = false;
		threadsPool = Executors.newFixedThreadPool(MAX_CLIENTS + 1); // + game thread
		banList = new ArrayList<>();
		host = null;
		
		spritesFolder = new File(Main.SERVER_SPRITE_PATH);
		if (!spritesFolder.exists()) {
			spritesFolder.mkdir();
		}
		
		write("Created server!");
	}
	
	public File getSpritesFolder() {
		return spritesFolder;
	}
	
	public synchronized void destroy() {
		if (stop) {
			return;
		}
		
		stop = true;
		game.setStatus(GameStatus.QUIT);
		
		try {
			clients.entrySet().forEach(e -> {
				if (!e.getKey().equals(host)) {
					e.getValue().sendMsg(Protocol.getErrorMessage("Server closed"));
				}
				e.getValue().destroy();
			});
			
			threadsPool.shutdown();
			
			if (socket != null) {
				socket.close();
			}
			
			if (spritesFolder != null) {
				File[] files = spritesFolder.listFiles();
				if (files != null) {
					for (File entry : files) {
						entry.delete();
					}
				}
				spritesFolder.delete();
			}
		} catch (IOException e) {
			e.printStackTrace();
			System.err.println(e.getMessage());
		}
		
		write("Server destroyed");
	}

	private boolean isBanned(Socket sock) {
		return banList.contains(sock.getInetAddress());
	}
	
	private void sendBanMessage(Socket sock) throws IOException {
		PrintWriter out = new PrintWriter(sock.getOutputStream());
		out.println(Protocol.getConnectionMessage(MsgType.CONN_ERR, "You're currently banned from this game").get().toString());
	}
	
	public ClientWorker getWorker(UUID id) {
		return clients.get(id);
	}
	
	@Override
	public void run() {
		try {
			write("Server started");
			while (!stop && !Thread.currentThread().isInterrupted()) {
				Socket client = socket.accept();
				write("Accepted client " + client);
				if (!isBanned(client)) {
					ClientWorker w = new ClientWorker(this, client);
					if (sendConnStatus(w)) {
						UUID u = addPlayer();
						w.setID(u);
						clients.put(u, w);
						threadsPool.execute(clients.get(u));
						
						if (host == null) {
							host = u;
						}
					}
				} else {
					sendBanMessage(client);
					client.close();
				}
			}
		} catch (IOException e) {
			if (!Thread.currentThread().isInterrupted()) {
				Logger.logException(e);
				destroy();
			}
		}
	}
	
	public UUID addPlayer() {
		return game.addPlayer();
	}
	
	private void handleDisconnect(ClientWorker client) {
		client.destroy();
	}
	
	private synchronized void handleDisconnect(UUID id) {
		handleDisconnect(clients.get(id));
		clients.remove(id);
	}
	
	private boolean sendConnStatus(ClientWorker client) {
		JSONObject msg;
		if (game.getStatus() != Game.GameStatus.CONFIG) {
			msg = Protocol.getConnectionMessage(MsgType.CONN_MATCH_STARTED, null).orElse(new JSONObject());
			client.sendMsg(msg);
			handleDisconnect(client);
			return false;
		}
		
		if (clients.size() >= game.getNumberOfPlayers()) {
			msg = Protocol.getConnectionMessage(MsgType.CONN_FULL, null).orElse(new JSONObject());
			client.sendMsg(msg);
			handleDisconnect(client);
			return false;
		}
		
		/* if (false) {
			msg = Protocol.getConnectionMessage(MsgType.CONN_ERR, "Unexpected connection error").orElse(new JSONObject());
			return false;
		} */
		
		msg = Protocol.getConnectionMessage(MsgType.CONN_SUCCESS, null).orElse(new JSONObject());
		client.sendMsg(msg);
		
		return true;
	}
	
	@SuppressWarnings("unused")
	private void broadcast(String msg) {
		if (stop) {
			return;
		}
		
		for (ClientWorker c : clients.values()) {
			c.sendMsg(msg);
		}
	}
	
	private void broadcast(JSONObject msg) {
		if (stop) {
			return;
		}
		
		for (ClientWorker c : clients.values()) {
			c.sendMsg(msg.toString());
		}
	}
	
	private void broadcast(Optional<JSONObject> msg) {
		if (stop) {
			return;
		}
		
		if (msg.isPresent()) {
			for (ClientWorker c : clients.values()) {
				c.sendMsg(msg.get().toString());
			}
		}
	}
	
	public void broadcastShipSunk(Object[] attack) {
		Map<Integer, Ship> arr = game.getPlayer((UUID) attack[0]).get().getBoard().getShips();
		Ship temp = null;
		for (Ship s : arr.values()) {
			if (s.occupiesCoords((Integer) attack[1], (Integer) attack[2])) {
				temp = s;
				break;
			}
		}
		
		if (temp != null) {
			broadcast(Protocol.getShipSunkMessage((UUID) attack[0], temp));
		} else {
			throw new RuntimeException("(broadcast ship sunk) No ship found!");
		}
	}
	
	private void broadcastPlayerList() {
		HashMap<UUID, String> names = new HashMap<>();
		game.getPlayers().forEach(e -> names.put(e, game.getPlayer(e).get().getUsername()));
		broadcast(Protocol.getUserListMessage(names));
	}
	
	private void broadcastConfig(GameConfig conf) {
		broadcast(Protocol.getConfigurationOkMessage(conf));
	}
	
	private void parseUsername(UUID id, JSONObject msg) {
		game.getPlayer(id).orElse(new Player()).setUsername(Protocol.parseUserNameMessage(msg));
		clients.get(id).sendMsg(Protocol.getUserNameAcceptMessage(id));
		broadcastPlayerList();
	}
	
	private void sendMatchNotHost(UUID id) {
		clients.get(id).sendMsg(Protocol.getMatchNotHostMessage());
	}
	
	private void sendHostAccept(UUID id) {
		clients.get(id).sendMsg(Protocol.getConfigHostAcceptMessage());
	}
	
	private void sendHostOk(UUID id, boolean error) {
		clients.get(id).sendMsg(Protocol.getMatchHostOkMessage(error));
	}
	
	private void parseConfigHost(UUID id, JSONObject msg) throws IOException, ParseException {
		if (id == host && game.getStatus() == GameStatus.CONFIG) {
			GameConfig conf = Protocol.parseConfigurationMessage(msg);
			game.setGameConfig(conf);
			sendHostAccept(id);
		} else if (id != host) {
			sendMatchNotHost(id);
		}
	}

	private void parseModMsg(UUID id, JSONObject msg) {
		if (id == host) {
			Object[] mod = Protocol.parseModMessage(msg);
			if (((UUID) mod[0]).equals(host)) {
				sendModExecuted(id);
				return;
			}
			
			if (!game.doesExist((UUID) mod[0])) {
				clients.get(id).sendMsg(Protocol.getErrorMessage("Invalid player ID"));
				sendModExecuted(id);
				return;
			}
			
			if ((Boolean) mod[1]) {
				banList.add(clients.get((UUID) mod[0]).getIP());
			}
			
			clients.get((UUID) mod[0]).sendMsg(Protocol.getModMessage((UUID) mod[0], (Boolean) mod[1]));
			handleDisconnect((UUID) mod[0]);
			
			sendModExecuted(id);
			
			game.removePlayer((UUID) mod[0]);
			broadcastPlayerList();
		} else {
			sendMatchNotHost(id);
		}
	}
	
	private void receiveSprites(JSONObject msg) {
		Map<Integer, String[]> data = Protocol.parseSpritesSendMessage(msg);
		data.forEach((i, d) -> {
			if (d[1].getBytes().length > 0) {
				try (FileOutputStream out = new FileOutputStream(Main.SERVER_SPRITE_PATH + "/" + i)) {
					byte[] dec = Base64.getDecoder().decode(d[1].getBytes());
					out.write(dec);
				} catch (Exception e) {}
			}
		});
	}
	
	private void parseSpritesData(UUID id, JSONObject msg) {
		if (id == host) {
			receiveSprites(msg);			
			sendHostAccept(id);
		} else {
			sendMatchNotHost(id);
		}
	}
	
	private void sendModExecuted(UUID id) {
		clients.get(id).sendMsg(Protocol.getModExecutedMessage());
	}
	
	private void broadcastMatchPlcmStarted() {
		broadcast(Protocol.getMatchPlcmStartedMessage());
	}
	
	private void broadcastSprites() {
		try {
			Map<Integer, String[]> toSend = new HashMap<>();
			for (ShipConfig s : game.getGameConfig().getShipsConfig().values()) {
				String[] data = new String[2];
				data[0] = String.valueOf(s.getID());
				if (new File(Main.SERVER_SPRITE_PATH + "/" + data[0]).exists()) {
					data[1] = new String(Base64.getEncoder().encode(Files.readAllBytes(Paths.get(Main.SERVER_SPRITE_PATH + "/" + data[0]))), "UTF-8");
				} else {
					data[1] = "";
				}
				toSend.put(s.getID(), data);
			}
			
			broadcast(Protocol.getSpritesSendMessage(toSend));
		} catch (Exception e) {}
	}
	
	private void writePlayerList() {
		StringBuilder list = new StringBuilder("----- Player list -----\n");
		for (UUID id : game.getPlayers()) {
			list.append(id).append(id.equals(host) ? " - host" : ((game.getPlayer(id).get() instanceof Bot) ? " - bot" : "")).append("\n");
		}
		write(list.toString());
	}
	
	private void parseMatchPlcmStart(UUID id) {
		synchronized (id) {
			if (id == host && game.getStatus() == GameStatus.CONFIG) {
				if (game.getGameConfig().getNumberOfBots() + clients.size() < 2) {
					sendHostOk(id, true);
					return;
				}
				
				sendHostOk(id, false);
				
				game.getGameConfig().setNumberOfPlayers(clients.size());
				
				broadcastConfig(game.getGameConfig());
				game.addBots();
				broadcastPlayerList();
				broadcastSprites();
				broadcastMatchPlcmStarted();
				
				writePlayerList();
				
				// START GAME THREAD
				game.setServer(this);
				threadsPool.execute(game);
			} else {
				sendMatchNotHost(id);
			}
		}
	}
	
	private void sendConfigBoardOk(UUID id) {
		clients.get(id).sendMsg(Protocol.getConfigBoardOkMessage());
	}
	
	private void sendConfigBoardErr(UUID id) {
		clients.get(id).sendMsg(Protocol.getConfigBoardErrMessage());
	}
	
	// GAME GUI
	public void broadcastTurn(UUID id) {
		broadcast(Protocol.getTurnMessage(id));
	}
	
	private void parseConfigBoard(UUID id, JSONObject msg) {
		Optional<Board> optBoard = Protocol.parseConfigBoardMessage(msg, game.getGameConfig().getShipsConfig());
		if (optBoard.isPresent()) {
			Board b = optBoard.get();
			synchronized (this) {
				if (b.checkAndPlace()) {
					sendConfigBoardOk(id);
					game.getPlayer(id).get().setBoard(b);
					game.checkAllBoardsOk();
				} else {
					sendConfigBoardErr(id);
				}
			}
		} else {
			sendConfigBoardErr(id);
		}
	}

	// GAME GUI
	public void broadcastMatchStart() {
		broadcast(Protocol.getMatchStartMessage());
	}
	
	private void sendBoardMessage(UUID id, JSONObject msg) {
		clients.get(id).sendMsg(msg);
	}
	
	private void parseBoardRequest(UUID id, JSONObject msg) {
		Object[] arr = Protocol.parseBoardRequestMessage(msg);
		boolean debug = false;
		if (((String) arr[1]).equals(game.getGameConfig().getDebugString())) {
			debug = true;
		}
		UUID playerId = (UUID) arr[0];
		
		if (!game.doesExist(playerId) || game.getPlayer(playerId).get().isDead()) {
			clients.get(id).sendMsg(Protocol.getBoardMessage(null)); // Invalid player ID | isDead
			return;
		}
		
		byte[][] board = !debug ? game.getPlayer(playerId).get().getBoard().getObfuscatedHits() : game.getPlayer(playerId).get().getBoard().getHits();
		
		sendBoardMessage(id, Protocol.getBoardMessage(board));
	}
	
	private boolean parseAttackRequest(UUID id, JSONObject msg) {
		Object[] ret = Protocol.parseAttackMessage(msg);
		if (((UUID) ret[0]).equals(id) || !game.doesExist((UUID) ret[0])) {
			clients.get(id).sendMsg(Protocol.getAttackStatusMessage(AttackStatus.ERROR));
			return false;
		}
		
		if (!id.equals(game.getTurn())) {
			clients.get(id).sendMsg(Protocol.getAttackStatusMessage(AttackStatus.NOT_TURN));
			return false;
		}
		
		if (game.getPlayer((UUID) ret[0]).get().isDead()) {
			clients.get(id).sendMsg(Protocol.getAttackStatusMessage(AttackStatus.DEAD));
			return false;
		}

		return game.playerAttackServer(clients.get(id), clients.get((UUID) ret[0]), ret);
	}
	
	// GAME GUI
	public void broadcastMatchEnd(Duration d, UUID winner, Map<UUID, Player> players) {
		UUID[] ids = new UUID[players.size()];
		String[] names = new String[players.size()];
		Stats[] stats = new Stats[players.size()];
		
		int index = 0;
		for (Map.Entry<UUID, Player> entry : players.entrySet()) {
			ids[index] = entry.getKey();
			names[index] = entry.getValue().getUsername();
			stats[index] = entry.getValue().getStats();
			
			index++;
		}
		
		broadcast(Protocol.getMatchEndMessage(d, winner, ids, names, stats));
	}
	
	public void handleQuit(UUID who) {
		game.getPlayer(who).ifPresent(p -> p.setStatus(PlayerStatus.QUIT));
		
		if (!who.equals(host) && clients.containsKey(who)) {
			clients.get(who).destroy();
			clients.remove(who);
		}
		
		long clientCount = clients.values().stream().count();
		if (clientCount == 0) {
			this.destroy();
			return;
		}
		
		if (clientCount == 1 && game.getStatus() != GameStatus.CONFIG) {
			if (game.isEndGameQuit()) {
				game.setStatus(GameStatus.END);
				game.setWinner();
			}
		}
		
		broadcast(Protocol.getLeftMessage(who));
		
		if (who.equals(game.getTurn())) {
			game.release(Game.sems_names[0]);
		}
	}
	
	private void parseChatMessage(UUID id, JSONObject msg) {
		broadcast(Protocol.getChatRecvMessage(id, Protocol.parseChatSendMessage(msg)));
	}
	
	public void broadcastPlayerElimination(UUID id) {
		broadcast(Protocol.getPlayerEliminationMessage(id));
	}
	
	public void parse(UUID id, JSONObject[] msgs) {
		for (JSONObject msg : msgs) {
			try {
				MsgType type = Protocol.getMessageType(msg);
				switch (type) {
					case USER_NAME:
						parseUsername(id, msg);
						break;
					case CONFIG_HOST:
						parseConfigHost(id, msg);
						break;
					case MOD:
						parseModMsg(id, msg);
						break;
					case MATCH_PLCM_START:
						parseMatchPlcmStart(id);
						break;
					case CONFIG_BOARD:
						parseConfigBoard(id, msg);
						break;
					case BOARD_REQUEST:
						parseBoardRequest(id, msg);
						break;
					case ATTACK:
						if (parseAttackRequest(id, msg)) {
							if (game.getAttacked().containsAll(game.getAvailablePlayers())) {
								write("player " + id + " attacked all players");
								game.release(Game.sems_names[0]);
							}
						}
						break;
					case QUIT:
						handleQuit(id);
						break;
					case CHAT_SEND:
						parseChatMessage(id, msg);
						break;
					case SPRITES_SEND:
						parseSpritesData(id, msg);
						break;
					default:
						break;
				}
			} catch (Exception e) {
				Logger.logException(e);
				Logger.logStackTrace("Exception thrown", "");
				clients.get(id).sendMsg(Protocol.getErrorMessage("An unexpected error occurred while parsing the message"));
			}
		}
	}
	
	public void write(String msg) {
		Logger.write("[SERVER] " + msg);
	}
	
	public void sendError(ClientWorker w) {
		w.sendMsg(Protocol.getErrorMessage("An unexpected error occurred, you got disconnected"));
	}

}
