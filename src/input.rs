#![allow(dead_code)]
use anyhow::Result;
use gettext::N_;
use nlog::{warn, warn_err};
use sdl3 as sdl;
use std::hash::{Hash, Hasher};

pub enum Keybind {
   // Movement
   Accel,
   Left,
   Right,
   Reverse,
   Face,
   // Gameplay Modifiers
   Stealth,
   GameSpeed,
   Pause,
   // Movement Modifiers
   Autonav,
   Approach,
   MouseFlying,
   Jump,
   // Targeting
   TargetNext,
   TargetPrev,
   TargetNear,
   TargetSpob,
   TargetJump,
   TargetClear,
   // Hostile Targets
   TargetHostileNext,
   TargetHostilePrev,
   TargetHostileNear,
   // Weapons
   FirePrimary,
   FireSecondary,
   Cooldown,
   // Weapon Sets
   WeapSet1,
   WeapSet2,
   WeapSet3,
   WeapSet4,
   WeapSet5,
   WeapSet6,
   WeapSet7,
   WeapSet8,
   WeapSet9,
   WeapSet0,
   // Map
   OverlayMap,
   StarMap,
   // Menus
   MenuSmall,
   MenuInfo,
   Console,
   // Escorts
   EscortNext,
   EscortPrev,
   EscortAttack,
   EscortHold,
   EscortReturn,
   EscortClear,
   // Communication
   Hail,
   AutoHail,
   Scan,
   LogUp,
   LogDown,
   // Display
   ZoomIn,
   ZoomOut,
   Fullscreen,
   Screenshot,
   Paste,
}
impl Keybind {
   pub fn name(&self) -> &'static str {
      use Keybind::*;
      match self {
         Accel => N_("Accelerate"),
         Left => N_("Turn Left"),
         Right => N_("Turn Right"),
         Reverse => N_("Reverse"),
         Face => N_("Face Target"),
         Stealth => N_("Stealth"),
         GameSpeed => N_("Toggle Speed"),
         Pause => N_("Pause"),
         Autonav => N_("Autonavigation On"),
         Approach => N_("Approach"),
         MouseFlying => N_("Mouse Flight"),
         Jump => N_("Initiate Jump"),
         TargetNext => N_("Target Next"),
         TargetPrev => N_("Target Previous"),
         TargetNear => N_("Target Nearest"),
         TargetSpob => N_("Target Space Object"),
         TargetJump => N_("Target Jumpgate"),
         TargetHostileNext => N_("Target Next Hostile"),
         TargetHostilePrev => N_("Target Previous Hostile"),
         TargetHostileNear => N_("Target Nearest Hostile"),
         TargetClear => N_("Clear Target"),
         FirePrimary => N_("Fire Primary Weapons"),
         FireSecondary => N_("Fire Secondary Weapons"),
         Cooldown => N_("Active Cooldown"),
         WeapSet1 => N_("Weapon Set 1"),
         WeapSet2 => N_("Weapon Set 2"),
         WeapSet3 => N_("Weapon Set 3"),
         WeapSet4 => N_("Weapon Set 4"),
         WeapSet5 => N_("Weapon Set 5"),
         WeapSet6 => N_("Weapon Set 6"),
         WeapSet7 => N_("Weapon Set 7"),
         WeapSet8 => N_("Weapon Set 8"),
         WeapSet9 => N_("Weapon Set 9"),
         WeapSet0 => N_("Weapon Set 0"),
         OverlayMap => N_("Overlay Map"),
         StarMap => N_("Star Map"),
         MenuSmall => N_("Small Menu"),
         MenuInfo => N_("Info Menu"),
         Console => N_("Console"),
         EscortNext => N_("Target Next Escort"),
         EscortPrev => N_("Target Previous Escort"),
         EscortAttack => N_("Escort Attack Command"),
         EscortHold => N_("Escort Hold Command"),
         EscortReturn => N_("Escort Return Command"),
         EscortClear => N_("Escort Clear Commands"),
         Hail => N_("Hail Target"),
         AutoHail => N_("Autohail"),
         Scan => N_("Scan Target"),
         LogUp => N_("Log Scroll Up"),
         LogDown => N_("Log Scroll Down"),
         ZoomIn => N_("Radar Zoom In"),
         ZoomOut => N_("Radar Zoom Out"),
         Fullscreen => N_("Toggle Fullscreen"),
         Screenshot => N_("Screenshot"),
         Paste => N_("Paste"),
      }
   }

   pub fn desc(&self) -> &'static str {
      use Keybind::*;
      match self {
         Accel => N_("Makes your ship accelerate forward."),
         Left => N_("Makes your ship turn left."),
         Right => N_("Makes your ship turn right."),
         Reverse => {
            N_("Makes your ship face the direction you're moving from. Useful for braking.")
         }
         Face => N_(
            "Faces the targeted ship if one is targeted, otherwise faces targeted spob, or jump point.",
         ),
         Stealth => N_("Tries to enter stealth mode."),
         GameSpeed => N_("Toggles speed modifier."),
         Pause => N_("Pauses the game."),
         /* Movement modifiers */
         Autonav => N_("Initializes the autonavigation system."),
         Approach => N_(
            "Attempts to approach the targeted ship or space object, or targets the nearest landable space object. Requests landing permission if necessary. Prioritizes ships over space objects.",
         ),
         MouseFlying => N_("Toggles mouse flying."),
         Jump => N_("Attempts to jump via a jump point."),
         TargetNext => N_("Cycles through ship targets."),
         TargetPrev => N_("Cycles backwards through ship targets."),
         TargetNear => N_("Targets the nearest non-disabled ship."),
         TargetSpob => N_("Cycles through space object targets."),
         TargetJump => N_("Cycles through jump points."),
         TargetHostileNext => N_("Cycles through hostile ship targets."),
         TargetHostilePrev => N_("Cycles backwards through hostile ship targets."),
         TargetHostileNear => N_("Targets the nearest hostile ship."),
         TargetClear => N_("Clears the currently-targeted ship, spob or jump point."),
         FirePrimary => N_("Fires primary weapons."),
         FireSecondary => N_("Fires secondary weapons."),
         Cooldown => N_("Begins active cooldown."),
         WeapSet1 => N_("Activates weapon set 1."),
         WeapSet2 => N_("Activates weapon set 2."),
         WeapSet3 => N_("Activates weapon set 3."),
         WeapSet4 => N_("Activates weapon set 4."),
         WeapSet5 => N_("Activates weapon set 5."),
         WeapSet6 => N_("Activates weapon set 6."),
         WeapSet7 => N_("Activates weapon set 7."),
         WeapSet8 => N_("Activates weapon set 8."),
         WeapSet9 => N_("Activates weapon set 9."),
         WeapSet0 => N_("Activates weapon set 0."),
         OverlayMap => N_("Opens the in-system overlay map."),
         StarMap => N_("Opens the star map."),
         MenuSmall => N_("Opens the small in-game menu."),
         MenuInfo => N_("Opens the information menu."),
         Console => N_("Opens the Lua console."),
         EscortNext => N_("Cycles through your escorts."),
         EscortPrev => N_("Cycles backwards through your escorts."),
         EscortAttack => N_("Orders escorts to attack your target."),
         EscortHold => N_("Orders escorts to hold their formation."),
         EscortReturn => N_("Orders escorts to return to your ship hangars."),
         EscortClear => N_("Clears your escorts of commands."),
         Hail => N_("Attempts to initialize communication with the targeted ship."),
         AutoHail => N_("Automatically initialize communication with a ship that is hailing you."),
         Scan => N_("Attempts to scan the target."),
         LogUp => N_("Scrolls the log upwards."),
         LogDown => N_("Scrolls the log downwards."),
         ZoomIn => N_("Zooms in on the radar."),
         ZoomOut => N_("Zooms out on the radar."),
         Fullscreen => N_("Toggles between windowed and fullscreen mode."),
         Screenshot => N_("Takes a screenshot."),
         Paste => N_("Paste from the operating system's clipboard."),
      }
   }

   pub fn handle(&self, value: Value) {
      let nohyp = || unsafe {
         !naevc::player.p.is_null()
            && (*naevc::player.p).flags[naevc::PILOT_HYP_PREP as usize] == 0
            && (*naevc::player.p).flags[naevc::PILOT_HYP_BEGIN as usize] == 0
            && (*naevc::player.p).flags[naevc::PILOT_HYPERSPACE as usize] == 0
      };
      let nodead = || unsafe {
         !naevc::player.p.is_null() && (*naevc::player.p).flags[naevc::PILOT_DEAD as usize] == 0
      };
      let player_set_flag = |f, v| unsafe {
         naevc::player.flags[f as usize] = v;
      };
      let player_restore_movement = || unsafe {
         naevc::player_restoreControl(naevc::PINPUT_MOVEMENT as i32, std::ptr::null());
      };

      let doubletap = false;
      let repeat = false;
      use Keybind::*;
      match self {
         Accel => {
            if repeat {
               return;
            }
            if let Value::Absolute(v) = value {
               player_restore_movement();
               unsafe {
                  naevc::player_accel(v as f64);
               }
            } else {
               if doubletap {
                  if nohyp() && nodead() {
                     unsafe {
                        naevc::pilot_outfitLOnkeydoubletap(
                           naevc::player.p,
                           naevc::OutfitKey__OUTFIT_KEY_ACCEL,
                        );
                        naevc::pilot_afterburn(naevc::player.p);
                        if !(*naevc::player.p).afterburner.is_null() {
                           (*(*naevc::player.p).afterburner).flags |=
                              naevc::PILOTOUTFIT_ISON_TOGGLE as i32;
                           naevc::pilot_weapSetUpdateOutfitState(naevc::player.p);
                        }
                     }
                  }
               } else if value.is_release() {
                  if nohyp() && nodead() {
                     unsafe {
                        naevc::pilot_outfitLOnkeyrelease(
                           naevc::player.p,
                           naevc::OutfitKey__OUTFIT_KEY_ACCEL,
                        );
                        if !(*naevc::player.p).afterburner.is_null() {
                           (*(*naevc::player.p).afterburner).flags &=
                              !naevc::PILOTOUTFIT_ISON_TOGGLE as i32;
                           naevc::pilot_weapSetUpdateOutfitState(naevc::player.p);
                        }
                        player_set_flag(naevc::PLAYER_ACCEL, 0);
                        if naevc::player.flags[naevc::PLAYER_REVERSE as usize] == 0 {
                           naevc::player_accelOver();
                        }
                     }
                  }
               }
               // Fallthrough for double tap
               if value.is_press() {
                  player_restore_movement();
                  player_set_flag(naevc::PLAYER_ACCEL, 1);
                  unsafe {
                     naevc::player_accel(1.);
                  }
               }
            }
         }
         Left => {
            if repeat {
               return;
            }
            if let Value::Absolute(v) = value {
               player_restore_movement();
               player_set_flag(naevc::PLAYER_TURN_LEFT, 1);
               unsafe {
                  naevc::player_left = v as f64;
               }
            } else {
               if doubletap {
                  if nohyp() && nodead() {
                     unsafe {
                        naevc::pilot_outfitLOnkeydoubletap(
                           naevc::player.p,
                           naevc::OutfitKey__OUTFIT_KEY_LEFT,
                        );
                     }
                  }
               } else if value.is_release() {
                  player_set_flag(naevc::PLAYER_TURN_LEFT, 0);
                  unsafe {
                     naevc::player_left = 0.;
                  }
               }
               // Fallthrough for double tap
               if value.is_press() {
                  player_restore_movement();
                  player_set_flag(naevc::PLAYER_TURN_LEFT, 1);
                  unsafe {
                     naevc::player_left = 1.;
                  }
               }
            }
         }
         _ => (),
      }
   }
}

pub enum Value {
   Press,
   Release,
   Absolute(f32),
}
impl Value {
   fn float(&self) -> f32 {
      match self {
         Value::Press => 1.0,
         Value::Release => 0.0,
         Value::Absolute(v) => *v,
      }
   }
}
impl Value {
   pub fn is_press(&self) -> bool {
      match self {
         Value::Release => true,
         _ => false,
      }
   }

   pub fn is_release(&self) -> bool {
      match self {
         Value::Release => true,
         _ => false,
      }
   }
}

pub struct Keypress {
   scancode: sdl::keyboard::Scancode,
   keymod: sdl::keyboard::Mod,
   press: bool,
}
impl PartialEq for Keypress {
   fn eq(&self, other: &Self) -> bool {
      if self.scancode != other.scancode {
         return false;
      }
      // TODO collapse left/right shift and friends?
      return self.keymod == other.keymod;
   }
}
impl Eq for Keypress {}
impl Hash for Keypress {
   fn hash<H: Hasher>(&self, state: &mut H) {
      self.scancode.hash(state);
      self.keymod.bits().hash(state);
   }
}

pub struct GamepadButton {
   button: sdl::gamepad::Button,
   press: bool,
}
impl PartialEq for GamepadButton {
   fn eq(&self, other: &Self) -> bool {
      self.button == other.button
   }
}
impl Eq for GamepadButton {}
impl Hash for GamepadButton {
   fn hash<H: Hasher>(&self, state: &mut H) {
      self.button.hash(state);
   }
}

pub struct GamepadAxis {
   axis: sdl::gamepad::Axis,
   value: i16,
}
impl PartialEq for GamepadAxis {
   fn eq(&self, other: &Self) -> bool {
      self.axis == other.axis
   }
}
impl Eq for GamepadAxis {}
impl Hash for GamepadAxis {
   fn hash<H: Hasher>(&self, state: &mut H) {
      self.axis.hash(state);
   }
}

#[derive(Eq, PartialEq, Hash)]
pub enum Input {
   Keypress(Keypress),
   GamepadButton(GamepadButton),
   GamepadAxis(GamepadAxis),
}
impl Input {
   fn value(&self) -> Value {
      match self {
         Input::Keypress(k) => {
            if k.press {
               Value::Press
            } else {
               Value::Release
            }
         }
         Input::GamepadButton(b) => {
            if b.press {
               Value::Press
            } else {
               Value::Release
            }
         }
         Input::GamepadAxis(a) => {
            if a.value > 0 {
               Value::Absolute(a.value as f32 / i16::MAX as f32)
            } else {
               Value::Absolute(a.value as f32 / i16::MIN as f32)
            }
         }
      }
   }

   fn press(&self) -> bool {
      match self {
         Input::Keypress(k) => k.press,
         Input::GamepadButton(b) => b.press,
         Input::GamepadAxis(_a) => true,
      }
   }

   fn from_event(event: &sdl::event::Event) -> Option<Self> {
      match event {
         sdl::event::Event::GamepadAxisMotion { axis, value, .. } => {
            Some(Input::GamepadAxis(GamepadAxis {
               axis: *axis,
               value: *value,
            }))
         }
         sdl::event::Event::GamepadButtonDown { button, .. } => {
            Some(Input::GamepadButton(GamepadButton {
               button: *button,
               press: true,
            }))
         }
         sdl::event::Event::GamepadButtonUp { button, .. } => {
            Some(Input::GamepadButton(GamepadButton {
               button: *button,
               press: false,
            }))
         }
         sdl::event::Event::KeyDown {
            scancode, keymod, ..
         } => {
            if let Some(scancode) = scancode {
               Some(Input::Keypress(Keypress {
                  scancode: *scancode,
                  keymod: *keymod,
                  press: false,
               }))
            } else {
               None
            }
         }
         _ => None,
      }
   }
}

static BINDINGS: LazyLock<Mutex<HashMap<Input, Keybind>>> =
   LazyLock::new(|| Mutex::new(HashMap::new()));

pub fn handle(event: &sdl::event::Event) {
   if let Some(inp) = Input::from_event(event)
      && let Some(b) = BINDINGS.lock().unwrap().get(&inp)
   {
      b.handle(inp.value());
   }
}

pub fn key_to_str(key: sdl::keyboard::Keycode) -> String {
   let name = key.name();
   if name.is_empty() {
      format!("SC-{}", key.to_ll().0)
   } else {
      name
   }
}

pub fn key_from_str(name: &str) -> Option<sdl::keyboard::Keycode> {
   if name.starts_with("SC-") {
      let name = match name.get(2..) {
         Some(n) => n,
         None => {
            return None;
         }
      };
      return match name.parse::<u32>() {
         Ok(kc) => sdl::keyboard::Keycode::from_u32(kc),
         Err(_) => None,
      };
   }
   sdl::keyboard::Keycode::from_name(name)
}

// Here be C API, yarr
use sdl::sys::keycode::SDL_Keycode;
//use sdl::sys::events::SDL_Event;
use sdl::event::Event;
use std::collections::HashMap;
use std::ffi::{CStr, CString, c_char};
use std::sync::{LazyLock, Mutex};

pub fn _input_handle(
   sdlctx: &sdl::Sdl,
   sdlvid: &sdl::VideoSubsystem,
   sdlevt: &sdl::EventSubsystem,
   event: Event,
) -> Result<()> {
   // Handle mouse motion
   if event.is_mouse() {
      //unsafe { input_mouseTimer = conf.mouse_hide; }
      sdlctx.mouse().show_cursor(true);
   };

   // Copy and Paste support
   if let Event::KeyDown { .. } = event {
      let clipboard = sdlvid.clipboard();
      if clipboard.has_clipboard_text() {
         let txtevent = Event::TextInput {
            timestamp: sdl::timer::ticks(),
            window_id: 0,
            text: clipboard.clipboard_text()?,
         };
         sdlevt.push_event(txtevent)?;
         return Ok(());
      }
   }

   Ok(())
}

#[unsafe(no_mangle)]
pub extern "C" fn input_keyToStr(key: SDL_Keycode) -> *const c_char {
   static KEYSTR: LazyLock<Mutex<HashMap<SDL_Keycode, CString>>> =
      LazyLock::new(|| Mutex::new(HashMap::new()));
   let mut keystr = KEYSTR.lock().unwrap();
   if let Some(name) = keystr.get(&key) {
      return name.as_ptr() as *const c_char;
   }

   let keycode = match sdl::keyboard::Keycode::from_u32(key.0) {
      Some(kc) => kc,
      None => {
         warn!("keycode '{}' not found!", key.0);
         return std::ptr::null();
      }
   };
   let name = key_to_str(keycode);
   keystr.insert(
      key,
      match CString::new(name) {
         Ok(name) => name,
         Err(e) => {
            warn_err!(e);
            c"Unknown".into()
         }
      },
   );
   keystr[&key].as_ptr() as *const c_char
}

#[unsafe(no_mangle)]
pub extern "C" fn input_keyFromStr(name: *const c_char) -> SDL_Keycode {
   if name.is_null() {
      return sdl::keyboard::Keycode::Unknown.to_ll();
   }
   let name = unsafe { CStr::from_ptr(name) };
   let key = match key_from_str(&name.to_string_lossy()) {
      Some(kc) => kc,
      None => sdl::keyboard::Keycode::Unknown,
   };
   key.to_ll()
}
