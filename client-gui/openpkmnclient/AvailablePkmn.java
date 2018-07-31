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
public class AvailablePkmn {
    int num;
    List<Integer> moves;
    int rules;
    int index;
    
    public AvailablePkmn(int n, List<Integer> ms,
			 int r, int i) {
	num = n;
	moves = ms;
	rules = r;
	index = i;
    }

    public int getIndex() {
	return index;
    }

    public int getNum() {
	return num;
    }

    public int getRules() {
	return rules;
    }

    public List<Integer> getMoves() {
	return moves;
    }

}
