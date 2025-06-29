use anyhow::Result;
use roxmltree::Node;
use std::ffi::CString;
use std::io::Error;

#[macro_export]
macro_rules! nxml_err_attr_missing {
    ($nodetype: expr, $name: expr) => {
        Err(std::io::Error::new(
            std::io::ErrorKind::Other,
            $crate::gettext::gettext(
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

#[macro_export]
macro_rules! nxml_warn_node_unknown {
    ($nodetype: expr, $name: expr, $node: expr) => {
        $crate::log::warn($crate::gettext::gettext(
            format!(
                "{nodetype} '{name}' has unknown node '{node}'.",
                nodetype = $nodetype,
                name = $name,
                node = $node,
            )
            .as_str(),
        ))
    };
}

#[macro_export]
macro_rules! nxml_warn_attr_missing {
    ($nodetype: expr, $name: expr) => {
        $crate::log::warn($crate::gettext::gettext(
            format!(
                "{nodetype} missing attribute '{name}'.",
                nodetype = $nodetype,
                name = $name,
            )
            .as_str(),
        ))
    };
}

pub fn node_str<'a>(node: Node<'a, 'a>) -> Result<&'a str> {
    match node.text() {
        Some(t) => Ok(t),
        None => Err(Error::other(
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

pub fn node_f32(node: Node) -> Result<f32> {
    Ok(node_str(node)?.parse::<f32>()?)
}

pub fn node_f64(node: Node) -> Result<f64> {
    Ok(node_str(node)?.parse::<f64>()?)
}

pub fn node_texturepath(node: Node, default: &str) -> Result<String> {
    let path = node_string(node)?;
    Ok(match path.starts_with('/') {
        true => path,
        false => format!("{default}{path}"),
    })
}
