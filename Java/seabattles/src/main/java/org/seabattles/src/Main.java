package org.seabattles.src;

import java.time.Duration;
import java.time.Instant;

import org.seabattles.gui.ascii.AsciiGUI;

public class Main {
	
	public static final String name = "SEABATTLES";
	public static final String version = "0.0.1";
	public static boolean DEBUG_MODE = false;
	public static String DEBUG_STRING = "";	// dbg=cheating_string
	public static final String SERVER_SPRITE_PATH = "seabattles_tmp_sprites_server";
	public static final String CLIENT_SPRITE_PATH = "seabattles_tmp_sprites_client";
	
	public static void main(String[] args) throws Exception {
		if (args.length == 1 && args[0].startsWith("dbg=")) {
			DEBUG_STRING = args[0].substring(4);
		}
		
		Game g = new Game();
		g.setGUI(new AsciiGUI(args, g));
		Logger.setGUI(g.getGUI());
		
		try {
			g.start();
		} catch (Exception e) {
			System.err.println(e.getMessage());
			e.printStackTrace();
			g.destroyAll();
		}

		// test();
	}

	private static void test() throws Exception {
		Instant prima = Instant.now();
		Thread.sleep(2000);
		Instant dopo = Instant.now();
		
		Duration d = Duration.between(prima, dopo);
		System.out.println(d.getSeconds());
		Duration d2 = Duration.ofSeconds(d.getSeconds());
		System.out.println(d2.getSeconds());
	}
	
}
