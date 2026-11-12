---
genpdf:
  format: book
  title: Laser Pointer User Guide
  subtitle: A laser pointer for seminars and screen sharing
  author: Qt Tools
  version: 1.0
  date: 2026-06-22
  font_size: 11pt
  page_numbers: true
---

# Laser Pointer User Guide

Laser Pointer is a Qt Widgets application that overlays a laser spot on the
screen. Use it to highlight areas during presentations, seminars, and screen sharing.

In addition to conventional cursor tracking, you can position the spot anywhere
by dragging it with the left mouse button. Cursor tracking, blinking, trails,
rainbow colors, ripples, and presets can be enabled as needed.

## Launching

If already built, launch the following application:

```text
build/LaserPointer.app
```

On first launch, the pointer uses the standard red laser, size 96, and 100%
opacity, with trails and cursor tracking off. Open the menu by right-clicking
the pointer or using the macOS menu bar / Windows system tray icon. On the first
launch only, a recommended-preset notice appears at the top of the menu.

## Basic Controls

| Operation | Action |
| --- | --- |
| Left drag | Move the laser pointer |
| Left click | Show a white ring-shaped ripple |
| Mouse wheel | Resize |
| Right click | Open the menu |
| Menu bar / system tray icon | Open the menu; on macOS / Windows, also show or hide the pointer |
| Esc / Q | Quit |

While dragging, the pointer temporarily becomes opaque even when blinking.
If trails are enabled, a trail appears during movement.

## Menu Structure

Settings are grouped to keep the menus easy to navigate.

| Menu | Purpose |
| --- | --- |
| Status | Summary of current color, shape, opacity, trail, and tracking settings |
| Show Pointer / Hide Pointer | Toggle visibility from the macOS menu bar / Windows system tray menu |
| Display | Appearance, color, shape, blinking, opacity, automatic fading, and related settings |
| Motion | Cursor tracking, smooth tracking, and screen-boundary constraints |
| Trail | Enable trails, select style, and set duration |
| Preset | Presets for different uses |
| Reset | Reset display, motion, trail, or all settings |
| Help | Controls and shortcut reference |
| Quit | Exit |

Menus and Help use Japanese when the system language is Japanese, and English otherwise.

# Display

The Display menu adjusts the appearance of the laser pointer itself.

## Size

Use `Larger` and `Smaller`, or the mouse wheel, to change size.

| Control | Action |
| --- | --- |
| Larger | Increase size |
| Smaller | Decrease size |
| Mouse wheel | Adjust size in small increments |

The size range is 24 to 260. Holding `Space` temporarily enlarges the displayed pointer.

## Opacity

Choose an opacity directly from the `Opacity` submenu.

| Value | Use |
| --- | --- |
| 25% | Very subtle |
| 50% | Less obstruction of the background |
| 75% | Balance between visibility and subtlety |
| 100% | Most prominent |

Use `O` to increase opacity and `P` to decrease it.

## Shape

Choose a shape from the `Shape` submenu.

| Shape | Description |
| --- | --- |
| Glow | Standard glowing spot |
| Ring | Glowing spot with a ring in the center |
| Cross | Glowing spot with a cross |
| Star | Glowing spot with a star |

Press `S` to cycle through shapes.

## Color and Rainbow

Choose any color with `Color...`. Enable `Rainbow` for rainbow colors.

Color changes are unavailable in rainbow mode: `Color...` is disabled, and `C`
does not open the color dialog.

## Blinking

`Blink` toggles blinking, using a smooth opacity animation.

Choose an interval directly from `Blink Interval`:

| Interval | Description |
| --- | --- |
| 0.2 s | Fast |
| 0.5 s | Moderately fast |
| 0.9 s | Standard |
| 1.2 s | Slow |
| 1.6 s | Very slow |

Use `,` for slower blinking and `.` for faster blinking.

## Ripples

Left-click to briefly display a white ring expanding from the center. Ripples
are always white, whether the pointer uses a regular color or rainbow colors.

## Temporary Enlargement

Hold `Space` to temporarily enlarge the pointer. Releasing it restores the
original size. The saved size setting does not change.

## Auto Fade

Enable `Auto Fade` to fade the pointer after the mouse remains still for a while.
Moving the mouse restores its original opacity.

## Hold H to Show

Enable `Hold H to Show` to display the pointer only while `H` is held down.
Use this when you want it visible only at particular moments.

# Motion

The Motion menu controls how the pointer moves.

## Follow Cursor

Enable `Follow Cursor` to make the center of the laser pointer track the mouse pointer.

Left-drag movement does not start while tracking is enabled. Turn tracking off
to place the pointer anywhere by dragging again.

## Smooth Follow

Enable `Smooth Follow` to have the laser pointer follow smoothly with a slight delay.

This option is available when `Follow Cursor` is enabled.

## Keep On Screen

Enable `Keep On Screen` to limit the position so the pointer does not move too far off screen.

# Trail

The Trail menu controls trails during movement. Trails are off by default.

## Enabling and Disabling Trails

`Trail` toggles trails. Turning it off immediately clears any remaining trail.

## Trail Style

Choose the appearance from `Trail Style`:

| Style | Description |
| --- | --- |
| Glow | Fading glowing spots |
| Dots | Lightweight dots |

Press `Y` to cycle through styles.

## Trail Duration

Choose how long trails remain visible from `Trail Duration`:

| Duration |
| --- |
| 0.5 s |
| 1.0 s |
| 1.5 s |
| 2.0 s |
| 2.5 s |
| 3.0 s |
| 4.0 s |
| 5.0 s |
| 6.0 s |

Use `[` to shorten the duration and `]` to lengthen it.

## Performance Limits

With trails enabled, a large pointer or the combination of rainbow colors and
smooth tracking limits trail duration to 3.0 seconds. This limits the runtime load.

# Preset

The Preset menu applies groups of settings for different uses.

| Preset | Purpose |
| --- | --- |
| Standard | Standard red laser |
| Noticeable | Prominent blue glow |
| Follow | Cursor tracking |
| Subtle | Unobtrusive appearance |
| Large Seminar | Seminars and large screens |

Individual settings can still be changed after applying a preset.

# Reset

The Reset menu lets you reset groups of settings.

| Item | Settings restored |
| --- | --- |
| Reset Display | Color, rainbow, opacity, shape, size, Auto Fade, Hold H to Show |
| Reset Motion | Tracking, smooth tracking, screen constraints |
| Reset Trail | Trail on/off, style, duration |
| Reset All | All settings |

`R` is equivalent to Reset All.

# Shortcut Reference

| Key | Action |
| --- | --- |
| B | Toggle blinking |
| A | Toggle Auto Fade |
| E | Toggle screen constraints |
| F | Toggle cursor tracking |
| G | Toggle smooth tracking |
| M | Toggle Hold H to Show |
| H | Show while held when Hold H to Show is enabled |
| O / P | Increase / decrease opacity |
| S | Cycle shape |
| Space | Enlarge while held |
| V | Toggle rainbow colors |
| C | Choose color |
| T | Toggle trails |
| Y | Cycle trail style |
| R | Reset all settings |
| + / - | Increase / decrease size |
| , / . | Slower / faster blinking |
| [ / ] | Shorter / longer trail |
| Esc / Q | Quit |

# Saving Settings

The following settings are saved with `QSettings` and restored on the next launch:

- Color
- Rainbow state
- Size
- Opacity
- Shape
- Blinking state
- Blink interval
- Cursor tracking
- Smooth tracking
- Screen constraints
- Auto Fade
- Hold H to Show
- Trail state
- Trail style
- Trail duration

Position is not saved. The pointer starts at the center of the screen as before.

# Help

The `Help` submenu lists controls and shortcuts. Help appears inside the menu,
not in a separate window.

Help entries are informational and cannot be selected as actions.

# Troubleshooting

## No Trail Appears

Check that `Trail` is enabled. It is off by default.

## Cannot Change Color

Color changes are unavailable while `Rainbow` is enabled. Turn it off before selecting `Color...`.

## Performance Is Slow

Try these in order: turn trails off, use Dots, shorten trail duration, reduce
pointer size, and disable smooth tracking.

## Cannot Drag the Pointer

Left-drag movement does not start while `Follow Cursor` is enabled. Turn it off
to place the pointer at a fixed position.

## The Pointer Is Faint

Check `Opacity`, `Auto Fade`, and `Hold H to Show`.
