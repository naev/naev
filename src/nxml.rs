//use std::io::{Error, ErrorKind};

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
        ))
    };
}
