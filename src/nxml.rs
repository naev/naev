use anyhow::Result;
use roxmltree::Node;
use std::ffi::CString;
use std::io::{Error, ErrorKind};

#[macro_export]
macro_rules! nxml_err_node_unknown {
    ($nodetype: expr, $name: expr, $node: expr) => {
        Err(Error::new(
            ErrorKind::Other,
            gettext(
                format!(
                    "{nodetype} '{name}' has unknown node '{node}'.",
                    nodetype = $nodetype,
                    name = $name,
                    node = $node,
                )
                .as_str(),
            ),
        )
        .into())
    };
}

#[macro_export]
macro_rules! nxml_err_attr_missing {
    ($nodetype: expr, $name: expr) => {
        Err(Error::new(
            ErrorKind::Other,
            gettext(
                format!(
                    "{nodetype} missing attribute '{name}'.",
                    nodetype = $nodetype,
                    name = $name,
                )
                .as_str(),
            ),
        )
        .into())
    };
}

pub fn node_str<'a>(node: Node<'a, 'a>) -> Result<&'a str> {
    match node.text() {
        Some(t) => Ok(t),
        None => Err(Error::new(
            ErrorKind::Other,
            format!(
                "Node '{node}' has invalid text!",
                node = node.tag_name().name()
            )
            .as_str(),
        )
        .into()),
    }
}

pub fn node_string(node: Node) -> Result<String> {
    Ok(String::from(node_str(node)?))
}

pub fn node_cstring(node: Node) -> Result<CString> {
    Ok(CString::new(node_str(node)?)?)
}
