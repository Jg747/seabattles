package org.seabattles.gui.jfx.controllers;

import org.seabattles.gui.jfx.EventGUI;
import org.seabattles.gui.jfx.FxGUIController;

import javafx.fxml.FXML;
import javafx.scene.control.Button;

public class MainMenuController extends FxGUIController {
	
	@FXML private Button singleBtn;
	@FXML private Button multiBtn;
	@FXML private Button exitBtn;
	
	@FXML
	@Override
	protected void initialize() {
		super.initialize();
	}
	
	@FXML
	private void singleplayer() {
		EventGUI.returnObj(1);
	}
	
	@FXML
	private void multiplayer() {
		EventGUI.returnObj(2);
	}
	
	@FXML
	private void exitAction() {
		EventGUI.returnObj(3);
	}

	@Override
	protected void applyParams() {
		// TODO Auto-generated method stub
		
	}

	@Override
	protected void onSceneReady() {
		// TODO Auto-generated method stub
		
	}
}
