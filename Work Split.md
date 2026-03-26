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


## Task board suggestion
### Person 1
- basic movement (2)
- aim with mouse (1)
- melee/ranged attack (3)
- getting hit / player damage (2)
- dash (2)
- active ability + cooldown (3)
- pickups / player stat updates (2)

### Person 2
- enemy movement toward player (2)
- melee enemy (2)
- ranged enemy + projectile (3)
- boss behavior (3)
- spawn/wave system (3)
- stage scaling system (2)
- stage theme hooks/data (1)

### Person 3
- main menu (2)
- pause menu + stats (2)
- game over logic (1)
- progression store (3)
- save/load per stage (3)
- high score persistence (2)
- scene/state manager (3)
- music switching (1)


## Shared conventions
- coordinate system
- units for speed/damage/cooldown
- update order
- naming conventions
- asset loading conventions
- JSON/text save format if used
- Shared interfaces

Define early:
- how player stats are accessed
- how enemies deal damage
- how waves tell the game a stage is complete
- how menus trigger new/load/continue
- how the save manager reads current state


## Start with:
- basic controls
- 1 enemy types
- 1 stage theme
- scaling numbers
- save/load working
- menus working
- Then add more themes and polish after the loop is stable.
