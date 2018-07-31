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

    
public interface MoveSelectionPolicyChangedListener extends EventListener {

   public enum Policy { SELECTED, RANDOM }
    public class MoveSelectionPolicyChangedEvent extends EventObject {
        
             private Policy policy;

            public MoveSelectionPolicyChangedEvent(Object source,
                    Policy policy) {
                super(source);
                this.policy = policy;
            }

            public Policy getPolicy() {
                return policy;
            }
        }

    public void moveSelectionPolicyChanged(MoveSelectionPolicyChangedEvent e);
}

