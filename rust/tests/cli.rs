use assert_cmd::Command;
use predicates::prelude::*;
use std::error::Error;
use std::fs;

type TestResult = Result<(), Box<dyn Error>>;

const PRG: &str = "ifcomp";
const EMPTY: &str = "tests/inputs/empty.txt";
const A: &str = "tests/inputs/a.txt";
const B: &str = "tests/inputs/b.txt";
const AB: &str = "tests/inputs/ab.txt";
const ABCDEG: &str = "tests/inputs/abcdeg.txt";
const DEFGAC: &str = "tests/inputs/defgac.txt";
const A_MASS: &str = "tests/inputs/a_mass.txt";
const MUCH_WRITING: &str = "tests/inputs/much_writing.txt";

#[test]
fn usage() -> TestResult {
    for flag in &["-h", "--help"] {
        Command::cargo_bin(PRG)?
            .arg(flag)
            .assert()
            .stdout(predicate::str::contains("USAGE"));
    }
    Ok(())
}

fn run(args: &[&str], expected_file: &str) -> TestResult {
    let expected = fs::read_to_string(expected_file)?;
    Command::cargo_bin(PRG)?
        .args(args)
        .assert()
        .success()
        .stdout(expected);
    Ok(())
}

#[test]
fn empty_empty() -> TestResult {
    run(&[EMPTY, EMPTY], "tests/expected/empty_empty.out")
}

#[test]
fn a_b() -> TestResult {
    run(&[A, B], "tests/expected/a_b.out")
}

#[test]
fn a_ab() -> TestResult {
    run(&[A, AB], "tests/expected/a_ab.out")
}

// This test fails with message:
// index out of bounds: the len is 1 but the index is 1, at hdiff-0.1.1/src/lib.rs:166:33
#[test]
fn b_ab() -> TestResult {
    run(&[B, AB], "tests/expected/b_ab.out")
}

#[test]
fn abcdeg_defgac() -> TestResult {
    run(&[ABCDEG, DEFGAC], "tests/expected/abcdeg_defgac.out")
}

#[test]
fn defgac_abcdeg() -> TestResult {
    run(&[DEFGAC, ABCDEG], "tests/expected/defgac_abcdeg.out")
}

#[test]
fn much_writing_a_mass() -> TestResult {
    run(&[MUCH_WRITING, A_MASS], "tests/expected/much_writing_a_mass.out")
}

// This test fails with message:
// index out of bounds: the len is 19 but the index is 19, at hdiff-0.1.1/src/lib.rs:166:33
#[test]
fn a_mass_much_writing() -> TestResult {
    run(&[A_MASS, MUCH_WRITING], "tests/expected/a_mass_much_writing.out")
}
