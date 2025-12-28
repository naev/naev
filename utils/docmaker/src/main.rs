use anyhow::{Context, Result};
use clap::{Args, Parser, Subcommand};
use fs_err as fs;
use regex::{Captures, Regex};
use std::collections::HashMap;
use std::io::Write;
use std::path::PathBuf;

#[derive(Parser)]
#[command(author, version, about)]
struct Cli {
    /// Command to run
    #[command(subcommand)]
    command: Mode,
}
impl Cli {
    fn output(&self) -> Option<PathBuf> {
        match &self.command {
            Mode::Ldoc(ld) => ld.output.clone(),
            Mode::Luacheck(lc) => lc.output.clone(),
        }
    }
}

#[derive(Args)]
struct Ldoc {
    /// Input source file
    #[arg(value_name = "INPUT")]
    input: PathBuf,
    /// Output file
    #[arg(short, long, value_name = "OUTPUT")]
    output: Option<PathBuf>,
}

#[derive(Args)]
struct Luacheck {
    /// Input source file
    #[arg(value_name = "INPUTS")]
    input: Vec<PathBuf>,
    /// Output file
    #[arg(short, long, value_name = "OUTPUT")]
    output: Option<PathBuf>,
}

#[derive(Subcommand)]
enum Mode {
    /// Converts the file to ldoc-compatible output
    Ldoc(Ldoc),
    /// Lists the available Lua API
    Luacheck(Luacheck),
}

fn extract_docs(input: &str) -> Vec<String> {
    // Match /*@ ... */ OR /** ... */ blocks
    let block_re = Regex::new(r"(?s)/\*(\*|@)\s*(.*?)\*/").unwrap();

    let mut blocks = Vec::new();

    for caps in block_re.captures_iter(input) {
        let marker = &caps[1]; // "*" or "@"
        let body = &caps[2];

        // /*@ ... */ → always valid
        if marker == "@" {
            blocks.push(body.trim().to_string());
            continue;
        }

        // /** ... */ → only if contains @lua
        if marker == "*" && body.contains("@lua") {
            blocks.push(body.trim().to_string());
        }
    }

    blocks
}

fn ldoc(blocks: &[String]) -> Result<String> {
    let line_re = Regex::new(r"(?m)^[ \t]*\*[ \t]?")?;

    let map = HashMap::from([
        ("brief", ""),
        ("luafunc", "@function"),
        ("luasee", "@see"),
        ("luaparam", "@param"),
        ("luatparam", "@tparam"),
        ("luareturn", "@return"),
        ("luatreturn", "@treturn"),
        ("luamod", "@module"),
        ("luatype", "@type"),
        ("luaset", "@set"),
        ("luafield", "@field"),
        ("fixme", "<em>FIXME</em>"),
        ("todo", "<em>TODO</em>"),
        ("TODO", "<em>TODO</em>"),
        ("warning", "<em>Warning</em>"),
        ("note", "<em>Note</em>"),
        ("code", "<pre>"),
        ("endcode", "</pre>"),
    ]);
    let syntax_re = Regex::new(r"@(\w+)")?;

    Ok(blocks
        .iter()
        .map(|body| {
            let body = syntax_re.replace_all(body, |caps: &Captures| {
                map.get(&caps[1])
                    .copied()
                    .unwrap_or(caps.get(0).unwrap().as_str())
                    .to_owned()
            });

            let body = line_re.replace_all(&body, "");
            format!("--[[--\n{}\n--]]", body)
        })
        .collect::<Vec<_>>()
        .join("\n\n"))
}

fn lua_funcs(blocks: &[String]) -> Vec<String> {
    let func_re = Regex::new(r"@luafunc\s+([A-Za-z_][A-Za-z0-9_]*)").unwrap();
    let mut funcs = Vec::new();
    for block in blocks {
        for caps in func_re.captures_iter(block) {
            funcs.push(caps[1].to_string());
        }
    }
    funcs
}

fn lua_fields(blocks: &[String]) -> Vec<String> {
    let func_re = Regex::new(r"@luafield\s+([A-Za-z_][A-Za-z0-9_]*)").unwrap();
    let mut funcs = Vec::new();
    for block in blocks {
        for caps in func_re.captures_iter(block) {
            funcs.push(caps[1].to_string());
        }
    }
    funcs
}

fn lua_mods(blocks: &[String]) -> Vec<String> {
    let mod_re = Regex::new(r"@luamod\s+([A-Za-z_][A-Za-z0-9_]*)").unwrap();
    let mut mods = Vec::new();
    for block in blocks {
        for caps in mod_re.captures_iter(block) {
            mods.push(caps[1].to_string());
        }
    }
    mods
}

fn luacheck(blocks: &[String]) -> Result<(String, String)> {
    let modname = {
        let mods = lua_mods(blocks);
        if mods.len() > 1 {
            anyhow::bail!(format!(
                "Multiple modules in one document not supported yet! Found: {}",
                mods.join(", ")
            ));
        } else if mods.len() == 0 {
            anyhow::bail!("No mods found!");
        }
        mods[0].clone()
    };
    let mut out = String::new();
    out.push_str(&format!("stds.{modname} = {{\n"));
    out.push_str("   read_globals = {\n");
    out.push_str(&format!("      {modname} = {{\n"));
    out.push_str("         fields = {\n");
    for funcname in lua_funcs(blocks) {
        out.push_str(&format!("            {funcname} = {{}},\n"));
    }
    for fieldname in lua_fields(blocks) {
        out.push_str(&format!("            {fieldname} = {{}},\n"));
    }
    out.push_str("         }\n");
    out.push_str("      }\n");
    out.push_str("   }\n");
    out.push_str("}\n");
    Ok((modname, out))
}

fn main() -> Result<()> {
    let args = Cli::parse();

    let output = match &args.command {
        Mode::Ldoc(ld) => {
            let txt = fs::read_to_string(&ld.input)?;
            let blocks = extract_docs(&txt);
            ldoc(&blocks)?
        }
        Mode::Luacheck(lc) => {
            let mut mods: Vec<String> = lc.input.iter().map( |file| {
                let txt = fs::read_to_string(file).unwrap();
                let blocks = extract_docs(&txt);
                lua_mods( &blocks )
            } ).flatten().collect();
            mods.sort();
            let dups: Vec<_> = mods.iter().zip( mods.iter().skip(1) ).filter_map( |(a,b)| {
                if a==b {
                    Some(a.clone())
                } else {
                    None
                }
            } ).collect();
            if dups.len() > 0 {
                anyhow::bail!(format!("Found the followingLua module duplicates: {}",dups.join(", ")));
            }

            let mut output = String::new();
            output.push_str("-- THIS FILE IS AUTOMATICALLY GENERATED BY 'utils/docmaker'\n");
            output.push_str(
                "-- ALL LOCAL CHANGES WILL BE LOST ON REGENERATION, SO DO NOT MODIFY PLEASE\n",
            );
            output.push_str("local stds = {}\n");
            let mut mods = Vec::new();
            for file in &lc.input {
                let txt = fs::read_to_string(file).unwrap();
                let blocks = extract_docs(&txt);
                let (modname, txt) = luacheck(&blocks).context(format!("{}", &file.display()))?;
                output.push_str(&txt);
                mods.push(modname);
            }
            for modname in mods {
                output.push_str(&format!("stds.naev.read_globals.naev.fields.{modname} = stds.{modname}.read_globals.{modname}\n"));
            }
            output.push_str("return stds\n");
            output
        }
    };

    if let Some(out) = args.output() {
        fs::write(&out, output)?;
    } else {
        let mut stdout = std::io::stdout();
        stdout.write_all(output.as_bytes())?;
    }
    Ok(())
}
