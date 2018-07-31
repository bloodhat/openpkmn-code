/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.net.URL;
import javax.swing.ImageIcon;
import javax.swing.JLabel;

/**
 *
 * @author matt
 */
public class BattleOpponentBalls extends JLabel {
    JLabel balls[];
    private ImageIcon createImageIcon(String path, String description) {
        URL imgURL = getClass().getResource(path);
        if (imgURL != null) {
            return new ImageIcon(imgURL, description);
        } else {
            System.err.println("Couldn't find file: " + path);
            return null;
        }
    }

    void setBalls(int ok, int status, int faint) {
        ImageIcon okIcon = createImageIcon("/resources/balls/normal.png", "ok");
        ImageIcon faintIcon = createImageIcon("/resources/balls/faint.png",
                "fainted");
        ImageIcon statusIcon = createImageIcon("/resources/balls/status.png",
                "status condition");
        for (int i = 0; i < balls.length; i++) {
            if (ok > 0) {
                balls[i].setIcon(okIcon);
                ok--;
            } else if (status > 0) {
                balls[i].setIcon(statusIcon);
                status--;
            } else if (faint > 0) {
                balls[i].setIcon(faintIcon);
                faint--;
            } else {
                balls[i].setVisible(false);
            }
        }
    }
}
