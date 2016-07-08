# Orbimania
Orbimania is an N-Body simulator that displays the potential field in real time.  The original version ran on Macs in the late 90s.  THe rewrite targets modern machines.

# Prerequisites
SDL-2 and Windows operating system.

# Status
Code is under development.  In particular, the File menu is inoperative.

# Note on Physics

The simulation uses Greenspan's [Completely conservative, covariant numerical methodology](http://www.sciencedirect.com/science/article/pii/089812219400236E), which conserves energy and momentum.  However, since the timestep is not infinitesimal, there may be deviations from real mechanics.

# Instructions

These instructions are preliminary and incomplete.

When the program starts, you see two particles orbiting each other.  The smooth yellow/blue gradients denote the potential field, with yellow for positive and blue for negative.  You can add, delete, move, or modify the characteristics of any particle.

## Handles

Each particle has four characteristics, each with an associated handle.

| Characteristic | Handle                      |
| -------------- | --------------------------- |
| position       | solid dot tail of arrow     |
| velocity       | head of arrow               |
| mass           | circle                      |
| charge         | hollow circle tail of arrow |

Handles are shown in gray, or pink if the handle is selected.  To change a characteristic, move the mouse until the associated handle is selected (turns pink) and then click and drag the mouse.  The charge handle is a little tricky to select.  Doing so requires moving the mouse to somewhere where the particle field is strong, but not near any of the other handles.

The sign of the mass or charge can be changed by clicking on its circle and dragging the cursor through the center of the particle to the other side.

## View Control

| Key | Action                                 |
| --- | -------------------------------------- | 
|  9  | center view on system's center of mass | 
|  0  | zoom in                                |
|  -  | zoom out                               |
|  =  | restore original unzoomed view         |

You can also drag the universe within the view by clicking in any area that does not select a handle and and dragging the mouse.

## Particle Editing

| Key | Action                                                                      |
| --- | ----------------------------------------------------------------------------| 
|  c  | Copy selected particle to clipboard (must select mass handle first).        |
|  v  | Paste particle from clipboard to current cursor position.                   |
|  x  | Cut selected particle and copy it to the clipboard.                         |
| del | Delete selected particle.                                                   |  
|  f  | Flip sign of characteristic for selected handle: charge, mass, or velocity. |
|  r  | Reverse all velocities.  Equivalent to reversing time.                      | 

## Start/Stop

The space bar starts/stops the simulation clock.
