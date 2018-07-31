/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.util.List;

/**
 *
 * @author matt
 */
public class AvailableTeam {
    List<Integer> members;
    int index;
    int rules;
    
    public AvailableTeam(List<Integer> ms,
			 int r, int i) {
	members = ms;
	rules = r;
	index = i;
    }

    public int getIndex() {
	return index;
    }
    public int getRules() {
	return rules;
    }
    public List<Integer> getMembers() {
	return members;
    }

}
