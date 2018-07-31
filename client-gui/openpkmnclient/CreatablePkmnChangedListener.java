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
interface CreatablePkmnChangedListener extends EventListener {
    public class CreatablePkmnChangedEvent extends EventObject {

            private int newId;

            public CreatablePkmnChangedEvent(Object source, int newId) {
                super(source);
                this.newId = newId;
            }

            public int getId() {
                return newId;
            }
        }

    public void creatablePkmnChanged(CreatablePkmnChangedEvent e);

}
