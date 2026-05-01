## pitty
pitty is simple terminal written in c 

it uses a small config file that's in /home/user/.config/pitty/pitty.conf


### installation | update | remove
dependencies that you need

- git
- gtk3
- vte3

```bash
git clone https://github.com/piadi-su/pitty.git /tmp/pitty_build

cd /tmp/pitty_build/scripts/

chmod +x installer.sh

./installer.sh
```




### example config 

```text
[main]
shell=/bin/zsh
font=Comic Mono 14

theme=gruvbox
transparency=0.85
mode=minimal

cursor=block
cursor_blink=true
```

#### themes

for in order to use custom themes 
your need to put them in .config/pitty/themes/
you can alsow use pywal by doing:
```text
theme=pywal
```

```text
pitty
├── pitty.conf
└── themes
    └── gruvbox.conf
```


#### keybinds
| shortcut          | action   |
|-------------------|-----------|
| Ctrl + Shift + C  | Copy      |
| Ctrl + Shift + V  | Paste     |
| Ctrl + Shift + +  | Zoom in   |
| Ctrl + Shift + -  | Zoom out  |
| Ctrl + L          | Clear     |

the transparency goes from 1 to 0.0

the are 2 mode
- minimal
- normal 

there are 2 cursor
- block
- beam

there are 2 blink
- true
- false
----
#### things to do
- [ ] split code in more sections
----
This project it's relised under the MIT license.

