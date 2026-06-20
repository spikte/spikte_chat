#ifndef GUI_SETTINGS_HPP
#define GUI_SETTINGS_HPP

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <raylib.h>
#include <algorithm>
#include "../../lib/raygui.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

constexpr int PANEL_HEADER_HEIGHT = 24;

// Just global variable,
// Maybe not the best idk, it works and is very simple to handle so
// -1 values means it depends on some non initialized value
// and will be initialized later (like the screen width nd height)
struct GuiSettings {
    // Font
    Font defaultFont;

    // Server connection
    Rectangle rectServerConnection;

    // Sign in/Login settings
    Rectangle rectAuth;

    // Chat windows
    Rectangle chatListRect;
    Rectangle chatRect;
    Rectangle chatCreationRect;
    Rectangle chatMemberRect;
    Vector2 margin;
    int windowGap;
    int authorGap;

    // Chat List
    int chatListItemHeight;


    // Chat settings
    int fontSize;
    int msgGap;
    int msgInterline;
    int spacing;
    Vector4 msgMargin;
    Vector2 msgMaxSize; // In pixel
    Vector2 msgMaxSizeServer;
    float msgBorderRadius;
    int msgSegments;
    Vector2 inputTextBoxDim;
    Vector4 inputTextBoxMargin;
};
extern GuiSettings guiSettings;

void initSettingsDefaults();
void initSettingsDefaultFont();
void initRectCentered(int screenWidth, int screenHeight, int width, int height);
void initSettingsServerConnection(int screenWidth, int screenHeight);
void initSettingsAuth(int screenWidth, int screenHeight);
void initSettingsChatList(int screenWidth, int screenHeight);
void initSettingsChat(int screenWidth, int screenHeight);
void initSettingsChatCreation(int screenWidth, int screenHeight);
void initSettingsChatMember(int screenWidth, int screenHeight);
void initSettingsMsg();
void initSettings(int screenWidth, int screenHeight);

#endif
