# Maki-Nita input

The stick and the button are the controls. They mount to the case. Their wires land on J2 and reach side A of the cart socket. The video doc lists the rest of that edge.

```mermaid
flowchart LR
  controls[Stick and button] --> header[J2]
  header --> socket[Cart socket side A]
  socket --> cpu[Game AVR PD1 to PD5]
```

The stick is a Sanwa JLF or the same kind of part. Four independent switches. A diagonal is two switches closed at once. A square gate or an octagonal gate is a mechanical plate. The circuit is the same either way.

The button is the same kind of switch, 30 mm across. Every switch common ties to GND. The normally open contact ties to its signal pin.

The five inputs are active low. The pin's own pull-up holds the line high. A closed switch ties the pin to GND. Up, down, left, right, and the button are PD1 through PD5, package pins 7 through 11.

| Signal | Package pin | Contact | Port |
| --- | --- | --- | --- |
| UP | 7 | A3 | PD1 |
| DOWN | 8 | A4 | PD2 |
| LEFT | 9 | A5 | PD3 |
| RIGHT | 10 | A6 | PD4 |
| BUTTON | 11 | A7 | PD5 |

Debounce is two ticks of the 60 Hz loop. The software doc describes that count and the `Input` struct.

## Header

J2 is a 2x4 pin header, 2.54 mm pitch. Pin 1 is the square pad.

| Qty | Part | Role |
| --- | --- | --- |
| 1 | Pin header, 2x4, 2.54 mm | Stick and button. J2 |

| Pin | Net |
| --- | --- |
| 1 | GND |
| 2 | +5 V |
| 3 | UP |
| 4 | DOWN |
| 5 | LEFT |
| 6 | RIGHT |
| 7 | BUTTON |
| 8 | GND |
