package org.seabattles.src;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.text.ParseException;
import java.util.Map;
import java.util.TreeMap;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.seabattles.main.Main;

public class GameConfig {
	
	public enum GameDifficulty {
		NORMAL,
		HARD,
		IMPOSSIBLE
	};
	
	private short numberOfPlayers;
	private short numberOfBots;
	
	private GameDifficulty botsDifficulty;
	
	private File configFile;
	private static final String DEFAULT_CONFIG_RES = "/org/seabattles/files/config.json";
	private Map<Integer, ShipConfig> shipsConfig;
	private String debugString;
	
	private int boardWidth;
	private int boardHeigth;
	
	public GameConfig() {
		numberOfPlayers = 1;
		numberOfBots = 1;
		botsDifficulty = GameDifficulty.NORMAL;
		configFile = null;
	}

	public short getNumberOfPlayers() {
		return numberOfPlayers;
	}

	public void setNumberOfPlayers(int numberOfPlayers) {
		this.numberOfPlayers = (short) numberOfPlayers;
	}

	public short getNumberOfBots() {
		return numberOfBots;
	}

	public void setNumberOfBots(int numberOfBots) {
		this.numberOfBots = (short) numberOfBots;
	}
	
	public void setBoardWidth(int width) {
		boardWidth = width;
	}
	
	public void setBoardHeigth(int heigth) {
		boardHeigth = heigth;
	}

	public GameDifficulty getBotsDifficulty() {
		return botsDifficulty;
	}

	public void setBotsDifficulty(GameDifficulty botsDifficulty) {
		this.botsDifficulty = botsDifficulty;
	}

	public File getConfigFile() {
		return configFile;
	}

	public void setConfigFile(File configFile) throws FileNotFoundException {
		this.configFile = configFile;
		if (!configFile.exists()) {
			throw new FileNotFoundException();
		}
	}
	
	public void applyConfigFile() throws ParseException, FileNotFoundException, IOException, JSONException {
		InputStream stream;
		if (configFile != null) {
			if (!configFile.exists()) {
				throw new FileNotFoundException();
			} else {
				stream = new FileInputStream(configFile);
			}
		} else {
			stream = getClass().getResourceAsStream(DEFAULT_CONFIG_RES);
		}
		
		JSONObject json = new JSONObject(new String(stream.readAllBytes()));
		
		boardWidth = json.getJSONObject("board").getInt("width");
		boardHeigth = json.getJSONObject("board").getInt("heigth");
		
		JSONArray ships = json.getJSONArray("ships");
		shipsConfig = new TreeMap<>();
		for (int i = 0; i < ships.length(); i++) {
			ShipConfig s = new ShipConfig(ships.getJSONObject(i));
			shipsConfig.put(s.getID(), s);
		}
		
		debugString = json.getString("debug");
	}
	
	public void setDebugString(String str) {
		debugString = str;
	}
	
	public String getDebugString() {
		return debugString;
	}
	
	public void setShipsConfigArray(Map<Integer, ShipConfig> config) {
		shipsConfig = config;
	}
	
	public int getTotalNumberOfPlayers() {
		return numberOfBots + numberOfPlayers;
	}
	
	public Map<Integer, ShipConfig> getShipsConfig() {
		return shipsConfig;
	}
	
	public int getBoardWidth() {
		return boardWidth;
	}
	
	public int getBoardHeigth() {
		return boardHeigth;
	}

	@Override
	public String toString() {
		return "GameConfig [numberOfPlayers=" + numberOfPlayers + ", numberOfBots=" + numberOfBots + ", botsDifficulty="
				+ botsDifficulty + ", configFile=" + configFile + ", shipsConfig=" + shipsConfig
				+ ", boardWidth=" + boardWidth + ", boardHeigth=" + boardHeigth + "]";
	}
	
}
