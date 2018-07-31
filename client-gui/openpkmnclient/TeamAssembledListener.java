/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.util.EventListener;
import java.util.EventObject;
import java.util.List;

/**
 *
 * @author matt
 */
public interface TeamAssembledListener extends EventListener {

    public void teamAssembled(TeamAssembledEvent e);
    /* The list of PkmnInfoItems MAY BE NULL, which indicates that a team could
     * not be assembled.
     */
    public class TeamAssembledEvent extends EventObject {
        private List<PkmnInfoItem> team;
        public TeamAssembledEvent(Object source, List<PkmnInfoItem> team) {
            super(source);
            this.team = team;
        }

        public List<PkmnInfoItem> getTeam() {
            return team;
        }
    }

}
