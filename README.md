# spikte_chat

Educational chat project in C++ to learn about the Linux networking stack.

![Demo](https://youtu.be/raoOjjr7Mjs)

## Running

>This is an exemple to run this project on localhost with self signed CA, not an exemple of a real TLS app deployment

```
make chain.pem
make
```

Then you can run:
- `./bin/spikte_chat server RAW` for unencrypted communication or `./bin/spikte_chat server TLS` to use TLS encryption
- `./bin/spikte_chat client` to run the client interface

Note that the `chain.pem` file generated must be accessible from the client and the server

## Features

**Implemented**:

* [X] Optional TLS encryption
* [X] Accounts and message persistance using SQLite
* [X] Password storage using Argon2D
* [X] raylib/raygui interface
* [X] Multiple chat rooms support
* [X] Themes

**TODO**:

General
* [ ] Fix bad/not great design choices, there are some here and there, like the state management
* [ ] Test a lot of edge cases
* [ ] Better error management

Specific:
* [ ] The chat role system feels a bit weak on the edge cases
* [ ] Add audio
* [ ] Maybe the theme should be client sided only and not shared between user of a same chat, it would allow that the theme is per user and not per chat
* [ ] Chat history when pressing up arrow
* [ ] `/help` to display availabe commands
* [ ] `/getMembers` to display members (with current status)
* [ ] Display who saw a message
* [ ] Display the timestamp
* [ ] UI support for deleting/editing messages
* [ ] UI support for deleting a chat
* [ ] Chat owner role transfere
* [ ] Informs the user if he tries to use a raw TCP connection when the server uses TLS encryption
* [ ] Support for image format (PNG, JPEG, GiF)
* [ ] Support for video format (MP4)
* [ ] Support for audio message (Does chats use MP3 ? I don't know)
