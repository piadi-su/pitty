#!/bin/bash
set -e

APP_NAME="pitty"
REPO_DIR="/tmp/pitty_build"
BIN_NAME="pitty"

# MENU
menu() {
    cat << EOF

  •      •      ┓┓    
┏┓┓╋╋┓┏  ┓┏┓┏╋┏┓┃┃┏┓┏┓
┣┛┗┗┗┗┫  ┗┛┗┛┗┗┻┗┗┗ ┛ 
┛     ┛   

1) install
2) update
3) remove

EOF
}

# CONFIRM 
confirm_install() {
    confirm="n"

    if [[ -t 0 ]]; then
        read -p "Continue? (y/N): " confirm
        confirm=${confirm:-n}
    else
        echo "Non-interactive mode detected (curl | bash)."
        echo "Aborting for safety. Run in terminal instead."
        exit 1
    fi

    if [[ "$confirm" != "y" && "$confirm" != "Y" ]]; then
        echo "Aborted."
        exit 1
    fi
}

# DESKTOP ENTRY
create_desktop() {
    mkdir -p "$HOME/.local/share/applications"

    cat > "$HOME/.local/share/applications/pitty.desktop" <<EOF
[Desktop Entry]
Name=Pitty Terminal
Exec=pitty
Icon=utilities-terminal
Type=Application
Categories=System;TerminalEmulator;
Terminal=false
EOF
}

# INSTALL
install() {
    echo "Installing Pitty..."

    rm -rf "$REPO_DIR"
    mkdir -p "$REPO_DIR"

    if [ -d "./src" ]; then
        cp -r ./src "$REPO_DIR/"
    else
        git clone https://github.com/piadi-su/pitty.git "$REPO_DIR"
    fi

    mkdir -p "$REPO_DIR/bin"

    gcc "$REPO_DIR/src/main.c" -o "$REPO_DIR/bin/$BIN_NAME" \
        $(pkg-config --cflags --libs gtk+-3.0 vte-2.91)

    sudo install -Dm755 "$REPO_DIR/bin/$BIN_NAME" /usr/local/bin/$BIN_NAME

    create_desktop

    echo "Installation complete ✔"
}

# UPDATE
update() {
    echo "Updating Pitty..."

    if [ ! -d "$REPO_DIR" ]; then
        echo "Not installed. Run install first."
        exit 1
    fi

    cd "$REPO_DIR"

    git pull

    mkdir -p bin

    gcc src/main.c -o bin/$BIN_NAME \
        $(pkg-config --cflags --libs gtk+-3.0 vte-2.91)

    sudo install -Dm755 bin/$BIN_NAME /usr/local/bin/$BIN_NAME

    create_desktop

    echo "Update complete ✔"
}

# REMOVE
remove() {
    echo "Removing Pitty..."

    sudo rm -f /usr/local/bin/$BIN_NAME
    rm -f "$HOME/.local/share/applications/pitty.desktop"

    echo "Removed ✔"
}

# MAIN
main() {
    menu
    read -p "➜ " usr_input

    case "$usr_input" in
        1)
            confirm_install
            install
            ;;
        2)
            confirm_install
            update
            ;;
        3)
            confirm_install
            remove
            ;;
        *)
            echo "Invalid choice"
            ;;
    esac
}

main
