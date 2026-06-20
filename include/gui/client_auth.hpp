#ifndef GUI_CLIENT_AUTH_HPP
#define GUI_CLIENT_AUTH_HPP

#include "../../lib/raygui.h"
#include "../core/api.hpp"
#include "../core/client.hpp"
#include "../core/network_utils.hpp"
#include "../core/state.hpp"
#include "../core/user.hpp"
#include "raylayout.hpp"
#include "raylib_utils.hpp"
#include "settings.hpp"
#include <array>
#include <cstdint>
#include <fcntl.h>
#include <string>

/* Server connection */
struct GuiServerInputLayout {
    Rectangle rectPanel;
    Rectangle rectHeader;
    Rectangle rectServerInputLabel;
    Rectangle rectServerInput;
    Rectangle rectServerInputAddr;
    Rectangle rectServerInputPort;
    Rectangle rectDropdown;
    Rectangle rectConnectBtn;
    Rectangle rectErrorMsg;
};
struct GuiServerInputData {
    // Input
    char addr[128];
    bool editAddr;
    char port[128];
    bool editPort;
    int tlsVersionChoice;
    bool tlsVersionEdit;
    // Strings
    std::string headerLabel;
    std::string serverInputLabel;
    std::string tlsVersionChoiceStr;
    std::string errorMsg;

    bool error;
};
extern GuiServerInputData guiServerInputData;
void initGuiServerInputData(bool setDefaultAddr=true);
GuiServerInputLayout getGuiServerInputLayout(Rectangle panel);
void GuiServerInput();
void GuiWaitServerInput();

/* Auth */
struct GuiAuthLayout {
    Rectangle rectPanel;
    Rectangle rectBackBtn;
    Rectangle rectServerInfo;
    Rectangle rectHeader;
    Rectangle rectUsernameLabel;
    Rectangle rectUsernameInput;
    Rectangle rectPasswordLabel;
    Rectangle rectPasswordInput;
    Rectangle rectAuthBtn;
    Rectangle rectAlreadyAuth;
    Rectangle rectAlreadyAuthQuestion;
    Rectangle rectAlreadyAuthOther;
    Rectangle rectError;
};
struct GuiAuthData {
    char username[MAX_SIZE_USERNAME];
    bool usernameEdit;
    char password[MAX_SIZE_USERNAME];
    bool passwordEdit;
    bool error;
    std::string errorMsg;
    // Strings
    std::string serverInfoLabel;
    std::string headerLabel;
    std::string usernameLabel;
    std::string passwordLabel;
    std::string btnLabel;
    std::string alreadyAuthLabel;
    std::string alreadyAuthOtherLabel;
};
extern GuiAuthData guiAuthData;
void initGuiLoginData(bool initError=true);
void initGuiSignInData(bool initError=true);
GuiAuthLayout getGuiAuthLayout();
void GuiAuth();
void GuiWaitAuth();

#endif
