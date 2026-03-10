package org.seabattles.src;

import org.seabattles.gui.ascii.AsciiGUI;
import org.seabattles.gui.jfx.FxGUI;
import org.seabattles.interfaces.GUI;
import org.seabattles.main.Main;

public class ArgAnalyzer {
	private final String[] args;
	private GUI g;
	
	public ArgAnalyzer(String[] args) {
		this.args = args;
		analyze();
		
		if (g == null) {
			g = getDefaultGUI();
		}
	}
	
	private GUI getDefaultGUI() {
		return new FxGUI();
	}
	
	private void initDebugString(String s) {
		s = s.substring(4);
		if (s.matches("\"\\s\"")) {
			s = s.substring(1, s.length() - 1);
		}
		Main.DEBUG_STRING = s;
		Main.DEBUG_MODE = true;
	}
	
	private void initGUI(String s) {
		s = s.substring(4);
		if (s.matches("\"\\s\"")) {
			s = s.substring(1, s.length() - 1);
		}
		
		if (s.equalsIgnoreCase("ascii")) {
			g = new AsciiGUI();
		} else {
			g = getDefaultGUI();
		}
	}
	
	private void initVerbose(String s) {
		s = s.substring(8);
		if (s.equalsIgnoreCase("true") || s.equals("1")) {
			Main.DEBUG_PRINT = true;
		}
	}
	
	private void analyze() {
		for (String s : args) {
			if (s.startsWith("dbg=")) {
				initDebugString(s);
			} else if (s.startsWith("gui=")) {
				initGUI(s);
			} else if (s.startsWith("verbose=")) {
				initVerbose(s);
			}
		}
	}
	
	public GUI getGUI() {
		return g;
	}
}
