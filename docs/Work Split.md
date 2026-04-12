# Work Split 3 people

### Martin 
- main menu
- player controls
- enemy behaviour
- special ability A

### Stefan 
- pause menu
- wave spawning
- progression store
- special ability B

### Robert
- game over screen
- hitboxes and damage systems
- progression
- special ability C
- stage design

### Secondary ownership
- Martin secondary: help Robert with hit detection / damage system
- Stefan secondary: help Martin with enemy behaviour
- Robert secondary: help Stefan with progression store

That way everyone contributes outside their own silo.


## Shared core

All 3 decide this together before serious coding:

- main loop
- folder structure
- coding style
- game state flow
- entity approach
- save format



## MileStone 1 — shared setup

All 3 together:
- define minimal playable loop
- decide class/component list
- create window
- create game state switching
- create test entity rendering
- decide save format
- set up Git branches and issue board

### Deliverable:

- empty project runs
- one placeholder entity renders and moves
- menu can switch into game scene

## MileStone 2 — core feature implementation

### Deliverable:
- basic combat loop works
- enemies spawn and attack
- game can move through waves
- menus exist and function

## MileStone 3 — integration and polish 

All 3:
- connect progression store to upgrades
- connect stage themes to visuals/music
- test save/load after stage clear
- difficulty tuning
- bug fixing
- balancing

### Deliverable:

one complete full loop:
menu → game → waves → boss → store → next stage → save/load → death/game over

This phase is where fairness often breaks, so assign explicit polish duties too:

Person 1: combat feel and player balance
Person 2: enemy/boss balance and wave pacing
Person 3: UI flow, save reliability, music/theme transitions



## Start with:
- basic controls
- 1 enemy types
- 1 stage theme
- scaling numbers
- save/load working
- menus working
- Then add more themes and polish after the loop is stable.
