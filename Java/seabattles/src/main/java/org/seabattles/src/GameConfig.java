package org.seabattles.src;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.text.ParseException;
import java.util.Arrays;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class GameConfig {
	
	public enum GameDifficulty {
		NORMAL,
		HARD,
		IMPOSSIBLE
	};
	
	private short numberOfPlayers;
	private short numberOfBots;
	
	private GameDifficulty botsDifficulty;
	
	private File configFile; 					// Contains ships length/width, can contain more than classic 5 ships
	private static final String DEFAULT_CONFIG_PATH = "config.json";
	private ShipConfig[] shipsConfig;
	
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
		if (configFile == null) {
			configFile = new File(DEFAULT_CONFIG_PATH);
			if (!configFile.exists()) {
				throw new FileNotFoundException();
			}
		}
		
		JSONObject json = new JSONObject(new String(Files.readAllBytes(Paths.get(configFile.toURI()))));
		
		boardWidth = json.getJSONObject("board").getInt("width");
		boardHeigth = json.getJSONObject("board").getInt("heigth");
		
		JSONArray ships = json.getJSONArray("ships");
		shipsConfig = new ShipConfig[ships.length()];
		for (int i = 0; i < shipsConfig.length; i++) {
			shipsConfig[i] = new ShipConfig(ships.getJSONObject(i));
		}
		
		if (json.getString("debug").equals(Main.DEBUG_STRING)) {
			Main.DEBUG_MODE = true;
		}
	}
	
	public void setShipsConfigArray(ShipConfig[] config) {
		shipsConfig = config;
	}
	
	public int getTotalNumberOfPlayers() {
		return numberOfBots + numberOfPlayers;
	}
	
	public ShipConfig[] getShipsConfig() {
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
				+ botsDifficulty + ", configFile=" + configFile + ", shipsConfig=" + Arrays.toString(shipsConfig)
				+ ", boardWidth=" + boardWidth + ", boardHeigth=" + boardHeigth + "]";
	}
	
}
