## pitty
pitty is simple terminal written in c 

it uses a small config file that's in /home/user/.config/pitty/pitty.conf


### installation

```bash
curl -fsSl https://raw.githubusercontent.com/piadi-su/pitty/refs/heads/master/installer.sh | bash
```
dependencies that you need
- gtk3
- vte3



### simple config 

example config:
```text
[main]
shell=/bin/zsh
font=Comic Mono 14

theme=dark
transparency=0.85
mode=minimal

cursor=block
cursor_blink=true
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

there are 2 Themes
- dark
- light

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
### thing's to add

- [x] blinking cursor
- [ ] curstom theme's

