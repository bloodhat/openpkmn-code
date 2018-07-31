/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.util.ArrayList;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.List;
import javax.swing.table.AbstractTableModel;
import openpkmnclient.MoveSelectionPolicyChangedListener.Policy;

/**
 *
 * @author matt
 */
public class LearnableMovesTableModel extends AbstractTableModel {
private String[] columnNames = {"Select", "Move", "PP Ups", "Learned"};
    private Class[] columnClasses = {Boolean.class, String.class,
        Integer.class, String.class};
    private HashMap<String, ArrayList> data;
    private ArrayList<Integer> ids;
    private int pkmnId;
    private EnumSet<Rule> rules;
    private Policy selectionPolicy;

    public LearnableMovesTableModel() {
        pkmnId = -1;
        rules = EnumSet.noneOf(Rule.class);
        data = new HashMap<String, ArrayList>();
        for (int i = 0; i < columnNames.length; i++) {
            data.put(columnNames[i], new ArrayList());
        }
        ids = new ArrayList<Integer>();
    }

    @Override
    public int getColumnCount() {
        return columnNames.length;
    }

    @Override
    public int getRowCount() {
        int rowCount = ids.size();
        return rowCount;
    }

    @Override
    public String getColumnName(int col) {
        return columnNames[col];
    }

    @Override
    public Object getValueAt(int row, int col) {
        return data.get(columnNames[col]).get(row);
    }

    @Override
    public Class getColumnClass(int c) {
        return columnClasses[c];
    }

    @Override
    public boolean isCellEditable(int row, int col) {
        if (col == 1) {
            return false;
        }
        return true;
    }

    private void updateSelection() {
        if(selectionPolicy != null) {
        if(selectionPolicy.equals(Policy.RANDOM)) {
            selectRandom();
        } else {
            clearSelected();
        }
        }
    }

    public void changeRules(EnumSet<Rule> rules) {
        this.rules = rules;
        List<Integer> moves = new MoveSelector(pkmnId, rules).getMoveList();
        load(moves);
        updateSelection();
    }

    public void changePkmn(int id) {
        this.pkmnId = id;
        List<Integer> moves = new MoveSelector(pkmnId, rules).getMoveList();
        load(moves);
        updateSelection();
    }

    @Override
    public void setValueAt(Object value, int row, int col) {
        data.get(columnNames[col]).set(row, value);
        fireTableCellUpdated(row, col);
    }

    public void changeSelectionPolicy(Policy selectionPolicy) {
        this.selectionPolicy = selectionPolicy;
        updateSelection();
    }

    private void load(List<Integer> moves) {
        for (ArrayList col : data.values()) {
            col.clear();
        }
        ids.clear();

        List<Boolean> select = data.get("Select");
        List<String> name = data.get("Move");
        List<Integer> ppUp = data.get("PP Ups");
        List<String> learned = data.get("Learned");
        for (int move : moves) {
            ids.add(move);
            name.add(Lexicon.moveNames[move]);
            select.add(Boolean.FALSE);
            ppUp.add(3);
            learned.add("");
        }
        this.fireTableDataChanged();
    }

    public List<Integer> getSelectedMoves() {
        List<Boolean> selectList = (List<Boolean>)data.get("Select");
        List<Integer> selectedIds = new ArrayList<Integer>();
        for(int i = 0; i < selectList.size(); i++) {
            if(selectList.get(i)) {
                selectedIds.add(ids.get(i));
            }
        }
        return selectedIds;
    }

    private void clearSelected() {
        List<Boolean> select = data.get("Select");
        int size = select.size();
        for(int i=0; i < size; i++) {
                select.set(i, Boolean.FALSE);
         }
    }

    private void selectRandom() {
        clearSelected();
        List<Boolean> select = data.get("Select");
        int size = select.size();
        if(size <= 4) {
            for(int i=0; i < size; i++) {
                select.set(i, Boolean.TRUE);
            }
        } else {
            int selected = 0;
            while(selected < 4) {
                int index = (int)(size * Math.random());
                if(select.get(index).equals(Boolean.FALSE)) {
                    select.set(index, Boolean.TRUE);
                    selected++;
                }
            }
        }
    }
}
