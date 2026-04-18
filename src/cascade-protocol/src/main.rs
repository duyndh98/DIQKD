use std::{cell::RefCell, fs, io::Write, rc::Rc};

use prototype::{
    algorithm::OriginalAlgorithm,
    block::{Block, BlockType},
    iteration::Iteration,
    key::Key,
    reconciliation::Reconciliation,
    shuffle::Shuffle,
    shuffled_key::{SharedKey, ShuffledKey},
};

fn create_test_shuffled_key(key_str: &str) -> (Rc<Key>, SharedKey) {
    const SEED: u64 = 0x1234567890ABCDEF;
    // correct key
    let correct_key = Key::from(key_str);
    // noise key from file
    let mut noise_key = correct_key.clone();
    noise_key.set_estimated_ber(0.1); // 10% BER, about 3 errors
    noise_key.apply_noise();

    assert_ne!(correct_key.to_string(), noise_key.to_string());
    (Rc::new(correct_key), Rc::new(RefCell::new(noise_key)))
}

fn print_keys(correct_key: &Rc<Key>, noise_key: &SharedKey) {
    println!("correct key: {}", correct_key.to_string());
    println!("noise key:   {}", noise_key.borrow().to_string());
}

fn test_reconciliation_large() {
    const NUM_ITERATIONS: u32 = 9;
    let key_str =
        "100100011001000110010100011001000101000110010001010001100100011100010001".repeat(200);
    assert_eq!(key_str.len(), 14400);

    let (correct_key, noise_key) = create_test_shuffled_key(&key_str);

    let initial_bit_err = correct_key.nr_bits_different(&*noise_key.borrow());
    let reconciliation =
        Reconciliation::new(NUM_ITERATIONS, correct_key.clone(), noise_key.clone());
    // print_keys(&correct_key, &noise_key);
    reconciliation.start_iterations();
    // print_keys(&correct_key, &noise_key);

    let final_bit_err = correct_key.nr_bits_different(&*noise_key.borrow());

    // println!(
    //     "bit differences: initial: {}, final: {}",
    //     initial_bit_err, final_bit_err
    // );
    assert_eq!(final_bit_err, 0);
}

use std::fs::File;
use std::io::{BufRead, BufReader};
use std::path::Path;

//const NUM_ITERATIONS: u32 = 20;

fn correction(correct_key_str: &str, noise_key_str: &str, n_iterations: u32) -> (u32, u32) {
    //let correct_key_str = "1100010111101011101100110100000110001111011100101000110000000011001011000010011000100011101001100000100011111001101101100110010001101110001111111001000100110100000011";
    let correct_key_ = Key::from(correct_key_str);
    let correct_key = Rc::new(correct_key_);

    //let noise_key_str =   "1000000001000011111100110100000110001111110101001001100010000111011011000011101001100111101001100000110011101011001101100111010001111110010010110001000001110110000101";
    let noise_key_ = Key::from(noise_key_str);
    let noise_key = Rc::new(RefCell::new(noise_key_));

    let initial_bit_err = correct_key.nr_bits_different(&*noise_key.borrow());

    let reconciliation =
        Reconciliation::new(n_iterations, correct_key.clone(), noise_key.clone());
    reconciliation.start_iterations();

    let final_bit_err = correct_key.nr_bits_different(&*noise_key.borrow());

    // println!(
    //     "bit differences: initial: {}, final: {}",
    //     initial_bit_err, final_bit_err
    // );
    //assert_eq!(final_bit_err, 0);
    (initial_bit_err, final_bit_err)
}

use std::io::BufWriter;

fn test(path: &str) {

    let file_path = Path::new(path);
    let file = File::open(file_path).unwrap();
    let reader = BufReader::new(file);

    //let mut final_bit_err_sum: [u32; 21] = [0; 21];

    let mut experiment_count = 0;

    let mut output_path = String::from(path);
    output_path.push_str(".txt");

    let mut output_file = File::options()
        .write(true)
        .create(true)
        .truncate(true)
        .open(output_path).unwrap();
    let mut writer = BufWriter::new(output_file);

    for line_result in reader.lines() {
        
        experiment_count += 1;

        let line = line_result.unwrap();
        let parts : Vec<&str> = line.split(",").collect();

        // println!("Alice {}", parts[0]);
        // println!("Bob   {}", parts[1]);

        for n_iterations in 1..21 {
        
            let (initial_bit_err, final_bit_err) = correction(parts[0], parts[1], n_iterations);
        
            if n_iterations == 1 {
                write!(writer, "{}, {}", parts[0].len(), initial_bit_err);
            }

            write!(writer, ", {}", final_bit_err);
        }

        writeln!(writer);

        // let mut output_path = String::from(path);
        // output_path.push_str(".txt");

        // let mut output_file = File::options()
        //     .append(true)
        //     .create(true)
        //     .open(output_path).unwrap();
        // let mut writer = BufWriter::new(output_file);

        // if n_iterations == 1 {
        //     writeln!(writer, "{}, {}", 0, initial_bit_err);
        // }
        
        // writeln!(writer, "{}, {}", n_iterations, final_bit_err);
        // writer.flush();

        // fs::write(output_path, content).expect("Could not write to file");

        //break;
    }

    // for value in final_bit_err_sum.iter_mut() {
    //     *value /= experiment_count;
    // }

    //println!("{:?}", final_bit_err_sum);
    
    writer.flush();

}

use std::env;

fn main() {

    //let args: Vec<String> = env::args().collect();

    for e in 0..101 {
        //for n_iterations in 0..20 {
        let path = format!("..\\..\\..\\DIQKD\\src\\ConsoleApplication1\\test_result_{}.csv.dat", e);
        test(&path);
        //}
    }
    
    
    //test_reconciliation_large();
}
