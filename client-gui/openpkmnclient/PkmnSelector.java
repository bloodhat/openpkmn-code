/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.util.ArrayList;
import java.util.Set;
import java.util.TreeSet;

/**
 *
 * @author matt
 */
public class PkmnSelector {
    TreeSet<Integer> pkmnSet;    
    public PkmnSelector(Set<Rule> p,
			boolean evolvedOnly) {
	pkmnSet = new TreeSet();
	if(evolvedOnly) {
	    /* set up the evolved pkmn set */
	    for(int i = 0; i < Environment.evolvedPkmn.length; i++) {
		pkmnSet.add(new Integer(Environment.evolvedPkmn[i]));
	    }
	}
	else {
	    for(int i = 1; i <Environment.PKMN_MAX; i++) {
		pkmnSet.add(new Integer(i));
	    }
	}
	// now do some selective removals
	if(p.contains(Rule.NO_MEW)) {
	    pkmnSet.remove(151);
	}
	if(p.contains(Rule.NO_MEWTWO)) {
	    pkmnSet.remove(150);
	}
    }

    /* public int[] getRandomSet(int size) {
	int[] pkmn = new int[size];
	int selection;
	
	for(int i = 0; i < size; i++) {
	    selection = (int)(Math.random() * pkmnSet.size());
	    pkmn[i] = pkmnSet.get(selection).intValue();
	    pkmnSet.remove(selection);
	}
	return pkmn;
	}*/

    public int getRandomPkmn(boolean replace) {
	int setSize;

	if((setSize = pkmnSet.size()) == 0) {
	    return 0;
	}
	else {
	    int chosenIndex = (int)(setSize * Math.random());
	    int pkmn = (Integer)((pkmnSet.toArray())[chosenIndex]);
	    if(!replace) {
		pkmnSet.remove(pkmn);
	    }
	    return pkmn;
	}
    }

    public int choosePkmn(int selection, boolean replace) {
	if(pkmnSet.contains(selection)) {
	    if(!replace) {
		pkmnSet.remove(selection);
	    }
	    return selection;
	}
	else {
	    return -1;
	}
    }

    public Set getSet() {
	return pkmnSet;
    }
    
    public ArrayList<Integer> getList() {
        ArrayList<Integer> list = new ArrayList<Integer>();
        for(Integer i : pkmnSet) {
	    list.add(i);
	}
        return list;
    }

    public void printPkmnChoices() {
	for(Integer i : pkmnSet) {
	    System.out.println(i + ": " + Lexicon.pkmnNames[i]);
	}
    }
}
