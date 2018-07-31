/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.util.EventListener;
import java.util.EventObject;
import openpkmnclient.MainModel.Status;

/**
 *
 * @author matt
 */
public interface UserSelectedListener extends EventListener {

    public class UserSelectedEvent extends EventObject {
        private int userRules;
        private int teamRules;
        private String userName;
        private Status status;
        public UserSelectedEvent(Object source, String userName, 
                Status status, int userRules, int teamRules) {
            super(source);
            this.userName = userName;
            this.status = status;
            this.userRules = userRules;
            this.teamRules = teamRules;
        }

        public Status getStatus() {
            return status;
        }

        public int getTeamRules() {
            return teamRules;
        }

        public int getUserRules() {
            return userRules;
        }

        String getUserName() {
            return userName;
        }

    }

   public void userSelected(UserSelectedEvent e);

}
