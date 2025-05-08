#include "new.h"
#include <string.h>

// Global heap
static a64* BUFF = NULL;
static u64 RNOD_INI = 0;
static u64 RNOD_END = 0;
static u64 RBAG = 0x1000;
static u64 RBAG_INI = 0;
static u64 RBAG_END = 0;

// Initialize the virtual machine with a given heap size
void hvm_init(u64 size) {
    BUFF = (a64*)calloc(size, sizeof(a64));
    if (!BUFF) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }
    RNOD_INI = 1;
    RNOD_END = 1;
    RBAG_INI = RBAG;
    RBAG_END = RBAG;
}

// Free allocated memory
void hvm_free(void) {
    if (BUFF == NULL) {
        return;
    }
    free(BUFF);
    BUFF = NULL;
}

// Reset node and bag indices
void hvm_reset(void) {
    if (BUFF == NULL) {
        fprintf(stderr, "Error: Cannot reset uninitialized VM. Call hvm_init first.\n");
        exit(1);
    }
    
    // Clear memory to prevent stale data
    memset(BUFF, 0, RBAG);
    
    // Reset node indices
    RNOD_INI = 0;
    RNOD_END = 0;
    
    // Reset bag indices
    RBAG_INI = RBAG;
    RBAG_END = RBAG;
    
    // Verify indices are valid
    if (RNOD_END >= RBAG_INI) {
        fprintf(stderr, "Error: Node space overlaps with reduction bag space.\n");
        fprintf(stderr, "RNOD_END: %lu, RBAG_INI: %lu\n", RNOD_END, RBAG_INI);
        exit(1);
    }
}

// Convert a tag to its string representation
const char* tag_to_string(Tag tag) {
    switch (tag) {
        case NUL: return "NUL";
        case VAR: return "VAR";
        case APP: return "APP";
        case LAM: return "LAM";
        case ERA: return "ERA";
        default: return "UNKNOWN";
    }
}

// Create a new term with given tag, label, and location
Term term_new(Tag tag, Lab lab, Location loc) {
    return ((u64)loc << (TAG_SIZE + LAB_SIZE)) |
           ((u64)lab << TAG_SIZE) |
           (u64)tag;
}

// Get the tag of a term
Tag term_tag(Term term) {
    return (Tag)(term & TAG_MASK);
}

// Get the label of a term
Lab term_lab(Term term) {
    return (Lab)((term >> TAG_SIZE) & LAB_MASK);
}

// Get the location of a term
Location term_loc(Term term) {
    return (Location)(term >> (TAG_SIZE + LAB_SIZE));
}

Location port(u64 n, Location x) {
  return n + x - 1;
}

// Atomic swap operation
Term swap(Location loc, Term term) {
    return atomic_exchange_explicit(&BUFF[loc], term, memory_order_relaxed);
}

// Get term at location
Term get(Location loc) {
    return atomic_load_explicit(&BUFF[loc], memory_order_relaxed);
}

// Set term at location
void set(Location loc, Term term) {
    atomic_store_explicit(&BUFF[loc], term, memory_order_relaxed);
}

// Create a new pair with given tag and terms
Term pair_make(Tag tag, Term fst, Term snd) {
    // Check if we have enough space for the pair
    if (RNOD_END + 2 >= RBAG_INI) {
        fprintf(stderr, "Error: Not enough space to create pair. RNOD_END=%lu, RBAG_INI=%lu\n",
                RNOD_END, RBAG_INI);
        exit(1);
    }
    
    Location loc = RNOD_END;
    RNOD_END += 2;
    
    // Store terms at adjacent locations
    set(loc, fst);
    set(port(2, loc), snd);
    
    return term_new(tag, 0, loc);
}

// Move a positive term into a negative location
void move(Location neg_loc, Term pos) {
    Term neg = swap(neg_loc, pos);
    if (term_tag(neg) != SUB) {
        link(neg, pos);
    }
}

// Link a negative node with a positive term
void link(Term neg, Term pos) {
    if (term_tag(pos) == VAR) {
        Term neg_var = swap(term_loc(pos), neg);
        if (term_tag(neg_var) != SUB) {
            move(term_loc(pos), neg_var);
        }
    } else {
        // Push redex to reduction bag
        Location redex_loc = RBAG_END;
        RBAG_END += 2;
        set(redex_loc, neg);
        set(port(2, redex_loc), pos);
    }
}

// Application-Lambda interaction
void applam(Location neg_loc, Location pos_loc) {
    // Bounds checking
    if (neg_loc >= RNOD_END || pos_loc >= RNOD_END) {
        fprintf(stderr, "Invalid locations: neg_loc=%u pos_loc=%u RNOD_END=%lu\n",
                neg_loc, pos_loc, RNOD_END);
        return;
    }

    // Get locations for each port
    Location arg_loc = port(2, neg_loc);
    Location ret_loc = port(3, neg_loc);
    Location var_loc = port(2, pos_loc);
    Location bod_loc = port(3, pos_loc);

    fprintf(stderr, "Ports: arg=%u ret=%u var=%u bod=%u\n",
            arg_loc, ret_loc, var_loc, bod_loc);

    // Take the positive terms
    fprintf(stderr, "Before swap - arg_loc term: tag=%d\n", term_tag(get(arg_loc)));
    Term arg_val = swap(arg_loc, term_new(NUL, 0, 0));
    fprintf(stderr, "arg_val: tag=%d lab=%d loc=%u\n",
            term_tag(arg_val), term_lab(arg_val), term_loc(arg_val));

    fprintf(stderr, "Before swap - bod_loc term: tag=%d\n", term_tag(get(bod_loc)));
    Term bod_val = swap(bod_loc, term_new(NUL, 0, 0));
    fprintf(stderr, "bod_val: tag=%d lab=%d loc=%u\n",
            term_tag(bod_val), term_lab(bod_val), term_loc(bod_val));

    // Create a new pair for variable binding
    Term var_pair = pair_make(VAR, arg_val, term_new(NUL, 0, 0));
    fprintf(stderr, "Created var_pair: tag=%d loc=%u\n",
            term_tag(var_pair), term_loc(var_pair));
    
    // Create a new pair for return value
    Term ret_pair = pair_make(APP, bod_val, term_new(NUL, 0, 0));
    fprintf(stderr, "Created ret_pair: tag=%d loc=%u\n",
            term_tag(ret_pair), term_loc(ret_pair));

    // Move terms into their new locations
    fprintf(stderr, "Moving pairs to new locations...\n");
    fprintf(stderr, "ret_loc=%u var_loc=%u\n", ret_loc, var_loc);
    fprintf(stderr, "ret_pair: tag=%s (%d) loc=%u\n", tag_to_string(term_tag(ret_pair)), term_tag(ret_pair), term_loc(ret_pair));
    fprintf(stderr, "var_pair: tag=%s (%d) loc=%u\n", tag_to_string(term_tag(var_pair)), term_tag(var_pair), term_loc(var_pair));
    
    set(var_loc, var_pair);
    set(ret_loc, ret_pair);
    
    fprintf(stderr, "Clearing original locations...\n");
    fprintf(stderr, "neg_loc=%u pos_loc=%u\n", neg_loc, pos_loc);
    
    // Clear original locations
    set(neg_loc, term_new(NUL, 0, 0));
    set(pos_loc, term_new(NUL, 0, 0));
    
    fprintf(stderr, "Interaction complete\n");
}

// Duplication-Lambda interaction
void duplam(Location neg_loc, Location pos_loc) {
    Location dp1_loc = neg_loc + 1;
    Location dp2_loc = neg_loc + 2;
    Location var_loc = pos_loc + 1;
    Location bod_loc = pos_loc + 2;

    // Take the positive term
    Term bod_val = swap(bod_loc, term_new(NUL, 0, 0));

    // Create the pairs
    Term co1 = pair_make(LAM, term_new(SUB, 0, 0), term_new(VAR, 0, 0));
    Term co2 = pair_make(LAM, term_new(SUB, 0, 0), term_new(VAR, 0, 0));
    Term du1 = pair_make(SUP, term_new(VAR, 0, term_loc(co1)), term_new(VAR, 0, term_loc(co2)));
    Term du2 = pair_make(DUP, term_new(SUB, 0, 0), term_new(SUB, 0, 0));

    // Update variable references
    Location co1_loc = term_loc(co1);
    Location co2_loc = term_loc(co2);
    Location du2_loc = term_loc(du2);
    
    // Update the second terms in co1 and co2 to point to du2
    set(co1_loc + 1, term_new(VAR, 0, du2_loc));
    set(co2_loc + 1, term_new(VAR, 0, du2_loc + 1));

    // Move positive terms into negative locations
    move(dp1_loc, co1);
    move(dp2_loc, co2);
    move(var_loc, du1);
    link(du2, bod_val);
}
