use assert_cmd::cargo::cargo_bin_cmd;
use predicates::prelude::*;
use std::error::Error;

type TestResult = Result<(), Box<dyn Error>>;

#[test]
fn usage() -> TestResult {
    for flag in &["-h", "--help"] {
        cargo_bin_cmd!("ifcomp")
            .arg(flag)
            .assert()
            .stdout(predicate::str::contains("Usage"));
    }
    Ok(())
}
