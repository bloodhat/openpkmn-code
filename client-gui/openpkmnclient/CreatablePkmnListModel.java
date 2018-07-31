/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.util.ArrayList;
import java.util.EnumSet;
import java.util.List;
import javax.swing.AbstractListModel;

/**
 *
 * @author matt
 */
 public class CreatablePkmnListModel extends AbstractListModel {
        private boolean evolvedOnly;
        private EnumSet<Rule> rules;
        private List<Integer> data;

        public CreatablePkmnListModel() {
            evolvedOnly = true;
            rules = EnumSet.noneOf(Rule.class);
            data = new ArrayList<Integer>();
        }

        @Override
        public int getSize() {
            return data.size();
        }

        @Override
        public Object getElementAt(int rowNum) {
            return Lexicon.pkmnNames[data.get(rowNum)];
        }

        

        public void updateFullyEvolved(boolean evolvedOnly) {
            this.evolvedOnly = evolvedOnly;
            data = new PkmnSelector(rules, evolvedOnly).getList();
            this.fireContentsChanged(this, 0, data.size());
        }

        public void updateRules(EnumSet<Rule> rules) {
            this.rules = rules;
            data = new PkmnSelector(rules, evolvedOnly).getList();
            this.fireContentsChanged(this, 0, data.size());
        }

        public int getRowID(int rowNum) {
            return (Integer) data.get(rowNum);
        }

        public int getRowOfID(int id) {
            for (int i = 0; i < data.size(); i++) {
                if (data.get(i) == id) {
                    return i;
                }
            }
            return -1;
        }
    }
