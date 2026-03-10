package org.seabattles.main;

import java.io.File;
import java.util.Map;

import org.seabattles.interfaces.GUI;
import org.seabattles.src.ArgAnalyzer;
import org.seabattles.src.Board;
import org.seabattles.src.Game;
import org.seabattles.src.GameConfig;
import org.seabattles.src.GameConfig.GameDifficulty;
import org.seabattles.src.Logger;
import org.seabattles.src.Ship;

public class Main {
	
	/*
	 * ARGS
	 * dbg=<debug_string>
	 * gui=[ascii, jfx]
	 * verbose=[true, 1]
	 */
	
	/* TODO
	 * quando un player quitta eliminare player dal game
	 * */
	
	public static final String name = "SEABATTLES";
	public static final String version = "0.0.1";
	public static boolean DEBUG_MODE = false;
	public static boolean DEBUG_TESTING_DATA = false;
	public static boolean DEBUG_PRINT = false;
	public static String DEBUG_STRING = "";
	public static final String SERVER_SPRITE_PATH = "seabattles_tmp_sprites_server";
	public static final String CLIENT_SPRITE_PATH = "seabattles_tmp_sprites_client";
	
	public static void main(String[] args) throws Exception {
		program(args);
		// test(args);
	}
	
	public static void sleep(long millis) {
		try {
			Thread.sleep(millis);
		} catch (InterruptedException e) {}
	}
	
	private static void program(String[] args) throws Exception {
		ArgAnalyzer a = new ArgAnalyzer(args);
		Game g = null;
		GUI gui = a.getGUI();
		boolean exit = false;
		
		Logger.setGUI(gui);
		new Thread(() -> gui.startGUI()).start();
		
		while (!exit) {
			try {
				g = new Game();
				g.setGUI(gui);
				gui.setGame(g);
				exit = !g.start();
			} catch (Exception e) {
				String log = Logger.logException(e);
				gui.errorScreen(log);
				
				if (g != null) {
					g.destroyAll();
				}
				
				stopPgm(1);
			}
		}
		
		g.getGUI().stopGUI();
	}
	
	public static void stopPgm() {
		System.exit(0);
	}
	
	public static void stopPgm(int code) {
		System.exit(code);
	}
	
	public static Object[] getStartTestData(boolean multi) {
		try {
			Object[] ret = new Object[2];
			GameConfig conf = new GameConfig();
			conf.setConfigFile(new File("test_config.json"));
			
			if (!multi) {
				conf.setNumberOfBots(1);
				conf.setBotsDifficulty(GameDifficulty.NORMAL);
				
				ret[0] = 1;
			} else {
				conf.setNumberOfPlayers(2);
				conf.setNumberOfBots(1);
				conf.setBotsDifficulty(GameDifficulty.NORMAL);
				
				ret[0] = 2;
			}
			
			ret[1] = conf;
			
			return ret;
		} catch (Exception e) {
			Logger.logException(e);
		}
		
		return null;
	}
	
	public static String[] getMultiplayerTestData(int choice) {
		String[] ret = new String[2];
		if (choice == 1) {
			ret[0] = "HOST";
			ret[1] = null;
		} else {
			ret[0] = "NOT_HOST";
			ret[1] = "127.0.0.1";
		}
		return ret;
	}
	
	public static void placeTestShips(Board b) {
		// TODO TESTING
		Map<Integer, Ship> ships = b.getShips();
		Ship testShip;
		testShip = ships.get(1);
		testShip.setX(0);
		testShip.setY(0);
		testShip.rotate(0);
		/*testShip = ships.get(2);
		testShip.setX(0);
		testShip.setY(1);
		testShip.rotate(0);
		testShip = ships.get(3);
		testShip.setX(2);
		testShip.setX(7);
		testShip.rotate(2);
		testShip = ships.get(4);
		testShip.setX(1);
		testShip.setY(6);
		testShip.rotate(3);
		testShip = ships.get(5);
		testShip.setX(2);
		testShip.setY(1);
		testShip.rotate(3);*/
		for (Ship s : ships.values()) {
			b.placeShip(s);
		}
	}
	
	public static int[] getBotTestShipPlacement(int id) {
		// TODO TESTING
		int[] ret = new int[3];
		switch (id) {
			case 1:
				ret[0] = 0; // x
				ret[1] = 0; // y
				ret[2] = 0; // rot
				break;
			/*case 2:
				ret[0] = 0; // x
				ret[1] = 1; // y
				ret[2] = 0; // rot
				break;*/
			default:
				throw new RuntimeException("Bot placement: " + id + " not configured");
		}
		return ret;
	}

	private static void test(String[] args) throws Exception {
		
	}
	
}
