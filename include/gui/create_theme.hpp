#ifndef GUI_CREATE_THEME_HPP
#define GUI_CREATE_THEME_HPP

#include "chat_theme/chat_theme.hpp"
#include "chat_theme/chat_theme_simple.hpp"
#include "chat_theme/chat_theme_gol.hpp"
#include "chat_theme/chat_theme_matrix.hpp"
#include "chat_theme/chat_theme_hellokitty.hpp"
#include <memory>

std::unique_ptr<GuiChatThemeData> createTheme(ChatTheme theme, Rectangle panelDim);

#endif
