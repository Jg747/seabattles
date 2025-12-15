package org.seabattles.net;

import java.nio.file.Paths;
import java.text.ParseException;
import java.time.Duration;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

import org.json.JSONArray;
import org.json.JSONObject;
import org.seabattles.src.Board;
import org.seabattles.src.Board.AttackStatus;
import org.seabattles.src.GameConfig;
import org.seabattles.src.Logger;
import org.seabattles.src.GameConfig.GameDifficulty;
import org.seabattles.src.Player.PlayerGrade;
import org.seabattles.src.Ship;
import org.seabattles.src.ShipConfig;
import org.seabattles.src.Stats;

public abstract class Protocol {
	
	public static final String NULL = "null";
	
	public enum MsgType {
		CONN_SUCCESS,
		CONN_FULL,
		CONN_MATCH_STARTED,
		CONN_ERR,
		
		CONFIG_HOST,
		CONFIG_HOST_ACCEPT,
		CONFIG,
		CONFIG_BOARD,
		CONFIG_BOARD_OK,
		CONFIG_BOARD_ERR,
		
		USER_NAME,
		USER_NAME_ACCEPT,
		USER_LIST,
		ID_REQUEST,
		ID_SEND,
		
		MOD,
		MOD_EXECUTED,
		
		MATCH_PLCM_START,
		MATCH_PLCM_STARTED,
		MATCH_NOT_HOST,
		MATCH_START,
		MATCH_END,
		
		TURN,
		
		BOARD_REQUEST,
		BOARD,
		
		ATTACK,
		ATTACK_STATUS,
		GOT_ATTACKED,
		
		ELIMINATED,
		
		QUIT,
		LEFT,
		
		CHAT_SEND,
		CHAT_RECV,
		
		ERROR,
		SPRITES_SEND
	};
	
	private enum MsgString {
		TYPE,
		MSG,
		CFG,
		BOARD,
		SHIPS,
		ID,
		WIDTH,
		HEIGTH,
		LENGTH,
		SPRITE,
		SPRITES,
		PLAYERCOUNT,
		BOTSCOUNT,
		GAMEDIFFICULTY,
		LIST,
		NAME,
		SUBTYPE,
		KICK,
		BAN,
		X,
		Y,
		R,
		WHO,
		DEBUG,
		ATTACK_OK,
		ATTACK_ERROR,
		STATUS,
		FROM,
		NEW_BOARD,
		DURATION,
		PLAYERS,
		STATS,
		SHOTS,
		HITS,
		SUNK,
		ELIMINATIONS,
		GRADE,
		BY
	};
	
	private static final Map<MsgString, String> strings = Map.ofEntries(
		Map.entry(MsgString.TYPE, "type"),
		Map.entry(MsgString.MSG, "msg"),
		Map.entry(MsgString.NAME, "name"),
		Map.entry(MsgString.CFG, "cfg"),
		Map.entry(MsgString.BOARD, "board"),
		Map.entry(MsgString.SHIPS, "ships"),
		Map.entry(MsgString.ID, "id"),
		Map.entry(MsgString.WIDTH, "width"),
		Map.entry(MsgString.HEIGTH, "heigth"),
		Map.entry(MsgString.LENGTH, "length"),
		Map.entry(MsgString.SPRITE, "sprite"),
		Map.entry(MsgString.SPRITES, "sprites"),
		Map.entry(MsgString.PLAYERCOUNT, "playerCount"),
		Map.entry(MsgString.BOTSCOUNT, "botsCount"),
		Map.entry(MsgString.GAMEDIFFICULTY, "gameDifficulty"),
		Map.entry(MsgString.LIST, "list"),
		Map.entry(MsgString.SUBTYPE, "subtype"),
		Map.entry(MsgString.KICK, "kick"),
		Map.entry(MsgString.BAN, "ban"),
		Map.entry(MsgString.X, "x"),
		Map.entry(MsgString.Y, "y"),
		Map.entry(MsgString.R, "r"),
		Map.entry(MsgString.WHO, "who"),
		Map.entry(MsgString.DEBUG, "debug"),
		Map.entry(MsgString.ATTACK_OK, "attack_ok"),
		Map.entry(MsgString.ATTACK_ERROR, "attack_error"),
		Map.entry(MsgString.STATUS, "status"),
		Map.entry(MsgString.FROM, "from"),
		Map.entry(MsgString.NEW_BOARD, "new_board"),
		Map.entry(MsgString.DURATION, "duration"),
		Map.entry(MsgString.PLAYERS, "players"),
		Map.entry(MsgString.STATS, "stats"),
		Map.entry(MsgString.SHOTS, "shots"),
		Map.entry(MsgString.HITS, "hits"),
		Map.entry(MsgString.SUNK, "sunk"),
		Map.entry(MsgString.ELIMINATIONS, "eliminations"),
		Map.entry(MsgString.GRADE, "grade"),
		Map.entry(MsgString.BY, "by")
	);
	
	private static final Map<MsgType, String> types = Map.ofEntries(
			Map.entry(MsgType.CONN_SUCCESS, "conn_success"),
			Map.entry(MsgType.CONN_FULL, "conn_full"),
			Map.entry(MsgType.CONN_MATCH_STARTED, "conn_match_started"),
			Map.entry(MsgType.CONN_ERR, "conn_err"),
			Map.entry(MsgType.CONFIG_HOST, "config_host"),
			Map.entry(MsgType.CONFIG_HOST_ACCEPT, "config_host_accept"),
			Map.entry(MsgType.CONFIG, "config"),
			Map.entry(MsgType.CONFIG_BOARD, "config_board"),
			Map.entry(MsgType.CONFIG_BOARD_OK, "config_board_ok"),
			Map.entry(MsgType.CONFIG_BOARD_ERR, "config_board_err"),
			Map.entry(MsgType.USER_NAME, "user_name"),
			Map.entry(MsgType.USER_NAME_ACCEPT, "user_name_accept"),
			Map.entry(MsgType.USER_LIST, "user_list"),
			Map.entry(MsgType.ID_REQUEST, "id_request"),
			Map.entry(MsgType.ID_SEND, "id_send"),
			Map.entry(MsgType.MOD, "mod"),
			Map.entry(MsgType.MOD_EXECUTED, "mod_executed"),
			Map.entry(MsgType.MATCH_PLCM_START, "match_plcm_start"),
			Map.entry(MsgType.MATCH_PLCM_STARTED, "match_plcm_started"),
			Map.entry(MsgType.MATCH_NOT_HOST, "match_not_host"),
			Map.entry(MsgType.MATCH_START, "match_start"),
			Map.entry(MsgType.MATCH_END, "match_end"),
			Map.entry(MsgType.TURN, "turn"),
			Map.entry(MsgType.BOARD_REQUEST, "board_request"),
			Map.entry(MsgType.BOARD, "board"),
			Map.entry(MsgType.ATTACK, "attack"),
			Map.entry(MsgType.ATTACK_STATUS, "attack_status"),
			Map.entry(MsgType.GOT_ATTACKED, "got_attacked"),
			Map.entry(MsgType.ELIMINATED, "eliminated"),
			Map.entry(MsgType.QUIT, "quit"),
			Map.entry(MsgType.LEFT, "left"),
			Map.entry(MsgType.CHAT_SEND, "chat_send"),
			Map.entry(MsgType.CHAT_RECV, "chat_recv"),
			Map.entry(MsgType.ERROR, "error"),
			Map.entry(MsgType.SPRITES_SEND, "sprites_send")
	);
	
	public static MsgType getMessageType(JSONObject msg) {
		String type = msg.getString(strings.get(MsgString.TYPE));
		for (MsgType key : types.keySet()) {
			if (types.get(key).equals(type)) {
				return key;
			}
		}
		return null;
	}
	
	public static Optional<JSONObject> getConnectionMessage(MsgType type, String msg) {
		JSONObject obj;
		switch (type) {
			case CONN_SUCCESS:
			case CONN_FULL:
			case CONN_MATCH_STARTED:
				obj = new JSONObject();
				obj.put(strings.get(MsgString.TYPE), types.get(type));
				break;
			case CONN_ERR:
				obj = new JSONObject();
				obj.put(strings.get(MsgString.TYPE), types.get(type));
				obj.put(strings.get(MsgString.MSG), (msg == null || msg.isEmpty()) ? "Unknown error" : msg);
				break;
			default:
				return Optional.empty();
		}
		return Optional.of(obj);
	}
	
	public static String parseConnErrMsg(JSONObject msg) {
		return msg.getString(strings.get(MsgString.MSG));
	}
	
	public static Map<MsgType, String> parseConnectionMessage(JSONObject msg) {
		return Map.of(Protocol.getMessageType(msg), msg.keySet().contains(strings.get(MsgString.MSG)) ? msg.getString(strings.get(MsgString.MSG)) : "null");
	}
	
	public static Optional<JSONObject> getUserNameMessage(String name) {
		JSONObject obj = new JSONObject();
		obj.put(strings.get(MsgString.TYPE), types.get(MsgType.USER_NAME));
		obj.put(strings.get(MsgString.MSG), name);
		return Optional.of(obj);
	}
	
	public static String parseUserNameMessage(JSONObject obj) {
		return obj.getString(strings.get(MsgString.MSG));
	}
	
	public static JSONObject getUserNameAcceptMessage(UUID id) {
		JSONObject obj = new JSONObject();
		obj.put(strings.get(MsgString.TYPE), types.get(MsgType.USER_NAME_ACCEPT));
		obj.put(strings.get(MsgString.ID), id.toString());
		return obj;
	}
	
	public static UUID parseUserNameAcceptMessage(JSONObject obj) {
		return UUID.fromString(obj.getString(strings.get(MsgString.ID)));
	}
	
	private static Optional<JSONObject> putConfigInObj(JSONObject obj, GameConfig config) {
		JSONObject cfg = new JSONObject();
		int diff = 0;
		switch (config.getBotsDifficulty()) {
			case HARD:
				diff = 1;
				break;
			case IMPOSSIBLE:
				diff = 2;
				break;
			default:
				break;
		}
		cfg.put(strings.get(MsgString.GAMEDIFFICULTY), diff);
		cfg.put(strings.get(MsgString.PLAYERCOUNT), config.getNumberOfPlayers());
		cfg.put(strings.get(MsgString.BOTSCOUNT), config.getNumberOfBots());
		
		JSONObject board = new JSONObject();
		board.put(strings.get(MsgString.WIDTH), config.getBoardWidth());
		board.put(strings.get(MsgString.HEIGTH), config.getBoardHeigth());
		
		JSONArray ships = new JSONArray();
		for (ShipConfig s : config.getShipsConfig()) {
			JSONObject shipObj = new JSONObject();
			shipObj.put(strings.get(MsgString.ID), s.getID());
			shipObj.put(strings.get(MsgString.LENGTH), s.getLength());
			shipObj.put(strings.get(MsgString.WIDTH), s.getWidth());
			shipObj.put(strings.get(MsgString.SPRITE), Paths.get(s.getSpritePath()).getFileName().toString());
			
			ships.put(shipObj);
		}
		
		cfg.put(strings.get(MsgString.SHIPS), ships);
		cfg.put(strings.get(MsgString.BOARD), board);
		
		obj.put(strings.get(MsgString.CFG), cfg);
		return Optional.of(obj);
	}
	
	public static Optional<JSONObject> getConfigurationMessage(GameConfig config) {
		JSONObject obj = new JSONObject();
		obj.put(strings.get(MsgString.TYPE), types.get(MsgType.CONFIG_HOST));
		return putConfigInObj(obj, config);
	}
	
	public static Optional<JSONObject> getConfigurationOkMessage(GameConfig config) {
		JSONObject obj = new JSONObject();
		obj.put(strings.get(MsgString.TYPE), types.get(MsgType.CONFIG));
		return putConfigInObj(obj, config);
	}
	
	public static GameConfig parseConfigurationMessage(JSONObject msg) throws RuntimeException, ParseException {
		GameConfig ret = new GameConfig();
		
		JSONObject cfg = msg.getJSONObject(strings.get(MsgString.CFG));
		switch (cfg.getInt(strings.get(MsgString.GAMEDIFFICULTY))) {
			case 0:
				ret.setBotsDifficulty(GameDifficulty.NORMAL);
				break;
			case 1:
				ret.setBotsDifficulty(GameDifficulty.HARD);
				break;
			case 2:
				ret.setBotsDifficulty(GameDifficulty.IMPOSSIBLE);
				break;
			default:
				throw new ParseException("", 0);
		}
		
		ret.setNumberOfPlayers(cfg.getInt(strings.get(MsgString.PLAYERCOUNT)));
		ret.setNumberOfBots(cfg.getInt(strings.get(MsgString.BOTSCOUNT)));
		
		ret.setBoardWidth(cfg.getJSONObject(strings.get(MsgString.BOARD)).getInt(strings.get(MsgString.WIDTH)));
		ret.setBoardHeigth(cfg.getJSONObject(strings.get(MsgString.BOARD)).getInt(strings.get(MsgString.HEIGTH)));
		
		JSONArray shipsArr = cfg.getJSONArray(strings.get(MsgString.SHIPS));
		ShipConfig[] ships = new ShipConfig[shipsArr.length()];
		for (int i = 0; i < shipsArr.length(); i++) {
			ships[i] = new ShipConfig(shipsArr.getJSONObject(i));
			if (!ships[i].getSpritePath().equals(NULL)) {
				ships[i].setSprite(Server.SPRITE_PATH + "/" + ships[i].getSpritePath());
			}
		}
		ret.setShipsConfigArray(ships);
		
		return ret;
	}
	
	public static JSONObject getErrorMessage(String msg) {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.ERROR));
		ret.put(strings.get(MsgString.MSG), msg);
		return ret;
	}
	
	public static String parseErrorMessage(JSONObject msg) {
		return msg.getString(strings.get(MsgString.MSG));
	}
	
	public static JSONObject getMatchNotHostMessage() {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.MATCH_NOT_HOST));
		return ret;
	}
	
	public static JSONObject getConfigHostAcceptMessage() {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.CONFIG_HOST_ACCEPT));
		return ret;
	}
	
	public static JSONObject getUserListMessage(Map<UUID, String> names) {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.USER_LIST));
		
		JSONArray arr = new JSONArray();
		for (UUID key : names.keySet()) {
			JSONObject tmp = new JSONObject();
			tmp.put(strings.get(MsgString.ID), key.toString());
			tmp.put(strings.get(MsgString.NAME), names.get(key));
			arr.put(tmp);
		}
		
		ret.put(strings.get(MsgString.LIST), arr);
		return ret;
	}
	
	public static HashMap<UUID, String> parseUserListMessage(JSONObject msg) {
		HashMap<UUID, String> ret = new HashMap<>();
		
		JSONArray arr = msg.getJSONArray(strings.get(MsgString.LIST));
		for (Object obj : arr) {
			ret.put(UUID.fromString(((JSONObject) obj).getString(strings.get(MsgString.ID))), ((JSONObject) obj).getString(strings.get(MsgString.NAME)));
		}
		
		return ret;
	}
	
	public static JSONObject getModMessage(UUID id, boolean ban) {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.MOD));
		ret.put(strings.get(MsgString.ID), id.toString());
		
		if (!ban) {
			// kick
			ret.put(strings.get(MsgString.SUBTYPE), strings.get(MsgString.KICK));
		} else {
			// ban
			ret.put(strings.get(MsgString.SUBTYPE), strings.get(MsgString.BAN));
		}
		return ret;
	}
	
	public static Object[] parseModMessage(JSONObject msg) {
		Object[] ret = new Object[2];
		ret[0] = UUID.fromString(msg.getString(strings.get(MsgString.ID)));
		ret[1] = msg.getString(strings.get(MsgString.SUBTYPE)).equals(strings.get(MsgString.KICK)) ? false : true;
		return ret;
	}
	
	public static JSONObject getModExecutedMessage() {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.MOD_EXECUTED));
		return ret;
	}
	
	public static JSONObject getMatchPlcmStartMessage() {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.MATCH_PLCM_START));
		return ret;
	}
	
	public static JSONObject getMatchPlcmStartedMessage() {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.MATCH_PLCM_STARTED));
		return ret;
	}
	
	public static Optional<JSONObject> getConfigBoardMessage(Board board) {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.CONFIG_BOARD));
		
		JSONArray arr = new JSONArray();
		Ship[] ships = board.getShips();
		for (Ship s : ships) {
			JSONObject obj = new JSONObject();
			obj.put(strings.get(MsgString.ID), s.getID());
			obj.put(strings.get(MsgString.X), s.getX());
			obj.put(strings.get(MsgString.Y), s.getY());
			obj.put(strings.get(MsgString.R), s.getRotation());
			arr.put(obj);
		}
		ret.put(strings.get(MsgString.SHIPS), arr);
		
		return Optional.of(ret);
	}
	
	public static Optional<Board> parseConfigBoardMessage(JSONObject msg, ShipConfig[] cfg) {
		Board ret = new Board(cfg);
		
		JSONArray arr = msg.getJSONArray(strings.get(MsgString.SHIPS));
		if (arr.length() != cfg.length) {
			return Optional.empty();
		}
		
		Ship[] ships = ret.getShips();
		for (Object o : arr) {
			int id = ((JSONObject) o).getInt(strings.get(MsgString.ID)) - 1;
			ships[id].setX(((JSONObject) o).getInt(strings.get(MsgString.X)));
			ships[id].setY(((JSONObject) o).getInt(strings.get(MsgString.Y)));
			ships[id].rotate(((JSONObject) o).getInt(strings.get(MsgString.R)));
		}
		
		return Optional.of(ret);
	}
	
	public static JSONObject getSpritesSendMessage(HashMap<Integer, String> sprites) {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.SPRITES_SEND));
		
		JSONArray arr = new JSONArray();
		for (Integer key : sprites.keySet()) {
			JSONObject obj = new JSONObject();
			obj.put(strings.get(MsgString.ID), key);
			obj.put(strings.get(MsgString.NAME), (String) sprites.get(key));
			
			arr.put(obj);
		}
		
		ret.put(strings.get(MsgString.SPRITES), arr);
		
		return ret;
	}
	
	public static HashMap<Integer, String> parseSpritesSendMessage(JSONObject msg) {
		HashMap<Integer, String> ret = new HashMap<>();
		JSONArray arr = msg.getJSONArray(strings.get(MsgString.SPRITES));
		for (Object o : arr) {
			ret.put(((JSONObject) o).getInt(strings.get(MsgString.ID)), ((JSONObject) o).getString(strings.get(MsgString.NAME)));
		}
		return ret;
	}
	
	public static JSONObject getIDRequestMessage() {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.ID_REQUEST));
		return ret;
	}
	
	public static JSONObject getIDSendMessage(UUID id) {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.ID_SEND));
		ret.put(strings.get(MsgString.ID), id.toString());
		return ret;
	}
	
	public static UUID parseIDSendMessage(JSONObject msg) {
		return UUID.fromString(msg.getString(strings.get(MsgString.ID)));
	}
	
	public static JSONObject getConfigBoardOkMessage() {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.CONFIG_BOARD_OK));
		return ret;
	}
	
	public static JSONObject getConfigBoardErrMessage() {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.CONFIG_BOARD_ERR));
		return ret;
	}
	
	public static JSONObject getMatchStartMessage() {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.MATCH_START));
		return ret;
	}
	
	public static JSONObject getTurnMessage(UUID id) {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.TURN));
		ret.put(strings.get(MsgString.WHO), id.toString());
		return ret;
	}
	
	public static UUID parseTurnMessage(JSONObject msg) {
		return UUID.fromString(msg.getString(strings.get(MsgString.WHO)));
	}
	
	public static JSONObject getBoardRequestMessage(UUID id, String debug) {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.BOARD_REQUEST));
		ret.put(strings.get(MsgString.ID), id.toString());
		ret.put(strings.get(MsgString.DEBUG), debug == null ? NULL : debug);
		return ret;
	}
	
	public static Object[] parseBoardRequestMessage(JSONObject msg) {
		Object[] ret = new Object[2];
		ret[0] = UUID.fromString(msg.getString(strings.get(MsgString.ID)));
		ret[1] = msg.getString(strings.get(MsgString.DEBUG));
		return ret;
	}
	
	public static JSONObject getBoardMessage() {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.BOARD));
		ret.put(strings.get(MsgString.BOARD), new JSONArray());
		
		return ret;
	}
	
	public static JSONObject getBoardMessage(byte[][] board) {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.BOARD));
		
		int[] arr = new int[board.length * board[0].length];
		int index = 0;
		for (int i = 0; i < board.length; i++) {
			for (int j = 0; j < board[i].length; j++) {
				arr[index] = board[i][j];
				index++;
			}
		}
		
		ret.put(strings.get(MsgString.BOARD), arr);
		
		return ret;
	}
	
	public static byte[][] parseBoardMessage(JSONObject msg) {
		JSONArray arr = msg.getJSONArray(strings.get(MsgString.BOARD));
		byte[][] ret = new byte[Board.getHeigth()][Board.getWidth()];
		
		if (arr.length() == 0) {
			ret[0][0] = -1;
			return ret;
		}
		
		int i = 0, j = 0;
		for (Object o : arr) {
			ret[i][j] = (Byte) o;
			j = (j + 1) % Board.getWidth();
			if (j == 0) {
				i++;
			}
		}
		
		return ret;
	}
	
	public static JSONObject getAttackMessage(UUID id, int x, int y) {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.ATTACK));
		ret.put(strings.get(MsgString.ID), id.toString());
		ret.put(strings.get(MsgString.X), x);
		ret.put(strings.get(MsgString.Y), y);
		return ret;
	}
	
	public static Object[] parseAttackMessage(JSONObject msg) {
		Object[] ret = new Object[3];
		ret[0] = UUID.fromString(msg.getString(strings.get(MsgString.ID)));
		ret[1] = msg.getInt(strings.get(MsgString.X));
		ret[2] = msg.getInt(strings.get(MsgString.Y));
		return ret;
	}
	
	public static JSONObject getAttackStatusMessage(AttackStatus status) {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.ATTACK_STATUS));
		
		switch (status) {
			case HIT:
			case MISS:
			case SUNK:
				ret.put(strings.get(MsgString.SUBTYPE), strings.get(MsgString.ATTACK_OK));
				break;
			case INVALID:
			case NOT_TURN:
			case DEAD:
			case ERROR:
				ret.put(strings.get(MsgString.SUBTYPE), strings.get(MsgString.ATTACK_ERROR));
				break;
			default:
				ret.put(strings.get(MsgString.SUBTYPE), strings.get(MsgString.ATTACK_ERROR));
				break;
		}
		
		ret.put(strings.get(MsgString.STATUS), Board.statusStrings.get(status));
		
		return ret;
	}
	
	public static AttackStatus parseAttackStatusMessage(JSONObject msg) {
		String status = msg.getString(strings.get(MsgString.STATUS));
		for (AttackStatus key : Board.statusStrings.keySet()) {
			if (Board.statusStrings.get(key).equals(status)) {
				return key;
			}
		}
		return AttackStatus.ERROR;
	}
	
	public static JSONObject getGotAttackedMessage(UUID id, byte[][] new_board) {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.GOT_ATTACKED));
		ret.put(strings.get(MsgString.FROM), id.toString());
		
		int[] arr = new int[new_board.length * new_board[0].length];
		int index = 0;
		for (int i = 0; i < new_board.length; i++) {
			for (int j = 0; j < new_board[i].length; j++) {
				arr[index] = new_board[i][j];
				index++;
			}
		}
		
		ret.put(strings.get(MsgString.NEW_BOARD), arr);
		
		return ret;
	}
	
	public static Object[] parseGotAttackedMessage(JSONObject msg) {
		Object[] ret = new Object[2];
		
		JSONArray arr = msg.getJSONArray(strings.get(MsgString.NEW_BOARD));
		Byte[][] board = new Byte[Board.getHeigth()][Board.getWidth()];
		
		int i = 0, j = 0;
		for (Object o : arr) {
			board[i][j] = (Byte) o;
			j = (j + 1) % Board.getWidth();
			if (j == 0) {
				i++;
			}
		}
		
		ret[0] = UUID.fromString(msg.getString(strings.get(MsgString.FROM)));
		ret[1] = board;
		
		return ret;
	}
	
	public static Optional<JSONObject> getMatchEndMessage(Duration d, UUID[] ids, String[] names, Stats[] stats) {
		if (!(ids.length == names.length && names.length == stats.length)) {
			return Optional.empty();
		}
		
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.MATCH_END));
		ret.put(strings.get(MsgString.DURATION), d.getNano());
		
		JSONArray players = new JSONArray();
		for (int i = 0; i < ids.length; i++) {
			JSONObject player = new JSONObject();
			player.put(strings.get(MsgString.ID), ids[i].toString());
			player.put(strings.get(MsgString.NAME), names[i]);
			
			Stats s = stats[i];
			JSONObject stat_obj = new JSONObject();
			stat_obj.put(strings.get(MsgString.SHOTS), s.getNumberOfShots());
			stat_obj.put(strings.get(MsgString.HITS), s.getNumberOfHits());
			stat_obj.put(strings.get(MsgString.SUNK), s.getNumberOfSunkShips());
			stat_obj.put(strings.get(MsgString.ELIMINATIONS), s.getNumberOfEliminations());
			stat_obj.put(strings.get(MsgString.GRADE), s.getGrade().toString());
			
			player.put(strings.get(MsgString.STATS), stat_obj);
			
			players.put(player);
		}
		
		return Optional.of(ret);
	}
	
	public static Object[] parseMatchEndMessage(JSONObject msg) {
		Object[] ret = new Object[4];
		JSONArray arr = msg.getJSONArray(strings.get(MsgString.PLAYERS));
		
		ret[0] = (Integer) msg.getInt(strings.get(MsgString.DURATION));
		ret[1] = new UUID[arr.length()];
		ret[2] = new String[arr.length()];
		ret[3] = new Stats[arr.length()];
		
		int index = 0;
		for (Object o : arr) {
			JSONObject obj = (JSONObject) o;
			((UUID[]) ret[1])[index] = UUID.fromString(obj.getString(strings.get(MsgString.ID)));
			((String[]) ret[2])[index] = obj.getString(strings.get(MsgString.NAME));
			
			JSONObject stat_obj = obj.getJSONObject(strings.get(MsgString.STATS));
			Stats stat = new Stats();
			stat.setNumberOfShots((short) stat_obj.getInt(strings.get(MsgString.SHOTS)));
			stat.setNumberOfHits((short) stat_obj.getInt(strings.get(MsgString.HITS)));
			stat.setNumberOfSunkShips((short) stat_obj.getInt(strings.get(MsgString.SUNK)));
			stat.setNumberOfPlayerEliminations((short) stat_obj.getInt(strings.get(MsgString.ELIMINATIONS)));
			stat.setGrade(Enum.valueOf(PlayerGrade.class, stat_obj.getString(strings.get(MsgString.GRADE))));
			((Stats[]) ret[3])[index] = stat;
			
			index++;
		}
		
		return ret;
	}
	
	public static JSONObject getEliminatedMessage(UUID by) {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.ELIMINATED));
		ret.put(strings.get(MsgString.BY), by.toString());
		return ret;
	}
	
	public static UUID parseEliminatedMessage(JSONObject msg) {
		return UUID.fromString(msg.getString(strings.get(MsgString.BY)));
	}
	
	public static JSONObject getQuitMessage() {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.QUIT));
		return ret;
	}
	
	public static JSONObject getLeftMessage(UUID id) {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.LEFT));
		ret.put(strings.get(MsgString.ID), id.toString());
		return ret;
	}
	
	public static UUID parseLeftMessage(JSONObject msg) {
		return UUID.fromString(msg.getString(strings.get(MsgString.ID)));
	}
	
	public static JSONObject getChatSendMessage(String msg) {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.CHAT_SEND));
		ret.put(strings.get(MsgString.MSG), msg);
		return ret;
	}
	
	public static String parseChatSendMessage(JSONObject msg) {
		return msg.getString(strings.get(MsgString.MSG));
	}
	
	public static JSONObject getChatRecvMessage(UUID id, String msg) {
		JSONObject ret = new JSONObject();
		ret.put(strings.get(MsgString.TYPE), types.get(MsgType.CHAT_RECV));
		ret.put(strings.get(MsgString.FROM), id.toString());
		ret.put(strings.get(MsgString.MSG), msg);
		return ret;
	}
	
	public static Object[] parseChatRecvMessage(JSONObject msg) {
		Object[] ret = new Object[2];
		ret[0] = UUID.fromString(msg.getString(strings.get(MsgString.FROM)));
		ret[1] = msg.getString(strings.get(MsgString.MSG));
		return ret;
	}
	
}
