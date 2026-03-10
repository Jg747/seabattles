package org.seabattles.gui.jfx.controllers;

import java.util.List;
import java.util.UUID;

import org.seabattles.gui.jfx.EventGUI;
import org.seabattles.gui.jfx.FxGUI;
import org.seabattles.gui.jfx.FxGUIController;

import javafx.collections.ListChangeListener;
import javafx.collections.SetChangeListener;
import javafx.fxml.FXML;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.TextField;
import javafx.scene.input.KeyCode;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.scene.text.Font;

public class WaitStartingGame extends FxGUIController {
	
	private boolean isHost;
	
	@FXML private ScrollPane pane;
	
	@FXML private VBox playerList;
	
	@FXML private VBox chat;
	@FXML private TextField textSend;
	@FXML private Button sendChat;
	
	@FXML private Button quitBtn;
	@FXML private Button startGameBtn;
	
	@FXML
	@Override
	protected void initialize() {
		super.initialize();
		
		FxGUI.getChat().addListener((ListChangeListener.Change<? extends String> c) -> {
			if (c.next()) {
				updateChat(c.getAddedSubList());
			}
		});
		
		chat.heightProperty().addListener((obs, oldVal, newVal) -> {
			pane.setVvalue(1);
        });
		
		textSend.setOnKeyPressed(e -> {
			if (e.getCode() == KeyCode.ENTER) {
				chatSend();
			}
		});
		
		sendChat.setOnAction(e -> {
			chatSend();
		});
		
		quitBtn.setOnAction(e -> {
			EventGUI.returnObj(false);
		});
		
		startGameBtn.setOnAction(e -> {
			startGameBtn.setVisible(false);
			startGameBtn.setManaged(false);
			
			if (FxGUI.getGame().startProcedure()) {
				EventGUI.returnObj(true);
			} else {
				startGameBtn.setVisible(true);
				startGameBtn.setManaged(true);
			}
		});
		
		FxGUI.getPlayerList().addListener((SetChangeListener.Change<? extends UUID> c) -> {
			if (c.wasAdded()) {
				addPlayerListVBox(c.getElementAdded());
			}
			
			if (c.wasRemoved()) {
				removePlayerListVBox(c.getElementRemoved());
			}
		});
		
		updatePlayerList();
		
	}
	
	private void updatePlayerList() {
		FxGUI.getPlayerList().forEach(u -> addPlayerListVBox(u));
	}
	
	private void addPlayerListVBox(UUID id) {
		HBox hBox = new HBox();
		hBox.setId(id.toString());
		hBox.setSpacing(200);
		hBox.setAlignment(Pos.CENTER);
		
		Label name = new Label(FxGUI.getGame().getPlayer(id).get().getUsername());
		name.setFont(new Font("System", 36));
		
		hBox.getChildren().add(name);
		
		if (isHost) {
			name.setPrefWidth(250);
			
			HBox btns = new HBox();
			btns.setPadding(new Insets(10, 10, 10, 10));
			btns.setAlignment(Pos.CENTER);
			btns.setSpacing(10);
			
			Button kick = new Button("Kick");
			kick.setFont(new Font("System", 24));
			kick.setPrefWidth(100);
			kick.setOnAction(e -> {
				FxGUI.getGame().getClient().sendKick(UUID.fromString(kick.getParent().getParent().getId()));
			});
			
			Button ban = new Button("Ban");
			ban.setFont(new Font("System", 24));
			ban.setPrefWidth(100);
			ban.setOnAction(e -> {
				FxGUI.getGame().getClient().sendBan(UUID.fromString(ban.getParent().getParent().getId()));
			});
			
			btns.getChildren().add(kick);
			btns.getChildren().add(ban);
			
			hBox.getChildren().add(btns);	
		}
		
		playerList.getChildren().add(hBox);
	}
	
	private void removePlayerListVBox(UUID id) {
		playerList.getChildren().removeIf(e -> e.getId().equals(id.toString()));
	}

	@Override
	protected void applyParams() {
		isHost = (boolean) params[0];
		
		if (!isHost) {
			startGameBtn.setVisible(false);
			startGameBtn.setManaged(false);
			
			playerList.setAlignment(Pos.CENTER);
		}
	}
	
	private void updateChat(List<? extends String> added) {
		for (String s : added) {
			Label l = new Label(s);
			l.setFont(new Font("System", 18));
			l.setWrapText(true);
			chat.getChildren().add(l);
		}
	}
	
	private void chatSend() {
		String msg = textSend.getText();
		
		if (msg.isEmpty() || msg.isBlank()) {
			return;
		}
		
		textSend.setText("");
		FxGUI.getGame().getClient().sendChatMessage(msg);
	}

	@Override
	protected void onSceneReady() {
		// TODO Auto-generated method stub
		
	}
}
