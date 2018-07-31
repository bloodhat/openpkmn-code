/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.net.*;
import java.io.*;
import java.util.EnumSet;
import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import openpkmnclient.BattleModel.BattleStatus;

/**
 *
 * @author matt
 */
public class MainController extends OpenpkmnNetworkController {

    DataInputStream waitInputStream;
    DataInputStream stateInputStream;
    DataOutputStream waitOutputStream;
    DataInputStream challengeInputStream;
    DataOutputStream challengeOutputStream;

    Object doStateUpdate() {
        Header header = new Header();
        if (header.receive(stateInputStream)) {
            if (header.getPacketType() == ChatTextPacket.CHAT_TEXT_PACKET_TYPE) {
                ChatTextPacket chatText = new ChatTextPacket();
                if (chatText.receiveHeadless(header.getPacketLen(),
                        stateInputStream, eScheme, userName)) {
                    return new ChatText(chatText.getUserName(),
                            chatText.getText());
                }
            } else if (header.getPacketType() == UserChangePacket.USER_CHANGE_PACKET_TYPE) {
                UserChangePacket userChangePacket = new UserChangePacket();
                userChangePacket.receiveHeadless(header.getPacketLen(),
                        stateInputStream, eScheme, userName);
                return userChangePacket.getUser();
            }
        }

        return null;
    }

    TeamAndPkmnUpdater doDeletePkmn(int id) {
        DeletePkmnPacket deletePkmnPacket = new DeletePkmnPacket(id);
        Socket sock = null;
        if ((sock = connect(serverName, port)) != null) {
            DataOutputStream outputStream = getOutputStream(sock);
            DataInputStream inputStream = getInputStream(sock);
            if (deletePkmnPacket.send(outputStream, eScheme, userName)) {
                ServerResponse response = new ServerResponse();
                if (response.receive(inputStream, eScheme, userName)) {
                    if (response.getResponseCode() == ServerResponse.ACK_CODE) {
                        List<AvailablePkmn> availablePkmn = doPkmnUpdate();
                        List<AvailableTeam> availableTeams = doTeamsUpdate();
                        return new TeamAndPkmnUpdater(availablePkmn,
                                availableTeams);
                    }
                }
            }
        }
        return null;
    }

     TeamAndPkmnUpdater doDeleteTeam(int id) {
        DeleteTeamPacket deleteTeamPacket = new DeleteTeamPacket(id);
        Socket sock = null;
        if ((sock = connect(serverName, port)) != null) {
            DataOutputStream outputStream = getOutputStream(sock);
            DataInputStream inputStream = getInputStream(sock);
            if (deleteTeamPacket.send(outputStream, eScheme, userName)) {
                ServerResponse response = new ServerResponse();
                if (response.receive(inputStream, eScheme, userName)) {
                    if (response.getResponseCode() == ServerResponse.ACK_CODE) {
                        List<AvailablePkmn> availablePkmn = doPkmnUpdate();
                        List<AvailableTeam> availableTeams = doTeamsUpdate();
                        return new TeamAndPkmnUpdater(availablePkmn,
                                availableTeams);
                    }
                }
            }
        }
        return null;
    }

    public enum BattleCommand {

        NoCommand, Attacking, Switching, Running
    };
    private int battlePort;
    private DataInputStream battleInputStream;
    private DataOutputStream battleOutputStream;

    BattleData doAcceptChallenge() {
        ChallengeResponsePacket response = new ChallengeResponsePacket(true);
        if (response.send(waitOutputStream, eScheme, userName)) {
            ServerResponse serverResponse = new ServerResponse();
            if (serverResponse.receive(waitInputStream, eScheme, userName)) {
                if (serverResponse.getResponseCode()
                        == ServerResponse.ACK_CODE) {
                    if (receiveEngineParams(waitInputStream) != null) {
                        System.err.println("doAcceptChallenge - received "
                                + "engine params");
                        return setUpBattle();
                    }
                } else {
                }
            }

        }
        return null;
    }

    boolean doCancel() {
        CancelPacket cancel = new CancelPacket();
        Socket sock = null;
        if ((sock = connect(serverName, port)) != null) {
            DataOutputStream outputStream = getOutputStream(sock);
            DataInputStream inputStream = getInputStream(sock);
            if (cancel.send(outputStream, eScheme, userName)) {
                ServerResponse response = new ServerResponse();
                if (response.receive(inputStream, eScheme, userName)) {
                    if (response.getResponseCode()
                            == ServerResponse.ACK_CODE) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    boolean doSendChatMessage(String text, int broadcast, List<Integer> users) {
        byte recipientMask[] = new byte[1024 / 8];
        if (broadcast == 0) {
            Arrays.fill(recipientMask, (byte) 0xff);
        } else {
            for (int userId : users) {
                recipientMask[userId / 8] |= (0x1 << (userId % 8));
            }
        }
        ChatMessagePacket chatMessage =
                new ChatMessagePacket(text, recipientMask);
        Socket sock = null;
        if ((sock = connect(serverName, port)) != null) {
            DataOutputStream outputStream = getOutputStream(sock);
            DataInputStream inputStream = getInputStream(sock);
            if (chatMessage.send(outputStream, eScheme, userName)) {
                return true;
            }
        }
        return false;
    }

    boolean doLogout() {
        LogoutPacket logout = new LogoutPacket();
        Socket sock = null;
        if ((sock = connect(serverName, port)) != null) {
            DataOutputStream outputStream = getOutputStream(sock);
            if (logout.send(outputStream, eScheme, userName)) {
                return true;
            }
        }
        return false;
    }

    boolean doRejectChallenge() {
        ChallengeResponsePacket response = new ChallengeResponsePacket(false);
        if (response.send(waitOutputStream, eScheme, userName)) {
            return true;
        }
        return false;
    }

    List<AvailableTeam> doFormTeam(Collection<Integer> members) {
        boolean success = true;
        FormTeamPacket formTeamPacket = new FormTeamPacket(members);
        Socket sock = null;
        if ((sock = connect(serverName, port)) != null) {
            DataInputStream inputStream = getInputStream(sock);
            DataOutputStream outputStream = getOutputStream(sock);
            formTeamPacket.send(outputStream, eScheme,
                    userName);
            ServerResponse response = new ServerResponse();
            if (response.receive(inputStream, eScheme, userName)) {
                if (response.getResponseCode()
                        == ServerResponse.ACK_CODE) {
                    System.err.println("Team accepted\n");
                } else {
                    success = false;
                }
            } else {
                success = false;
            }
            if (success) {
                return getTeams();
            }
        }
        return null;
    }

    List<AvailablePkmn> doPkmnSend(String nickname, int number,
            Collection<Integer> moveNumbers, String levelText,
            boolean attackDVSelected, String attackText,
            boolean defenseDVSelected, String defenseText,
            boolean speedDVSelected, String speedText,
            boolean specialDVSelected, String specialText, String hpText) {
        boolean success = true;
        boolean selection[] = new boolean[5];
        String statText[] = new String[5];
        int stats[] = new int[5];
        int statExp[] = Environment.getMaxStatExp();
        int dvs[] = new int[5];

        selection[1] = attackDVSelected;
        selection[2] = defenseDVSelected;
        selection[3] = speedDVSelected;
        selection[4] = specialDVSelected;
        selection[0] = selection[1] && selection[2] && selection[3]
                && selection[4];

        statText[0] = hpText;
        statText[1] = attackText;
        statText[2] = defenseText;
        statText[3] = speedText;
        statText[4] = specialText;

        if (nickname.equals("")) {
            nickname = Lexicon.pkmnNames[number];
        }

        int level = Integer.parseInt(levelText);
        for (int i = 1; i < 5; i++) {
            if (selection[i]) {
                int dv = 0;
                dvs[i] = dv = Integer.parseInt(statText[i]);
                stats[i] = Environment.statFormula(
                        Environment.pkmnStats[number][i],
                        dv, level, statExp[i]);
            } else {
                stats[i] = 0;
                stats[i] = Integer.parseInt(statText[i]);
            }
        }
        if (selection[0]) {
            int dv = ((dvs[1] % 2) * 8)
                    + ((dvs[2] % 2) * 4)
                    + ((dvs[3] % 2) * 2)
                    + ((dvs[4] % 2));

            stats[0] = Environment.hpFormula(Environment.pkmnStats[number][0],
                    dv, level, statExp[0]);
        } else {
            stats[0] = 0;
            stats[0] = Integer.parseInt(hpText);
        }
        ArrayList<Move> moves = new ArrayList<Move>();
        for (Integer m : moveNumbers) {
            moves.add(new Move(m, 3));
        }
        ServerResponse response = new ServerResponse();

        PkmnSetupPacket pkmn = new PkmnSetupPacket(nickname, number, level,
                stats, moves);
        Socket sock = null;
        if ((sock = connect(serverName, port)) != null) {
            DataInputStream inputStream = getInputStream(sock);
            DataOutputStream outputStream = getOutputStream(sock);
            if (pkmn.send(outputStream, eScheme, userName)) {
                if (response.receive(inputStream, eScheme, userName)) {
                    if (response.getResponseCode()
                            == ServerResponse.ACK_CODE) {
                    } else {
                        success = false;
                    }
                } else {
                    success = false;
                }
            } else {
                success = false;
            }
            if (success) {
                return doPkmnUpdate();
            }
        }
        return null;
    }

    public class TeamAndPkmnUpdater {

        private List<AvailablePkmn> availablePkmn;
        private List<AvailableTeam> availableTeams;

        public TeamAndPkmnUpdater(List<AvailablePkmn> availablePkmn,
                List<AvailableTeam> availableTeams) {
            this.availablePkmn = availablePkmn;
            this.availableTeams = availableTeams;
        }

        public List<AvailablePkmn> getAvailablePkmn() {
            return availablePkmn;
        }

        public List<AvailableTeam> getAvailableTeams() {
            return availableTeams;
        }
    }

    TeamAndPkmnUpdater doSendRandomTeam(EnumSet<Rule> prefs) {
        boolean success = true;
        ArrayList<PkmnSetupPacket> pkmnList = new ArrayList();
        PkmnSelector pSelect;
        MoveSelector mSelect;

        boolean evolvedOnly = true;

        pSelect = new PkmnSelector(prefs, evolvedOnly);

        int pkmnNum;
        Collection<Integer> moveset;

        int dvs[] = new int[5];
        dvs[0] = 15;
        dvs[1] = 15;
        dvs[2] = 15;
        dvs[3] = 15;
        dvs[4] = 15;

        int[] statExp = Environment.getMaxStatExp();

        ArrayList<Move> moveList;


        for (int pkmnCount = 0; pkmnCount < 6; pkmnCount++) {
            pkmnNum = pSelect.getRandomPkmn(false);

            mSelect = new MoveSelector(pkmnNum, prefs);

            moveset = mSelect.getRandomMoveset(4);

            moveList = new ArrayList();
            for (int move : moveset) {
                moveList.add(new Move(move, 3));
            }

            int stats[] = new int[5];
            stats[0] =
                    Environment.hpFormula(Environment.pkmnStats[pkmnNum][0],
                    dvs[0], 100, statExp[0]);
            for (int i = 1; i < 5; i++) {
                stats[i] = Environment.statFormula(
                        Environment.pkmnStats[pkmnNum][i], dvs[i], 100,
                        statExp[i]);
            }

            PkmnSetupPacket pkmn = new PkmnSetupPacket(
                    Lexicon.pkmnNames[pkmnNum],
                    pkmnNum, 100, stats, moveList);
            pkmnList.add(pkmn);
        }


        Socket sock;
        for (int pkmnCount = 0; pkmnCount < 6; pkmnCount++) {

            if ((sock = connect(serverName, port)) != null) {
                DataInputStream inputStream = getInputStream(sock);
                DataOutputStream outputStream = getOutputStream(sock);
                ServerResponse response = new ServerResponse();
                if (pkmnList.get(pkmnCount).send(outputStream, eScheme,
                        userName)) {
                    if (response.receive(inputStream, eScheme, userName)) {
                        if (response.getResponseCode()
                                == ServerResponse.ACK_CODE) {
                            System.out.println("Success");
                        } else {
                            success = false;
                            System.out.println("Failure");
                        }

                    } else {
                        success = false;
                        System.out.println("Could not receive response");
                    }

                } else {
                    success = false;
                    System.out.println("Could not send pkmn");
                }
            } else {
                success = false;
                System.out.println("Could not connect");
            }
        }
        // Gets the six new pkmn.
        List<AvailablePkmn> availablePkmnList =
                new ArrayList<AvailablePkmn>();
        RequestAvailablePkmnPacket requestAvailablePacket =
                new RequestAvailablePkmnPacket();

        if ((sock = connect(serverName, port)) != null) {
            DataInputStream inputStream = getInputStream(sock);
            DataOutputStream outputStream = getOutputStream(sock);

            if (requestAvailablePacket.send(outputStream, eScheme,
                    userName)) {
                ServerResponse response = new ServerResponse();
                if (response.receive(inputStream, eScheme, userName)) {
                    if (response.getResponseCode()
                            == ServerResponse.ACK_CODE) {
                        AvailablePkmnListPacket availablePkmn =
                                new AvailablePkmnListPacket();
                        if (availablePkmn.receive(inputStream, eScheme,
                                userName)) {
                            // Sets availablePkmnList to the last six available pkmn.
                            availablePkmnList =
                                    availablePkmn.getPkmn().
                                    subList(availablePkmn.getPkmn().size() - 6,
                                    availablePkmn.getPkmn().size());
                        } else {
                            success = false;
                            System.err.println("Could not receive list");
                        }
                    } else {
                        success = false;
                        System.err.println("Server rejected request available");
                    }
                } else {
                    success = false;
                    System.err.println("Server rejected request available");
                }
            } else {
                System.err.println("Bad response to request available");
            }
        } else {
            System.err.println("could not connect");
        }


        // Forms a team from the six new pkmn.
        ArrayList<Integer> pkmnIndices = new ArrayList<Integer>();
        for (AvailablePkmn currAvailablePkmn : availablePkmnList) {
            pkmnIndices.add(currAvailablePkmn.getIndex());
        }
        FormTeamPacket formTeamPacket = new FormTeamPacket(pkmnIndices);
        if ((sock = connect(serverName, port)) != null) {
            DataInputStream inputStream = getInputStream(sock);
            DataOutputStream outputStream = getOutputStream(sock);

            if (formTeamPacket.send(outputStream, eScheme, userName)) {

                ServerResponse teamResponse = new ServerResponse();
                if (teamResponse.receive(inputStream, eScheme, userName)) {
                    if (teamResponse.getResponseCode()
                            == ServerResponse.ACK_CODE) {
                        System.err.println("Team accepted");
                    }
                }
            } else {
                success = false;
                System.err.println("could not send form team pkt");
            }
        } else {
            success = false;
            System.err.println("could not connect");
        }
        if (success) {
            List<AvailablePkmn> availablePkmn = doPkmnUpdate();
            List<AvailableTeam> availableTeams = doTeamsUpdate();
            return new TeamAndPkmnUpdater(availablePkmn, availableTeams);
        }
        return null;
    }

    private boolean sendLogin() {
        LoginPacket login = new LoginPacket();
        Socket sock = null;
        if ((sock = connect(serverName, port)) != null) {
            DataInputStream inputStream = getInputStream(sock);
            DataOutputStream outputStream = getOutputStream(sock);
            if (login.send(outputStream, eScheme, userName)) {
                // receive the server response
                ServerResponse response = new ServerResponse();
                if (response.receive(inputStream, eScheme, userName)) {
                    if (response.getResponseCode() == ServerResponse.ACK_CODE) {
                        stateInputStream = inputStream;
                        return true;
                    } else {
                        System.out.println("the server rejected"
                                + " your login");
                    }
                }
            }
        }
        return false;
    }

    private List<AvailableUser> getUsers() {
        RequestAvailableUsersPacket rauPacket =
                new RequestAvailableUsersPacket();
        Socket sock = null;
        if ((sock = connect(serverName, port)) != null) {
            DataInputStream inputStream = getInputStream(sock);
            DataOutputStream outputStream = getOutputStream(sock);
            rauPacket.send(outputStream, eScheme, userName);

            ServerResponse response = new ServerResponse();
            if (response.receive(inputStream, eScheme, userName)) {
                if (response.getResponseCode()
                        == ServerResponse.ACK_CODE) {
                    AvailableUsersListPacket availableUsers =
                            new AvailableUsersListPacket();
                    availableUsers.receive(inputStream,
                            eScheme, userName);
                    return availableUsers.getUsers();
                } else {
                }
            } else {
            }
        }
        return null;
    }

    private List<AvailablePkmn> getPkmn() {
        RequestAvailablePkmnPacket requestAvailablePacket =
                new RequestAvailablePkmnPacket();
        Socket sock = null;
        if ((sock = connect(serverName, port)) != null) {
            DataInputStream inputStream = getInputStream(sock);
            DataOutputStream outputStream = getOutputStream(sock);
            requestAvailablePacket.send(outputStream, eScheme,
                    userName);
            ServerResponse response = new ServerResponse();
            if (response.receive(inputStream, eScheme, userName)) {
                if (response.getResponseCode()
                        == ServerResponse.ACK_CODE) {
                    AvailablePkmnListPacket availablePkmn =
                            new AvailablePkmnListPacket();
                    availablePkmn.receive(inputStream,
                            eScheme,
                            userName);
                    return availablePkmn.getPkmn();
                } else {
                    System.err.println("Server rejected request available");
                }
            } else {
                System.err.println("Bad response to request available");
            }
        }
        return null;
    }

    private List<AvailableTeam> getTeams() {
        RequestAvailableTeamsPacket requestAvailablePacket =
                new RequestAvailableTeamsPacket();
        Socket sock = null;
        if ((sock = connect(serverName, port)) != null) {
            DataInputStream inputStream = getInputStream(sock);
            DataOutputStream outputStream = getOutputStream(sock);
            requestAvailablePacket.send(outputStream, eScheme,
                    userName);
            ServerResponse response = new ServerResponse();
            if (response.receive(inputStream, eScheme, userName)) {
                if (response.getResponseCode()
                        == ServerResponse.ACK_CODE) {
                    AvailableTeamsListPacket availableTeams =
                            new AvailableTeamsListPacket();
                    availableTeams.receive(inputStream,
                            eScheme,
                            userName);
                    return availableTeams.getTeams();
                } else {
                    System.err.println("Server rejected request available");
                }
            } else {
                System.err.println("Bad response to request available");
            }
        }
        return null;
    }

    public class InitialStatus {

        List<AvailableUser> au;
        List<AvailablePkmn> ap;
        List<AvailableTeam> at;
        String userName;

        public InitialStatus(List<AvailableUser> au, List<AvailablePkmn> ap,
                List<AvailableTeam> at, String userName) {
            this.au = au;
            this.ap = ap;
            this.at = at;
            this.userName = userName;
        }

        public List<AvailableUser> getAvailableUsers() {
            return au;
        }

        public List<AvailablePkmn> getAvailablePkmn() {
            return ap;
        }

        public List<AvailableTeam> getAvailableTeams() {
            return at;
        }

        public String getUserName() {
            return userName;
        }

        public int getRules() {
            int rules = 0xFFFFFFFF;
            for (AvailableUser u : au) {
                if (u.getUserName().equals(userName)) {
                    rules = u.getRuleset();
                    break;
                }
            }
            return rules;
        }
    }

    public InitialStatus doLogin(Credentials cred) {
        List<AvailableUser> au;
        List<AvailablePkmn> ap;
        List<AvailableTeam> at;
        if (setConnectionInfo(cred)) {
            if (sendLogin()) {
                if ((au = getUsers()) != null) {
                    if ((ap = getPkmn()) != null) {
                        if ((at = getTeams()) != null) {
                            {
                                return new InitialStatus(au, ap, at,
                                        userName);
                            }
                        }
                    }
                }
            }

        }
        return null;
    }

    public List<AvailableUser> doUsersUpdate() {
        return getUsers();
    }

    public List<AvailablePkmn> doPkmnUpdate() {
        return getPkmn();
    }

    public List<AvailableTeam> doTeamsUpdate() {
        return getTeams();
    }

    public BattleAndOpponentData doImmediateBattle() {
        String opponentName = receiveEngineParams(waitInputStream);
        if(opponentName != null) {
            if(!opponentName.equals(userName)) {
                BattleData data = setUpBattle();
                return new BattleAndOpponentData(opponentName,
                        data.getEventLog(), data.getServerStatus());
            } else {
                return new BattleAndOpponentData(opponentName, null, null);
            }
            
        }
        return null;
    }

    private String receiveEngineParams(DataInputStream inputStream) {
        EngineParametersPacket engineParamsPacket =
                new EngineParametersPacket();
        if (engineParamsPacket.receive(inputStream, eScheme, userName)) {
            System.err.println("Received engine params\n");
            setBattlePort(engineParamsPacket.getPort());
            return engineParamsPacket.getOpponentName();
        }
        return null;
    }

    public boolean doWait(int teamId, int starter) {
        WaitPacket waitPacket = new WaitPacket(teamId, starter);
        Socket sock = null;
        if ((sock = connect(serverName, port)) != null) {
            waitInputStream = getInputStream(sock);
            waitOutputStream = getOutputStream(sock);
            if (waitPacket.send(waitOutputStream, eScheme, userName)) {
                ServerResponse response = new ServerResponse();
                if (response.receive(waitInputStream, eScheme, userName)) {
                    if (response.getResponseCode() == ServerResponse.ACK_CODE) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    public boolean doFindBattle(int teamId, int starter) {
        BattleNowPacket battleNowPacket = new BattleNowPacket(teamId,
                starter);
        Socket sock = null;
        if ((sock = connect(serverName, port)) != null) {
            waitInputStream = getInputStream(sock);
            waitOutputStream = getOutputStream(sock);
            if (battleNowPacket.send(waitOutputStream, eScheme, userName)) {
                ServerResponse response = new ServerResponse();
                if (response.receive(waitInputStream, eScheme, userName)) {
                    if (response.getResponseCode() == ServerResponse.ACK_CODE) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    public int doReceiveChallenge() {
        ChallengeNotificationPacket notification =
                new ChallengeNotificationPacket();
        if (notification.receive(waitInputStream,
                eScheme, userName)) {
            return notification.getChallengerID();

        }
        return -1;
    }

    public boolean doChallenge(int userId, int teamId, int starter) {
        ChallengePacket challengePacket = new ChallengePacket(userId, teamId,
                starter);
        Socket sock = null;
        if ((sock = connect(serverName, port)) != null) {
            challengeInputStream = getInputStream(sock);
            challengeOutputStream = getOutputStream(sock);
            if (challengePacket.send(challengeOutputStream,
                    eScheme, userName)) {
                ServerResponse response = new ServerResponse();
                if (response.receive(challengeInputStream, eScheme, userName)) {
                    if (response.getResponseCode() == ServerResponse.ACK_CODE) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    public BattleData doReceiveChallengeResponse() {
        ChallengeResponsePacket challengeResponse =
                new ChallengeResponsePacket();
        if (challengeResponse.receive(challengeInputStream, eScheme,
                userName)) {
            if (challengeResponse.getAcceptance()) {
                if (receiveEngineParams(challengeInputStream) != null) {       
                    return setUpBattle();
                }
            } else {
                /* If the battle is not accepted it's not an error, so we can't
                 * send back null. We make a known invalid BattleData.
                 */
                return new BattleData(null, null);
            }
        }
        return null;
    }

    public boolean doRuleChange(EnumSet<Rule> prefs) {
        boolean retVal = false;
        int rulesValue = Rule.getRuleValueForSet(prefs);
        PreferencePacket prefPacket =
                new PreferencePacket(rulesValue);
        Socket sock = null;
        if ((sock = connect(serverName, port)) != null) {
            DataInputStream inputStream = getInputStream(sock);
            DataOutputStream outputStream = getOutputStream(sock);
            prefPacket.send(outputStream, eScheme, userName);
            ServerResponse response = new ServerResponse();
            if (response.receive(inputStream, eScheme, userName)) {
                if (response.getResponseCode()
                        == ServerResponse.ACK_CODE) {
                    System.out.println("Success");
                    retVal = true;
                } else {
                    System.out.println("Failure");
                    rulesValue = -1;
                }
            }
        }
        return retVal;
    }

    public BattleData doTurn(int command, int option) {
        ClientResponse cr = new ClientResponse(command, option);
        if (cr.send(battleOutputStream, eScheme, userName)) {
            return getBattleData(battleInputStream);
        }
        return null;
    }

    public BattleData setUpBattle() {
        Socket sock = null;
        boolean notConnected = true;
        while (notConnected) {
            notConnected = false;
            /* the server may not have synced up, but keep trying */
            if ((sock = connect(serverName, battlePort)) == null) {
                notConnected = true;
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                }
            }
        }
        battleInputStream = getInputStream(sock);
        battleOutputStream = getOutputStream(sock);
        LoginPacket battleLogin = new LoginPacket();
        if (battleLogin.send(battleOutputStream, eScheme, userName)) {
            return getBattleData(battleInputStream);
        } else {
            System.err.println("could not login to battle");
        }
        return null;
    }

    private BattleData getBattleData(DataInputStream input) {
        ServerStatus status = new ServerStatus();
        if (status.receive(input, eScheme, userName)) {

            EventLog eventLog = new EventLog();
            if (eventLog.receive(input, eScheme, userName)) {
                System.err.println("received event log\n");
                return new BattleData(eventLog, status);
            }
        } else {
            System.err.println("could not read server status\n");
        }
        return null;
    }

    void setBattlePort(int battlePort) {
        this.battlePort = battlePort;
    }

    public class BattleAndOpponentData extends BattleData{
        private String opponentName;
       public BattleAndOpponentData(String opponentName, EventLog eventLog,
               ServerStatus serverStatus) {
           super(eventLog, serverStatus);
           this.opponentName = opponentName;
       }

       String getOpponentName() {
           return opponentName;
       }
    }
    
    public class BattleData {

        private EventLog eventLog;
        private ServerStatus serverStatus;

        public BattleData(EventLog eventLog, ServerStatus serverStatus) {
            this.eventLog = eventLog;
            this.serverStatus = serverStatus;
        }

        public EventLog getEventLog() {
            return eventLog;
        }

        public ServerStatus getServerStatus() {
            return serverStatus;
        }
    }
}
