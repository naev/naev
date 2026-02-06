#![allow(dead_code, unused_variables)]
use renderer::Context;
use sdl3 as sdl;

pub trait Message: Send + std::fmt::Debug + Clone {}

#[derive(Copy, Clone, Debug)]
pub struct Shape {
   x: f32,
   y: f32,
   w: f32,
   h: f32,
}
impl Shape {
   pub fn inbounds(&self, x: f32, y: f32) -> bool {
      x > self.x && y > self.y && x <= self.x + self.w && y <= self.y + self.h
   }
}

#[derive(PartialEq)]
enum WgtStatus {
   Normal,
   MouseOver,
   MouseDown,
   Scrolling,
}

pub enum WgtMessage {
   KeyDown(Option<sdl::keyboard::Keycode>, sdl::keyboard::Mod, bool),
   KeyUp(Option<sdl::keyboard::Keycode>, sdl::keyboard::Mod, bool),
   TextInput(String),
   MouseMove(f32, f32, f32, f32),
   MouseButtonDown(u8, f32, f32),
   MouseButtonUp(u8, f32, f32),
   MouseDoubleClick(u8, f32, f32),
   MouseWheel(u8, f32, f32),
   Resize(Shape),
   ScrollDone,
   VisibilityGain,
   VisibilityLost,
   FocusGain,
   FocusLost,
}

pub trait Widget<Message>
where
   Message: Clone + Send + std::fmt::Debug,
{
   type Message;

   /// Gets the name of the widget
   fn name(&self) -> &str;
   /// Gets the shape of the widget
   fn shape(&self) -> Shape;

   /// Widget is dynamic and is always drawn when visible
   fn is_dynamic(&self) -> bool;
   /// Widget can be focused and will receive focus event and loss
   fn is_focusable(&self) -> bool;

   /// Draws on the first pass
   fn draw(&self, ctx: &Context, bx: f32, by: f32);
   /// Draws on the second pass
   fn draw_overlay(&self, ctx: &Context, bx: f32, by: f32);

   /// Return true if message was consumed, false if we pass it through
   /// Some messages will be passed through irregardless such as mouse movement.
   fn message(&mut self, msg: WgtMessage) -> (bool, Option<Message>);
}

enum OnPress<'a, Message> {
   Direct(Message),
   Closure(Box<dyn Fn() -> Message + 'a>),
}

impl<Message: Clone> OnPress<'_, Message> {
   fn get(&self) -> Message {
      match self {
         OnPress::Direct(message) => message.clone(),
         OnPress::Closure(f) => f(),
      }
   }
}

pub struct Button<'a, Message> {
   name: String,
   status: WgtStatus,
   shape: Shape,

   display: String,
   disabled: bool,
   softdisable: bool,
   key: u32,

   on_press: Option<OnPress<'a, Message>>,
}
impl<Message> Widget<Message> for Button<'_, Message>
where
   Message: Clone + Send + std::fmt::Debug,
{
   type Message = Message;

   fn name(&self) -> &str {
      &self.name
   }
   fn shape(&self) -> Shape {
      self.shape
   }

   fn is_dynamic(&self) -> bool {
      false
   }
   fn is_focusable(&self) -> bool {
      true
   }

   fn draw(&self, ctx: &Context, bx: f32, by: f32) {}
   fn draw_overlay(&self, ctx: &Context, bx: f32, by: f32) {}

   fn message(&mut self, msg: WgtMessage) -> (bool, Option<Message>) {
      match msg {
         WgtMessage::Resize(shape) => {
            self.shape = shape;
            (true, None)
         }
         WgtMessage::MouseButtonDown(btn, x, y) => {
            self.status = WgtStatus::MouseDown;
            (true, None)
         }
         WgtMessage::MouseButtonUp(btn, x, y) => {
            if self.status != WgtStatus::MouseDown {
               return (false, None);
            }
            self.status = WgtStatus::Normal;
            match self.shape.inbounds(x, y) {
               true => {
                  self.status = WgtStatus::Normal;
                  match &self.on_press {
                     Some(on_press) => (true, Some(on_press.get())),
                     None => (true, None),
                  }
               }
               false => (false, None),
            }
         }
         _ => (false, None),
      }
   }
}
impl<Message> Button<'_, Message> {}
