package org.seabattles.gui.jfx.controllers;

import org.seabattles.gui.jfx.EventGUI;
import org.seabattles.gui.jfx.FxGUIController;
import org.seabattles.main.Main;

import javafx.fxml.FXML;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.input.KeyCode;
import javafx.scene.layout.VBox;

public class GetMultiplayerModeController extends FxGUIController {
	
	private boolean userNameWrong;
	private boolean ipAddressWrong;
	
	@FXML private VBox modeSelection;
	@FXML private Button hostBtn;
	@FXML private Button notHostBtn;
	@FXML private Button backBtn;
	
	@FXML private VBox infoScreen;
	@FXML private Label userNameText;
	@FXML private TextField userName;
	@FXML private Label ipAddressText;
	@FXML private TextField ipAddress;
	@FXML private Button connBtn;
	@FXML private Button backInfoBtn;
	
	@FXML
	@Override
	protected void initialize() {
		super.initialize();
		
		modeSelection.setVisible(true);
		modeSelection.setManaged(true);
		
		hostBtn.setOnAction(e -> {
			if (Main.DEBUG_TESTING_DATA) {
				EventGUI.returnObj(Main.getMultiplayerTestData(1));
				return;
			}
			
			swapScreens();
			
			ipAddressText.setVisible(false);
			ipAddressText.setManaged(false);
			
			ipAddress.setVisible(false);
			ipAddress.setManaged(false);
		});
		
		notHostBtn.setOnAction(e -> {
			if (Main.DEBUG_TESTING_DATA) {
				EventGUI.returnObj(Main.getMultiplayerTestData(2));
				return;
			}
			
			swapScreens();
			
			ipAddressText.setVisible(true);
			ipAddressText.setManaged(true);
			
			ipAddress.setVisible(true);
			ipAddress.setManaged(true);
		});
		
		backBtn.setOnAction(e -> {
			EventGUI.returnObj(null);
		});
		
		infoScreen.setVisible(false);
		infoScreen.setManaged(false);
		
		userName.setOnAction(e -> {
			userNameWrong = false;
			userName.setStyle("");
		});
		
		userName.setOnKeyPressed(e -> {
			if (e.getCode() == KeyCode.ENTER) {
				if (!ipAddress.isVisible()) {
					tryConnection();
				} else {
					ipAddress.requestFocus();
				}
			}
		});
		
		ipAddress.setOnAction(e -> {
			ipAddressWrong = false;
			ipAddress.setStyle("");
		});
		
		ipAddress.setOnKeyPressed(e -> {
			if (e.getCode() == KeyCode.ENTER) {
				tryConnection();
			}
		});
		
		connBtn.setOnAction(e -> {
			tryConnection();
		});
		
		backInfoBtn.setOnAction(e -> {
			swapScreens();
		});
		
		userName.setText("");
		ipAddress.setText("");
	}
	
	private void swapScreens() {
		modeSelection.setVisible(!modeSelection.isVisible());
		modeSelection.setManaged(!modeSelection.isManaged());
			
		infoScreen.setVisible(!infoScreen.isVisible());
		infoScreen.setManaged(!infoScreen.isManaged());
		
		userName.setText("");
		ipAddress.setText("");
	}
	
	private boolean checkInput() {
		String name = userName.getText();
		String ip = ipAddress.getText();
		
		userNameWrong = false;
		ipAddressWrong = false;
		
		if (name.isEmpty() || name.isBlank()) {
			userNameWrong = true;
		}
		
		if (ipAddress.isVisible()) {
			if (ip.isEmpty() || ip.isBlank()) {
				ipAddressWrong = true;
			}
			
			if (!ip.equalsIgnoreCase("localhost") && !ip.matches("\\d+\\.\\d+\\.\\d+.\\d+")) {
				ipAddressWrong = true;
			}
		}
		
		return !userNameWrong && !ipAddressWrong;
	}
	
	private void setAndRetObject() {
		String[] ret = new String[2];
		
		ret[0] = userName.getText();
		if (ipAddress.isVisible()) {
			ret[1] = ipAddress.getText();
		}
		
		EventGUI.returnObj(ret);
	}
	
	private void wrongInput() {
		if (userNameWrong) {
			userName.setStyle("-fx-border-color: red");
		}
		
		if (ipAddressWrong) {
			ipAddress.setStyle("-fx-border-color: red");
		}
	}
	
	private void tryConnection() {
		if (checkInput()) {
			setAndRetObject();
		} else {
			wrongInput();
		}
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
