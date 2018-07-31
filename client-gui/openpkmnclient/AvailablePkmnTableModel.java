/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import javax.swing.table.AbstractTableModel;

/**
 *
 * @author matt
 */
public class AvailablePkmnTableModel extends AbstractTableModel {

    private String[] columnNames = {"Select", "Species", "ID", "Move 1",
        "Move 2", "Move 3", "Move 4"};
    private Class[] columnClasses = {Boolean.class, String.class,
        Integer.class, String.class, String.class, String.class,
        String.class, String.class};
    private HashMap<String, ArrayList> data;

    public AvailablePkmnTableModel() {
        data = new HashMap<String, ArrayList>();
        for (int i = 0; i < columnNames.length; i++) {
            data.put(columnNames[i], new ArrayList());
        }
    }

    @Override
    public int getColumnCount() {
        return columnNames.length;
    }

    @Override
    public int getRowCount() {
        int rowCount = data.get("ID").size();
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
        if (col == 0) {
            return true;
        }
        return false;
    }

    @Override
    public void setValueAt(Object value, int row, int col) {
        data.get(columnNames[col]).set(row, value);
        fireTableCellUpdated(row, col);
    }

    public void load(List<AvailablePkmn> pkmn) {
        data = new HashMap<String, ArrayList>();
        for (int i = 0; i < columnNames.length; i++) {
            data.put(columnNames[i], new ArrayList());
        }

        for (AvailablePkmn thisPkmn : pkmn) {
            data.get("Select").add(Boolean.FALSE);
            data.get("Species").add(Lexicon.pkmnNames[thisPkmn.getNum()]);
            int i = 0;
            for (Integer move : thisPkmn.getMoves()) {
                data.get("Move " + (i + 1)).add(Lexicon.moveNames[move]);
                i++;
            }
            for (; i < 4; i++) {
                data.get("Move " + (i + 1)).add(Lexicon.moveNames[0]);
            }
            data.get("ID").add(thisPkmn.getIndex());
        }
        this.fireTableDataChanged();
    }

    public int getRowID(int row) {
        return (Integer) data.get("ID").get(row);
    }

    int getRowOfPkmn(int pkmnID) {
        int i = 0;
        for (int testID : (List<Integer>) data.get("ID")) {
            if (pkmnID == testID) {
                return i;
            }
            i++;
        }
        return -1;
    }

    String getSpeciesAtRow(int row) {
        return ((List<String>) data.get("Species")).get(row);
    }

    List<Integer> getSelectedIds() {
        List<Integer> ids = new ArrayList<Integer>();
        List<Integer> idList = (List<Integer>)data.get("ID");
        List<Boolean> selectList = (List<Boolean>)data.get("Select");
        
        for(int i = 0; i < selectList.size(); i++) {
            if(selectList.get(i)) {
                ids.add(idList.get(i));
            }
        }
        return ids;
    }
}
