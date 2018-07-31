/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.util.ArrayList;
import java.util.Collection;
import java.util.EventListener;
import java.util.EventObject;

/**
 *
 * @author matt
 */
public class BattleModel {
    String userName;
    String opponentName;
    private final Collection<BattleModelChangedEventListener> listeners;

    public BattleModel() {
        listeners = new ArrayList<BattleModelChangedEventListener>();
    }

    public enum BattleStatus { NormalRound, OwnFaint, OpponentFaint,
        BothFaint, OwnLoss, OpponentLoss, Tie };

    BattleStatus battleStatus = BattleStatus.NormalRound;
    boolean battleWaiting = false;

    public void setUserName(String u) {
        userName = u;
    }

    public String getUserName() {
        return userName;
    }

    public void setOpponentName(String u) {
        opponentName = u;
    }

    public String getOpponentName() {
        return opponentName;
    }


    public BattleStatus getStatus() {
        return battleStatus;
    }

    public void setBattleWaiting() {
        battleWaiting = true;
    }

    public void setBattleStatus(BattleStatus s) {
        battleWaiting = false;
        battleStatus = s;
        fireBattleStatusChanged(s);
    }

    public void addListener(BattleModelChangedEventListener l) {
        synchronized(listeners) {
            listeners.add(l);
        }
    }
    
    public void removeListener(BattleModelChangedEventListener l) {
        synchronized(listeners) {
            listeners.remove(l);
        }
    }

    public void fireBattleStatusChanged(BattleStatus newStatus) {
        synchronized(listeners) {
            for(BattleModelChangedEventListener l : listeners) {
                l.battleModelChanged(new BattleModelChangedEventListener.
                        BattleModelChangedEvent(this, newStatus));
            }
        }
    }

    public interface BattleModelChangedEventListener extends EventListener {
        public class BattleModelChangedEvent extends EventObject {
            BattleStatus status;
            public BattleModelChangedEvent(Object source, BattleStatus status) {
                super(source);
                this.status = status;
            }

            public BattleStatus getStatus() {
                return status;
            }
        }

        public void battleModelChanged(BattleModelChangedEvent e);
    }
}
