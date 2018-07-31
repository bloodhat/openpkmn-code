/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.util.EventListener;
import java.util.EventObject;

/**
 *
 * @author matt
 */
public interface StarterSelectedListener extends EventListener {

    public void starterSelected(StarterSelectedEvent e);

    public class StarterSelectedEvent extends EventObject {
        private int id;
        private String name;
        public StarterSelectedEvent(Object source, int id, String name) {
            super(source);
            this.id = id;
            this.name = name;
        }

        public int getId() {
            return id;
        }

        public String getName() {
            return name;
        }
    }

}
