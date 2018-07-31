/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.io.Serializable;
import java.util.ArrayList;
import javax.swing.JTable;
import java.util.Collection;
import java.util.List;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.TableModelEvent;
import openpkmnclient.TeamSelectedListener.TeamSelectedEvent;

/**
 *
 * @author matt
 */
public class AvailableTeamsTable extends JTable implements Serializable {

    private List<Integer> selectedTeamIds;
    private Collection<TeamSelectedListener> listeners;

    public AvailableTeamsTable() {
        super(new AvailableTeamsTableModel());
        setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
        listeners = new ArrayList<TeamSelectedListener>();
        selectedTeamIds = new ArrayList<Integer>();
    }

    public void load(List<AvailableTeam> teams) {
        ((AvailableTeamsTableModel) getModel()).load(teams);
    }

    public Collection<Integer> getSelectedTeamMembers() {
        int[] rows = this.getSelectedRows();
        List<Integer> members = new ArrayList<Integer>();
        for (int row : rows) {
            members.addAll(((AvailableTeamsTableModel) getModel()).getRowMembers(row));
        }
        return members;
    }

    int getSelectedTeamRules() {
        int[] rows = this.getSelectedRows();
        if (rows.length == 1) {
            return Rule.getRuleValueFromCodes(
                    ((AvailableTeamsTableModel) getModel()).getRowRules(rows[0]));
        } else {
            return -1;
        }
    }

    public void addTeamSelectedListener(TeamSelectedListener l) {
        listeners.add(l);
    }

    public void removeTeamSelectedListener(TeamSelectedListener l) {
        listeners.remove(l);
    }

    public void fireTeamSelectedChanged(List<Integer> teams,
            Collection<Integer> members, String rules) {
        for (TeamSelectedListener l : listeners) {
            l.selectedTeamChanged(new TeamSelectedEvent(this, teams,
                    members, rules));
        }
    }

    @Override
    public void valueChanged(ListSelectionEvent e) {
        super.valueChanged(e);
        int[] selectedRows = getSelectedRows();
        /* our value can get changed to -1 if an external source causes a
         * deselection, so we need to cope with that by making it so no team is
         * selected
         */
        List<Integer> teams = new ArrayList<Integer>();
        Collection<Integer> members = new ArrayList<Integer>();
        for (int selectedRow : selectedRows) {
            teams.add(((AvailableTeamsTableModel) getModel()).getRowID(selectedRow));
            members.addAll(((AvailableTeamsTableModel) getModel()).getRowMembers(selectedRow));
        }

        String rules;
        int numTeams = selectedRows.length;
        if (numTeams == 1) {
            rules = ((AvailableTeamsTableModel) getModel()).getRowRules(selectedRows[0]);
            selectedTeamIds.clear();
            selectedTeamIds.add(teams.get(0));
            fireTeamSelectedChanged(teams, members, rules);
        } else if(numTeams == 0) {
            selectedTeamIds.clear();
            fireTeamSelectedChanged(null, null, null);
        } else {
            selectedTeamIds.clear();
            selectedTeamIds.addAll(teams);
            fireTeamSelectedChanged(teams, members, null);
        }
    }

    @Override
    public void tableChanged(TableModelEvent e) {
        super.tableChanged(e);
        /* It turns out setModel calls this method, so we need to make sure
         * we aren't dealing with that specific call */
        AvailableTeamsTableModel tmodel = (AvailableTeamsTableModel) getModel();
        if (tmodel != null) {
            /* We want our selected team id to be -1 if not already set
             * (i.e. it is being called from the constructor) and we have not
             * set it to -1 yet. */
            int selectedRow;
            if (selectedTeamIds != null) {
                for (int selectedTeamId : selectedTeamIds) {
                    selectedRow = tmodel.getRowOfId(selectedTeamId);
                    if (selectedRow >= 0) {
                        setRowSelectionInterval(selectedRow, selectedRow);
                    }
                }
            }
        }
    }

    // Getters and setters allow this to be a Java Bean
    public Collection<TeamSelectedListener> getListeners() {
        return listeners;
    }

    public void setListeners(final Collection<TeamSelectedListener> listeners) {
        this.listeners = listeners;
    }

    public List<Integer> getSelectedTeamIds() {
        return selectedTeamIds;
    }

    public void setSelectedTeamIds(final List<Integer> selectedTeamIds) {
        this.selectedTeamIds = selectedTeamIds;
    }

    List<Integer> getSelectedIds() {
        return ((AvailableTeamsTableModel) this.getModel()).getSelectedIds();
    }

    public static void main(String args[]) {
        new AvailableTeamsTable();
    }
}
