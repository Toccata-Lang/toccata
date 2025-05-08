#include "new.h"
#include <sys/wait.h>
#include <unistd.h>

// Print contents of BUFF between start and end locations
void print_raw_term(Term t) {
  if (t == 0) {
    printf("  FREE   ");
  } else {
    Tag tag = term_tag(t);
    Lab lab = term_lab(t);
    Location loc = term_loc(t);
    printf("%s %x %0.3x", tag_to_string(tag), lab, loc);
  }
}

void print_buff(Location start, Location end) {
    a64* buff = get_buff();
    if (!buff) {
        printf("BUFF is not initialized\n");
        return;
    }
    if (start >= end) {
        printf("Invalid range: start=%u end=%u\n", start, end);
        return;
    }
    printf("BUFF contents from %u to %u:\n", start, end);
    for (Location i = start; i < end; i += 2) {
        printf(" %0.3x  ", i);
	print_raw_term(buff[i]);
	printf("  ");
	print_raw_term(buff[i + 1]);
	printf("\n");
    }
    printf("\n");
}

// Helper to print a term's details
void print_term(const char* prefix, Term term) {
  printf("%s:\n", prefix);
  printf("  Tag: %s (%d)\n", tag_to_string(term_tag(term)), term_tag(term));
  printf("  Location: %u\n", term_loc(term));
    
  // If this is a pair, print its contents
  if (term_loc(term) >= 0) {
    Term first = get(port(1, term_loc(term)));
    Term second = get(port(2, term_loc(term)));
    printf("  First term: ");
    print_raw_term(first);
    printf("\n");
    printf("  Second term: ");
    print_raw_term(second);
    printf("\n");
  }
  printf("\n");
}

// Test pair creation
void test_pair_creation(void) {
    Term t1 = term_new(ERA, 0, 0);  // negative term for port 1
    Term t2 = term_new(NUL, 0, 0);  // positive term for port 2
    Term pair = pair_make(LAM, t1, t2);
    print_term("Simple pair", pair);
    
    // Verify pair structure
    Location loc = term_loc(pair);
    if (get(loc) != t1 || get(port(2, loc)) != t2) {
        printf("[FAIL] test_pair_creation: Incorrect values stored in BUFF\n");
        printf("Expected: BUFF[%u]=%lu, BUFF[%u]=%lu\n", loc, t1, port(2, loc), t2);
        printf("Got:      BUFF[%u]=%lu, BUFF[%u]=%lu\n", loc, get(loc), port(2, loc), get(port(2, loc)));
        exit(1);
    }
    
    printf("[PASS] test_pair_creation\n\n");
}

// Test application-lambda interaction
void test_applam(void) {
    // Initialize VM
    hvm_init(1024);
    hvm_reset();

    // Create test terms with correct polarities
    Term arg_term = term_new(NUL, 1, 0);  // Positive argument term
    Term ret_term = term_new(SUB, 0, 0);  // Negative return term
    Term var_term = term_new(ERA, 0, 0);  // Negative variable term
    Term bod_term = term_new(NUL, 0, 0);  // Positive body term
    
    // Create application and lambda terms
    Term app = pair_make(APP, arg_term, ret_term);
    Term lam = pair_make(LAM, var_term, bod_term);
    
    Location app_loc = term_loc(app);
    Location lam_loc = term_loc(lam);
    
    // Get port locations
    Location var_loc = port(1, lam_loc);  // Variable port
    Location bod_loc = port(2, lam_loc);  // Body port
    Location arg_loc = port(1, app_loc);  // Argument port
    Location ret_loc = port(2, app_loc);  // Return port
    
    // Perform interaction
    applam(app_loc, lam_loc);
    
    // Check that argument was moved to variable port with VAR tag
    Term actual_var = get(var_loc);
    if (term_tag(actual_var) != VAR) {
        printf("[FAIL] test_applam: Expected VAR tag in variable port, got: tag=%d\n", term_tag(actual_var));
        exit(1);
    }
    
    // Check that body was moved to return port with APP tag
    Term actual_ret = get(ret_loc);
    if (term_tag(actual_ret) != NUL) {
        printf("[FAIL] test_applam: Expected APP tag in return port, got: tag=%d\n", term_tag(actual_ret));
        exit(1);
    }
    
    // Check that original locations are cleared
    if (get(arg_loc) != 0 || get(bod_loc) != 0) {
        printf("[FAIL] test_applam: Original locations not cleared\n");
        exit(1);
    }
    
    printf("[PASS] test_applam\n");
}

// Test pair manipulation
void test_pair_manipulation(void) {
    
    // Create nested pairs
    Term inner = pair_make(APP, term_new(NUL, 0, 0), term_new(SUB, 0, 0));  // positive port 1, negative port 2
    Term outer = pair_make(APP, term_new(NUL, 0, 0), term_new(SUB, 0, 0));  // positive port 1, negative port 2
    
    print_term("Inner pair", inner);
    print_term("Outer pair", outer);
    
    // Modify inner pair's first term
    set(term_loc(inner), term_new(APP, 0, 0));
    
    printf("After modification:\n");
    print_term("Modified inner pair", inner);
    print_term("Outer pair (showing modified inner)", outer);
    printf("[PASS] test_pair_manipulation\n\n");
}

// Helper to test invalid pair creation
void try_invalid_pair(Tag tag, Term fst, Term snd, const char* desc) {
    bool caught_error = false;
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        pair_make(tag, fst, snd);
        exit(0);  // Should not reach here
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 1) {
            printf("[PASS] Correctly rejected %s\n", desc);
        } else {
            printf("[FAIL] Failed to reject %s\n", desc);
            exit(1);
        }
    }
}

// Test pair polarity validation
void test_pair_polarity() {
    // Test valid LAM pair (port 1 negative, port 2 positive)
    Term era = term_new(ERA, 0, 0);  // negative term
    Term var = term_new(VAR, 0, 0);  // positive term
    Term lam = pair_make(LAM, era, var);
    printf("[PASS] Created LAM pair with correct port polarities\n");
    
    // Test valid APP pair (port 1 positive, port 2 negative)
    Term nul = term_new(NUL, 0, 0);  // positive term
    Term sub = term_new(SUB, 0, 0);  // negative term
    Term app = pair_make(APP, nul, sub);
    printf("[PASS] Created APP pair with correct port polarities\n");
    
    // Test invalid LAM pair (wrong port polarities)
    printf("Testing invalid LAM pair...\n");
    try_invalid_pair(LAM, var, era, "wrong port polarities in LAM pair");
    
    // Test invalid APP pair (wrong port polarities)
    printf("Testing invalid APP pair...\n");
    try_invalid_pair(APP, sub, var, "wrong port polarities in APP pair");
    
    printf("[PASS] test_pair_polarity\n");
}

// Test term polarity
void test_polarity() {
    // Test positive terms
    Term var = term_new(VAR, 0, 0);
    Term nul = term_new(NUL, 0, 0);
    Term lam = term_new(LAM, 0, 0);
    
    if (!is_positive(var) || !is_positive(nul) || !is_positive(lam)) {
        printf("[FAIL] test_polarity: Expected VAR, NUL, LAM to be positive\n");
        exit(1);
    }
    
    if (is_negative(var) || is_negative(nul) || is_negative(lam)) {
        printf("[FAIL] test_polarity: VAR, NUL, LAM should not be negative\n");
        exit(1);
    }
    
    // Test negative terms
    Term sub = term_new(SUB, 0, 0);
    Term era = term_new(ERA, 0, 0);
    Term app = term_new(APP, 0, 0);
    
    if (!is_negative(sub) || !is_negative(era) || !is_negative(app)) {
        printf("[FAIL] test_polarity: Expected SUB, ERA, APP to be negative\n");
        exit(1);
    }
    
    if (is_positive(sub) || is_positive(era) || is_positive(app)) {
        printf("[FAIL] test_polarity: SUB, ERA, APP should not be positive\n");
        exit(1);
    }
    
    printf("[PASS] test_polarity\n");
}

// Test error conditions
void test_error_conditions(void) {
    // Test uninitialized VM
    
    // Ensure VM is not initialized
    hvm_free();
    
    // Fork to test error condition
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        hvm_reset(); // Should fail with error message
        exit(0);  // Should not reach here
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 1) {
            printf("[PASS] Correctly failed on uninitialized VM\n");
        } else {
            printf("[FAIL] Did not fail on uninitialized VM\n");
            exit(1);
        }
    }
}

// Test boundary validation
void test_boundary_validation(void) {
    hvm_init(4);  // Very small memory to force overlap
    hvm_reset();
    
    // Fork to test boundary validation
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        // Create a pair and link terms to fill up reduction bag space
        Term pair1 = pair_make(APP, term_new(NUL, 0, 0), term_new(SUB, 0, 0));
        
        // Link terms to fill up reduction bag space
        // Each term_link uses 2 slots, and we want to fill up the small memory
        for (int i = 0; i < 10; i++) {  // More iterations with smaller memory
            // Use non-VAR terms to ensure they go to reduction bag
            Term pos = term_new(LAM, 0, 0);
            Term neg = term_new(APP, 0, 0);
            term_link(neg, pos);
            printf("Linked terms iteration %d\n", i);
        }
        exit(0);  // Should not reach here due to boundary error
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 1) {
            printf("[PASS] Correctly failed on boundary error\n");
        } else {
            printf("[FAIL] Did not fail on boundary error\n");
            exit(1);
        }
    }
}

// Main function
// Test DUP LAM interaction
void test_duplam(void) {
    // Create test terms with correct polarities
    Term var_term = term_new(SUB, 0, 0);  // Negative variable term
    Term bod_term = term_new(NUL, 0, 0);  // Positive body term
    Term dup1_term = term_new(SUB, 1, 0); // Negative first copy term
    Term dup2_term = term_new(SUB, 2, 0); // Negative second copy term
    
    // Create lambda and duplicator terms
    Term lam = pair_make(LAM, var_term, bod_term);
    Term dup = pair_make(DUP, dup1_term, dup2_term);
    
    Location lam_loc = term_loc(lam);
    Location dup_loc = term_loc(dup);
    
    // Get port locations
    Location var_loc = port(1, lam_loc);   // Variable port
    Location bod_loc = port(2, lam_loc);   // Body port
    Location dup1_loc = port(1, dup_loc);  // First copy port
    Location dup2_loc = port(2, dup_loc);  // Second copy port
    
    // Perform interaction
    duplam(dup_loc, lam_loc);
    
    Term lam1 = get(dup1_loc);
    Term lam2 = get(dup2_loc);
    Term sup = get(var_loc);

    // Check that first copy has correct structure
    if (term_tag(lam1) != LAM) {
        printf("[FAIL] test_duplam: Expected LAM tag in first copy port, got: tag=%d\n", term_tag(lam1));
        exit(1);
    }

    // Check that second copy has correct structure
    if (term_tag(lam2) != LAM) {
        printf("[FAIL] test_duplam: Expected LAM tag in second copy port, got: tag=%d\n", term_tag(lam2));
        exit(1);
    }
    
    // Check that variable port contains a SUP term
    if (term_tag(sup) != SUP) {
        printf("[FAIL] test_duplam: Expected SUP tag in variable port, got: tag=%d\n", term_tag(sup));
        exit(1);
    }
    if (term_loc(get(port(1, term_loc(sup)))) != port(1, term_loc(lam1))) {
        printf("[FAIL] test_duplam: Expected SUP port 1 points to wrong place\n");
        exit(1);
    }
    if (term_loc(get(port(2, term_loc(sup)))) != port(1, term_loc(lam2))) {
        printf("[FAIL] test_duplam: Expected SUP port 2 points to wrong place\n");
        exit(1);
    }

    printf("[PASS] test_duplam\n");
}

// Test ERA NUL interaction
void test_eranul(void) {
    // Create ERA and NUL terms
    Term era = term_new(ERA, 0, 0);  // Negative eraser
    Term nul = term_new(NUL, 0, 0);  // Positive eraser
    
    // Perform interaction
    eranul(era, nul);
    
    // They should just annihilate - nothing else to check
    printf("[PASS] test_eranul\n");
}

// Test ERA LAM interaction
void test_eralam(void) {
    // Create LAM term with ports
    Term var = term_new(SUB, 0, 0);  // Negative variable port
    Term bod = term_new(NUL, 0, 0);  // Positive body port
    Term lam = pair_make(LAM, var, bod);
    
    // Create ERA term
    Term era = term_new(ERA, 0, 0);
    
    // Perform interaction
    eralam(era, lam);
    
    // Check that NUL was sent to variable port
    Location var_loc = port(1, term_loc(lam));
    Term result_var = get(var_loc);
    if (term_tag(result_var) != NUL) {
        printf("[FAIL] test_eralam: Expected NUL in variable port, got tag=%d\n", term_tag(result_var));
        exit(1);
    }
    
    printf("[PASS] test_eralam\n");
}

int main(int argc, char *argv[]) {
    // Initialize the VM with some memory
    hvm_init(1024);
    
    printf("\n=== Running test_error_conditions ===\n");
    test_error_conditions();

    // Re-initialize VM after error conditions test
    hvm_init(1024);

    printf("\n=== Running test_polarity ===\n");
    hvm_reset();
    test_polarity();

    printf("\n=== Running test_pair_polarity ===\n");
    hvm_reset();
    test_pair_polarity();

    printf("\n=== Running test_pair_creation ===\n");
    hvm_reset();
    test_pair_creation();
    
    printf("\n=== Running test_pair_manipulation ===\n");
    hvm_reset();
    test_pair_manipulation();
    
    printf("\n=== Running test_applam ===\n");
    hvm_reset();
    test_applam();
    
    printf("\n=== Running test_duplam ===\n");
    hvm_reset();
    test_duplam();
    
    printf("\n=== Running test_eranul ===\n");
    hvm_reset();
    test_eranul();
    
    printf("\n=== Running test_eralam ===\n");
    hvm_reset();
    test_eralam();
    
    // printf("\n=== Running test_boundary_validation ===\n");
    // test_boundary_validation();
    
    // Final cleanup
    hvm_free();
    return 0;
}
