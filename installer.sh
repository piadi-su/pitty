#!/usr/bin/

set -e

APP_NAME="pitty"
REPO_DIR="/tmp/pitty_build"
BIN_NAME="pitty"

echo "Installing Pitty Terminal..."

if ! command -v gcc >/dev/null; then
    echo "gcc not found"
    exit 1
fi

if ! pkg-config --exists gtk+-3.0 vte-2.91; then
    echo "Installing dependencies..."
    sudo pacman -S --noconfirm gtk3 vte3
fi

rm -rf "$REPO_DIR"
mkdir -p "$REPO_DIR"

if [ -d "./src" ]; then
    cp -r ./src "$REPO_DIR/"
else
    echo "Downloading source..."
    git clone https://github.com/yourname/pitty.git "$REPO_DIR"
fi

echo "Compiling..."

mkdir -p "$REPO_DIR/bin"

gcc "$REPO_DIR/src/main.c" -o "$REPO_DIR/bin/$BIN_NAME" \
    $(pkg-config --cflags --libs gtk+-3.0 vte-2.91)

echo "Installing binary..."

sudo install -Dm755 "$REPO_DIR/bin/$BIN_NAME" /usr/local/bin/$BIN_NAME

echo "Creating desktop entry..."

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

echo "Updating desktop database..."
update-desktop-database "$HOME/.local/share/applications" 2>/dev/null || true

echo " Done!"
