fn main() {
    if let Err(e) = ifcomp::get_args().and_then(ifcomp::run) {
        eprintln!("{}", e);
        std::process::exit(1);
    }
}
