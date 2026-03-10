package org.seabattles.gui.jfx.controllers;

import java.time.Duration;
import java.util.Map;
import java.util.UUID;

import org.seabattles.gui.jfx.EventGUI;
import org.seabattles.gui.jfx.FxGUIController;
import org.seabattles.src.Player.PlayerStatus;
import org.seabattles.src.Stats;

import javafx.fxml.FXML;
import javafx.geometry.Pos;
import javafx.scene.control.Button;
import javafx.scene.control.Control;
import javafx.scene.control.Label;
import javafx.scene.layout.Border;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.BorderStroke;
import javafx.scene.layout.BorderStrokeStyle;
import javafx.scene.layout.BorderWidths;
import javafx.scene.layout.CornerRadii;
import javafx.scene.layout.Priority;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;

public class EndScreenController extends FxGUIController {
	
	@FXML private Label statusLabel;
	@FXML private VBox dataBox;
	@FXML private Button backBtn;
	
	private UUID myID;
	private PlayerStatus myStatus;
	private Duration duration;
	private Map<UUID, Object[]> playerStats;
	
	@FXML
	@Override
	protected void initialize() {
		super.initialize();
		
		backBtn.prefWidthProperty().bind(dataBox.widthProperty().multiply(0.5));
		backBtn.setOnAction(e -> {
			EventGUI.releaseGUI();
		});
		
		VBox parent = (VBox) dataBox.getParent();
		parent.setAlignment(Pos.CENTER);
		parent.setFillWidth(false);
		
		dataBox.setBorder(new Border(new BorderStroke(Color.BLACK, BorderStrokeStyle.SOLID, new CornerRadii(25), new BorderWidths(1))));
		dataBox.setAlignment(Pos.CENTER);
		dataBox.prefWidthProperty().bind(parent.widthProperty().multiply(0.5));
	}

	@Override
	protected void applyParams() {
		myID = (UUID) params[0];
		myStatus = (PlayerStatus) params[1];
		duration = (Duration) params[2];
		playerStats = (Map<UUID, Object[]>) params[3];
		
		printThings();
		styleLabels();
	}
	
	private void styleLabels() {
		dataBox.getChildren().forEach(e -> {
			if (!(e instanceof Label)) {
				return;
			}
			
			Label l = (Label) e;
			
			l.setFont(new Font("System Bold", 18));
			l.setPrefWidth(Control.USE_COMPUTED_SIZE);
		});
	}
	
	private void printThings() {
		switch (myStatus) {
			case WINNER:
				statusLabel.setText("YOU WON!");
				statusLabel.setTextFill(Color.rgb(18, 230, 16));
				break;
			case LOSER:
				statusLabel.setText("YOU LOST!");
				statusLabel.setTextFill(Color.rgb(206, 7, 7));
				break;
			default:
				statusLabel.setText("Unknown status?");
				break;
		}
		
		long time = Math.abs(duration.getSeconds());
		Label t = new Label(String.format("Finish time: %dm %ds", time / 60, time % 60));
		
		Object[] mystats = playerStats.get(myID);
		Label name = new Label("Player: " + (String) mystats[0]);

		Stats s = (Stats) mystats[1];
		Label shots = new Label("Number of shots: " + s.getNumberOfShots());
		Label hits = new Label("Number of hits: " + s.getNumberOfHits());
		Label sunk = new Label("Number of sunk ships: " + s.getNumberOfSunkShips());
		Label elim = null;
		if (s.getNumberOfEliminations() > 0) {
			elim = new Label("You eliminated " + s.getNumberOfEliminations() + " players!");
		}
		Label grade = new Label("Your grade: " + s.getGrade());
		
		dataBox.getChildren().add(t);
		dataBox.getChildren().add(name);
		dataBox.getChildren().add(shots);
		dataBox.getChildren().add(hits);
		dataBox.getChildren().add(sunk);
		if (elim != null) {
			dataBox.getChildren().add(elim);
		}
		dataBox.getChildren().add(grade);
	}

	@Override
	protected void onSceneReady() {
		// TODO Auto-generated method stub
		
	}
}
