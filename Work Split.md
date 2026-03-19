# Work Split 3 people


### Person 1 — Player, combat, progression

#### Owns:
- basic movement
- dash
- defensive/active ability
- cooldown logic
- unlock / upgrade integration
- aiming with mouse
- melee and/or ranged combat
- pickups (currency (xp), potions, temporary movement speed, temp shields, etc...)
- player stats and progression hooks

#### Touches SFML through:

- sf::Keyboard / events
- sf::Mouse
- delta time / timing
- sprite switching / simple animation
- UI feedback for cooldowns if possible

Why this is a fair chunk:
This is a big gameplay-heavy pillar with lots of “feel” work and enough complexity to balance the other two.

### Person 2 — Enemies, waves, stages, difficulty

#### Owns:
- enemy base behavior
- fast melee enemy variant
- tanky melee enemy variant
- ranged enemy variant
- boss attacks / boss logic
- wave spawning
- stage progression
- 5 waves per stage
- boss on wave 5
- infinitely progressing stage scaling
- hp/damage/count scaling
- “theme data” per stage

#### Touches SFML through:
- enemy movement and update timing
- collision/hitbox behavior
- enemy projectiles
- loading enemy sprites / themed assets
- maybe boss attack telegraphs or effects

Why this is a fair chunk:
This is another large gameplay/system pillar. It is comparable to player/combat in effort, especially because balancing enemies and bosses takes time.

### Person 3 — Menus, save/load, state management, presentation

#### Owns:
- main menu
- pause menu
- game over screen
- progression store
- high score across restarts
- save/load after each stage
- scene/state switching
- menu ↔ game ↔ pause ↔ game over ↔ shop
- audio switching
- stage music
- menu music
- stage tone presentation support
- music/background/theme hookups

#### Touches SFML through:
- sf::Text, buttons, menu rendering
- sf::RenderWindow
- event handling for menu navigation
- sf::Music, sf::Sound
- serialization triggers from gameplay state changes

Why this is a fair chunk:
Menus plus save/load plus scene/state handling is bigger than it sounds. It is not “easy UI work”; it is core glue code for the entire game.

### Secondary ownership

The problem with main ownership alone is that one person may end up buried in core architecture while others do isolated features. So give each person a secondary support role:
- Person 1 secondary: help Person 2 with hit detection / damage system
- Person 2 secondary: help Person 3 with shop integration and progression scaling
- Person 3 secondary: help Person 1 with HUD / cooldown display / stat display

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

Parallel work:

### Person 1
- player movement
- aiming
- combat
- dash / active ability
- pickups and stat changes

### Person 2
- enemy base class/system
- wave spawning
- ranged/melee variants
- boss prototype
- stage scaling

### Person 3
- main menu
- pause menu
- game over
- save/load
- high score
- progression store shell

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
- 1 boss prototype
- 2 enemy types
- 1 stage theme
- scaling numbers
- save/load working
- menus working
- Then add more themes and polish after the loop is stable.

## Summary 

Person 1: player movement, advanced abilities, aiming, combat, pickups
Person 2: enemies, bosses, waves, infinite stage progression, scaling
Person 3: menus, shop, save/load, high score, state management, music/theme integration


## Entity Management
either EnTT small custom ECS/entity manager
