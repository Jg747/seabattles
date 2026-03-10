package org.seabattles.gui.jfx;

import java.io.IOException;
import java.util.concurrent.Semaphore;

import org.seabattles.interfaces.GUI;
import org.seabattles.main.Main;
import org.seabattles.src.Logger;

import javafx.application.Application;
import javafx.application.Platform;
import javafx.fxml.FXMLLoader;
import javafx.geometry.Insets;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.ButtonBar;
import javafx.scene.control.ButtonType;
import javafx.scene.control.Dialog;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.ScrollPane.ScrollBarPolicy;
import javafx.scene.control.TextArea;
import javafx.scene.image.Image;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.VBox;
import javafx.stage.Modality;
import javafx.stage.Stage;

public class EventGUI extends Application {
	private static EventGUI instance;
	private static Semaphore haltCallerThread = new Semaphore(0);
	private static FxGUIController current;

	private static Object retObj;
	private static boolean isSet;
	
	private static Dialog<String> errorDialog;
    private Scene scene;
    private Stage stage = null;
    private Stage debugWindow;
    
    public EventGUI() {
    	synchronized (EventGUI.class) {
			if (instance == null) {
				instance = this;
				Platform.runLater(() -> {
					if (Main.DEBUG_PRINT) {
						createDebugWindow();
					}
				});
				releaseGUI();
			}
		}
    }
    
    public static boolean isSceneReady() {
    	if (current == null) {
    		return false;
    	}
    	return current.isSceneReady();
    }
    
    public static boolean isEverythingReady() {
    	if (current == null) {
    		return false;
    	}
    	return current.isEverythingReady();
    }
    
    public static FxGUIController getCurrentScene() {
    	return current;
    }
    
    public static EventGUI getInstance() {
    	if (instance == null) {
    		waitGUI();
    	}
    	return instance;
    }
    
    public static int permits() {
    	return haltCallerThread.availablePermits();
    }
    
    public static void waitGUI() {
		try {
			Logger.logStackTrace("WAIT\n", permits() + "\n\n\n");
			haltCallerThread.drainPermits();
			haltCallerThread.acquire();
		} catch (Exception e) {}
	}
    
    public static void releaseGUI() {
    	Logger.logStackTrace("RELEASE\n", permits() + "\n\n\n");
    	if (haltCallerThread.availablePermits() == 0) {
			haltCallerThread.release();
		}
	}
    
    private static void createErrorDialog() {
    	errorDialog = new Dialog<>();
    	errorDialog.setResizable(false);
    	errorDialog.initModality(Modality.WINDOW_MODAL);
		Stage s = (Stage) errorDialog.getDialogPane().getScene().getWindow();
		s.setAlwaysOnTop(true);
		
		errorDialog.setTitle(GUI.TITLE);
		errorDialog.setHeaderText("An error occured during execution!");
		errorDialog.getDialogPane().getButtonTypes().add(new ButtonType("OK", ButtonBar.ButtonData.OK_DONE));
    }
    
    public static void errorDialog(String msg) {
    	Platform.runLater(() -> {
    		if (errorDialog == null) {
    			createErrorDialog();
    		}
			
    		errorDialog.setContentText(msg);
    		if (!errorDialog.isShowing()) {
    			errorDialog.showAndWait();
    		}
		});
    }
    
    public static void returnObj(Object ret) {
    	setRetObj(ret);
    	releaseGUI();
    }
    
    public static boolean isRetObjSet() {
    	return isSet;
    }
    
    public static void setRetObj(Object obj) {
    	isSet = true;
    	retObj = obj;
    }
    
    private Object consumeObj() {
    	Object tmp = retObj;
    	retObj = null;
    	isSet = false;
    	return tmp;
    }
    
    public void consumeReturn() {
    	consumeObj();
    }
    
    public int getInt() {
    	if (retObj == null) {
    		return 0;
    	}
    	return (int) consumeObj();
    }
    
    public String getString() {
    	if (retObj == null) {
    		return "";
    	}
    	return (String) consumeObj();
    }
    
    public boolean getBoolean() {
    	if (retObj == null) {
    		return false;
    	}
    	return (boolean) consumeObj();
    }
    
    public Object getObject() {
    	return consumeObj();
    }
    
    private static Image getIcon() {
    	return new Image(EventGUI.class.getResourceAsStream("/org/seabattles/files/icon.png"));
    }
    
    @Override
    public void start(Stage stage) throws IOException {
    	this.stage = stage;
        
    	stage.setMinWidth(FxGUI.WIDTH);
        stage.setMinHeight(FxGUI.HEIGHT);
        
        stage.setTitle(GUI.TITLE);

        stage.getIcons().add(getIcon());
        
        stage.setOnCloseRequest(e -> {
        	Main.stopPgm();
        });
        
        releaseGUI();
        
        stage.show();
    }
    
    @Override
    public void stop() throws Exception {
    	super.stop();
    	Main.stopPgm();
    }
    
    public static void launchApp() throws Exception {
    	Application.launch();
    }
    
    public void changeSceneUnblocking(String fxml, Object... params) {
    	if (stage == null) {
    		waitGUI();
    	}
    	
    	double width, height;
    	if (scene == null) {
    		width = FxGUI.WIDTH;
    		height = FxGUI.HEIGHT;
    	} else {
    		width = scene.getWidth();
        	height = scene.getHeight();
    	}
    	
    	boolean error = false;
    	do {
    		try {
        		FXMLLoader fxmlLoader = new FXMLLoader(EventGUI.class.getResource("controllers/" + fxml + ".fxml"));
        		scene = new Scene((Parent) fxmlLoader.load(), width, height);
            	
        		current = fxmlLoader.<FxGUIController>getController();
        		
            	Platform.runLater(() -> {
            		current.setParams(params);
            		stage.setScene(scene);
            	});
            	
            	error = false;
        	} catch (Exception e) {
        		error = true;
        		Logger.write(String.format("%s", EventGUI.class.getResource("controllers/" + fxml + ".fxml")));
        		Logger.logException(e);
        	}
    	} while (error);
    }
    
    public void changeScene(String fxml, Object... params) {
    	changeSceneUnblocking(fxml, params);
    	waitGUI();
    }
    
    private void createDebugWindow() {
    	debugWindow = new Stage();
    	
    	debugWindow.getIcons().add(getIcon());
    	debugWindow.setTitle(GUI.TITLE + " | Debug");
    	debugWindow.setMinHeight(300);
    	debugWindow.setMinWidth(300);

        VBox vbox = new VBox(2);
        vbox.setPadding(new Insets(5));
        vbox.setFillWidth(true);
        vbox.setMaxWidth(Double.MAX_VALUE);

        ScrollPane scroll = new ScrollPane(vbox);
        scroll.setFitToWidth(true);
        scroll.setVbarPolicy(ScrollBarPolicy.AS_NEEDED);
        scroll.setHbarPolicy(ScrollBarPolicy.AS_NEEDED);

        BorderPane root = new BorderPane(scroll);
        
        vbox.heightProperty().addListener((obs, oldVal, newVal) -> {
        	scroll.setVvalue(1);
        });

        Scene scene = new Scene(root, 500, 400);
        debugWindow.setScene(scene);
        debugWindow.show();
    }
    
    private TextArea t;
    public void writeDebug(String msg) {
    	if (t == null) {
    		t = new TextArea();
    		t.setText("");
    		t.setEditable(false);
    		ScrollPane scroll = (ScrollPane) ((BorderPane) debugWindow.getScene().getRoot()).getChildren().get(0);
    		VBox v = (VBox) scroll.getContent();
    		t.prefWidthProperty().bind(scroll.widthProperty());
    		t.prefHeightProperty().bind(scroll.heightProperty());
        	v.getChildren().add(t);
    	}
    	t.setText(t.getText() + "\n" + msg);
    	/*Label l = new Label(msg);
    	l.setFont(new Font("System", 14));
    	l.setWrapText(true);
    	ScrollPane scroll = (ScrollPane) ((BorderPane) debugWindow.getScene().getRoot()).getChildren().get(0);
    	VBox v = (VBox) scroll.getContent();
    	v.getChildren().add(l);*/
    }
    
    public static void test() {
    	System.out.println("GUISemaphore [permits=" + haltCallerThread.availablePermits() + "]");
    }

}