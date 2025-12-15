package org.seabattles.src;

import org.seabattles.gui.ascii.AsciiGUI;

public class Main {
	
	public static final String name = "SEABATTLES";
	public static final String version = "0.0.1";

	public static void main(String[] args) throws Exception {
		Game g = new Game();
		g.setGUI(new AsciiGUI(args, g));
		Logger.setGUI(g.getGUI());
		
		try {
			g.start();
		} catch (Exception e) {
			if (g.isServer()) {
				g.destroyServer();
			}
		}
		
		//test();
	}

	private static void test() throws Exception {
		Game g = new Game();
		Game g2 = new Game();
		System.out.println(g.toString() + "\n" + g2.toString());
	}
	
}
