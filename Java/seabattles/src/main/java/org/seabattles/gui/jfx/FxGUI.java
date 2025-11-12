package org.seabattles.gui.jfx;

import java.io.IOException;

import org.seabattles.src.Game;

import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;

public class FxGUI extends Application {

    private static Scene scene;
    
    private static Game game;

    @Override
    public void start(Stage stage) throws IOException {
        scene = new Scene(loadFXML("primary"), 640, 480);
        stage.setScene(scene);
        stage.show();
    }

    public static void setRoot(String fxml) throws IOException {
        scene.setRoot(loadFXML(fxml));
    }

    private static Parent loadFXML(String fxml) throws IOException {
        FXMLLoader fxmlLoader = new FXMLLoader(FxGUI.class.getResource(fxml + ".fxml"));
        return fxmlLoader.load();
    }

    public static void startFxGUI(String[] args) {
        launch(FxGUI.class, args);
    }

}