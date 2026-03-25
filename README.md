# BOSTO Tablet Linux driver

This project provides Linux support for BOSTO tablet.

Currently, it has been tested on the following model:
- BOSTO BT-13HD : 13" FullHD pen tablet, 8-buttons pad, 2 buttons pen, 8192 pressure level, 200LPMM

## Build and deploy

```sh
make all
sudo make install
```

### Debian packages

Debian packages are available in [releases](https://github.com/jeanparpaillon/bosto_linux/releases)

## Components

- Linux input kernel driver
- udev rules (for pad support)
- libwacom support for UI configuration tools (ie Gnome)

## Architecture

Without this driver, tablet is recognized as an hidraw device.

Driver decodes frames from hidraw devices and creates 2 input devices:
- BOSTO 13HD Pen : pen position, pressure and buttons
- BOSTO 13HD Pad : pad buttons

udev rules are required to recognize Pad input device as tablet pad.

## Reverse engineering

USB debug:

```
01 bb cc dd ee ff gg hh ii jj (tablet)
04 bb cc dd ee ff gg (aux)
```


### Tablet report

```
bb (1)
  hover = bit(4)
  contact = bit(0)
  button 1 = bit(5)
  button 2 = bit(1)
```

```
cc dd (2, 3)
  h pos
```

```
ee ff (4, 5)
  v pos
```

```
gg hh (6, 7)
  pressure
```

```
ii (8)
  00
```

```
jj (9)
  00
```

### Aux report

```
ee = side button
  button 1 = 01
  button 2 = 02
  button 3 = 04
  button 4 = 08
  button 5 = 10
  button 6 = 20
  button 7 = 40
  button 8 = 80
```
