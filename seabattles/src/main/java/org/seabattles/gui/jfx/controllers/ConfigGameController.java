package org.seabattles.gui.jfx.controllers;

import java.io.File;
import java.util.Arrays;

import org.seabattles.gui.jfx.EventGUI;
import org.seabattles.gui.jfx.FxGUI;
import org.seabattles.gui.jfx.FxGUIController;
import org.seabattles.src.Game;
import org.seabattles.src.GameConfig;
import org.seabattles.src.GameConfig.GameDifficulty;

import javafx.beans.value.ChangeListener;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.fxml.FXML;
import javafx.scene.control.Button;
import javafx.scene.control.ChoiceBox;
import javafx.scene.control.Spinner;
import javafx.scene.control.SpinnerValueFactory;
import javafx.scene.input.KeyCode;
import javafx.scene.input.TransferMode;
import javafx.scene.layout.HBox;
import javafx.stage.FileChooser;
import javafx.stage.FileChooser.ExtensionFilter;
import javafx.stage.Stage;

public class ConfigGameController extends FxGUIController {
	
	@FXML private Spinner<Integer> bots;
	@FXML private Spinner<Integer> players;
	@FXML private ChoiceBox<GameDifficulty> diff;
	@FXML private Button customPath;
	
	@FXML private Button backBtn;
	@FXML private Button okBtn;
	
	private FileChooser fileChooser;
	private File selectedFile;
	
	@FXML
	@Override
	protected void initialize() {
		super.initialize();
		
		fileChooser = null;
		selectedFile = null;
		
		bots.setValueFactory(new SpinnerValueFactory.IntegerSpinnerValueFactory(0, 3, 1));
		bots.setOnKeyPressed(e -> {
			if (e.getCode() == KeyCode.ENTER) {
				players.requestFocus();
			}
		});
		
		bots.valueProperty().addListener((obs, oldVal, newVal) -> {
			if (newVal > oldVal) {
				if (newVal + players.getValue() > Game.MAX_PLAYERS) {
					players.decrement();
				}
			}
		});
		
		players.setValueFactory(new SpinnerValueFactory.IntegerSpinnerValueFactory(1, 4, 1));
		players.setOnKeyPressed(e -> {
			if (e.getCode() == KeyCode.ENTER) {
				diff.requestFocus();
			}
		});
		
		players.valueProperty().addListener((obs, oldVal, newVal) -> {
			if (newVal > oldVal) {
				if (newVal + bots.getValue() > Game.MAX_PLAYERS) {
					bots.decrement();
				}
			}
		});
		
		ObservableList<GameDifficulty> diffList = FXCollections.observableArrayList();
		Arrays.stream(GameDifficulty.values()).forEach(i -> diffList.add(i));
		diff.setItems(diffList);
		diff.setValue(GameDifficulty.values()[0]);
		diff.setOnKeyPressed(e -> {
			if (e.getCode() == KeyCode.ENTER) {
				customPath.requestFocus();
			}
		});
		
		customPath.setOnAction(e -> {
			if (fileChooser == null) {
				initFileChooser();
			}
			
			selectedFile =  fileChooser.showOpenDialog(Stage.getWindows().get(0));
			if (selectedFile != null) {
				customPath.setText(selectedFile.getName());
			}
		});
		
		customPath.setOnDragOver(e -> {
			if (e.getDragboard().hasFiles()) {
				e.acceptTransferModes(TransferMode.COPY_OR_MOVE);
			}
			e.consume();
		});
		
		customPath.setOnDragDropped(e -> {
			if (!e.getDragboard().hasFiles()) {
				return;
			}
			
			selectedFile = e.getDragboard().getFiles().get(0);
			if (!selectedFile.getName().endsWith(".json") && !selectedFile.getName().endsWith(".txt")) {
				selectedFile = null;
			} else {
				customPath.setText(selectedFile.getName());
			}
			
			e.setDropCompleted(true);
			e.consume();
		});
		
		backBtn.setOnAction(e -> {
			EventGUI.returnObj(null);
		});
		
		okBtn.setOnAction(e -> {
			GameConfig conf = new GameConfig();
			
			conf.setNumberOfBots(bots.getValue());
			conf.setNumberOfPlayers(players.getValue());
			conf.setBotsDifficulty(diff.getValue());
			
			if (conf.getTotalNumberOfPlayers() <= 1) {
				EventGUI.errorDialog("You must play with at least one opponent!");
				return;
			}
			
			if (conf.getTotalNumberOfPlayers() > Game.MAX_PLAYERS) {
				EventGUI.errorDialog("Total number of participants (including you) must be no more than " + Game.MAX_PLAYERS);
				return;
			}
			
			try {
				if (selectedFile != null) {
					conf.setConfigFile(selectedFile);
				}
				conf.applyConfigFile();
			} catch (Exception ex) {
				EventGUI.errorDialog("Invalid file config provided");
				return;
			}
			
			EventGUI.returnObj(conf);
		});
		
		HBox b = (HBox) players.getParent();
		if (!FxGUI.getGame().isMultiplayer()) {
			b.setVisible(false);
			b.setManaged(false);
		} else {
			b.setVisible(true);
			b.setManaged(true);
		}
	}
	
	private void initFileChooser() {
		fileChooser = new FileChooser();
		fileChooser.setTitle("Select config file...");
		fileChooser.getExtensionFilters().addAll(
			new ExtensionFilter("JSON", "*.json"),
			new ExtensionFilter("Text Files", "*.txt")
		);
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
