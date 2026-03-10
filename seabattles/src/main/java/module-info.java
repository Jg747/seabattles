module seabattles {
    requires javafx.controls;
    requires javafx.fxml;
    requires org.json;

    opens org.seabattles.gui.jfx to javafx.fxml;
    opens org.seabattles.gui.jfx.controllers to javafx.fxml;
    
    exports org.seabattles.main;
    exports org.seabattles.gui.jfx;
}