//! The mod documentation
use std::fmt;

struct FooBar {
}   

impl FooBar {
    pub fn new() -> FooBar {
        FooBar { } 
    }
    
    fn do_smth(&self) {
        println!("something")
    }
}

// This function only gets compiled if the target OS is linux
#[cfg(target_os = "linux")]
fn are_you_on_linux() {
    println!("You are running linux!")
}

mod bla {
    // this a create attribute notice the #! 
    #![cfg(target_os = "linux")]

    fn are_you_on_linux() {
        println!("You are not on MacOS")
    }
}

/// Function documentation
fn main() {
    // regular documentation 

    println!("1 + 2 = {}", 1u32 + 2);
    println!("1 - 2 = {}", 1i32 - 2);
    println!("1 + 2 = {}", 1u64 + 2);

    // Short-circuiting boolean logic
    println!("true AND false is {}", true && false);
    println!("true OR false is {}", true || false);
    println!("NOT true is {}", !true);

    // Bitwise operations
    println!("0011 AND 0101 is {:04b}", 0b0011u32 & 0b0101);
    println!("0011 OR 0101 is {:04b}", 0b0011u32 | 0b0101);
    println!("0011 XOR 0101 is {:04b}", 0b0011u32 ^ 0b0101);
    println!("1 << 5 is {}", 1u32 << 5);
    println!("0x80 >> 2 is 0x{:x}", 0x80u32 >> 2);
    let a = 0o4u32;
    let b = 0o6u32;
    println!("octal: {} + {} = {:o}", a, b, a + b);

    let a = 0.4e-3f64;
    let b = 1.4E+4f64;
    println!("float: {} + {} = {}", a, b, a + b);

    // Use underscores to improve readability!
    println!("One million is written as {}", 1_000_000u32);

    let foo = format!("just a {}", "string");
    println!("{}", foo);

    let ys: [i32; 500] = [0; 500];

    let mut count = 0u32;
    // Infinite loop
    loop {
        count += 1;

        if count == 3 {
            println!("three");

            // Skip the rest of this iteration
            continue;
        }

        if count == 5 {
            println!("OK, that's enough");

            // Exit this loop
            break;
        }
    }

}
