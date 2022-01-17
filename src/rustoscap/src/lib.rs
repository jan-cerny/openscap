use std::ffi::CStr;
use std::os::raw::c_char;

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn test_compute_answer() {
        assert_eq!(compute_answer(0), 40);
        assert_eq!(compute_answer(17), 57);
    }
}

/// Takes a number and computes a magic result
///
/// # Arguments
///
/// * `x` - any number that you wish
///
/// # Examples
///
/// ```
/// let y = compute_answer(17);
/// ```
pub fn compute_answer(x: u32) -> u32 {
    4 * 10 + x
}

/// Process a file
///
/// # Arguments
/// * `file_path_ptr` - pointer to file path (C string)
///
#[no_mangle]
pub extern "C" fn process_file(file_path_ptr: *const c_char) {
    let file_path_cstr = unsafe {
        assert!(!file_path_ptr.is_null());
        CStr::from_ptr(file_path_ptr)
    };
    let file_path = file_path_cstr.to_str().unwrap();
    let answer = compute_answer(2);
    println!(
        "The result of processing file '{}' by magic Rust code is '{}'.",
        file_path, answer
    );
}
