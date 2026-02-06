//! ALC_SOFT_system_events
#![allow(clippy::upper_case_acronyms, dead_code)]
use crate::openal::al_types::*;
use crate::openal::alc_types::*;
use crate::openal::*;
use anyhow::Result;
use nlog::{warn, warn_err};
use std::sync::OnceLock;

pub mod consts {
   use crate::openal::alc_types::*;
   pub const ALC_EVENT_TYPE_DEFAULT_DEVICE_CHANGED_SOFT: ALCenum = 0x19D6;
   pub const ALC_EVENT_TYPE_DEVICE_ADDED_SOFT: ALCenum = 0x19D7;
   pub const ALC_EVENT_TYPE_DEVICE_REMOVED_SOFT: ALCenum = 0x19D8;

   pub const ALC_PLAYBACK_DEVICE_SOFT: ALCenum = 0x19D4;
   pub const ALC_CAPTURE_DEVICE_SOFT: ALCenum = 0x19D5;

   pub const ALC_EVENT_SUPPORTED_SOFT: ALCenum = 0x19D9;
   pub const ALC_EVENT_NOT_SUPPORTED_SOFT: ALCenum = 0x19DA;
}

pub type ALCEVENTPROCTYPESOFT = unsafe extern "C" fn(
   event_type: ALCenum,
   device_type: ALCenum,
   device: *mut ALCdevice,
   length: ALCsizei,
   message: *const ALCchar,
   user_param: *mut ALCvoid,
);

pub type ALCEVENTISSUPPORTEDSOFT =
   unsafe extern "C" fn(event_type: ALCenum, device_type: ALCenum) -> ALCenum;
pub type ALCEVENTCONTROLSOFT =
   unsafe extern "C" fn(count: ALCsizei, events: *const ALCenum, enable: ALCboolean) -> ALCboolean;
pub type ALCEVENTCALLBACKSOFT =
   unsafe extern "C" fn(callback: ALCEVENTPROCTYPESOFT, user_param: *const ALCvoid);

#[derive(PartialEq)]
pub enum DeviceType {
   Playback,
   Capture,
}
impl DeviceType {
   pub fn to_alc(&self) -> ALCenum {
      match self {
         Self::Playback => consts::ALC_PLAYBACK_DEVICE_SOFT,
         Self::Capture => consts::ALC_CAPTURE_DEVICE_SOFT,
      }
   }

   pub fn from_alc(raw: ALCenum) -> Option<Self> {
      match raw {
         consts::ALC_PLAYBACK_DEVICE_SOFT => Some(Self::Playback),
         consts::ALC_CAPTURE_DEVICE_SOFT => Some(Self::Capture),
         _ => None,
      }
   }
}

#[derive(PartialEq)]
pub enum EventType {
   DefaultDeviceChanged,
   DeviceAdded,
   DeviceRemoved,
}
impl EventType {
   fn to_alc(&self) -> ALCenum {
      match self {
         Self::DefaultDeviceChanged => consts::ALC_EVENT_TYPE_DEFAULT_DEVICE_CHANGED_SOFT,
         Self::DeviceAdded => consts::ALC_EVENT_TYPE_DEVICE_ADDED_SOFT,
         Self::DeviceRemoved => consts::ALC_EVENT_TYPE_DEVICE_REMOVED_SOFT,
      }
   }

   fn from_alc(raw: ALCenum) -> Option<Self> {
      match raw {
         consts::ALC_EVENT_TYPE_DEFAULT_DEVICE_CHANGED_SOFT => Some(Self::DefaultDeviceChanged),
         consts::ALC_EVENT_TYPE_DEVICE_ADDED_SOFT => Some(Self::DeviceAdded),
         consts::ALC_EVENT_TYPE_DEVICE_REMOVED_SOFT => Some(Self::DeviceRemoved),
         _ => None,
      }
   }
}

unsafe extern "C" fn callback_c<F>(
   event_type: ALCenum,
   device_type: ALCenum,
   _device: *mut ALCdevice,
   length: ALCsizei,
   message: *const ALCchar,
   user_param: *mut ALCvoid,
) where
   F: FnMut(EventType, DeviceType, &str),
{
   let ptr = user_param.cast::<F>();
   let callback = unsafe { &mut *ptr };
   if let Some(evt) = EventType::from_alc(event_type)
      && let Some(dev) = DeviceType::from_alc(device_type)
   {
      let msg = match str::from_utf8(unsafe {
         std::slice::from_raw_parts(message as *const u8, length as usize)
      }) {
         Ok(msg) => msg,
         Err(err) => {
            warn_err!(err);
            "unknown message"
         }
      };
      callback(evt, dev, msg);
   } else {
      warn!("Received wrong types in system_events callback!");
   }
}

#[allow(non_snake_case)]
pub struct SystemEvents {
   alcEventIsSupportedSOFT: ALCEVENTISSUPPORTEDSOFT,
   alcEventControlSOFT: ALCEVENTCONTROLSOFT,
   alcEventCallbackSOFT: ALCEVENTCALLBACKSOFT,
}
impl SystemEvents {
   pub fn is_supported_playback(&self, event: EventType, device: DeviceType) -> bool {
      unsafe {
         (self.alcEventIsSupportedSOFT)(event.to_alc(), device.to_alc())
            == consts::ALC_EVENT_SUPPORTED_SOFT
      }
   }

   pub fn control(&self, evts: &[EventType], enable: bool) -> Result<()> {
      let events: Vec<ALCenum> = evts.iter().map(|e| e.to_alc()).collect();
      let ok = unsafe {
         (self.alcEventControlSOFT)(
            events.len() as ALCsizei,
            events.as_ptr(),
            enable as ALCboolean,
         ) == ALC_TRUE
      };
      if !ok {
         anyhow::bail!("alcEventControlSOFT failed");
      }
      Ok(())
   }

   pub fn callback<F>(&self, callback: F)
   where
      F: FnMut(EventType, DeviceType, &str),
   {
      let ptr = Box::into_raw(Box::new(callback));
      unsafe {
         (self.alcEventCallbackSOFT)(callback_c::<F>, ptr.cast());
      }
   }
}
pub static SYSTEMEVENTS: OnceLock<SystemEvents> = OnceLock::new();

pub fn supported(device: &Device) -> bool {
   device.is_extension_present(c"ALC_SOFT_system_events")
}

#[allow(non_snake_case)]
pub fn init() -> Result<()> {
   let alcEventIsSupportedSOFT = proc_address!(c"alcEventIsSupportedSOFT", ALCEVENTISSUPPORTEDSOFT);
   let alcEventControlSOFT = proc_address!(c"alcEventControlSOFT", ALCEVENTCONTROLSOFT);
   let alcEventCallbackSOFT = proc_address!(c"alcEventCallbackSOFT", ALCEVENTCALLBACKSOFT);
   match SYSTEMEVENTS.set(SystemEvents {
      alcEventIsSupportedSOFT,
      alcEventControlSOFT,
      alcEventCallbackSOFT,
   }) {
      Ok(()) => Ok(()),
      Err(_) => anyhow::bail!("failed to initialize ALC_SOFT_system_events"),
   }
}
