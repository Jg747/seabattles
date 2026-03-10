package org.seabattles.src;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.util.Arrays;

import org.seabattles.interfaces.GUI;
import org.seabattles.main.Main;

public class Logger {
	private static GUI gui = null;
	private static final String LOG_FILE = "debug.log";
	
	public static void setGUI(GUI gui) {
		if (Logger.gui == null) {
			Logger.gui = gui;
		}
	}
	
	public static synchronized void write(String msg) {
		if (Main.DEBUG_PRINT) {
			gui.threadWriteDebug(msg);
			writeToFile(msg);
		}
	}
	
	private static void writeToFile(String msg) {
		try (FileWriter out = new FileWriter(LOG_FILE, true)) {
			out.write(msg + "\n");
			out.close();
		} catch (Exception e) {}
	}
	
	public static String logException(Exception e) {
		String out = null;
		if (Main.DEBUG_PRINT) {
			StringBuilder log = new StringBuilder(e.getMessage() + "\n");
			Arrays.stream(e.getStackTrace()).forEachOrdered(s -> log.append(s).append("\n"));
			out = log.toString();
			write(out);
		}
		return out != null ? out : "null";
	}
	
	@SuppressWarnings("unused")
	public static String logStackTrace(String header, String footer) {
		if (true) {
			return null;
		}
		
		String out = null;
		if (Main.DEBUG_PRINT) {
			StringBuilder log = new StringBuilder();
			Arrays.stream(Thread.currentThread().getStackTrace()).forEachOrdered(s -> log.append(s).append("\n"));
			out = header + log.toString() + footer;
			write(out);
		}
		return out != null ? out : "null";
	}
}
