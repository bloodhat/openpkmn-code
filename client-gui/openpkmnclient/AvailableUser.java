/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

/**
 *
 * @author matt
 */
public class AvailableUser {
        int id;
    private String userName;
    private int ruleset;
    private int status;
    private int teamRules;
    private boolean isNew;
    
    public AvailableUser(int i, String u, int r, int s, int tr, boolean isNew) {
	id = i;
	userName = u;
	ruleset = r;
	status = s;
	teamRules = tr;
        this.isNew = isNew;
    }

    public int getID() {
	return id;
    }

    public String getUserName() {
	return userName;
    }
    
    public int getRuleset() {
        return ruleset;
    }

    public int getStatus() {
	return status;
    }
    
    public int getTeamRules() {
	return teamRules;
    }

    public boolean isNew() {
        return isNew;
    }
}
