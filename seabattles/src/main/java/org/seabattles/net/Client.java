package org.seabattles.net;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.ObjectOutputStream;
import java.io.PrintWriter;
import java.net.Socket;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.text.ParseException;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

import org.json.JSONObject;
import org.seabattles.main.Main;
import org.seabattles.net.Protocol.MsgType;
import org.seabattles.net.Server.ConnStatus;
import org.seabattles.src.Board;
import org.seabattles.src.Game;
import org.seabattles.src.Game.GameStatus;
import org.seabattles.src.GameConfig;
import org.seabattles.src.Logger;
import org.seabattles.src.Player;
import org.seabattles.src.Player.PlayerStatus;
import org.seabattles.src.Ship;
import org.seabattles.src.ShipConfig;

public class Client implements Runnable {
	
	private Socket sock;
	private BufferedReader in;
	private PrintWriter out;
	
	private boolean interrupted;
	
	private ConnStatus connStatus;
	private String errorMsg;
	
	private Game game;
	
	public Client(String ipAddress, Game game) throws IOException {
		this.game = game;
		
		sock = new Socket(ipAddress, Server.SERVER_PORT);
		in = new BufferedReader(new InputStreamReader(sock.getInputStream()));
		out = new PrintWriter(sock.getOutputStream());
		
		write("Created client - connected to " + sock);
		write("Waiting for ConnStatus");
		
		Object[] obj = waitMsg().orElse(null);
		if (obj == null || !(obj instanceof JSONObject[]) || !connOk((JSONObject) obj[0])) {
			write("Received ConnError, terminating");
			this.destroy();
		}
		
		interrupted = false;
	}
	
	public boolean isConnected() {
		if (sock.isClosed()) {
			game.setErrMsg("Disconnected from the server");
			return false;
		}
		return true;
	}
	
	public void startRecvThread() {
		write("Received ConnOk, starting RecvThread");
		game.createRecvThread();
	}
	
	// GUI INTERFACE
	public ConnStatus getConnStatus() {
		return connStatus;
	}
	
	// GUI INTERFACE
	private String getErrorMsg() {
		return errorMsg;
	}
	
	public void sendSprites(Map<Integer, ShipConfig> ships) throws Exception {
		Map<Integer, String[]> toSend = new HashMap<>();
		for (ShipConfig s : ships.values()) {
			String[] data = new String[2];
			if (!s.getSpritePath().equals(Protocol.NULL)) {
				data[0] = s.getSpritePath();
				data[1] = new String(Base64.getEncoder().encode(Files.readAllBytes(Paths.get(s.getSpritePath()))), "UTF-8");
			} else {
				data[0] = Protocol.NULL;
				data[1] = "";
			}
			toSend.put(s.getID(), data);
		}
		
		sendMsg(Protocol.getSpritesSendMessage(toSend));
	}
	
	private boolean connOk(JSONObject msg) {
		Map<MsgType, String> status = Protocol.parseConnectionMessage(msg);
		if (status.keySet().contains(MsgType.CONN_SUCCESS)) {
			connStatus = Server.ConnStatus.STATUS_OK;
			return true;
		}
		
		if (status.keySet().contains(MsgType.CONN_FULL)) {
			connStatus = Server.ConnStatus.STATUS_FULL;
		}
		
		if (status.keySet().contains(MsgType.CONN_MATCH_STARTED)) {
			connStatus = Server.ConnStatus.STATUS_STARTED;
		}
		
		if (status.keySet().contains(MsgType.CONN_ERR)) {
			connStatus = Server.ConnStatus.STATUS_ERR;
			errorMsg = status.get(MsgType.CONN_ERR);
		}
		
		return false;
	}
	
	public void destroy() {
		try {
			interrupted = true;
			in.close();
			out.close();
			sock.close();
			
			File folder = new File(Main.CLIENT_SPRITE_PATH + "/" + game.getGameID());
			if (folder.exists()) {
				for (File f : folder.listFiles()) {
					f.delete();
				}
				folder.delete();
			}
		} catch (IOException e) {
			e.printStackTrace();
			System.err.println(e);
		}
	}

	public Optional<Object[]> waitMsg() {
		try {
			String m = Server.getMessage(in);
			if (m == null || m.isEmpty() || m.isBlank()) {
				return Optional.empty();
			}
			
			String[] msgs = m.split("\\n");
			JSONObject[] ret = new JSONObject[msgs.length];
			for (int i = 0; i < msgs.length; i++) {
				ret[i] = new JSONObject(msgs[i]);
				write("Received msg: \'" + ret[i] + "\'");
			}
			return Optional.of((Object[]) ret);
		} catch (IOException e) {
			if (!Thread.currentThread().isInterrupted()) {
				destroy();
			}
		}
		return Optional.empty();
	}
	
	public void sendMsg(String msg) {
		if (sock.isClosed()) {
			return;
		}
		write("Sending msg: \'" + msg + "\'");
		Server.sendMsg(out, msg);
	}
	
	public void sendMsg(JSONObject msg) {
		sendMsg(msg.toString());
	}
	
	public void sendMsg(Optional<JSONObject> msg) {
		if (msg.isPresent()) {
			sendMsg(msg.get().toString());
		}
	}

	@Override
	public void run() {
		write("Started RecvThread");
		try {
			while (!interrupted && !Thread.currentThread().isInterrupted()) {
				Optional<Object[]> ret = waitMsg();
				if (ret.isPresent()) {
					Object obj = ret.get();
					if (obj instanceof JSONObject[]) {
						parse((JSONObject[]) obj);
					} else {
						// SPRITES
					}
				}
			}
		} catch (Exception e) {
			interrupted = true;
			destroy();
		}
		
		if (game.getStatus() == GameStatus.CONFIG) {
			game.setStatus(GameStatus.PLACING);
			game.setErrMsg("Disconnected from the server");
		}
	}
	
	// GUI INTERFACE
	public Player getPlayer() {
		return game.getOwnPlayer();
	}
	
	private void parseGameConfig(JSONObject obj) throws IOException, ParseException {
		GameConfig cfg = Protocol.parseConfigurationMessage(obj);
		game.setGameConfig(cfg);
	}
	
	private void parseUserList(JSONObject obj) {
		game.setPlayerList(Protocol.parseUserListMessage(obj));
		game.getGUI().raiseAsyncEvent("userList");
	}
	
	private void parseConnErrMsg(JSONObject msg) {
		game.setErrMsg(Protocol.parseConnErrMsg(msg));
	}
	
	private void parseModMessage(JSONObject msg) {
		game.setErrMsg((Boolean) Protocol.parseModMessage(msg)[1] ? "ban" : "kick");
		this.destroy();
		game.getGUI().errorScreen(game.getErrMsg().equals("Ban") ? "You've been banned from the match" : "Kicked from the server");
		game.setStatus(GameStatus.QUIT);
		game.getGUI().releaseFromInput();
	}
	
	private void sendIDRequest() {
		sendMsg(Protocol.getIDRequestMessage());
	}
	
	private void parseIDSend(JSONObject msg) {
		game.setOwnID(Protocol.parseIDSendMessage(msg));
	}
	
	private void setTurn(JSONObject msg) {
		UUID who_turn = Protocol.parseTurnMessage(msg);
		game.setTurn(who_turn);
		
		if (game.getOwnID().equals(who_turn)) {
			game.flipGenericStatus();
			game.getGUI().setWaitingTurn(false);
			game.getGUI().raiseAsyncEvent("playerTurn");
		} else {
			game.getGUI().setWaitingTurn(true);
		}
	}
	
	private void parseBoard(JSONObject msg) {
		byte[][] board = Protocol.parseBoardMessage(msg);
		if (board[0][0] == -1) {
			game.setErrMsg("Client not available");
		} else {
			try {
				ByteArrayOutputStream out = new ByteArrayOutputStream();
				ObjectOutputStream stream = new ObjectOutputStream(out);
				stream.writeObject(board);
				stream.close();
				game.setResource((Object) out.toByteArray());
			} catch (IOException e) {}
		}
	}
	
	private void parseGotAttacked(JSONObject msg) {
		Object[] ret = Protocol.parseGotAttackedMessage(msg);
		
		UUID from = (UUID) ret[0];
		byte[][] board = (byte[][]) ret[1];
		
		game.getOwnPlayer().getBoard().setBoardMatrix(board);
		
		game.getGUI().raiseAsyncEvent("gotAttacked");
	}
	
	private void parseShipSunk(JSONObject msg) {
		Object[] ret = Protocol.parseShipSunkMessage(msg);
		
		UUID who = (UUID) ret[0];
		Ship s = game.getPlayer(who).get().getBoard().getShips().get((Integer) ret[1]);
		s.setX((Integer) ret[2]);
		s.setY((Integer) ret[3]);
		s.rotate((Integer) ret[4]);
	}
	
	private void parseMatchEnd(JSONObject msg) {
		Object[] ret = Protocol.parseMatchEndMessage(msg);
		game.setStatus(GameStatus.END);
		game.setResource((Object) ret);
		game.flipGenericStatus();
		
		game.getGUI().raiseAsyncEvent("gameEnd");
	}
	
	private void parsePlayerQuit(JSONObject msg) {
		UUID who = Protocol.parseLeftMessage(msg);
		game.removePlayer(who);
		
		game.getGUI().raiseAsyncEvent("playerQuit");
	}
	
	private void parseMatchHostOk(JSONObject msg) {
		boolean error = Protocol.parseMatchHostOkMessage(msg);
		if (!error) {
			game.setResource(0);
		} else {
			game.setResource(1);
		}
		release(Game.sems_names[2]);
	}
	
	private void handleChat(JSONObject msg) {
		Object[] ret = Protocol.parseChatRecvMessage(msg);
		UUID from = (UUID) ret[0];
		String message = (String) ret[1];
		
		game.getGUI().addChatMessage(from, message);
	}
	
	private void parseUserNameAccept(JSONObject msg) {
		game.setOwnID(Protocol.parseUserNameAcceptMessage(msg));
	}
	
	private void handleElimination(JSONObject msg) {
		UUID id = Protocol.parsePlayerEliminationMessage(msg);
		game.getPlayer(id).ifPresent(e -> e.setStatus(PlayerStatus.LOSER));
		
		game.getGUI().raiseAsyncEvent("playerElimination");
	}
	
	private void handleEliminated(JSONObject msg) {
		UUID by = Protocol.parseEliminatedMessage(msg);
		
		game.getOwnPlayer().setStatus(PlayerStatus.LOSER);
		
		game.getGUI().releaseFromInput();
	}
	
	private void getSprites(JSONObject msg) {
		Map<Integer, String[]> data = Protocol.parseSpritesSendMessage(msg);
		String path = Main.CLIENT_SPRITE_PATH + "/" + game.getGameID();
		File folder = new File(path);
		if (!folder.exists()) {
			folder.mkdirs();
		}
		
		data.forEach((i, d) -> {
			if (d[1].getBytes().length > 0) {
				try (FileOutputStream out = new FileOutputStream(path + "/" + i)) {
					byte[] dec = Base64.getDecoder().decode(d[1].getBytes());
					out.write(dec);
					game.getGameConfig().getShipsConfig().get(i).setSprite(path + "/" + i);
				} catch (Exception e) {}
			}
		});
	}
	
	private void parse(JSONObject[] msgs) {
		for (JSONObject msg : msgs) {
			try {
				MsgType type = Protocol.getMessageType(msg);
				switch (type) {
					case CONN_SUCCESS:
						Thread.sleep(100);
						sendIDRequest();
						break;
					case CONN_FULL:
						game.setErrMsg("Server full");
						this.destroy();
						break;
					case CONN_MATCH_STARTED:
						game.setErrMsg("Match has already started");
						this.destroy();
						break;
					case CONN_ERR:
						parseConnErrMsg(msg);
						this.destroy();
						break;
					case USER_NAME_ACCEPT:
						parseUserNameAccept(msg);
						release(Game.sems_names[1]);
						break;
					case CONFIG_BOARD_OK:
						game.setResource(new Boolean(true));
						release(Game.sems_names[3]);
						break;
					case CONFIG_BOARD_ERR:
						game.setResource(new Boolean(false));
						release(Game.sems_names[3]);
					case USER_LIST:
						parseUserList(msg);
						break;
					case ID_SEND:
						parseIDSend(msg);
						break;
					case MATCH_HOST_OK:
						parseMatchHostOk(msg);
						break;
					case MATCH_NOT_HOST:
						game.setResource(2);
						game.setErrMsg("!host");
						release(Game.sems_names[2]);
						break;
					case CONFIG_HOST_ACCEPT:
						game.setResource(new Boolean(true));
						release(Game.sems_names[2]);
						break;
					case MOD_EXECUTED:
						release(Game.sems_names[2]);
						break;
					case CONFIG:
						parseGameConfig(msg);
						break;
					case MOD:
						parseModMessage(msg);
						break;
					case MATCH_PLCM_STARTED:
						game.setStatus(GameStatus.PLACING);
						release(Game.sems_names[1]);
						game.getGUI().releaseFromInput();
						break;
					case SPRITES_SEND:
						getSprites(msg);
						break;
					case MATCH_START:
						release(Game.sems_names[1]);
						break;
					case TURN:
						setTurn(msg);
						break;
					case BOARD:
						parseBoard(msg);
						release(Game.sems_names[1]);
						break;
					case ATTACK_STATUS:
						game.setResource(Protocol.parseAttackStatusMessage(msg));
						release(Game.sems_names[1]);
						break;
					case GOT_ATTACKED:
						parseGotAttacked(msg);
						break;
					case SHIP_SUNK:
						parseShipSunk(msg);
						break;
					case MATCH_END:
						parseMatchEnd(msg);
						break;
					case LEFT:
						parsePlayerQuit(msg);
						break;
					case CHAT_RECV:
						handleChat(msg);
						break;
					case ELIMINATED:
						handleEliminated(msg);
						break;
					case PLAYER_ELIMINATION:
						handleElimination(msg);
						break;
					case ERROR:
						game.setErrMsg(Protocol.parseErrorMessage(msg));
						game.getGUI().errorScreen(game.getErrMsg());
						game.setStatus(GameStatus.QUIT);
						game.flipGenericStatus();
						game.getGUI().releaseFromInput();
						break;
					default:
						break;
				}
			} catch (Exception e) {
				this.destroy();
			}
		}
	}
	
	public void acquire(String name) {
		game.acquire(name);
	}
	
	public void release(String name) {
		game.release(name);
	}
	
	public void sendConfiguration(GameConfig config) {
		sendMsg(Protocol.getConfigurationMessage(config));
	}
	
	public void sendUsername(String userName) {
		sendMsg(Protocol.getUserNameMessage(userName));
	}
	
	// GUI INTERFACE
	public void sendKick(UUID id) {
		sendMsg(Protocol.getModMessage(id, false));
	}
	
	// GUI INTERFACE
	public void sendBan(UUID id) {
		sendMsg(Protocol.getModMessage(id, true));
	}
	
	// GUI INTERFACE
	public void sendStart() {
		sendMsg(Protocol.getMatchPlcmStartMessage());
	}

	// GUI INTERFACE
	public boolean sendBoard(Board board) {
		Optional<JSONObject> ret = Protocol.getConfigBoardMessage(board);
		if (ret.isPresent()) {
			sendMsg(ret.get());
			return true;
		}
		return false;
	}
	
	// GUI INTERFACE
	public void sendBoardRequest(UUID id, String debug) {
		sendMsg(Protocol.getBoardRequestMessage(id, debug));
	}
	
	// GUI INTERFACE
	public void sendAttack(UUID id, int x, int y) {
		sendMsg(Protocol.getAttackMessage(id, x, y));
	}
	
	// GUI INTERFACE
	public void sendQuit() {
		sendMsg(Protocol.getQuitMessage());
	}
	
	// GUI INTERFACE
	public void sendChatMessage(String msg) {
		if (msg == null || msg.isEmpty() || msg.trim().strip().isEmpty()) {
			return;
		}
		sendMsg(Protocol.getChatSendMessage(msg));
	}
	
	public void write(String msg) {
		Logger.write("[LOCAL CLIENT] " + msg);
	}
	
}
