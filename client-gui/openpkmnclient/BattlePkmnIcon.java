/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.net.URL;
import java.text.NumberFormat;
import javax.swing.ImageIcon;
import javax.swing.JLabel;

/**
 *
 * @author matt
 */
public class BattlePkmnIcon extends JLabel {

    NumberFormat nf;

    public BattlePkmnIcon() {
        nf = NumberFormat.getInstance();
        nf.setMinimumIntegerDigits(3);
    }

    public void setPkmnNumber(int number, boolean front) {
        String path;
        String description;
        if (front) {
            path = "/resources/backimg/";
            description = "Your " + Lexicon.pkmnNames[number];
        } else {
            path = "/resources/frontimg/";
            description = "Opponent's " + Lexicon.pkmnNames[number];
        }
        path += nf.format(number) + ".png";

        URL imgURL = getClass().getResource(path);
        if (imgURL != null) {
            this.setIcon(new ImageIcon(imgURL, description));
        } else {
            System.err.println("Couldn't find file: " + path);
        }
    }
}
