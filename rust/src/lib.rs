mod types;
mod pass1;
mod pass2;
mod pass3;
mod pass4;
mod pass5;
mod pass6;
mod pass7;
mod pass8;

pub use types::*;

pub struct Ifcomp {
    pub hash_state: HashTableState,
    pub file_state: FileState,
    pub line_matching_state: LineMatchingState,
    pub tree_state: TreeState,
    pub stats: Statistics,

    // Debug flags
    pub debug_dont_free: bool,
    pub debug_syt_full: bool,
    pub debug_syt: bool,
    pub debug_dump_trees: bool,
    pub debug_dump_trees_full: bool,
    pub debug_alloc: bool,
    pub debug_read_current_line: bool,
}

impl Ifcomp {
    pub fn new() -> Self {
        let mut ifc = Self {
            hash_state: HashTableState::default(),
            file_state: FileState::default(),
            line_matching_state: LineMatchingState::default(),
            tree_state: TreeState::default(),
            stats: Statistics::default(),
            debug_dont_free: false,
            debug_syt_full: false,
            debug_syt: false,
            debug_dump_trees: false,
            debug_dump_trees_full: false,
            debug_alloc: false,
            debug_read_current_line: false,
        };
        ifc.initialize_tables();
        ifc
    }

    pub fn initialize_tables(&mut self) {
        // Initialize hash table with empty buckets
        self.hash_state.sec_hash_start_node = [NULL_HASH_LIST; N_BUCKETS];

        // Initialize file_line arrays with index 0 entry
        self.file_state.file_line[0].push(FileLineDecl {
            ptr0: 0,
            file_line_text: NULL_STRING_LIST,
            linen: 0,
            ptr_type: LineType::SytType,
        });
        self.file_state.file_line[1].push(FileLineDecl {
            ptr0: 0,
            file_line_text: NULL_STRING_LIST,
            linen: 0,
            ptr_type: LineType::SytType,
        });

        // Add dummy entries at index 0 to match legacy 1-based indexing
        self.line_matching_state.line_table.push(LineTableDecl {
            linen: 0,
            next: NULL_LINE_LIST,
        });
        self.line_matching_state.string_table.push(StringDecl {
            text: String::new(),
            next_text_with_same_hash: NULL_STRING_LIST,
            file_n_lines: [0, 0],
            file_list: [NULL_LINE_LIST, NULL_LINE_LIST],
        });
        self.hash_state.hash_node.push(HashNodeDecl {
            h: 0,
            text_list: NULL_STRING_LIST,
            next_in_bucket: NULL_HASH_LIST,
        });
    }

    pub fn clear(&mut self) {
        // Clear all state
        self.hash_state.clear();
        self.file_state.clear();
        self.line_matching_state.clear();
        self.tree_state.clear();
        self.stats.clear();

        // Reinitialize dummy entries
        self.line_matching_state.line_table.push(LineTableDecl {
            linen: 0,
            next: NULL_LINE_LIST,
        });
        self.line_matching_state.string_table.push(StringDecl {
            text: String::new(),
            next_text_with_same_hash: NULL_STRING_LIST,
            file_n_lines: [0, 0],
            file_list: [NULL_LINE_LIST, NULL_LINE_LIST],
        });
        self.hash_state.hash_node.push(HashNodeDecl {
            h: 0,
            text_list: NULL_STRING_LIST,
            next_in_bucket: NULL_HASH_LIST,
        });
    }
}

pub type MyResult<T> = Result<T, Box<dyn std::error::Error>>;

#[derive(Debug)]
pub struct Config {
    pub first_file: String,
    pub second_file: String,
    pub debug_flag: bool,
    pub statistics: bool,
    pub debug_st: bool,
    pub debug_stfull: bool,
    pub debug_trees: bool,
    pub debug_treesfull: bool,
    pub debug_alloc: bool,
    pub debug_nofree: bool,
}

//
// Parse command line arguments and return the Config structure.
//
pub fn get_args() -> MyResult<Config> {
    let matches = clap::Command::new("ifcomp")
        .version("0.1.0")
        .author("Serge Vakulenko <serge.vakulenko@gmail.com>")
        .about("Rust version of IFCOMP")
        .arg(
            clap::Arg::new("stat")
                .long("stat")
                .short('s')
                .help("Print detailed memory usage statistics")
                .action(clap::ArgAction::SetTrue),
        )
        .arg(
            clap::Arg::new("debug")
                .long("debug")
                .short('d')
                .help("Enable all debug output modes")
                .action(clap::ArgAction::SetTrue),
        )
        .arg(
            clap::Arg::new("st")
                .long("st")
                .help("Enable symbol table debugging")
                .action(clap::ArgAction::SetTrue),
        )
        .arg(
            clap::Arg::new("stfull")
                .long("stfull")
                .help("Enable full symbol table debugging")
                .action(clap::ArgAction::SetTrue),
        )
        .arg(
            clap::Arg::new("trees")
                .long("trees")
                .help("Enable tree structure debugging")
                .action(clap::ArgAction::SetTrue),
        )
        .arg(
            clap::Arg::new("treesfull")
                .long("treesfull")
                .help("Enable full tree structure debugging")
                .action(clap::ArgAction::SetTrue),
        )
        .arg(
            clap::Arg::new("alloc")
                .long("alloc")
                .help("Enable memory allocation debugging")
                .action(clap::ArgAction::SetTrue),
        )
        .arg(
            clap::Arg::new("nofree")
                .long("nofree")
                .help("Disable memory freeing (for debugging)")
                .action(clap::ArgAction::SetTrue),
        )
        .arg(
            clap::Arg::new("first_file")
                .value_name("FILE")
                .help("First input file")
                .required(true)
                .index(1),
        )
        .arg(
            clap::Arg::new("second_file")
                .value_name("FILE")
                .help("Second input file")
                .required(true)
                .index(2),
        )
        .get_matches();

    Ok(Config {
        first_file: matches.get_one::<String>("first_file").unwrap().to_string(),
        second_file: matches.get_one::<String>("second_file").unwrap().to_string(),
        debug_flag: matches.get_flag("debug"),
        statistics: matches.get_flag("stat"),
        debug_st: matches.get_flag("st"),
        debug_stfull: matches.get_flag("stfull"),
        debug_trees: matches.get_flag("trees"),
        debug_treesfull: matches.get_flag("treesfull"),
        debug_alloc: matches.get_flag("alloc"),
        debug_nofree: matches.get_flag("nofree"),
    })
}

//
// Run the application.
//
pub fn run(config: Config) -> MyResult<()> {
    // Create Ifcomp instance
    let mut ifc = Ifcomp::new();

    // Clear and reinitialize
    ifc.clear();

    // Set debug flags from Config
    if config.debug_flag {
        // If --debug, set all flags
        ifc.debug_syt_full = true;
        ifc.debug_syt = true;
        ifc.debug_dump_trees = true;
        ifc.debug_dump_trees_full = true;
    } else {
        // Otherwise set individual flags
        if config.debug_stfull {
            ifc.debug_syt_full = true;
        }
        if config.debug_st {
            ifc.debug_syt = true;
        }
        if config.debug_trees {
            ifc.debug_dump_trees = true;
        }
        if config.debug_treesfull {
            ifc.debug_dump_trees_full = true;
        }
    }
    if config.debug_alloc {
        ifc.debug_alloc = true;
    }
    if config.debug_nofree {
        ifc.debug_dont_free = true;
    }

    println!("Comparing: {} {}\n", config.first_file, config.second_file);

    // Open input files
    let file1 = std::fs::File::open(&config.first_file)
        .map_err(|e| format!("can't open file {}: {}", config.first_file, e))?;
    let file2 = std::fs::File::open(&config.second_file)
        .map_err(|e| format!("can't open file {}: {}", config.second_file, e))?;

    // Pass 1-4
    if let Err(e) = ifc.pass1(file1, file2) {
        return Err(format!("Error: {}", e).into());
    }
    if ifc.debug_syt {
        ifc.test_list(1);
    }

    ifc.pass2();
    if ifc.debug_syt {
        ifc.test_list(2);
    }

    ifc.pass3();
    if ifc.debug_syt {
        ifc.test_list(3);
    }

    ifc.pass4();
    if ifc.debug_syt {
        ifc.test_list(4);
    }

    // Pass 5-8
    ifc.pass5();
    if ifc.debug_dump_trees {
        ifc.dump_trees(5);
    }

    ifc.pass6();
    if ifc.debug_dump_trees {
        ifc.dump_trees(6);
    }

    if let Err(e) = ifc.pass7() {
        return Err(format!("Error in pass7: {}", e).into());
    }
    if ifc.debug_dump_trees {
        ifc.dump_trees(7);
    }

    if let Err(e) = ifc.pass8() {
        return Err(format!("Error in pass8: {}", e).into());
    }
    if ifc.debug_dump_trees {
        ifc.dump_trees(8);
    }

    // Always print summary
    ifc.summary();

    // Print statistics if flag is set
    if config.statistics {
        println!("\nStatistics:");
        ifc.print_statistics();
    }

    Ok(())
}