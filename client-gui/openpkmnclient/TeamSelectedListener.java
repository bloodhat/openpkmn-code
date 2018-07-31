/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.util.Collection;
import java.util.EventListener;
import java.util.EventObject;
import java.util.List;

/**
 *
 * @author matt
 */
public interface TeamSelectedListener extends EventListener {
    /* BIG WARNING: selected team can be -1 if no team is selected, so make sure
     * your listeners handle this well (in this case (and only in this case)
     * teamMembers and teamRules will be null too!)
     */
    public class TeamSelectedEvent extends EventObject {
        private List<Integer> selectedTeams;
        private Collection<Integer> teamMembers;
        private String teamRules;
        public TeamSelectedEvent(Object source,
                List<Integer> selectedTeams,
                Collection<Integer> teamMembers, String teamRules) {
            super(source);
            this.selectedTeams = selectedTeams;
            this.teamMembers = teamMembers;
            this.teamRules = teamRules;
        }

        public List<Integer> getSelectedTeams() {
            return selectedTeams;
        }

        public Collection<Integer> getTeamMembers() {
            return teamMembers;
        }

        public String getTeamRules() {
            return teamRules;
        }
    }

    public void selectedTeamChanged(TeamSelectedEvent e);
}
