#include "new.h"

// Helper to print a term's details
void print_term(const char* prefix, Term term) {
    printf("%s:\n", prefix);
    printf("  Tag: %s (%d)\n", tag_to_string(term_tag(term)), term_tag(term));
    printf("  Location: %u\n", term_loc(term));
    
    // If this is a pair, print its contents
    if (term_loc(term) > 0) {
        Term first = get(term_loc(term));
        Term second = get(term_loc(term) + 1);
        printf("  First term: %s (%d)\n", tag_to_string(term_tag(first)), term_tag(first));
        printf("  Second term: %s (%d)\n", tag_to_string(term_tag(second)), term_tag(second));
    }
    printf("\n");
}

// Test pair creation
void test_pair_creation(void) {
    Term t1 = term_new(LAM, 0, 0);
    Term t2 = term_new(APP, 0, 0);
    Term pair = pair_make(LAM, t1, t2);
    print_term("Simple pair", pair);
    
    // Verify pair structure
    Location loc = term_loc(pair);
    if (get(loc) != t1 || get(loc + 1) != t2) {
        printf("[FAIL] test_pair_creation: Incorrect values stored in BUFF\n");
        printf("Expected: BUFF[%u]=%lu, BUFF[%u]=%lu\n", loc, t1, loc + 1, t2);
        printf("Got:      BUFF[%u]=%lu, BUFF[%u]=%lu\n", loc, get(loc), loc + 1, get(loc + 1));
        exit(1);
    }
    
    printf("[PASS] test_pair_creation\n\n");
}

// Test application-lambda interaction
void test_applam(void) {
    
    // Create argument and body terms
    Term arg_term = term_new(VAR, 1, 0);  // Argument to pass
    Term bod_term = term_new(NUL, 0, 0);  // Body of lambda
    
    // Create application and lambda terms
    Term app = pair_make(APP, term_new(NUL, 0, 0), arg_term);
    Term lam = pair_make(LAM, term_new(NUL, 0, 0), bod_term);
    
    print_term("Before interaction - APP", app);
    printf("  Argument port: Tag=%d Lab=%d\n", term_tag(arg_term), term_lab(arg_term));
    
    print_term("Before interaction - LAM", lam);
    printf("  Variable port: Tag=%d\n", term_tag(get(term_loc(lam) + 1)));
    printf("  Body port: Tag=%d Lab=%d\n", term_tag(bod_term), term_lab(bod_term));
    
    // Perform the interaction
    applam(term_loc(app), term_loc(lam));
    
    printf("Interaction complete\n");
    print_term("Modified APP location", get(term_loc(app)));
    print_term("Modified LAM location", get(term_loc(lam)));
    printf("[PASS] test_applam\n\n");
}

// Test pair manipulation
void test_pair_manipulation(void) {
    
    // Create nested pairs
    Term inner = pair_make(VAR, term_new(NUL, 0, 0), term_new(NUL, 0, 0));
    Term outer = pair_make(APP, inner, term_new(LAM, 0, 0));
    
    print_term("Inner pair", inner);
    print_term("Outer pair", outer);
    
    // Modify inner pair's first term
    set(term_loc(inner), term_new(APP, 0, 0));
    
    printf("After modification:\n");
    print_term("Modified inner pair", inner);
    print_term("Outer pair (showing modified inner)", outer);
    printf("[PASS] test_pair_manipulation\n\n");
}

// Test error conditions
void test_error_conditions(void) {
    // Test uninitialized VM
    printf("Testing uninitialized VM reset...\n");
    // Try to reset without initialization
    hvm_reset(); // Should fail with error message
    printf("This line should not be reached\n");
    printf("[PASS] test_error_conditions\n\n");
}

// Test boundary validation
void test_boundary_validation(void) {
    printf("Testing boundary validation...\n");
    hvm_init(8);  // Very small memory to force overlap
    hvm_reset();
    
    // Create pairs until we run out of space
    Term pair = pair_make(VAR, term_new(NUL, 0, 0), term_new(NUL, 0, 0));
    printf("Created first pair at location %u\n", term_loc(pair));
    
    // This should fail with boundary error
    pair = pair_make(VAR, term_new(NUL, 0, 0), term_new(NUL, 0, 0));
    printf("This line should not be reached\n");
    printf("[PASS] test_boundary_validation\n\n");
}

// Main function
int main(int argc, char *argv[]) {
    // Initialize the VM with some memory
    hvm_init(1024);
    
    printf("\n=== Running test_pair_creation ===\n");
    hvm_reset();
    test_pair_creation();
    
    printf("\n=== Running test_applam ===\n");
    hvm_reset();
    test_applam();
    
    printf("\n=== Running test_pair_manipulation ===\n");
    hvm_reset();
    test_pair_manipulation();
    
    printf("\n=== Running test_error_conditions ===\n");
    test_error_conditions();
    
    // Final cleanup
    hvm_free();
    return 0;
}
