//! ALC_SOFT_reopen_device
#![allow(clippy::upper_case_acronyms, dead_code)]
use crate::openal::al_types::*;
use crate::openal::alc_types::*;
use crate::openal::*;
use anyhow::Result;
use std::sync::OnceLock;

pub type ALCREOPENDEVICESOFT = unsafe extern "C" fn(
   device: *mut ALCdevice,
   device_name: *const ALCchar,
   attribs: *const ALCint,
) -> ALCboolean;

#[allow(non_snake_case)]
pub struct ReopenDevice {
   alcReopenDeviceSOFT: ALCREOPENDEVICESOFT,
}
impl ReopenDevice {
   pub fn reopen_device(
      &self,
      device: &Device,
      devicename: Option<&str>,
      attributes: &[ALCenum],
   ) -> Result<()> {
      let ok = unsafe {
         (self.alcReopenDeviceSOFT)(
            device.raw(),
            match devicename {
               None => std::ptr::null(),
               Some(n) => n.as_ptr() as *const ALCchar,
            },
            attributes.as_ptr(),
         )
      } != 0;
      if !ok {
         if let Some(e) = device.get_error() {
            anyhow::bail!(e);
         } else {
            anyhow::bail!("unknown error!");
         }
      }
      Ok(())
   }
}
pub static REOPENDEVICE: OnceLock<ReopenDevice> = OnceLock::new();

pub fn supported(device: &Device) -> bool {
   device.is_extension_present(c"ALC_SOFT_reopen_device")
}

#[allow(non_snake_case)]
pub fn init() -> Result<()> {
   let alcReopenDeviceSOFT = proc_address!(c"alcReopenDeviceSOFT", ALCREOPENDEVICESOFT);
   match REOPENDEVICE.set(ReopenDevice {
      alcReopenDeviceSOFT,
   }) {
      Ok(()) => Ok(()),
      Err(_) => anyhow::bail!("failed to set REOPENDEVICE"),
   }
}
