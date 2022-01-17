use std::ffi::CStr;
use std::ffi::CString;
use std::os::raw::c_char;
use std::os::raw::c_int;

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn test_glob_to_regex() {
        let globs_and_regexes = vec![
            ("list.?", "^list\\.[^/]$", "^list\\.[^/]$"),
            ("project.*", "^project\\.[^/]*$", "^project\\.[^/]*$"),
            ("*old", "^(?=[^.])[^/]*old$", "^(?=[^.])[^/]*old$"),
            ("type*.[ch]", "^type[^/]*\\.[ch]$", "^type[^/]*\\.[ch]$"),
            ("*.*", "^(?=[^.])[^/]*\\.[^/]*$", "^(?=[^.])[^/]*\\.[^/]*$"),
            ("*", "^(?=[^.])[^/]*$", "^(?=[^.])[^/]*$"),
            ("?", "^[^./]$", "^[^./]$"),
            ("\\*", "^\\*$", "^\\\\[^/]*$"),
            ("\\?", "^\\?$", "^\\\\[^/]$"),
            ("\\[hello\\]", "^\\[hello\\]$", "^\\\\[hello\\\\]$"),
            (
                "x[[:digit:]]\\*",
                "^x[[:digit:]]\\*$",
                "^x[[:digit:]]\\\\[^/]*$",
            ),
            ("", "^$", "^$"),
            (
                "~/files/*.txt",
                "^~/files/(?=[^.])[^/]*\\.txt$",
                "^~/files/(?=[^.])[^/]*\\.txt$",
            ),
            ("\\", "^\\\\$", "^\\\\$"),
            (".*.conf", "^\\.[^/]*\\.conf$", "^\\.[^/]*\\.conf$"),
            ("docs/?b", "^docs/[^./]b$", "^docs/[^./]b$"),
            ("xy/??z", "^xy/[^./][^/]z$", "^xy/[^./][^/]z$"),
        ];
        for (glob, ex_esc_regex, ex_noesc_regex) in globs_and_regexes {
            let esc_regex = glob_to_regex(glob, false).unwrap();
            assert_eq!(esc_regex, ex_esc_regex);
            let noesc_regex = glob_to_regex(glob, true).unwrap();
            assert_eq!(noesc_regex, ex_noesc_regex);
        }
    }

    #[test]
    fn test_glob_to_regex_errors() {
        assert!(glob_to_regex("[ab", false).is_err());
    }
}

enum States {
    Start,
    Normal,
    LeftBracket,
    Class,
    Escape,
    Slash,
}

/// Converts unix shell glob to Perl 5 regular expression
///
/// # Arguments
/// * `glob` - input glob
/// * `noescape` - Tells if backslash is treated as an escape character
///
/// Return regular expression
pub fn glob_to_regex(glob: &str, noescape: bool) -> Result<String, &str> {
    let mut regex = String::from("^");
    let mut state = States::Start;
    for c in glob.chars() {
        match state {
            States::Start => {
                if c == '/' {
                    regex.push(c);
                    state = States::Slash;
                } else if c == '?' {
                    // a ? matches any single character, but
                    // a ? at the beginning of glob pattern can't match a .
                    // a ? never matches a / (see man 7 glob - Pathnames)
                    regex.push_str("[^./]");
                    state = States::Normal;
                } else if c == '*' {
                    // a * matches any string, but
                    // a * at the beginning of glob pattern can't match a .
                    // a * never matches a / (see man 7 glob - Pathnames)
                    regex.push_str("(?=[^.])[^/]*");
                    state = States::Normal;
                } else if c == '.'
                    || c == '|'
                    || c == '^'
                    || c == '('
                    || c == ')'
                    || c == '{'
                    || c == '}'
                    || c == '+'
                    || c == '$'
                {
                    regex.push('\\');
                    regex.push(c);
                    state = States::Normal;
                } else if c == '[' {
                    regex.push('[');
                    state = States::LeftBracket;
                } else if c == '\\' {
                    if noescape {
                        regex.push('\\');
                        regex.push('\\');
                        state = States::Normal;
                    } else {
                        state = States::Escape;
                    }
                } else {
                    regex.push(c);
                    state = States::Normal;
                }
            }
            States::Normal => {
                if c == '/' {
                    regex.push(c);
                    state = States::Slash;
                } else if c == '?' {
                    // a ? matches any single character, but
                    // it can never match a /
                    regex.push_str("[^/]");
                } else if c == '*' {
                    // a * matches any string, but
                    // it can never match a /
                    regex.push_str("[^/]*");
                } else if c == '.'
                    || c == '|'
                    || c == '^'
                    || c == '('
                    || c == ')'
                    || c == '{'
                    || c == '}'
                    || c == '+'
                    || c == '$'
                {
                    regex.push('\\');
                    regex.push(c);
                } else if c == '[' {
                    regex.push('[');
                    state = States::LeftBracket;
                } else if c == '\\' {
                    if noescape {
                        regex.push('\\');
                        regex.push('\\');
                    } else {
                        state = States::Escape;
                    }
                } else {
                    regex.push(c);
                }
            }
            States::LeftBracket => {
                if c == '!' {
                    regex.push('^');
                } else {
                    regex.push(c);
                }
                state = States::Class;
            }
            States::Class => {
                if c == '\\' {
                    if noescape {
                        regex.push('\\');
                    }
                } else if c == ']' {
                    state = States::Normal;
                }
                regex.push(c);
            }
            States::Escape => {
                // ?, *, [ and ] are special characters, they must be escaped in glob.
                // A backslash is treated as an escape character only for these characters.
                // For all other characters the backslash is just an ordinary character.
                // Other characters, that are special in perl's regex,
                // are not special in a glob.
                if c == '?' || c == '*' || c == '[' || c == ']' {
                    regex.push('\\');
                    regex.push(c);
                } else {
                    regex.push('\\');
                    regex.push('\\');
                    regex.push(c);
                }
                state = States::Normal;
            }
            States::Slash => {
                if c == '?' {
                    // a ? matches any single character, but
                    // a ? at the beginning of glob pattern can't match a .
                    // a ? never matches a / (see man 7 glob - Pathnames)
                    regex.push_str("[^./]");
                    state = States::Normal;
                } else if c == '*' {
                    // a * matches any string, but
                    // a * at the beginning of glob pattern can't match a .
                    // a * never matches a / (see man 7 glob - Pathnames)
                    regex.push_str("(?=[^.])[^/]*");
                    state = States::Normal;
                } else if c == '.'
                    || c == '|'
                    || c == '^'
                    || c == '('
                    || c == ')'
                    || c == '{'
                    || c == '}'
                    || c == '+'
                    || c == '$'
                {
                    regex.push('\\');
                    regex.push(c);
                    state = States::Normal;
                } else if c == '[' {
                    state = States::LeftBracket;
                } else if c == '\\' {
                    if noescape {
                        regex.push('\\');
                        regex.push('\\');
                        state = States::Normal;
                    } else {
                        state = States::Escape;
                    }
                } else {
                    regex.push(c);
                    state = States::Normal;
                }
            }
        }
    }
    match state {
        States::LeftBracket | States::Class => {
            return Err("Expecting ']' at the end of glob.");
        }
        States::Escape => {
            regex.push_str("\\\\");
        }
        _ => {}
    }
    regex.push('$');

    Ok(regex)
}

/// C interface for `glob_to_regex`
///
/// `char *oval_glob_to_regex(const char *glob, int noescape)`
///
/// There is a problem with C strings returned from Rust code. The returned
/// value is a pointer to Rust's heap, not to C heap, and was created by a
/// different allocator. Ownership of the string is transferred to the caller,
/// but the caller must return the string to Rust in order to properly
/// deallocate the memory.
///
/// See http://jakegoulding.com/rust-ffi-omnibus/string_return/
///
/// The C code calling this function can't free the
/// result using `free()`, instead it should call `rust_free_cstr()` from this
/// module.
#[no_mangle]
pub extern "C" fn oval_glob_to_regex(glob_ptr: *const c_char, noescape: c_int) -> *mut c_char {
    let glob_c_str = unsafe {
        assert!(!glob_ptr.is_null());
        CStr::from_ptr(glob_ptr)
    };
    let glob = glob_c_str.to_str().unwrap();

    let regex = match glob_to_regex(glob, noescape != 0) {
        Ok(regex) => regex,
        Err(error) => {
            eprintln!("Error in converting glob '{}': {}", glob, error);
            return std::ptr::null_mut();
        }
    };

    let c_str_regex = CString::new(regex).unwrap();
    c_str_regex.into_raw()
}

/// Free a `char *` pointer
/// Has to be used instead of `free()` in caller C code.
#[no_mangle]
pub extern "C" fn rust_free_cstr(s: *mut c_char) {
    unsafe {
        if s.is_null() {
            return;
        }
        CString::from_raw(s)
    };
}
