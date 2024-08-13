// cspell: words rustc cbindgen ostream autogen amqp

use std::env;

use cbindgen::{Config, Language, StructConfig};

fn main() {
    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap_or(".".to_string());

    //    let config = cbindgen::Config::from_root_or_default(crate_dir);

    let struct_config = StructConfig {
        derive_ostream: true,
        associated_constants_in_body: true,
        ..StructConfig::default()
    };

    let mut config = Config::default();
    config.structure = struct_config;
    config.language = Language::Cxx;
    config.namespaces = Some(
        vec![
            "Azure",
            "Core",
            "Amqp",
            "_detail",
            "RustInterop",
        ]
        .iter()
        .map(|x| x.to_string())
        .collect(),
    );
    config.cpp_compat = true;
    config.autogen_warning = Some(
        "/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */"
            .to_string(),
    );
    config.header = Some(
 "/* Copyright (c) Microsoft Corp. All Rights Reserved.
  * Licensed under the MIT License.
  **/
 // cspell: words cbindgen amqp amqpvalue

 #pragma once
 ".to_string());

    cbindgen::Builder::new()
        .with_crate(crate_dir)
        .with_namespace("ffi")
        .with_std_types(true)
        .with_config(config)
        .generate()
        .expect("Unable to generate C++ bindings.")
        .write_to_file("rust_amqp_wrapper.h");
}