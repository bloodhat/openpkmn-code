/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import openpkmnclient.TeamAssembledListener.TeamAssembledEvent;

/**
 *
 * @author matt
 */
public class AvailablePkmnTable extends JTable implements TeamSelectedListener {

    private Collection<TeamAssembledListener> listeners;

    public AvailablePkmnTable() {
        super(new AvailablePkmnTableModel());
        setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
        listeners = new ArrayList<TeamAssembledListener>();
    }

    public void load(List<AvailablePkmn> pkmn) {
        ((AvailablePkmnTableModel) getModel()).load(pkmn);
    }

    int getRowOfPkmn(int pkmnID) {
        return ((AvailablePkmnTableModel) getModel()).getRowOfPkmn(pkmnID);
    }

    String getSpeciesAtRow(int row) {
        return ((AvailablePkmnTableModel) getModel()).getSpeciesAtRow(row);
    }

    public void addTeamAssembledListener(TeamAssembledListener l) {
        listeners.add(l);
    }

    public void removeTeamAssembledListener(TeamAssembledListener l) {
        listeners.remove(l);
    }

    List<Integer> getSelectedIds() {
        return ((AvailablePkmnTableModel) this.getModel()).getSelectedIds();
    }

    public void fireTeamAssembled(TeamAssembledEvent e) {
        for (TeamAssembledListener l : listeners) {
            l.teamAssembled(e);
        }
    }

    @Override
    public void selectedTeamChanged(TeamSelectedEvent e) {
        clearSelection();
        if (e.getSelectedTeams() != null) {
            Collection<Integer> members = e.getTeamMembers();
            String species;
            int row = -1;
            ArrayList<PkmnInfoItem> pkmn = new ArrayList<PkmnInfoItem>();
            for (int member : members) {
                row = getRowOfPkmn(member);
                if (row != -1) {
                    species = getSpeciesAtRow(row);
                    pkmn.add(new PkmnInfoItem(species, member));
                    addRowSelectionInterval(row, row);
                } else {
                    break;
                }
            }
            if (row != -1 && e.getSelectedTeams().size() == 1) {
                fireTeamAssembled(new TeamAssembledEvent(this, pkmn));
                scrollRectToVisible(getCellRect(row, 0, false));
            }
            if (e.getSelectedTeams().size() != 1) {
                /* We couldn't assemble a team because we got an invalid team,
                 * but we need to let our listeners know that by sending them a null
                 * team
                 */
                fireTeamAssembled(new TeamAssembledEvent(this, null));
            }
        } else {
            fireTeamAssembled(new TeamAssembledEvent(this, null));
        }
    }

    public Collection<TeamAssembledListener> getListeners() {
        return listeners;
    }

    public void setListeners(final Collection<TeamAssembledListener> listeners) {
        this.listeners = listeners;
    }
}

