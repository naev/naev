use crate::faction::FactionRef;
use helpers::ReferenceC;
use std::ffi::{CStr, CString, c_char, c_int};

#[allow(dead_code)]
pub enum HookParam {
   Nil,
   Number(f64),
   String(&'static CStr),
   StringFree(*mut c_char),
   //StringFree,
   Bool(bool),
   //Pilot(),
   //Ship(),
   //Outfit(),
   //Commodity()
   Faction(FactionRef),
   Ssys(i64),
   //Spob(),
   //Jump(),
   //Asteroid(),
   //Ref(),
}

impl HookParam {
   fn sentinal() -> naevc::HookParam {
      naevc::HookParam {
         type_: naevc::HookParamType_e_HOOK_PARAM_SENTINEL,
         u: naevc::HookParam_s__bindgen_ty_1 { num: 0.0 },
      }
   }

   fn to_c(&self) -> naevc::HookParam {
      match self {
         Self::Nil => naevc::HookParam {
            type_: naevc::HookParamType_e_HOOK_PARAM_NIL,
            u: naevc::HookParam_s__bindgen_ty_1 { num: 0.0 },
         },
         Self::Number(n) => naevc::HookParam {
            type_: naevc::HookParamType_e_HOOK_PARAM_NUMBER,
            u: naevc::HookParam_s__bindgen_ty_1 { num: *n },
         },
         Self::String(s) => naevc::HookParam {
            type_: naevc::HookParamType_e_HOOK_PARAM_STRING,
            u: naevc::HookParam_s__bindgen_ty_1 { str_: s.as_ptr() },
         },
         Self::StringFree(s) => naevc::HookParam {
            type_: naevc::HookParamType_e_HOOK_PARAM_STRING_FREE,
            u: naevc::HookParam_s__bindgen_ty_1 { str_free: *s },
         },
         Self::Bool(b) => naevc::HookParam {
            type_: naevc::HookParamType_e_HOOK_PARAM_BOOL,
            u: naevc::HookParam_s__bindgen_ty_1 { b: *b as c_int },
         },
         Self::Faction(f) => naevc::HookParam {
            type_: naevc::HookParamType_e_HOOK_PARAM_FACTION,
            u: naevc::HookParam_s__bindgen_ty_1 { lf: f.as_ffi() },
         },
         Self::Ssys(s) => naevc::HookParam {
            type_: naevc::HookParamType_e_HOOK_PARAM_SSYS,
            u: naevc::HookParam_s__bindgen_ty_1 { ls: *s as i32 },
         },
      }
   }
}

pub fn run_param_deferred(stack: &str, params: &[HookParam]) {
   let mut params: Vec<_> = params.iter().map(|p| p.to_c()).collect();
   params.push(HookParam::sentinal());
   let stack = CString::new(stack).unwrap();
   unsafe { naevc::hooks_runParamDeferred(stack.as_ptr(), params.as_ptr()) };
}
