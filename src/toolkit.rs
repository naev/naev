#![allow(dead_code)]
use crate::context::Context;

/*
enum WidgetOnPress<'a, Message> {
    Direct(Message),
    Closure(Box<dyn Fn() -> Message + 'a>),
}
*/

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

enum Status {
    Normal,
    MouseOver,
    MouseDown,
    Scrolling,
}

pub enum Message<'a> {
    KeyEvent(u32, u32),
    TextEvent(&'a str),
    MouseMove(f32, f32, f32, f32),
    MousePressed(u32, f32, f32),
    MouseReleased(u32, f32, f32),
    MouseDoubleClick(u32, f32, f32),
    MouseWheel(u32, f32, f32),
    Resize(Shape),
    ScrollDone,
    VisibilityGain,
    VisibilityLost,
    FocusGain,
    FocusLost,
}

pub trait Widget {
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
    fn message(&mut self, msg: Message) -> bool;
}

pub struct Button {
    name: String,
    status: Status,
    shape: Shape,

    display: String,
    disabled: bool,
    softdisable: bool,
    key: u32,
    //on_press: Option<WidgetOnPress>,
}
impl Widget for Button {
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

    fn message(&mut self, msg: Message) -> bool {
        match msg {
            Message::MousePressed(btn, x, y) => true,
            Message::MouseReleased(btn, x, y) => {
                match self.shape.inbounds(x, y) {
                    true => {
                        self.status = Status::Normal;
                    }
                    false => {
                        todo!();
                    }
                }
                false
            }
            _ => false,
        }
    }
}
impl Button {}
