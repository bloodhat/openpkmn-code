/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.util.EnumSet;
import java.util.EventListener;
import java.util.EventObject;

/**
 *
 * @author matt
 */

    public interface RulesTableUpdatedListener extends EventListener {

        public class RulesTableUpdatedEvent extends EventObject {

            private EnumSet<Rule> newRules;

            public RulesTableUpdatedEvent(Object source,
                    EnumSet<Rule> newRules) {
                super(source);
                this.newRules = newRules;
            }

            public EnumSet<Rule> getRules() {
                return newRules;
            }
        }

        public void rulesTableUpdated(RulesTableUpdatedEvent e);
    }
