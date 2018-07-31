/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.EnumSet;
import java.util.List;
import javax.swing.AbstractListModel;
import javax.swing.JList;

/**
 *
 * @author matt
 */
public class LearnableMovesList extends JList {

    public LearnableMovesList() {
        this.setModel(new LearnableMovesListModel());
    }

    public void updatePkmnId(int pkmnId) {
        ((LearnableMovesListModel) getModel()).updatePkmnId(pkmnId);
    }

    public void updateRules(EnumSet<Rule> rules) {
        ((LearnableMovesListModel) getModel()).updateRules(rules);
    }

    public Collection<Integer> getSelectedMoves() {
        int[] indices = this.getSelectedIndices();
        ArrayList<Integer> moves = new ArrayList<Integer>();
        for (int index : indices) {
            int move = ((LearnableMovesListModel) getModel()).getRowID(index);
            if(move > 0) {
                moves.add(move);
            }
        }
        return moves;
    }

    public void makeRandomSelection() {
        Collection<Integer> moveset = ((LearnableMovesListModel) getModel()).getRandomMoveset(4);
        for (int move : moveset) {
            int index = ((LearnableMovesListModel) getModel()).getRowOfId(move);
            this.addSelectionInterval(index, index);
        }
    }

    public class LearnableMovesListModel extends AbstractListModel {

        private List<Integer> data;
        private EnumSet<Rule> rules;
        private int pkmnId;

        public LearnableMovesListModel() {
            data = new ArrayList<Integer>();
            rules = EnumSet.noneOf(Rule.class);
            pkmnId = -1;
        }

        @Override
        public int getSize() {
            return data.size();
        }

        @Override
        public Object getElementAt(int rowNum) {
            return Lexicon.moveNames[data.get(rowNum)];
        }

        public int getRowID(int rowNum) {
            return data.get(rowNum);
        }

        public int getRowOfId(int id) {
            for (int i = 0; i < data.size(); i++) {
                if (data.get(i) == id) {
                    return i;
                }
            }
            return -1;
        }

        public Collection<Integer> getRandomMoveset(int maxMoves) {
            return new MoveSelector(pkmnId, rules).getRandomMoveset(maxMoves);
        }

        public void updateRules(EnumSet<Rule> rules) {
            this.rules = rules;
            data = new MoveSelector(pkmnId, rules).getMoveList();
            this.fireContentsChanged(this, 0, data.size());
        }

        public void updatePkmnId(int pkmnId) {
            this.pkmnId = pkmnId;
            data = new MoveSelector(pkmnId, rules).getMoveList();
            this.fireContentsChanged(this, 0, data.size());
        }
    }
}
