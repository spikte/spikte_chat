#include "../../include/gui/client_auth.hpp"

/* Server connection */
static uint16_t choiceToTLSVersionNumber(int choice) {
    switch(choice) {
        case 0:
            return 0;
        case 1:
            return TLS1_2_VERSION;
        case 2:
            return TLS1_3_VERSION;
        default:
            return 3;
    }
}
GuiServerInputData guiServerInputData;
void initGuiServerInputData(bool setDefaultAddr) {
    GuiServerInputData& data = guiServerInputData;

    if(setDefaultAddr) {
        std::memset(data.addr, '\0', sizeof(data.addr));
        strcpy(data.addr, "localhost");
        std::memset(data.port, '\0', sizeof(data.port));
        strcpy(data.port, "50000");
    }
    data.editAddr = false;
    data.editPort = false;


    data.errorMsg = "Couldn't connect to " + std::string(data.addr);

    data.headerLabel = "Connect to Server";
    data.serverInputLabel = "Server Address";
    data.tlsVersionChoiceStr = "NONE (unencrypted);TLS 1.2;TLS 1.3";
}
GuiServerInputLayout getGuiServerInputLayout(Rectangle panel) {
    GuiServerInputData& data = guiServerInputData;
    Font& dfFont = guiSettings.defaultFont;
    int dfFontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
    int dfSpacing = GuiGetStyle(DEFAULT, TEXT_SPACING);
    GuiServerInputLayout layout;

    layout.rectPanel = panel;
    panel.y += 20;
    layout.rectHeader = posTextEx(dfFont, panel, RAYLYT_CENTER_X, {0}, data.headerLabel.c_str(), dfFontSize * 2, dfSpacing);
    panel.y += layout.rectHeader.height;
    layout.rectServerInputLabel = posText(panel, RAYLYT_CENTER_X, {30, 0}, data.serverInputLabel.c_str());
    panel.y += layout.rectServerInputLabel.height;
    layout.rectServerInput = pos(panel, RAYLYT_CENTER_X, {30, 0}, {300, 40});
    layout.rectServerInputAddr = pos(layout.rectServerInput, RAYLYT_LEFT, {0}, {150, 40});
    layout.rectServerInputPort = pos(layout.rectServerInput, RAYLYT_RIGHT, {0}, {150, 40});
    panel.y += layout.rectServerInput.height + 10;
    layout.rectDropdown = pos(panel, RAYLYT_CENTER_X, {0}, {180, 20});
    panel.y += layout.rectDropdown.height + 10;
    layout.rectConnectBtn = pos(panel, RAYLYT_CENTER_X, {0}, {80, 25});
    panel.y += layout.rectConnectBtn.height + 10;
    layout.rectErrorMsg = posText(panel, RAYLYT_CENTER_X, {0}, data.errorMsg.c_str());

    return layout;
}
void GuiServerInput() {
    GuiServerInputData &data = guiServerInputData;
    GuiServerInputLayout layout;

    layout = getGuiServerInputLayout(guiSettings.rectServerConnection);
    GuiPanel(layout.rectPanel, NULL);
    DrawTextRectEx(layout.rectHeader,
                   guiSettings.defaultFont,
                   data.headerLabel.c_str(),
                   GuiGetStyle(DEFAULT, TEXT_SIZE) * 2,
                   GuiGetStyle(DEFAULT, TEXT_SPACING),
                   GRAY);
    DrawTextRect(layout.rectServerInputLabel, data.serverInputLabel.c_str(), GRAY);
    if(GuiTextBox(layout.rectServerInputAddr, data.addr, sizeof(data.addr), data.editAddr)) {
        data.editAddr = !data.editAddr;
        data.error = false;
    }
    if(GuiTextBox(layout.rectServerInputPort, data.port, sizeof(data.addr), data.editPort)) {
        data.editPort = !data.editPort;
        data.error = false;
    }
    if(GuiButton(layout.rectConnectBtn, "Connect") && !data.tlsVersionEdit) {
        if(data.addr[0] == '\0' || data.port[0] == '\0')
            return;
        clientConfig.serverAddr = data.addr;
        clientConfig.serverPort = data.port;
        clientConfig.connection.tls = false;
        if(data.tlsVersionChoice != 0) {
            clientConfig.minTlsVersion = choiceToTLSVersionNumber(data.tlsVersionChoice);
            clientConfig.connection.tls = true;
        }
        clientCoreState = ClientCoreState::WAIT_SERVER_CONNECTION;
    }
    if(GuiDropdownBox(layout.rectDropdown, data.tlsVersionChoiceStr.c_str(), &data.tlsVersionChoice, data.tlsVersionEdit)) {
        data.tlsVersionEdit = !data.tlsVersionEdit;
        data.error = false;
    }
    if(data.error)
        DrawTextRect(layout.rectErrorMsg, data.errorMsg.c_str(), RED);
}
void GuiWaitServerInput() {
    GuiDisable();
    GuiServerInput();
    GuiEnable();
    std::string label = "Connecting...";
    DrawTextRect(posText(guiSettings.rectServerConnection, RAYLYT_CENTER_X, {0}, label.c_str()), label.c_str(), BLACK);
}

/* Auth */
GuiAuthData guiAuthData;
void initGuiSignInData(bool initError) {
    GuiAuthData& data = guiAuthData;

    std::memset(data.username, '\0', sizeof(data.username));
    std::memset(data.password, '\0', sizeof(data.password));
    data.usernameEdit = false;
    data.passwordEdit = false;
    if(!initError) {
        data.error = false;
        data.errorMsg = "";
    }

    data.serverInfoLabel = "Connected to " + clientConfig.serverAddr + ":" + clientConfig.serverPort;
    data.headerLabel = "Create account";
    data.usernameLabel = "Username";
    data.passwordLabel = "Password";
    data.btnLabel = "Sign In";
    data.alreadyAuthLabel = "Already have an account ? ";
    data.alreadyAuthOtherLabel = "Login";
}
void initGuiLoginData(bool initError) {
    GuiAuthData& data = guiAuthData;

    std::memset(data.username, '\0', sizeof(data.username));
    std::memset(data.password, '\0', sizeof(data.password));
    data.usernameEdit = false;
    data.passwordEdit = false;
    if(!initError) {
        data.error = false;
        data.errorMsg = "";
    }

    data.serverInfoLabel = "Connected to " + clientConfig.serverAddr + ":" + clientConfig.serverPort;
    data.headerLabel = "Login";
    data.usernameLabel = "Username";
    data.passwordLabel = "Password";
    data.btnLabel = "Login";
    data.alreadyAuthLabel = "Don't have an account ? ";
    data.alreadyAuthOtherLabel = "Sign In";
}
GuiAuthLayout getGuiAuthLayout() {
    GuiAuthData& data = guiAuthData;
    GuiAuthLayout layout;
    Rectangle panel;
    std::string alreadyAuthTotalLabel;

    alreadyAuthTotalLabel = data.alreadyAuthLabel + data.alreadyAuthOtherLabel;
    panel = guiSettings.rectAuth;
    layout.rectPanel = panel;
    panel.y += 20;
    layout.rectServerInfo = posText(panel, RAYLYT_CENTER_X, {0}, data.serverInfoLabel.c_str());
    panel.y += lastLayout.height;
    layout.rectHeader = posTextEx(guiSettings.defaultFont, panel, RAYLYT_CENTER_X, {0}, data.headerLabel.c_str(), GuiGetStyle(DEFAULT, TEXT_SIZE) * 2, GuiGetStyle(DEFAULT, TEXT_SPACING));
    panel.y += lastLayout.height;
    layout.rectUsernameLabel = pos(panel, RAYLYT_CENTER_X, {30, 0}, {240, 20});
    panel.y += lastLayout.height;
    layout.rectUsernameInput = pos(panel, RAYLYT_CENTER_X, {30, 0}, {240, 20});
    panel.y += lastLayout.height;
    layout.rectPasswordLabel = pos(panel, RAYLYT_CENTER_X, {30, 0}, {240, 20});
    panel.y += lastLayout.height;
    layout.rectPasswordInput = pos(panel, RAYLYT_CENTER_X, {30, 0}, {240, 20});
    panel.y += lastLayout.height + 10;
    layout.rectAuthBtn = pos(panel, RAYLYT_CENTER_X, {0}, {80, 25});
    panel.y += lastLayout.height + 10;
    layout.rectAlreadyAuth = posText(panel, RAYLYT_CENTER_X, {0}, alreadyAuthTotalLabel.c_str());
    layout.rectAlreadyAuthQuestion = posText(layout.rectAlreadyAuth, RAYLYT_LEFT, {0}, data.alreadyAuthLabel.c_str());
    layout.rectAlreadyAuthOther = posText(layout.rectAlreadyAuth, RAYLYT_RIGHT, {0}, data.alreadyAuthOtherLabel.c_str());
    layout.rectBackBtn = pos(layout.rectPanel, RAYLYT_TOP | RAYLYT_RIGHT, {0, 5, 5, 0}, {30, 30});
    // Error rect is done dynamicaly when an error is set
    return layout;
}
void GuiAuth() {
    GuiAuthData &data = guiAuthData;
    Font& dfFont = guiSettings.defaultFont;
    int dfFontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
    int dfSpacing = GuiGetStyle(DEFAULT, TEXT_SPACING);
    GuiAuthLayout layout = getGuiAuthLayout();

    GuiPanel(layout.rectPanel, NULL);

    // Header
    DrawTextRect(layout.rectServerInfo, data.serverInfoLabel.c_str(), LIGHTGRAY);
    DrawTextRectEx(layout.rectHeader, dfFont, data.headerLabel.c_str(), dfFontSize * 2, dfSpacing, GRAY);

    // Back btn
    if(GuiButton(layout.rectBackBtn, "#113#")) {
        clientCoreState = ClientCoreState::SERVER_CONNECTION;
    }

    // Username input
    DrawTextRect(layout.rectUsernameLabel, data.usernameLabel.c_str(), GRAY);
    if(GuiTextBox(layout.rectUsernameInput, data.username, sizeof(data.username), data.usernameEdit))
        data.usernameEdit = !data.usernameEdit;

    // Password input
    DrawTextRect(layout.rectPasswordLabel, data.passwordLabel.c_str(), GRAY);
    if(GuiTextBox(layout.rectPasswordInput, data.password, sizeof(data.password), data.passwordEdit))
        data.passwordEdit = !data.passwordEdit;

    // Auth button
    if(GuiButton(layout.rectAuthBtn, data.btnLabel.c_str())) {
        if(data.username[0] == '\0' || data.password[0] == '\0')
            return;
        if(data.btnLabel == "Sign In") {
            sendNetMsg(clientConfig.connection, [username=data.username, password=data.password]{ return buildNetMsgRequestSignIn(username, password); });
            clientCoreState = ClientCoreState::WAIT_SIGNIN;
        }
        else if(data.btnLabel == "Login"){
            sendNetMsg(clientConfig.connection, [username=data.username, password=data.password]{ return buildNetMsgRequestLogin(username, password); });
            clientCoreState = ClientCoreState::WAIT_LOGIN;
        }
        localUser = User(0, data.username);
        return;
    }

    DrawTextRect(layout.rectAlreadyAuthQuestion, data.alreadyAuthLabel.c_str(), GRAY);
    if(GuiLink(layout.rectAlreadyAuthOther, data.alreadyAuthOtherLabel.c_str())) {
        if(data.btnLabel == "Sign In")
            clientCoreState = ClientCoreState::LOGIN;
        else if(data.btnLabel == "Login")
            clientCoreState = ClientCoreState::SIGNIN;
        return;
    }
    if(data.error) {
        Rectangle temp = posText(layout.rectPanel, RAYLYT_BOTTOM | RAYLYT_CENTER_X, {0}, data.errorMsg.c_str());
        DrawTextRect(temp, data.errorMsg.c_str(), RED);
    }
}
void GuiWaitAuth() {
    GuiDisable();
    GuiAuth();
    GuiEnable();
    std::string label = "Connecting...";
    DrawTextRect(posText(guiSettings.rectAuth, RAYLYT_CENTER_X, {0}, label.c_str()), label.c_str(), BLACK);
}
