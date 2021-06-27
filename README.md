# Whacky Weapons

This [BZFlag](https://github.com/BZFlag-Dev/bzflag) plugin adds a bunch of new or altered weapons to the game, albeit not all are implemented yet.  Implementation is currently stymied by many deficiencies of the current BZFS API.  If these were to be fixed, there's a good chance I could implement more flags.  Contact me if you have news or suggestions.



---

## Implemented Flags

### Anti-Aircraft (AA)

Bullets fire upward, and can be deflected forward/backward by moving forward/backward to aim.

| BZDB Variable    | Default Value | Effect |
| ---------------- | ------------- | ------ |
| `_wwAAdeflect`   | `0.5`         | Scales the effect of moving forward/backward on the deflection of the bullets. |
| `_wwAAvelfactor` | `2.0`         | Scales the shot velocity (relative to built-in `_shotSpeed`). |

### Lightning Bolt (LB)

Shooting instead creates a thick beam of "lightning" (lasers) that zaps down ahead of (if you're driving forward) or behind (if you're driving backward) you, with the amount being related to the speed you're moving.  If you are standing still, you will of course hit yourself!  Note that the lightning bolt can be blocked by buildings above, so a strike on-target does not necessarily guarantee a hit.

| BZDB Variable | Default Value | Effect |
| ------------- | ------------- | ------ |
| `_wwLBfactor` | `5.0`         | Scales the distance the beam appears ahead/behind you as a function of your speed. |

Possible variant to implement: the beam always appears ahead of you, but the amount varies by whether you are driving forward or backward.

Admins can invoke a lightning bolt strike with `/smite <player>`.

### Nyan Cat (NC)

You cannot shoot forward, but rainbow bullets spew uncontrollably backward.  Inspired by [the meme](https://en.wikipedia.org/wiki/Nyan_Cat).

| BZDB Variable      | Default Value | Effect |
| ------------------ | ------------- | ------ |
| `_wwNCrate`        | `0.03`        | The time between each bullet of each color being spewed. |
| `_wwNCvelfactor`   | `1.0`         | Scales the shot velocity (relative to built-in `_shotSpeed`). |
| `_wwNCspreadangle` | `0.5`         | The maximum angle, in degrees, a bullet may randomly deviate from heading directly backward. |



---

## Planned Flags

### Flamethrower (FL) (NOT IMPLEMENTED)

Shooting creates a small patch of fire (shot explosions) that persists for some time.

### Fractal (FR) (NOT IMPLEMENTED)

A certain distance out from the shooting tank, the bullet splits.  Further distance, and those bullets themselves split.  And so on.  The number of splits, degree of splitting, etc. is configurable:

| BZDB Variable      | Default Value | Effect |
| ------------------ | ------------- | ------ |
| `_wwFRsplittime`   | `3.0`         | The amount of time before a bullet splits. |
| `_wwAAsplitamount` | `3`           | The number of bullets a bullet splits into. |
| `_wwAAsplitangle`  | `45.0`        | The angle, in degrees, that separates the bullets. |
| `_wwAAsplitcount`  | `2`           | The number of splits a bullet is allowed to do. |

See also Trident (TN), which this flag can be made to work similarly to (though TN bullets are SB).

### Girl Scout (GS) (NOT IMPLEMENTED)

Bullets are cookies.  When a tank gets hit by a cookie, it forcibly gains the obesity flag and [pays](https://en.wikipedia.org/wiki/Girl_Scout_Cookies#Sales) the girl scout one point.

### Hook Shot (HS) (NOT IMPLEMENTED)

A certain distance out from the firing tank, the bullet turns 90 degrees.  The chosen direction (left or right) alternates between shots.  Long reload time.

Variant: when it turns, maybe we should also create another bullet (two bullets?) going off at a diagonal for "balancing momentum"?

### Mortar (MR) (NOT IMPLEMENTED)

Shooting makes a bullet that travels along the trajectory your tank would take if you forward-jumped: the bullet takes a parabolic path.  When it lands, it "explodes" (creates a small SW).  Inspired by the (often-disingenuously-)[rejected flags page](https://wiki.bzflag.org/Rejected_flag_ideas#MortarBomb_.28MB.29).

### Perpendicular Laser (PL) (NOT IMPLEMENTED)

Cannot shoot forward.  Instead, the first surface the tank's shot would hit sprouts a laser emerging perpendicularly from (i.e. normal to) the surface.  Variant: the laser doesn't bounce afterward.

Implementation is currently impossible because the plugin system has no facilities to compute intersections with the world.

### Railgun (RA) (NOT IMPLEMENTED)

Fires a very fast projectile that penetrates walls like a super bullet and also leaves a destructive "sonic boom" (SWs).  Long reload.

### Rico Laser (RL) (NOT IMPLEMENTED)

Like an ordinary Laser (L) flag, except beams only exist *after* the first bounce.  That is, you *cannot* shoot a tank directly, but you *can* hit them via ricos.  The aim of this flag is to teach people how to use the ordinary L flag *correctly*: that is, by utilizing and generally being aware of the ricos, instead of spraying coherent light straight ahead and hoping it hits something immediately.

Disappointingly, again, implementation is currently impossible because the plugin system has no facilities to compute intersections with the world.

### Sidewinder Missiles (SM) (PARTIALLY IMPLEMENTED)

Fires two guided missiles sideways instead of a single missile forward.  Curving GMs, by firing and then locking on after some time of the missile flying straight, is critical to high-level GM play.  This flag helps teach this by ensuring the trajectories curve significantly almost no matter what the player does.

The flag is implemented, but the server crashes when attempting to fire the shot.  Under investigation.

### Snake Shot (SS) (NOT IMPLEMENTED)

Bullet takes a wavy path.  Inspired by the [classic (and now dysfunctional) plugin](https://forums.bzflag.org/viewtopic.php?p=163155#p163155).

### Temporal Displacement (TD) (NOT IMPLEMENTED)

The bullet appears some time after it was fired.  This can be deliciously sneaky, especially with long delays.  Corral your enemies into apparently-safe locations where your bullets will soon appear!  Admittedly, this flag will likely be difficult to make effective, even if the results are occasionally spectacular.

| BZDB Variable | Default Value | Effect |
| ------------- | ------------- | ------ |
| `_wwTDtime`   | `3.0`         | Delay time, in seconds, before the bullet appears. |

### Trident (TN) (NOT IMPLEMENTED)

Firing creates a super bullet (as from SB) that splits after a delay.

| BZDB Variable    | Default Value | Effect |
| ---------------- | ------------- | ------ |
| `_wwTNsplittime` | `3.0`         | The amount of time before the bullet splits. |

Compare to the Fractal (FR) flag, which can split more, but is not SB.



---

## Fun Modifications to Existing Flags You Might Try

Totally optional, but might make things more interesting.

- GM: make it super slow, but have an infinite turning rate.



---

## Building (Linux Ubuntu)

This hasn't been tested in a while, but should be similar to:

    git clone -b 2.4 https://github.com/BZFlag-Dev/bzflag.git bzflag
    cd bzflag/plugins/
    git clone https://github.com/agatha2/Whacky-Weapons.git
    cd ../
    
    ./autogen.sh
    ./configure --enable-custom-plugins=Whacky-Weapons
    
    make
    
    sudo make install



---

## Configuring and Running

Load the plugin by passing the following to your server:

    -loadplugin WhackyWeapons

You can unload/load the plugin in-game with:

    /unloadplugin WhackyWeapons
    /loadplugin WhackyWeapons



---

## Contact

Contact me (Agatha) on Libera IRC (Agatha or #bzflag), or PM [Agatha on the forum](https://forums.bzflag.org/memberlist.php?mode=viewprofile&u=58731) to suggest new flag ideas or to comment on implementation issues.  Development on this project is *very* sporadic and slow, but if you understand and accept that, collaboration is possible, too.
