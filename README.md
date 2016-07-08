# Orbimania
Orbimania is an N-Body simulator that displays the potential field in real time.  The original version ran on Macs in the late 90s.  THe rewrite targets modern machines.

# Prerequisites
SDL-2 and Windows operating system.

# Status
Code is under development. 

# Instructions

These instructions are preliminary and incomplete.

When the program starts, you see two particles orbiting each other.  The smooth yellow/blue gradients denote the potential field, with yellow for positive and blue for negative.

## Handles

Each particle has four characteristics, each with an associated handle.

| Characteristic | Handle                      |
| -------------- | --------------------------- |
| position       | solid dot tail of arrow     |
| velocity       | head of arrow               |
| mass           | circle                      |
| charge         | hollow circle tail of arrow |

Handles are shown in gray, or pink if clicking the mouse will select it.  To change a characteristic, move the mouse until the associated handle turns ping and then click and drag the mouse.  The charge handle is a little tricky to select.  Doing so requires moving the mouse to somewhere where the particle field is strong, but not near any of the other handles.

## Keys

### View Control

* 9 : center  
* 0 : zoom in
* - : zoom out
* = : restore view to unit zoom.

You can also drag the universe within the view by clicking in any area that does not select a handle and and dragging the mouse.

### Particle Editing

* c : Copy selected particle to clipboard (must select mass handle first)
* v : Paste particle from clipboard to current cursor position
* x : Cut selected particle and copy it to the clipboard.
* f : flip quantity corresponding to selected handle: charge, mass, or velocity.
* r : reverse all velocities.  Equivalent to reversing time.

### Start/Stop

* space bar : toggles start/stop of simulation clock
