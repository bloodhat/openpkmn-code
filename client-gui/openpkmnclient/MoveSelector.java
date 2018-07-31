/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.EnumSet;
import java.util.Set;
import java.util.TreeSet;

/**
 *
 * @author matt
 */
public class MoveSelector {

    Set<Integer> moveSet;

    public MoveSelector(int pkmnNum, EnumSet<Rule> p) {
        moveSet = new TreeSet<Integer>();
        // If invalid pkmn, return empty list
        if (pkmnNum >= 0 && pkmnNum < Environment.pkmnStats.length) {
            if (p.contains(Rule.NO_ILLEGAL_MOVES)) {
                int[][] legalMoves = Environment.legalMoves[pkmnNum];
                int[] moveGroup;
                // put in legal moves first
                moveGroup = legalMoves[0];
                for (int j = 0; j < moveGroup.length; j++) {
                    moveSet.add(moveGroup[j]);
                }
                if (!p.contains(Rule.NO_TM)) {
                    moveGroup = legalMoves[1];
                    for (int j = 0; j < moveGroup.length; j++) {
                        moveSet.add(moveGroup[j]);
                    }
                }
                if (!p.contains(Rule.NO_GS)) {
                    // not yet implemented
                }
            } else {
                for (int i = 1; i < Environment.MOVE_MAX; i++) {
                    moveSet.add(i);
                }
            }
            // now do some selective removals
            if (p.contains(Rule.NO_EVADE)) {
                moveSet.removeAll(Arrays.asList(Environment.EVADE_MOVES));
            }
            if (p.contains(Rule.NO_OHKO)) {
                moveSet.removeAll(Arrays.asList(Environment.OHKO_MOVES));
            }
        }
    }

    /* Select a valid (no repeats) moveset of at most numMoves moves */
    public Collection<Integer> getRandomMoveset(int numMoves) {
        int max = Math.min(moveSet.size(), numMoves);
        Collection<Integer> moveset = new ArrayList<Integer>();
        for (int i = 0; i < max; i++) {
            moveset.add(getRandomMove(false));
        }
        return moveset;
    }

    public int getRandomMove(boolean replace) {
        int setSize = moveSet.size();
        if (setSize == 0) {
            return Environment.NONE;
        } else {
            int chosenIndex = (int) (setSize * Math.random());
            int move = (Integer) ((moveSet.toArray())[chosenIndex]);
            System.err.println("chose " + move);
            if (!replace) {
                moveSet.remove(move);
            }
            return move;
        }
    }

    public int chooseMove(int selection, boolean replace) {
        if (moveSet.contains(selection)) {
            if (!replace) {
                moveSet.remove(selection);
            }
            return selection;
        } else {
            return -1;
        }
    }

    public Set getSet() {
        return moveSet;
    }

    public ArrayList<Integer> getMoveList() {
        ArrayList<Integer> list = new ArrayList<Integer>();
        for (Integer i : moveSet) {
            list.add(i);
        }
        return list;
    }

    public void printMoveChoices() {
        for (Integer i : moveSet) {
            System.out.println(i + ": " + Lexicon.moveNames[i]);
        }
    }
}
