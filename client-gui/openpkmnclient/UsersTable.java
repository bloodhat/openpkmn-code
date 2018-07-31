/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.awt.Component;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.TableModelEvent;
import javax.swing.table.DefaultTableCellRenderer;
import openpkmnclient.UserSelectedListener.UserSelectedEvent;

/**
 *
 * @author matt
 */
public class UsersTable extends JTable implements Serializable {

    private Integer selectedUserId;
    private Collection<UserSelectedListener> listeners;

    public UsersTable() {
        super(new UsersTableModel());
        setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        this.setDefaultRenderer(this.getClass(), new UsersTableCellRenderer());
        selectedUserId = -1;
        listeners = new ArrayList<UserSelectedListener>();
    }

    public void load(List<AvailableUser> users) {
        ((UsersTableModel) getModel()).load(users);
    }

    public int getSelectedUserId() {
        return selectedUserId;
    }

    public String getNameById(int id) {
        return ((UsersTableModel) getModel()).getNameById(id);
    }

    public String getRulesById(int id) {
        return ((UsersTableModel) getModel()).getRulesById(id);
    }

    public String getTeamRulesById(int id) {
        return ((UsersTableModel) getModel()).getTeamRulesById(id);
    }

    @Override
    public void valueChanged(ListSelectionEvent e) {
        int selectedRow = getSelectedRow();
        if (selectedRow >= 0) {
            selectedUserId = ((UsersTableModel) getModel()).
                    getRowId(selectedRow);
            
            UsersTableModel model = (UsersTableModel)getModel();
            this.fireUserSelectedChanged(new UserSelectedEvent(this,
                    model.getNameById(selectedUserId),
                    model.getStatusById(selectedUserId),
                    Rule.getRuleValueFromCodes(
                    model.getRulesById(selectedUserId)),
                    Rule.getRuleValueFromCodes(
                    model.getTeamRulesById(selectedUserId))));
        }
        /* Call the super method last, then there's minimal lag time between
         * tableChanged being called */
        super.valueChanged(e);
    }

    @Override
    public void tableChanged(TableModelEvent e) {
        // Call the super method first. Otherwise our reselection is for naught
        super.tableChanged(e);
        /* It turns out setModel calls this method, so we need to make sure
         * we aren't dealing with that specific call */
        UsersTableModel model = (UsersTableModel) getModel();
        if (model != null) {
            int selectedRow = model.getRowById((selectedUserId == null) ? -1 :
                selectedUserId);
            if (selectedRow >= 0) {
                setRowSelectionInterval(selectedRow, selectedRow);
            }
        }
    }

    void updateUser(AvailableUser u) {
        ((UsersTableModel) getModel()).updateUser(u);
    }

    public class UsersTableCellRenderer extends DefaultTableCellRenderer {

        @Override
        public Component getTableCellRendererComponent(JTable table,
                Object value, boolean isSelected, boolean hasFocus, int row,
                int column) {
            Component c = super.getTableCellRendererComponent(table, value,
                    isSelected, hasFocus, row, column);

            // Only for specific cell
            if (true) {
                table.getModel();
            }
            return c;
        }
    }

    public void setSelectedUserId(final Integer selectedUserId) {
        this.selectedUserId = selectedUserId;
    }

    public Collection<UserSelectedListener> getListeners() {
        return listeners;
    }

    public void setListeners(Collection<UserSelectedListener> listeners) {
        this.listeners = listeners;
    }

    public void addUserSelectedListener(UserSelectedListener l) {
        listeners.add(l);
    }

    public void removeUserSelectedListener(UserSelectedListener l) {
        listeners.remove(l);
    }

    public void fireUserSelectedChanged(UserSelectedEvent e) {
        for(UserSelectedListener l : listeners) {
            l.userSelected(e);
        }

    }
}
