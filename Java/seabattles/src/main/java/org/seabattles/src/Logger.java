package org.seabattles.src;

import org.seabattles.interfaces.GUI;

public class Logger {
	private static GUI gui = null;;
	
	public static void setGUI(GUI gui) {
		if (Logger.gui == null) {
			Logger.gui = gui;
		}
	}
	
	public static void write(String msg) {
		if (Main.DEBUG_MODE) {
			gui.threadWriteDebug(msg);
		}
	}
}
