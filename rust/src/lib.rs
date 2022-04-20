use std::io::BufRead;

type MyResult<T> = Result<T, Box<dyn std::error::Error>>;

#[derive(Debug)]
pub struct Config {
    first_file: String,
    second_file: String,
    debug_flag: bool,
}

//
// Parse command line arguments and return the Config structure.
//
pub fn get_args() -> MyResult<Config> {
    let matches = clap::App::new("ifcomp")
        .version("0.1.0")
        .author("Serge Vakulenko <serge.vakulenko@gmail.com>")
        .about("Rust version of IFCOMP")
        .arg(
            clap::Arg::with_name("first_file")
                .value_name("FILE")
                .help("First input file")
                .required(true)
        )
        .arg(
            clap::Arg::with_name("second_file")
                .value_name("FILE")
                .help("Second input file")
                .required(true)
        )
        .arg(
            clap::Arg::with_name("debug")
                .long("debug")
                .help("Print debug info")
                .takes_value(false)
        )
        .get_matches();

    Ok(Config {
        first_file: matches.value_of("first_file").unwrap().to_string(),
        second_file: matches.value_of("second_file").unwrap().to_string(),
        debug_flag: matches.is_present("debug"),
    })
}

//
// Run the application.
//
pub fn run(config: Config) -> MyResult<()> {
    if config.debug_flag {
        println!("ifcomp {} {}", config.first_file, config.second_file);
    }

    // Read input files.
    let a = lines_from_file(config.first_file)?;
    let b = lines_from_file(config.second_file)?;

    if config.debug_flag {
        println!("a = {:?}", a);
        println!("b = {:?}", b);
    }

    // Compare two files and print the difference.
    let diff = hdiff::diff(&a, &b);
    println!("diff = {:?}", diff);

    Ok(())
}

//
// Read text file as a vector of lines.
//
fn lines_from_file(filename: impl AsRef<std::path::Path>) -> MyResult<Vec<String>> {
    let file = std::fs::File::open(filename)?;
    let buf = std::io::BufReader::new(file);
    let lines: Vec<String> = buf.lines()
                                .map(|item| item.unwrap())
                                .collect();
    //println!("lines = {:?}", lines);
    Ok(lines)
}
