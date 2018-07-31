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
public class PkmnIcon extends JLabel {
    public PkmnIcon() {
        super();
    }

    public void setPkmn(int id, boolean front) {
       NumberFormat nf = NumberFormat.getInstance();
       nf.setMinimumIntegerDigits(3);
        String path = "/resources/" + (front ? "frontimg":"backimg") + "/" +
                nf.format(id) + ".png";
        URL imgURL = getClass().getResource(path);
        this.setIcon(new ImageIcon(imgURL, (front ? "Front of " : "Back of"
                + Lexicon.pkmnNames[id])));
    }
}
