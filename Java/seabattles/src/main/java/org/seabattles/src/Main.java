package org.seabattles.src;

import org.seabattles.gui.ascii.AsciiGUI;

public class Main {
	
	public static final String name = "SEABATTLES";
	public static final String version = "0.0.1";

	public static void main(String[] args) throws Exception {
		Game g = new Game();
		g.setGUI(new AsciiGUI(args, g));
		g.start();
	}

}
